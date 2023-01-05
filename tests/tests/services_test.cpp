#include <boost/test/unit_test.hpp>

#include <koinos/services/services.hpp>

#include <koinos/chain/value.pb.h>
#include <koinos/crypto/elliptic.hpp>
#include <koinos/crypto/multihash.hpp>
#include <koinos/protocol/protocol.pb.h>
#include <koinos/util/conversion.hpp>
#include <koinos/util/hex.hpp>

#include <chrono>
#include <memory>

using namespace koinos;
using namespace std::chrono_literals;

struct services_fixture
{
   services_fixture()
   {
      std::string seed1 = "alpha bravo charlie delta";
      _key1 = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed1 ) );

      std::string seed2 = "echo foxtrot golf hotel";
      _key2 = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed2 ) );

      std::string seed3 = "india juliet kilo lima";
      _key3 = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed3 ) );

      std::string seed4 = "mike november oscar papa";
      _key4 = crypto::private_key::regenerate( crypto::hash( crypto::multicodec::sha2_256, seed4 ) );
   }

   crypto::private_key _key1;
   crypto::private_key _key2;
   crypto::private_key _key3;
   crypto::private_key _key4;
};

BOOST_FIXTURE_TEST_SUITE( services_tests, services_fixture )

BOOST_AUTO_TEST_CASE( services_basic_test )
{
}

}
