#pragma once
// include/gln/keygen.hpp
#include <vector>
#include "gln/bigint.hpp"
#include "gln/params.hpp"
#include "gln/random.hpp"

namespace gln {
    struct PublicKey { std::vector<BigInt> t; BigInt g; }; // t_i in [0,g)
    struct PrivateKey { std::vector<BigInt> p; BigInt g, u; };
    struct KeyPair { PublicKey pk; PrivateKey sk; };

    KeyPair keygen(const Params& prm, Random& rng);
}
