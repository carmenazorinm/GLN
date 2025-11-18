// src/keygen.cpp
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

#include "gln/keygen.hpp"
#include "gln/nt_utils.hpp"   // gcd, invmod, next_prime, mod, submod
#include "gln/params.hpp"
#include "gln/random.hpp"

namespace gln {


KeyPair keygen(const Params& prm, gln::Random& rng) {
    // 1) Elegir n primos p_i en [b+1, b+beta] bits (>= z) y mutuamente coprimos
    //    (siendo primos y distintos, ya son coprimos).  :contentReference[oaicite:4]{index=4}
    const size_t n     = prm.n;
    const size_t b     = prm.b;       // ceil(log2 z)
    const size_t beta  = prm.beta;    // ventana de bits para primos
    const size_t t     = prm.t_w;     // peso (t)
    const BigInt z_min = BigInt(static_cast<long long>(prm.z));

    std::vector<BigInt> p; p.reserve(n);

    // Estrategia: alternar entre tamaños de bits en [b+1, b+beta]
    for (size_t i = 0; i < n; ++i) {
        size_t bits = (beta == 0 ? (b+1) : (b + 1 + (i % beta)));
        for (;;) {
            BigInt cand = rng.random_prime_bits(bits, z_min);
            // evitar duplicados
            bool dup = false;
            for (const auto& q : p) { if (cand == q) { dup = true; break; } }
            if (!dup) { p.push_back(cand); break; }
        }
    }

    // 2) Elegir g de prm.bits_g bits, coprimo con todos los p_i.  :contentReference[oaicite:5]{index=5}
    const size_t bits_g = prm.bits_g; // calculado con choose_params(), ver Remark 1
    BigInt g;
    for (;;) {
        g = rng.random_of_bitlen(bits_g, /*force_odd=*/true);
        // Asegura gcd(g, p_i)=1
        bool ok = true;
        for (const auto& pi : p) {
            if (gcd(g, pi) != BigInt(1)) { ok = false; break; }
        }
        if (ok) break;
    }

    // 3) h_i = p_i^{-1} mod g   (0 < h_i < g)  :contentReference[oaicite:6]{index=6}
    std::vector<BigInt> h; h.reserve(n);
    h.resize(n);
    for (size_t i = 0; i < n; ++i) {
        h[i] = invmod(p[i], g); // lanza si no invertible; debería serlo por gcd=1
    }

    // 4) u aleatorio en (0,g), distinto de todos los h_i  :contentReference[oaicite:7]{index=7}
    BigInt u;
    for (;;) {
        u = rng.random_range_1_to_m1(g, bits_g);
        bool clash = false;
        for (const auto& hi : h) { if (u == hi) { clash = true; break; } }
        if (!clash) break;
    }

    // 5) t_i = (h_i - u) mod g  (0 <= t_i < g)  :contentReference[oaicite:8]{index=8}
    std::vector<BigInt> tpub; tpub.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        tpub.push_back( mod(submod(h[i], u, g), g) );
    }

    PublicKey pk{ std::move(tpub), g };
    PrivateKey sk{ std::move(p), g, u };
    return KeyPair{ std::move(pk), std::move(sk) };
}

} // namespace gln
