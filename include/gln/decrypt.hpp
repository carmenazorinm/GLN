#pragma once
// include/gln/decrypt.hpp
#include "gln/keygen.hpp"
#include "gln/nt_utils.hpp"
#include "gln/params.hpp"
#include "gln/encrypt.hpp"

namespace gln {
    std::vector<uint32_t> decrypt(const Ciphertext& ct, const PrivateKey& sk, const Params& prm); // devuelve e
}