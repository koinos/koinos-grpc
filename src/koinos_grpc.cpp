#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/program_options.hpp>

#include <yaml-cpp/yaml.h>

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <koinos/broadcast/broadcast.pb.h>
#include <koinos/exception.hpp>
#include <koinos/mq/client.hpp>
#include <koinos/mq/request_handler.hpp>
#include <koinos/rpc/mempool/mempool_rpc.pb.h>
#include <koinos/services/grpc.hpp>
#include <koinos/util/conversion.hpp>
#include <koinos/util/options.hpp>
#include <koinos/util/random.hpp>
#include <koinos/util/services.hpp>

#include "git_version.h"

#define HELP_OPTION          "help"
#define VERSION_OPTION       "version"
#define BASEDIR_OPTION       "basedir"
#define AMQP_OPTION          "amqp"
#define AMQP_DEFAULT         "amqp://guest:guest@localhost:5672/"
#define LOG_LEVEL_OPTION     "log-level"
#define LOG_LEVEL_DEFAULT    "info"
#define LOG_DIR_OPTION       "log-dir"
#define LOG_DIR_DEFAULT      ""
#define LOG_COLOR_OPTION     "log-color"
#define LOG_COLOR_DEFAULT    true
#define LOG_DATETIME_OPTION  "log-datetime"
#define LOG_DATETIME_DEFAULT true
#define INSTANCE_ID_OPTION   "instance-id"
#define JOBS_OPTION          "jobs"
#define JOBS_DEFAULT         uint64_t( 2 )
#define MQ_TIMEOUT_OPTION    "mq-timeout"
#define MQ_TIMEOUT_DEFAULT   5
#define ENDPOINT_OPTION      "endpoint"
#define ENDPOINT_DEFAULT     "0.0.0.0:50051"
#define WHITELIST_OPTION     "whitelist"
#define BLACKLIST_OPTION     "blacklist"

namespace constants {
const std::string qualified_name_prefix = "koinos.rpc.";
} // namespace constants

KOINOS_DECLARE_EXCEPTION( service_exception );
KOINOS_DECLARE_DERIVED_EXCEPTION( invalid_argument, service_exception );

using namespace boost;
using namespace koinos;

const std::string& version_string();
using timer_func_type = std::function< void( const boost::system::error_code& ) >;

using namespace std::chrono_literals;

int main( int argc, char** argv )
{
  std::atomic< bool > stopped = false;
  int retcode                 = EXIT_SUCCESS;
  std::vector< std::thread > threads;
  std::atomic< uint64_t > request_count = 0;

  boost::asio::io_context server_ioc, client_ioc;
  auto request_handler = koinos::mq::request_handler( server_ioc );
  auto client          = koinos::mq::client( client_ioc );
  auto timer           = boost::asio::system_timer( server_ioc );

  timer_func_type timer_func = [ & ]( const boost::system::error_code& ec )
  {
    LOG( info ) << "Recently handled " << request_count << " request(s)";

    request_count = 0;

    timer.expires_after( 1min );
    timer.async_wait( boost::bind( timer_func, boost::asio::placeholders::error ) );
  };

  try
  {
    program_options::options_description options;

    // clang-format off
    options.add_options()
      ( HELP_OPTION                  ",h", "Print this help message and exit" )
      ( VERSION_OPTION               ",v", "Print version string and exit" )
      ( BASEDIR_OPTION               ",d", program_options::value< std::string >()->default_value( util::get_default_base_directory().string() ), "Koinos base directory" )
      ( AMQP_OPTION                  ",a", program_options::value< std::string >(), "AMQP server URL" )
      ( LOG_LEVEL_OPTION             ",l", program_options::value< std::string >(), "The log filtering level" )
      ( INSTANCE_ID_OPTION           ",i", program_options::value< std::string >(), "An ID that uniquely identifies the instance" )
      ( JOBS_OPTION                  ",j", program_options::value< uint64_t >(), "The number of worker jobs" )
      ( MQ_TIMEOUT_OPTION            ",m", program_options::value< uint64_t >(), "The timeout for MQ requests" )
      ( ENDPOINT_OPTION              ",e", program_options::value< std::string >(), "The endpoint the server listens on" )
      ( WHITELIST_OPTION             ",w", program_options::value< std::vector< std::string > >(), "RPC targets to whitelist" )
      ( BLACKLIST_OPTION             ",b", program_options::value< std::vector< std::string > >(), "RPC targets to blacklist" )
      ( LOG_DIR_OPTION                   , program_options::value< std::string >(), "The logging directory" )
      ( LOG_COLOR_OPTION                 , program_options::value< bool >(), "Log color toggle" )
      ( LOG_DATETIME_OPTION              , program_options::value< bool >(), "Log datetime on console toggle" );
    // clang-format on

    program_options::variables_map args;
    program_options::store( program_options::parse_command_line( argc, argv, options ), args );

    if( args.count( HELP_OPTION ) )
    {
      std::cout << options << std::endl;
      return EXIT_SUCCESS;
    }

    if( args.count( VERSION_OPTION ) )
    {
      const auto& v_str = version_string();
      std::cout.write( v_str.c_str(), v_str.size() );
      std::cout << std::endl;
      return EXIT_SUCCESS;
    }

    auto basedir = std::filesystem::path{ args[ BASEDIR_OPTION ].as< std::string >() };
    if( basedir.is_relative() )
      basedir = std::filesystem::current_path() / basedir;

    YAML::Node config;
    YAML::Node global_config;
    YAML::Node grpc_config;

    auto yaml_config = basedir / "config.yml";
    if( !std::filesystem::exists( yaml_config ) )
    {
      yaml_config = basedir / "config.yaml";
    }

    if( std::filesystem::exists( yaml_config ) )
    {
      config        = YAML::LoadFile( yaml_config );
      global_config = config[ "global" ];
      grpc_config   = config[ util::service::grpc ];
    }

    // clang-format off
    auto amqp_url     = util::get_option< std::string >( AMQP_OPTION, AMQP_DEFAULT, args, grpc_config, global_config );
    auto log_level    = util::get_option< std::string >( LOG_LEVEL_OPTION, LOG_LEVEL_DEFAULT, args, grpc_config, global_config );
    auto log_dir      = util::get_option< std::string >( LOG_DIR_OPTION, LOG_DIR_DEFAULT, args, grpc_config, global_config );
    auto log_color    = util::get_option< bool >( LOG_COLOR_OPTION, LOG_COLOR_DEFAULT, args, grpc_config, global_config );
    auto log_datetime = util::get_option< bool >( LOG_DATETIME_OPTION, LOG_DATETIME_DEFAULT, args, grpc_config, global_config );
    auto instance_id  = util::get_option< std::string >( INSTANCE_ID_OPTION, util::random_alphanumeric( 5 ), args, grpc_config, global_config );
    auto jobs         = util::get_option< uint64_t >( JOBS_OPTION, std::max( JOBS_DEFAULT, uint64_t( std::thread::hardware_concurrency() ) ), args, grpc_config, global_config );
    auto mq_timeout   = util::get_option< uint64_t >( MQ_TIMEOUT_OPTION, MQ_TIMEOUT_DEFAULT, args, grpc_config, global_config );
    auto endpoint     = util::get_option< std::string >( ENDPOINT_OPTION, ENDPOINT_DEFAULT, args, grpc_config, global_config );
    auto whitelist    = util::get_options< std::string >( WHITELIST_OPTION, args, grpc_config, global_config );
    auto blacklist    = util::get_options< std::string >( BLACKLIST_OPTION, args, grpc_config, global_config );
    // clang-format on

    std::optional< std::filesystem::path > logdir_path;
    if( !log_dir.empty() )
    {
      logdir_path = std::make_optional< std::filesystem::path >( log_dir );
      if( logdir_path->is_relative() )
        logdir_path = basedir / util::service::grpc / *logdir_path;
    }

    koinos::initialize_logging( util::service::grpc, instance_id, log_level, logdir_path, log_color, log_datetime );

    LOG( info ) << version_string();

    KOINOS_ASSERT( jobs > 1, invalid_argument, "jobs must be greater than 1" );

    if( config.IsNull() )
    {
      LOG( warning ) << "Could not find config (config.yml or config.yaml expected), using default values";
    }

    LOG( info ) << "Starting services...";
    LOG( info ) << "Number of jobs: " << jobs;

    if( whitelist.size() )
    {
      std::string entries;
      for( auto& entry: whitelist )
      {
        entries += entry;
        if( &entry != &whitelist.back() )
          entries += " ";
      }
      LOG( info ) << "Whitelist: [" << entries << "]";
    }

    if( blacklist.size() )
    {
      std::string entries;
      for( auto& entry: blacklist )
      {
        entries += entry;
        if( &entry != &blacklist.back() )
          entries += " ";
      }
      LOG( info ) << "Blacklist: [" << entries << "]";
    }

    boost::asio::signal_set signals( server_ioc );
    signals.add( SIGINT );
    signals.add( SIGTERM );
#if defined( SIGQUIT )
    signals.add( SIGQUIT );
#endif

    services::configuration svc_config{ .client    = client,
                                        .timeout   = std::chrono::seconds{ mq_timeout },
                                        .whitelist = whitelist,
                                        .blacklist = blacklist };

    // Instantiate our service
    services::koinos_service koinos_svc( svc_config );

    ::grpc::ServerBuilder builder;
    builder.AddListeningPort( endpoint, ::grpc::InsecureServerCredentials() );

    // Register our service
    builder.RegisterService( &koinos_svc );

    ::grpc::Server::SetGlobalCallbacks( new koinos::services::callbacks( request_count ) );

    std::unique_ptr< ::grpc::Server > server( builder.BuildAndStart() );

    signals.async_wait(
      [ & ]( const boost::system::error_code& err, int num )
      {
        LOG( info ) << "Caught signal, shutting down...";
        stopped = true;
        server->Shutdown();
        server_ioc.stop();
      } );

    threads.emplace_back(
      [ & ]()
      {
        client_ioc.run();
      } );
    threads.emplace_back(
      [ & ]()
      {
        client_ioc.run();
      } );

    for( std::size_t i = 0; i < jobs; i++ )
      threads.emplace_back(
        [ & ]()
        {
          server_ioc.run();
        } );

    timer.expires_after( 1min );
    timer.async_wait( boost::bind( timer_func, boost::asio::placeholders::error ) );

    LOG( info ) << "Connecting AMQP client...";
    client.connect( amqp_url );
    LOG( info ) << "Established AMQP client connection to the server";

    LOG( info ) << "Connecting AMQP request handler...";
    request_handler.connect( amqp_url );
    LOG( info ) << "Established request handler connection to the AMQP server";

    LOG( info ) << "Listening for requests on " << endpoint;
    server->Wait();
  }
  catch( const invalid_argument& e )
  {
    LOG( error ) << "Invalid argument: " << e.what();
    retcode = EXIT_FAILURE;
  }
  catch( const koinos::exception& e )
  {
    if( !stopped )
    {
      LOG( fatal ) << "An unexpected error has occurred: " << e.what();
      retcode = EXIT_FAILURE;
    }
  }
  catch( const boost::exception& e )
  {
    LOG( fatal ) << "An unexpected error has occurred: " << boost::diagnostic_information( e );
    retcode = EXIT_FAILURE;
  }
  catch( const std::exception& e )
  {
    LOG( fatal ) << "An unexpected error has occurred: " << e.what();
    retcode = EXIT_FAILURE;
  }
  catch( ... )
  {
    LOG( fatal ) << "An unexpected error has occurred";
    retcode = EXIT_FAILURE;
  }

  timer.cancel();

  for( auto& t: threads )
    t.join();

  LOG( info ) << "Shut down gracefully";

  return retcode;
}

const std::string& version_string()
{
  static std::string v_str = "Koinos Services v";
  v_str += std::to_string( KOINOS_MAJOR_VERSION ) + "." + std::to_string( KOINOS_MINOR_VERSION ) + "."
           + std::to_string( KOINOS_PATCH_VERSION );
  v_str += " (" + std::string( KOINOS_GIT_HASH ) + ")";
  return v_str;
}
