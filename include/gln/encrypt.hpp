#pragma once
#include <vector>
#include <stdexcept>
#include "gln/bigint.hpp"
#include "gln/keygen.hpp"   // PublicKey
#include "gln/params.hpp"   // (solo para la versión checked)

namespace gln {

struct Ciphertext {
    BigInt c1; // en [0, g-1]
    BigInt c2; // suma de los símbolos
};

// Cifra un mensaje e (mismo tamaño que pk.t). No valida rangos ni peso.
Ciphertext encrypt(const std::vector<uint32_t>& e, const PublicKey& pk);

// Versión opcional con validaciones: tamaño, peso exacto t_w y símbolos < z.
// Lanza std::invalid_argument si no se cumple.
Ciphertext encrypt_checked(const std::vector<uint32_t>& e,
                           const PublicKey& pk,
                           const Params& prm);

} // namespace gln
