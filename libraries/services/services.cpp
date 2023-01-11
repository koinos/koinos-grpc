#include <chrono>
#include <functional>
#include <tuple>

#include <boost/multiprecision/cpp_int.hpp>

#include <koinos/chain/value.pb.h>
#include <koinos/protocol/protocol.pb.h>
#include <koinos/mempool/mempool.pb.h>

#include <koinos/util/base58.hpp>
#include <koinos/util/conversion.hpp>
#include <koinos/util/hex.hpp>
#include <koinos/util/services.hpp>

#include <koinos/services/services.hpp>

using namespace std::chrono_literals;

namespace koinos::services {

mempool_service::mempool_service( mq::client& c ) : _client( c ) {}

::grpc::Status mempool_service::get_pending_transactions(
   ::grpc::ServerContext* context,
   const ::koinos::rpc::mempool::get_pending_transactions_request* request,
   ::koinos::rpc::mempool::get_pending_transactions_response* response )
{
   try
   {
      auto future = _client.rpc( util::service::mempool, util::converter::as< std::string >( *request ), 750ms, mq::retry_policy::none );
      rpc::mempool::mempool_response resp;
      response->ParseFromString( future.get() );
   }
   catch ( const std::exception& e )
   {
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );
   }
   catch ( ... )
   {
      return ::grpc::Status( ::grpc::StatusCode::UNKNOWN, "an unknown exception has occurred" );
   }

   return ::grpc::Status::OK;
}

::grpc::Status mempool_service::check_pending_account_resources(
   ::grpc::ServerContext* context,
   const ::koinos::rpc::mempool::check_pending_account_resources_request* request,
   ::koinos::rpc::mempool::check_pending_account_resources_response* response )
{
   try
   {
      auto future = _client.rpc( util::service::mempool, util::converter::as< std::string >( *request ), 750ms, mq::retry_policy::none );
      rpc::mempool::mempool_response resp;
      response->ParseFromString( future.get() );
   }
   catch ( const std::exception& e )
   {
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );
   }
   catch ( ... )
   {
      return ::grpc::Status( ::grpc::StatusCode::UNKNOWN, "an unknown exception has occurred" );
   }

   return ::grpc::Status::OK;
}

account_history_service::account_history_service( mq::client& c ) : _client( c ) {}

::grpc::Status account_history_service::get_account_history(
   ::grpc::ServerContext* context,
   const ::koinos::rpc::account_history::get_account_history_request* request,
   ::koinos::rpc::account_history::get_account_history_response* response )
{
   try
   {
      std::cout << "Got request: " << *request << std::endl;
      auto future = _client.rpc( "account_history", util::converter::as< std::string >( *request ), 750ms, mq::retry_policy::none );
      rpc::account_history::get_account_history_response resp;
      response->ParseFromString( future.get() );
   }
   catch ( const std::exception& e )
   {
      std::cout << "Exception: " << e.what() << std::endl;
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );
   }
   catch ( ... )
   {
      std::cout << "Unknown exception" << std::endl;
      return ::grpc::Status( ::grpc::StatusCode::UNKNOWN, "an unknown exception has occurred" );
   }

   return ::grpc::Status::OK;
}

} // koinos::services
