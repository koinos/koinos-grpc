#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>

#include <grpcpp/server.h>

#include <koinos/crypto/multihash.hpp>
#include <koinos/exception.hpp>
#include <koinos/mq/client.hpp>

#include <koinos/rpc/services.grpc.pb.h>

#define _GRPC_SYNC_METHOD_DECLARATION( r, svc, method )                      \
virtual ::grpc::Status method( ::grpc::ServerContext* context,           \
const ::koinos::rpc::svc::BOOST_PP_CAT(method, _request*) request,       \
::koinos::rpc::svc::BOOST_PP_CAT(method, _response*) response ) override;

#define GRPC_SYNC_METHOD_DECLARATIONS( svc, args ) BOOST_PP_SEQ_FOR_EACH( _GRPC_SYNC_METHOD_DECLARATION, svc, args )

namespace koinos::services {

bool entry_matches( const std::vector< std::string >& list, std::string service, std::string method );
std::pair< std::string, std::string > split_list_entry( const std::string& entry );

struct configuration {
   mq::client& client;
   std::chrono::seconds timeout;
   std::vector< std::string > whitelist;
   std::vector< std::string > blacklist;
};

class callbacks final : public ::grpc::Server::GlobalCallbacks {
public:
   callbacks( std::atomic< uint64_t >& req_count );
   ~callbacks() override;

   virtual void PreSynchronousRequest( grpc_impl::ServerContext* context ) override;
    /// Called after application callback for each synchronous server request
   virtual void PostSynchronousRequest( grpc_impl::ServerContext* context ) override;

private:
   std::atomic< uint64_t >& _request_count;
};

class mempool_service final : public mempool::Service {
public:

   explicit mempool_service( configuration& cfg );

   std::pair< bool, std::string > call_permitted( const std::string& service, const std::string& method );

   GRPC_SYNC_METHOD_DECLARATIONS( mempool,
      (get_pending_transactions)
      (check_pending_account_resources)
   );

private:

   configuration& _config;
};

class account_history_service final : public account_history::Service {
public:

   explicit account_history_service( configuration& cfg );

   std::pair< bool, std::string > call_permitted( const std::string& service, const std::string& method );

   GRPC_SYNC_METHOD_DECLARATIONS( account_history,
      (get_account_history)
   );

private:

   configuration& _config;
};


} // koinos::services
