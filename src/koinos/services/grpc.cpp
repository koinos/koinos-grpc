#include <chrono>
#include <functional>
#include <tuple>

#include <boost/multiprecision/cpp_int.hpp>

#include <koinos/chain/value.pb.h>
#include <koinos/mempool/mempool.pb.h>
#include <koinos/protocol/protocol.pb.h>

#include <koinos/util/base58.hpp>
#include <koinos/util/conversion.hpp>
#include <koinos/util/hex.hpp>
#include <koinos/util/services.hpp>

#include <koinos/services/grpc.hpp>

#define _GRPC_SYNC_METHOD_DEFINITION( r, svc, method )                                                                 \
  ::grpc::Status koinos_service::method( ::grpc::ServerContext* context,                                               \
                                         const ::koinos::rpc::svc::BOOST_PP_CAT( method, _request* ) request,          \
                                         ::koinos::rpc::svc::BOOST_PP_CAT( method, _response* ) response )             \
  {                                                                                                                    \
    const auto& [ permitted, msg ] = call_permitted( #svc, #method );                                                  \
    if( !permitted )                                                                                                   \
      return ::grpc::Status( ::grpc::StatusCode::PERMISSION_DENIED, msg );                                             \
                                                                                                                       \
    try                                                                                                                \
    {                                                                                                                  \
      rpc::svc::BOOST_PP_CAT( svc, _request ) req;                                                                     \
      BOOST_PP_CAT( *req.mutable_, method )() = *request;                                                              \
      auto future                             = _config.client.rpc( util::service::svc,                                \
                                        util::converter::as< std::string >( req ),         \
                                        _config.timeout,                                   \
                                        mq::retry_policy::none );                          \
      rpc::svc::BOOST_PP_CAT( svc, _response ) resp;                                                                   \
      resp.ParseFromString( future.get() );                                                                            \
      if( resp.has_error() )                                                                                           \
        throw ::koinos::exception( uint32_t( ::grpc::StatusCode::INVALID_ARGUMENT ), resp.error().message() );         \
      else if( !BOOST_PP_CAT( resp.has_, method )() )                                                                  \
        throw ::koinos::exception( uint32_t( ::grpc::StatusCode::INTERNAL ), "unexpected response type" );             \
      *response = resp.method();                                                                                       \
    }                                                                                                                  \
    catch( const ::koinos::exception& e )                                                                              \
    {                                                                                                                  \
      LOG( warning ) << "An exception has occurred: " << e.get_message();                                              \
      return ::grpc::Status( ::grpc::StatusCode( e.get_code() ), e.what() );                                           \
    }                                                                                                                  \
    catch( const std::exception& e )                                                                                   \
    {                                                                                                                  \
      LOG( warning ) << "An exception has occurred: " << e.what();                                                     \
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );                                                 \
    }                                                                                                                  \
    catch( ... )                                                                                                       \
    {                                                                                                                  \
      LOG( warning ) << "An unknown exception has occurred";                                                           \
      return ::grpc::Status( ::grpc::StatusCode::UNKNOWN, "an unknown exception has occurred" );                       \
    }                                                                                                                  \
    return ::grpc::Status::OK;                                                                                         \
  }

#define GRPC_SYNC_METHOD_DEFINITIONS( svc, args ) BOOST_PP_SEQ_FOR_EACH( _GRPC_SYNC_METHOD_DEFINITION, svc, args )

using namespace std::chrono_literals;

namespace koinos::services {

std::pair< std::string, std::string > split_list_entry( const std::string& entry )
{
  static const std::string delim{ "." };

  auto delim_pos = entry.find( delim );

  if( delim_pos == std::string::npos )
    return std::make_pair( entry, std::string{} );

  return std::make_pair( entry.substr( 0, delim_pos ), entry.substr( delim_pos + 1, entry.size() ) );
}

// Global callback implementation

callbacks::callbacks( std::atomic< uint64_t >& req_count ):
    _request_count( req_count )
{}

callbacks::~callbacks() {}

void callbacks::PreSynchronousRequest( grpc_impl::ServerContext* context )
{
  _request_count++;
}

void callbacks::PostSynchronousRequest( grpc_impl::ServerContext* context ) {}

// Koinos service implementation

koinos_service::koinos_service( configuration& cfg ):
    _config( cfg )
{}

std::pair< bool, std::string > koinos_service::call_permitted( const std::string& service, const std::string& method )
{
  if( _config.whitelist.size() )
  {
    bool in_whitelist = false;

    for( const auto& entry: _config.whitelist )
    {
      const auto& [ svc, fn ] = split_list_entry( entry );

      // Entire service was whitelist
      if( service == svc )
      {
        if( fn.empty() )
        {
          in_whitelist = true;
          break;
        }
      }
      // Check if this particular method is whitelisted
      else if( method == fn )
      {
        in_whitelist = true;
        break;
      }
    }

    if( !in_whitelist )
      return std::make_pair( false, "method not whitelisted" );
  }

  for( const auto& entry: _config.blacklist )
  {
    const auto& [ svc, fn ] = split_list_entry( entry );

    // Entire service was whitelist
    if( service == svc )
    {
      if( fn.empty() )
      {
        return std::make_pair( false, "method is blacklisted" );
      }
    }
    // Check if this particular method is whitelisted
    else if( method == fn )
    {
      return std::make_pair( false, "method is blacklisted" );
    }
  }

  return std::make_pair( true, std::string{} );
}

GRPC_SYNC_METHOD_DEFINITIONS( account_history, ( get_account_history ) );

GRPC_SYNC_METHOD_DEFINITIONS( block_store, (get_blocks_by_id)(get_blocks_by_height)( get_highest_block ) );

GRPC_SYNC_METHOD_DEFINITIONS(
  chain,
  (submit_block)(submit_transaction)(get_head_info)(get_chain_id)(get_fork_heads)(read_contract)(get_account_nonce)(get_account_rc)( get_resource_limits ) );

GRPC_SYNC_METHOD_DEFINITIONS( contract_meta_store, ( get_contract_meta ) );

GRPC_SYNC_METHOD_DEFINITIONS( mempool, (get_pending_transactions)( check_pending_account_resources ) );

GRPC_SYNC_METHOD_DEFINITIONS( p2p, ( get_gossip_status ) );

GRPC_SYNC_METHOD_DEFINITIONS( transaction_store, ( get_transactions_by_id ) );

} // namespace koinos::services
