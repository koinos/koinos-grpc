#include <grpcpp/grpcpp.h>
#include <grpcpp/security/server_credentials.h>

#include <koinos/account_history/account_history.grpc.pb.h>
#include <koinos/services/grpc.hpp>
#include <koinos/util/base58.hpp>
#include <koinos/util/conversion.hpp>
#include <koinos/util/services.hpp>

int main( int argc, char** argv )
{
  std::string target_address( "0.0.0.0:50051" );
  auto channel = ::grpc::CreateChannel( target_address, grpc::InsecureChannelCredentials() );

  std::unique_ptr< koinos::services::koinos::Stub > stub( koinos::services::koinos::NewStub( channel ) );

  koinos::rpc::account_history::get_account_history_request request;

  request.set_address(
    koinos::util::from_base58< std::string >( std::string{ "1NsQbH5AhQXgtSNg1ejpFqTi2hmCWz1eQS" } ) );
  request.set_limit( 10 );

  // Container for server response
  koinos::rpc::account_history::get_account_history_response response;
  // Context can be used to send meta data to server or modify RPC behaviour
  grpc::ClientContext context;

  // Actual Remote Procedure Call
  grpc::Status status = stub->get_account_history( &context, request, &response );

  // Returns results based on RPC status
  if( status.ok() )
  {
    std::cout << response;
  }
  else
  {
    std::cout << status.error_code() << ": " << status.error_message() << std::endl;
  }

  return EXIT_SUCCESS;
}
