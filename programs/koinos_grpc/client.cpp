#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/program_options.hpp>

#include <yaml-cpp/yaml.h>

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/grpcpp.h>

#include <koinos/broadcast/broadcast.pb.h>
#include <koinos/exception.hpp>
#include <koinos/grpc/grpc.hpp>
#include <koinos/mq/client.hpp>
#include <koinos/mq/request_handler.hpp>
#include <koinos/rpc/mempool/mempool_rpc.pb.h>
#include <koinos/util/conversion.hpp>
#include <koinos/util/options.hpp>
#include <koinos/util/random.hpp>
#include <koinos/util/services.hpp>

#include <koinos/util/base58.hpp>

#include "git_version.h"

int main( int argc, char** argv )
{
   std::string target_address("0.0.0.0:50051");
   auto channel = ::grpc::CreateChannel( target_address, grpc::InsecureChannelCredentials() );

   std::unique_ptr< koinos::services::account_history::Stub > stub( koinos::services::account_history::NewStub( channel ) );

   koinos::rpc::account_history::get_account_history_request request;

   request.set_address( koinos::util::from_base58< std::string >( std::string{ "1NsQbH5AhQXgtSNg1ejpFqTi2hmCWz1eQS" } ) );
   request.set_limit( 10 );

   // Container for server response
   koinos::rpc::account_history::get_account_history_response response;
   // Context can be used to send meta data to server or modify RPC behaviour
   grpc::ClientContext context;

   // Actual Remote Procedure Call
   grpc::Status status = stub->get_account_history( &context, request, &response );

   // Returns results based on RPC status
   if ( status.ok() )
   {
      std::cout << response;
   }
   else
   {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
   }

   return EXIT_SUCCESS;
}
