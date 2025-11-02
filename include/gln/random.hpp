#pragma once
#include <random>
#include <cstdint>

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
};

} // namespace gln
