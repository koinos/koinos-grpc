#include <boost/test/unit_test.hpp>

#include <koinos/services/grpc.hpp>

#include <koinos/crypto/elliptic.hpp>
#include <koinos/crypto/multihash.hpp>
#include <koinos/protocol/protocol.pb.h>

using namespace koinos;

struct grpc_fixture
{
  grpc_fixture()
  {
    std::string seed1 = "alpha bravo charlie delta";
    _key1             = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed1 ) );

    std::string seed2 = "echo foxtrot golf hotel";
    _key2             = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed2 ) );

    std::string seed3 = "india juliet kilo lima";
    _key3             = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed3 ) );

    std::string seed4 = "mike november oscar papa";
    _key4             = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed4 ) );
  }

  crypto::private_key _key1;
  crypto::private_key _key2;
  crypto::private_key _key3;
  crypto::private_key _key4;
};

BOOST_FIXTURE_TEST_SUITE( grpc_tests, grpc_fixture )

BOOST_AUTO_TEST_CASE( split_entry_test )
{
  std::vector< std::string > list{ "service.method1", "service.method2", "service", "service.", ".method" };

  auto [ svc1, method1 ] = koinos::services::split_list_entry( list[0] );
  BOOST_REQUIRE( svc1 == "service" );
  BOOST_REQUIRE( method1 == "method1" );

  auto [ svc2, method2 ] = koinos::services::split_list_entry( list[1] );
  BOOST_REQUIRE( svc2 == "service" );
  BOOST_REQUIRE( method2 == "method2" );

  auto [ svc3, method3 ] = koinos::services::split_list_entry( list[2] );
  BOOST_REQUIRE( svc3 == "service" );
  BOOST_REQUIRE( method3.empty() );

  auto [ svc4, method4 ] = koinos::services::split_list_entry( list[3] );
  BOOST_REQUIRE( svc4 == "service" );
  BOOST_REQUIRE( method4.empty() );

  auto [ svc5, method5 ] = koinos::services::split_list_entry( list[4] );
  BOOST_REQUIRE( svc5.empty() );
  BOOST_REQUIRE( method5 == "method" );
}

BOOST_AUTO_TEST_CASE( grpc_basic_test ) {}

BOOST_AUTO_TEST_SUITE_END()
