#include "gln/encrypt.hpp"
#include "gln/nt_utils.hpp"   // mod, addmod, mulmod

#include <cstddef>

namespace gln {

Ciphertext encrypt(const std::vector<uint32_t>& e, const PublicKey& pk) {
    if (e.size() != pk.t.size()) {
        throw std::invalid_argument("encrypt: |e| must equal |pk.t|");
    }

    BigInt c1(0);
    BigInt c2(0);

    // c1 = sum e_i * t_i 
    // c2 = sum e_i
    for (std::size_t i = 0; i < e.size(); ++i) {
        uint32_t ei_u32 = e[i];
        if (ei_u32 == 0) continue;

        BigInt ei(static_cast<long long>(ei_u32));

        c2 += ei;
        // c1 <- (c1 + ei * t_i) 
        BigInt term = ei*pk.t[i];
        c1 = c1+ term;
    }

    return Ciphertext{ std::move(c1), std::move(c2) };
}

Ciphertext encrypt_checked(const std::vector<uint32_t>& e,
                           const PublicKey& pk,
                           const Params& prm) {
    if (e.size() != pk.t.size()) {
        throw std::invalid_argument("encrypt_checked: |e| must equal |pk.t|");
    }

    // Valida que cada símbolo e_i está en [0, z-1] y que el peso es t_w
    std::size_t weight = 0;
    for (uint32_t ei : e) {
        if (ei >= prm.z) {
            throw std::invalid_argument("encrypt_checked: symbol out of range [0, z-1]");
        }
        if (ei != 0) ++weight;
    }
    if (weight != prm.t_w) {
        throw std::invalid_argument("encrypt_checked: Hamming weight(e) != t_w");
    }

    return encrypt(e, pk);
}

} // namespace gln
