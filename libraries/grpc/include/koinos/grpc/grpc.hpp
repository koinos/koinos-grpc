#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include <boost/preprocessor/cat.hpp>

#include <grpcpp/server.h>

#include <koinos/crypto/multihash.hpp>
#include <koinos/exception.hpp>
#include <koinos/mq/client.hpp>

#include <koinos/rpc/services.grpc.pb.h>

#define GRPC_SYNC_METHOD_DECLARATION( svc, method )                      \
virtual ::grpc::Status method( ::grpc::ServerContext* context,           \
const ::koinos::rpc::svc::BOOST_PP_CAT(method, _request*) request,       \
::koinos::rpc::svc::BOOST_PP_CAT(method, _response*) response ) override

namespace koinos::services {

class callbacks final : public ::grpc::Server::GlobalCallbacks {
public:
   callbacks( std::atomic< uint64_t >& req_count );
   ~callbacks() override;

   virtual void PreSynchronousRequest(grpc_impl::ServerContext* context) override;
    /// Called after application callback for each synchronous server request
   virtual void PostSynchronousRequest(grpc_impl::ServerContext* context) override;

private:
   std::atomic< uint64_t >& _request_count;
};

class mempool_service final : public mempool::Service {
public:

   explicit mempool_service( mq::client& c );

   GRPC_SYNC_METHOD_DECLARATION( mempool, get_pending_transactions );
   GRPC_SYNC_METHOD_DECLARATION( mempool, check_pending_account_resources );

private:

   mq::client& _client;
};

class account_history_service final : public account_history::Service {
public:

   explicit account_history_service( mq::client& c );

   GRPC_SYNC_METHOD_DECLARATION( account_history, get_account_history );

private:

   mq::client& _client;
};


} // koinos::services
