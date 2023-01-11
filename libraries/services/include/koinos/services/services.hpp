#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include <grpcpp/server.h>

#include <koinos/crypto/multihash.hpp>
#include <koinos/exception.hpp>
#include <koinos/mq/client.hpp>

#include <koinos/rpc/services.grpc.pb.h>

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

   virtual ::grpc::Status get_pending_transactions(
      ::grpc::ServerContext* context,
      const ::koinos::rpc::mempool::get_pending_transactions_request* request,
      ::koinos::rpc::mempool::get_pending_transactions_response* response ) override;

   virtual ::grpc::Status check_pending_account_resources(
      ::grpc::ServerContext* context,
      const ::koinos::rpc::mempool::check_pending_account_resources_request* request,
      ::koinos::rpc::mempool::check_pending_account_resources_response* response ) override;

private:

   mq::client& _client;
};

class account_history_service final : public account_history::Service {
public:

   explicit account_history_service( mq::client& c );

   virtual ::grpc::Status get_account_history(
      ::grpc::ServerContext* context,
      const ::koinos::rpc::account_history::get_account_history_request* request,
      ::koinos::rpc::account_history::get_account_history_response* response ) override;

private:

   mq::client& _client;
};


} // koinos::services
