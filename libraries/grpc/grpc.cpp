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

#include <koinos/grpc/grpc.hpp>

#define _GRPC_SYNC_METHOD_DEFINITION( r, svc, method )                                \
::grpc::Status BOOST_PP_CAT( svc, _service )::method( ::grpc::ServerContext* context, \
const ::koinos::rpc::svc::BOOST_PP_CAT(method, _request*) request,                    \
::koinos::rpc::svc::BOOST_PP_CAT(method, _response*) response )                       \
{                                                                                     \
   try                                                                                \
   {                                                                                  \
      rpc::svc::BOOST_PP_CAT( svc, _request ) req;                                    \
      BOOST_PP_CAT(*req.mutable_, method)() = *request;                               \
      auto future = _client.rpc( util::service::svc,                                  \
         util::converter::as< std::string >( req ), 750ms, mq::retry_policy::none );  \
      rpc::svc::BOOST_PP_CAT( svc, _response ) resp;                                  \
      resp.ParseFromString( future.get() );                                           \
      KOINOS_ASSERT( BOOST_PP_CAT( resp.has_, method )(),                             \
         koinos::exception, "unexpected response type" );                             \
      *response = resp.method();                                                      \
   }                                                                                  \
   catch ( const koinos::exception& e )                                               \
   {                                                                                  \
      LOG(warning) << "An exception has occurred: " << e.get_message();               \
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );                \
   }                                                                                  \
   catch ( const std::exception& e )                                                  \
   {                                                                                  \
      LOG(warning) << "An exception has occurred: " << e.what();                      \
      return ::grpc::Status( ::grpc::StatusCode::INTERNAL, e.what() );                \
   }                                                                                  \
   catch ( ... )                                                                      \
   {                                                                                  \
      LOG(warning) << "An unknown exception has occurred";                            \
      return ::grpc::Status( ::grpc::StatusCode::UNKNOWN,                             \
         "an unknown exception has occurred" );                                       \
   }                                                                                  \
   return ::grpc::Status::OK;                                                         \
}

#define GRPC_SYNC_METHOD_DEFINITIONS( svc, args ) BOOST_PP_SEQ_FOR_EACH( _GRPC_SYNC_METHOD_DEFINITION, svc, args )

using namespace std::chrono_literals;

namespace koinos::services {

// Global callback implementation

callbacks::callbacks( std::atomic< uint64_t >& req_count ) : _request_count( req_count ) {}
callbacks::~callbacks() {}

void callbacks::PreSynchronousRequest( grpc_impl::ServerContext* context )
{
   _request_count++;
}

void callbacks::PostSynchronousRequest( grpc_impl::ServerContext* context ) {}

// Mempool service implementation

mempool_service::mempool_service( mq::client& c ) : _client( c ) {}

GRPC_SYNC_METHOD_DEFINITIONS( mempool,
   (get_pending_transactions)
   (check_pending_account_resources)
);

// Account history implementation

account_history_service::account_history_service( mq::client& c ) : _client( c ) {}

GRPC_SYNC_METHOD_DEFINITIONS( account_history,
   (get_account_history)
);

} // koinos::services
