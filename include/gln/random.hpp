#pragma once
#include <random>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include "gln/nt_utils.hpp"   // gcd, invmod, next_prime, mod, submod

namespace gln {

class Random {
    std::mt19937_64 eng;
public:
    Random() : eng(std::random_device{}()) {}
    explicit Random(uint64_t seed) : eng(seed) {}
    
    // Rellena un buffer con bytes aleatorios
    void random_bytes(uint8_t* dst, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = static_cast<uint8_t>(eng() & 0xFF);
        }
    }

    // Genera un entero uniforme en [low, high]
    uint64_t uniform_uint64(uint64_t low, uint64_t high) {
        std::uniform_int_distribution<uint64_t> dist(low, high);
        return dist(eng);
    }

    // Genera un BigInt uniforme (misma probabilidad para cada valor) en [0, 2^bits - 1]
    BigInt random_bits(size_t bits) {
        if (bits == 0) return BigInt(0);
        size_t bytes = (bits + 7) / 8;
        std::vector<uint8_t> buf(bytes);
        random_bytes(buf.data(), buf.size()); // rellena el buffer con bytes aleatorios
        // clear bits above 'bits'
        unsigned extra = static_cast<unsigned>(bytes * 8 - bits);
        if (extra) buf[0] &= static_cast<uint8_t>(0xFFu >> extra);
        return BigInt(buf.data(), buf.size());
    }

    /*
    random_of_bitlen(rng, 4) -> yo quiero 2^3=8 y 2^4-1=15
        x entre 0 y 15 -> x = 4
        i = 0, 1, 2
        high = 2, 4, 8
        if (x < 8) -> x += 8 == 12
        x = 13
        ---------
        lo que hace es, si sale:
        0 -> 0+8=8
        1 -> 1+8=9
        2 -> 2+8=10
        ...
        7 -> 7+8=15
        8, 9, 10, ..., 15 los deja igual
    */

    // Genera un BigInt en [2^(bits-1), 2^bits - 1] y lo fuerza impar si force_odd
    BigInt random_of_bitlen(size_t bits, bool force_odd = false) {
        if (bits == 0) return BigInt(0);
        BigInt x = random_bits(bits);
        // asegúrate del bit alto
        BigInt high(1);
        for (size_t i = 0; i + 1 < bits; ++i) {
            high += high; // equivale a high *= 2; usa lo que tengas implementado
        }
        if (x < high) x += high;

        if (force_odd) {
            if (((x % gln::BigInt(2)) == gln::BigInt(0))) x += BigInt(1);
        }
        return x;
    }

    // Devuelve uniforme en [1, m-1] usando rechazo exacto.
    // Precondición: bits_m >= bit_length(m) y m > 1.
    BigInt random_range_1_to_m1(const BigInt& m, size_t bits_m) {
        if (m <= BigInt(1)) throw std::invalid_argument("m must be > 1");

        // two_k = 2^bits_m
        BigInt two_k(1);
        for (size_t i = 0; i < bits_m; ++i) two_k += two_k;

        // Aceptaremos x solo si x < limit, donde limit es múltiplo de m
        BigInt limit = two_k - (two_k % m);  // = floor(2^k / m) * m

        BigInt r(0);
        while( r == BigInt(0) ){
            BigInt x = random_bits(bits_m);   // uniforme en [0, 2^k - 1]
            if (x >= limit) continue;           // rechaza la "cola"
            r = x % m;                   // ahora r es uniforme en [0, m-1]
            if (r != BigInt(0)) return r;       // queremos [1, m-1]
        }
    }


    // Genera un impar aleatorio de 'bits' y sube al siguiente primo >= ese impar.
    BigInt random_prime_bits(size_t bits, const BigInt& z_min) {
        for (;;) {
            BigInt start = random_of_bitlen( bits, /*force_odd=*/true);
            BigInt p = next_prime(start);
            if (p >= z_min) return p;
            // si el primer primo queda por debajo de z, intenta con más bits o repite
        }
    }
};

} // namespace gln
