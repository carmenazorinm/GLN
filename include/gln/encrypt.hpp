#pragma once
#include <vector>
#include <stdexcept>
#include "gln/bigint.hpp"
#include "gln/keygen.hpp"
#include "gln/params.hpp"

namespace gln {

struct Ciphertext {
    BigInt c1; 
    BigInt c2; 
};

Ciphertext encrypt(const std::vector<uint32_t>& e, const PublicKey& pk);

Ciphertext encrypt_checked(const std::vector<uint32_t>& e,
                           const PublicKey& pk,
                           const Params& prm);

} // namespace gln
