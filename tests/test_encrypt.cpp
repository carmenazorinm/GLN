#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "gln/bigint.hpp"
#include "gln/encrypt.hpp"
#include "gln/keygen.hpp"
#include "gln/params.hpp"

using gln::BigInt;

static std::size_t ceil_log2(std::size_t x) {
    if (x <= 1) return 0;
    std::size_t r = 0, v = x - 1;
    while (v > 0) { v >>= 1; ++r; }
    return r;
}

static gln::Params make_params_encrypt(std::size_t n, std::size_t t, std::size_t z) {
    gln::Params prm{};
    prm.n = n;
    prm.t_w = t;
    prm.z = z;
    prm.b = ceil_log2(z);
    prm.ell = ceil_log2(t == 0 ? 1 : t);
    prm.beta = 1;     // no se usa en encrypt_checked
    prm.bits_g = 0;   // no se usa en encrypt_checked
    return prm;
}

static void test_example_encrypt() {
    // Public key del ejemplo (solo necesitamos t y un g suficientemente grande)
    gln::PublicKey pk;
    pk.t = {
        BigInt(210409), BigInt(115019), BigInt(66240),
        BigInt(37056),  BigInt(97484),  BigInt(109824)
    };
    pk.g = BigInt(1000003); // > 493281, así c1 no cambia al hacer mod g

    // Parámetros del ejemplo
    auto prm = make_params_encrypt(/*n=*/6, /*t=*/2, /*z=*/5);

    // Mensaje e = (0,3,0,4,0,0)
    std::vector<uint32_t> e = {0, 3, 0, 4, 0, 0};

    gln::Ciphertext ct = gln::encrypt_checked(e, pk, prm);

    // Comprobaciones esperadas:
    // c1 = 3*115019 + 4*37056 = 493281
    // c2 = 3 + 4 = 7
    assert(ct.c1 == BigInt(493281));
    assert(ct.c2 == BigInt(7));

    std::cout << "[OK] encrypt example: c1=" << ct.c1.str()
              << ", c2=" << ct.c2.str() << "\n";
}

static void test_example2_encrypt() {
    // Public key del ejemplo (solo necesitamos t y un g suficientemente grande)
    gln::PublicKey pk;
    pk.t = {
        BigInt(18192546541), BigInt(13401350405), BigInt(20098223587),
        BigInt(47609702605),  BigInt(48747879116),  BigInt(47828863679),
        BigInt(42300027494),  BigInt(53469879149),  BigInt(30403226795),
        BigInt(18284591765)
    };
    pk.g = BigInt(55411224877); // > 493281, así c1 no cambia al hacer mod g

    // Parámetros del ejemplo
    auto prm = make_params_encrypt(/*n=*/10, /*t=*/3, /*z=*/10);

    // Mensaje e = (0,3,0,4,0,0)
    std::vector<uint32_t> e = {8, 0, 0, 0, 0, 0, 3, 1, 0, 0};

    gln::Ciphertext ct = gln::encrypt_checked(e, pk, prm);

    // Comprobaciones esperadas:
    assert(ct.c1 == BigInt(325910333959));
    assert(ct.c2 == BigInt(12));

    std::cout << "[OK] encrypt example: c1=" << ct.c1.str()
              << ", c2=" << ct.c2.str() << "\n";
}

static void test_example3_encrypt() {
    // Public key del ejemplo (solo necesitamos t y un g suficientemente grande)
    gln::PublicKey pk;
    pk.t = {
        BigInt(1369054601), BigInt(1431232831), BigInt(1116489199),
        BigInt(1084294049),  BigInt(757704263),  BigInt(257196107),
        BigInt(1127987467)
    };
    pk.g = BigInt(1529269632); // > 493281, así c1 no cambia al hacer mod g

    // Parámetros del ejemplo
    auto prm = make_params_encrypt(/*n=*/7, /*t=*/2, /*z=*/5);

    // Mensaje e = (0,3,0,4,0,0)
    std::vector<uint32_t> e = {0, 2, 0, 0, 0, 0, 4};

    gln::Ciphertext ct = gln::encrypt_checked(e, pk, prm);

    // Comprobaciones esperadas:
    assert(ct.c1 == BigInt(7374415530));
    assert(ct.c2 == BigInt(6));

    std::cout << "[OK] encrypt example: c1=" << ct.c1.str()
              << ", c2=" << ct.c2.str() << "\n";
}

static void test_bad_weight_throws() {
    gln::PublicKey pk;
    pk.t = { BigInt(10), BigInt(20), BigInt(30), BigInt(40), BigInt(50), BigInt(60) };
    pk.g = BigInt(1000003);

    auto prm = make_params_encrypt(/*n=*/6, /*t=*/2, /*z=*/5);

    // Solo 1 posición no nula (peso=1) -> debe lanzar
    std::vector<uint32_t> e = {0, 3, 0, 0, 0, 0};

    bool thrown = false;
    try {
        (void)gln::encrypt_checked(e, pk, prm);
    } catch (const std::invalid_argument&) {
        thrown = true;
    }
    assert(thrown);
    std::cout << "[OK] encrypt_checked: bad weight throws\n";
}

static void test_out_of_range_symbol_throws() {
    gln::PublicKey pk;
    pk.t = { BigInt(10), BigInt(20), BigInt(30), BigInt(40), BigInt(50), BigInt(60) };
    pk.g = BigInt(1000003);

    auto prm = make_params_encrypt(/*n=*/6, /*t=*/2, /*z=*/5);

    // Símbolo 5 (fuera de [0,4]) -> debe lanzar
    std::vector<uint32_t> e = {0, 5, 0, 4, 0, 0};

    bool thrown = false;
    try {
        (void)gln::encrypt_checked(e, pk, prm);
    } catch (const std::invalid_argument&) {
        thrown = true;
    }
    assert(thrown);
    std::cout << "[OK] encrypt_checked: out-of-range symbol throws\n";
}

int main() {
    test_example_encrypt();
    test_example2_encrypt();
    test_example3_encrypt();
    test_bad_weight_throws();
    test_out_of_range_symbol_throws();
    return 0;
}
