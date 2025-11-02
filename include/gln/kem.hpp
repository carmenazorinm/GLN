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

using SKey = std::array<uint8_t, 32>; // clave compartida de 256 bits

struct KemCiphertext {
    Ciphertext ct;
};

struct KemEncapOut {
    KemCiphertext ct;
    SKey key;
};

// KDF por puntero a función: devuelve 32 bytes desde (domain_tag, payload)
using KdfFunc = SKey (*)(uint8_t domain_tag, const std::vector<uint8_t>& payload);

// KDF de ejemplo (NO criptográficamente seguro). Úsalo solo para tests.
SKey default_kdf(uint8_t domain_tag, const std::vector<uint8_t>& payload);

// Genera e uniforme válido, encripta y deriva clave K
KemEncapOut kem_encap(const PublicKey& pk,
                      const Params& prm,
                      Random& rng,
                      KdfFunc kdf = default_kdf);

// Intenta decapsular; si el check Encrypt(e')==ct pasa, usa dominio 1;
// si falla, dominio 0 y un “secreto” derivado de la SK para robustez.
SKey kem_decap(const KemCiphertext& c,
               const PrivateKey& sk,
               const PublicKey& pk,   // para re-encriptar y comprobar
               const Params& prm,
               KdfFunc kdf = default_kdf);

} // namespace gln
