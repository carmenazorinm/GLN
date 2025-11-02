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

namespace {
// Convierte un buffer de bytes big-endian a BigInt (usa ctor desde string decimal si no tienes uno binario)
static BigInt be_bytes_to_bigint(const uint8_t* buf, size_t len) {
    // Implementación simple: construir en base 256 acumulando.
    // Asumimos que BigInt tiene operadores básicos *=, +=
    BigInt x(0);
    for (size_t i = 0; i < len; ++i) {
        x *= BigInt(256);
        x += BigInt(static_cast<unsigned long long>(buf[i]));
    }
    return x;
}

// Genera un BigInt uniforme en [0, 2^bits - 1]
static BigInt random_bits(gln::Random& rng, size_t bits) {
    if (bits == 0) return BigInt(0);
    size_t bytes = (bits + 7) / 8;
    std::vector<uint8_t> buf(bytes);
    rng.random_bytes(buf.data(), buf.size());
    // clear bits above 'bits'
    unsigned extra = static_cast<unsigned>(bytes * 8 - bits);
    if (extra) buf[0] &= static_cast<uint8_t>(0xFFu >> extra);
    return be_bytes_to_bigint(buf.data(), buf.size());
}

// Genera un BigInt en [2^(bits-1), 2^bits - 1] y lo fuerza impar si force_odd
static BigInt random_of_bitlen(gln::Random& rng, size_t bits, bool force_odd = false) {
    if (bits == 0) return BigInt(0);
    BigInt x = random_bits(rng, bits);
    // asegúrate del bit alto
    BigInt one(1);
    BigInt high(1);
    for (size_t i = 0; i + 1 < bits; ++i) {
        high += high; // equivale a high *= 2; usa lo que tengas implementado
    }
    if (x < high) x += high;
    // Si tu BigInt aún no tiene operador <<, reemplaza esta línea por:
    // high = BigInt("1"); for (size_t i=0;i<bits-1;i++) high = high + high;
    if (x < high) x += high;
    // si se pasó, recorta a bits ajustando con mod 2^bits (opcional)
    if (force_odd) {
        if (((x % gln::BigInt(2)) == gln::BigInt(0))) x += one;
    }
    return x;
}

// Uniforme en [1, m-1]
static BigInt random_range_1_to_m1(gln::Random& rng, const BigInt& m) {
    // Rechazo simple: genera k bits ~ bitlen(m), toma mod m, reintenta si 0
    // (m es grande; la expectativa de reintentos es baja)
    // Necesitas bit_length(); si no lo tienes, aproxima con m.str() base10 -> bits~log2(10)*digits
    const std::string ms = m.str();
    // aprox bits por decimal:  log2(10)≈3.322
    size_t bits = static_cast<size_t>(ms.size() * 3323ull / 1000ull) + 2u;
    for (;;) {
        BigInt x = mod(random_bits(rng, bits), m);
        if (!(x == BigInt(0))) return x;
    }
}

// Genera un impar aleatorio de 'bits' y sube al siguiente primo >= ese impar.
static BigInt random_prime_bits(gln::Random& rng, size_t bits, const BigInt& z_min) {
    for (;;) {
        BigInt start = random_of_bitlen(rng, bits, /*force_odd=*/true);
        BigInt p = next_prime(start);
        if (p >= z_min) return p;
        // si el primer primo queda por debajo de z, intenta con más bits o repite
    }
}

} // anonymous namespace

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
            BigInt cand = random_prime_bits(rng, bits, z_min);
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
        g = random_of_bitlen(rng, bits_g, /*force_odd=*/true);
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
        u = random_range_1_to_m1(rng, g);
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
