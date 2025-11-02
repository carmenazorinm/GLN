#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "gln/params.hpp"

using gln::Params;
using gln::choose_params;

static std::size_t ceil_log2_u64(std::size_t x) {
    if (x <= 1) return 1;
    // usar double para test; es suficiente para rangos típicos de parámetros
    return static_cast<std::size_t>(std::ceil(std::log2(static_cast<double>(x))));
}

// Verifica internamente la fórmula de bits_g
static void check_bits_g_formula(const Params& P) {
    std::size_t term1 = 2 * P.t_w * (P.b + P.beta) + 3;
    std::size_t term2 = 2 * (P.t_w * (P.b + P.beta) + P.ell - P.beta) + 3;
    std::size_t expect = std::max(term1, term2);

    assert(P.bits_g >= term1);
    assert(P.bits_g >= term2);
    assert(P.bits_g == expect);
}

static void case_basic(std::size_t n, std::size_t t, std::size_t z, std::size_t beta) {
    Params P = choose_params(n, t, z, beta);
    assert(P.n == n);
    assert(P.t_w == t);
    assert(P.z == z);
    assert(P.beta == beta);

    std::size_t expect_b   = ceil_log2_u64(z);
    std::size_t expect_ell = ceil_log2_u64(t);
    assert(P.b   == expect_b);
    assert(P.ell == expect_ell);

    check_bits_g_formula(P);
}

int main() {
    // Casos típicos
    case_basic(/*n*/16, /*t*/4,  /*z*/8,  /*beta*/1);   // b=3, ell=2
    case_basic(32,      5,       10,      2);          // b=4, ell=3
    case_basic(64,      7,       256,     1);          // b=8, ell=3
    case_basic(128,     10,      17,      3);          // b=5, ell=4

    // Edge cases razonables
    case_basic(8,  1,  2, 1);   // t=1 -> ell=1; z=2 -> b=1
    case_basic(8,  1,  3, 1);   // z=3 -> b=2
    case_basic(8,  2,  1, 1);   // z=1 -> b=1 (definido en implementación)
    case_basic(8,  64, 1, 2);   // z=1 -> b=1; t grande

    // Barrido de algunos tamaños para estresar la fórmula
    for (std::size_t t : {2ul,3ul,4ul,8ul,16ul,31ul,32ul}) {
        for (std::size_t z : {2ul,3ul,4ul,7ul,8ul,9ul,16ul,31ul,32ul,33ul}) {
            Params P = choose_params(/*n*/64, t, z, /*beta*/1);
            // Verifica b, ell y fórmula de bits_g
            assert(P.b   == ceil_log2_u64(z));
            assert(P.ell == ceil_log2_u64(t));
            check_bits_g_formula(P);
        }
    }

    std::cout << "test_params: OK\n";
    return 0;
}
