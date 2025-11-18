#pragma once
#include <array>
#include <vector>
#include <cstdint>

#include "gln/bigint.hpp"
#include "gln/params.hpp"
#include "gln/random.hpp"
#include "gln/keygen.hpp"
#include "gln/encrypt.hpp"
#include "gln/decrypt.hpp"

namespace gln {

using SKey = std::array<uint8_t, 32>; 

struct KemCiphertext {
    Ciphertext ct;
};

struct KemEncapOut {
    KemCiphertext ct;
    SKey key;
};

using KdfFunc = SKey (*)(uint8_t domain_tag, const std::vector<uint8_t>& payload);

SKey default_kdf(uint8_t domain_tag, const std::vector<uint8_t>& payload);

KemEncapOut kem_encap(const PublicKey& pk,
                      const Params& prm,
                      Random& rng,
                      KdfFunc kdf = default_kdf);

SKey kem_decap(const KemCiphertext& c,
               const PrivateKey& sk,
               const PublicKey& pk, 
               const Params& prm,
               KdfFunc kdf = default_kdf);

} // namespace gln
