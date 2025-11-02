#include <cassert>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <stdexcept>

#include "gln/bigint.hpp"
#include "gln/nt_utils.hpp"    // mod
#include "gln/random.hpp"
#include "gln/params.hpp"
#include "gln/keygen.hpp"
#include "gln/encrypt.hpp"
#include "gln/decrypt.hpp"

using gln::BigInt;

static std::size_t ceil_log2(std::size_t x) {
    if (x <= 1) return 0;
    std::size_t r = 0, v = x - 1;
    while (v > 0) { v >>= 1; ++r; }
    return r;
}

static gln::Params make_params(std::size_t n, std::size_t t, std::size_t z, std::size_t beta=1) {
    gln::Params prm{};
    prm.n = n; prm.t_w = t; prm.z = z; prm.beta = beta;
    prm.b = ceil_log2(z);
    prm.ell = ceil_log2(t == 0 ? 1 : t);
    // bits_g = max{ 2*t*(b+beta)+3, 2*( b + ell + (t-1)*(b+beta) ) + 3 }
    const std::size_t b = prm.b, ell = prm.ell;
    const std::size_t bits1 = 2 * t * (b + beta) + 3;
    const std::size_t bits2 = 2 * (b + ell + (t - 1) * (b + beta)) + 3;
    prm.bits_g = (bits1 > bits2) ? bits1 : bits2;
    return prm;
}

// ====================== TESTS DEL PAPER ======================
// Ejemplo 1 (paper §2.3): n=7, t=2, z=5; P primo; sin ajuste.
// P=[29,67,19,5,59,71,7]; g=1529269632; u=1056683436;
// c1=7374415530, c2=6; s=1480359090; TrEEA -> lambda=469, omega=282;
// ecuaciones 7*e2=282 (mod 67), 67*e7=282 (mod 7) -> e2=2, e7=4.
// Fuente: paper, Sección 2.3, Ejemplo 2.  :contentReference[oaicite:2]{index=2}
static void test_memoria() {
    auto prm = make_params(/*n=*/6, /*t=*/2, /*z=*/5);

    gln::PrivateKey sk;
    sk.p = { BigInt(5), BigInt(7), BigInt(11), BigInt(13), BigInt(17), BigInt(19) };
    sk.g = BigInt(417331);
    sk.u = BigInt(123456);

    gln::Ciphertext ct{ BigInt(493281), BigInt(7) };

    BigInt s_expected(105480); // paper
    BigInt s = gln::mod(ct.c1 + sk.u * ct.c2, sk.g);
    assert(s == s_expected);

    std::vector<uint32_t> e = gln::decrypt(ct, sk, prm);
    std::vector<uint32_t> e_expected = {0, 3, 0, 4, 0, 0};
    assert(e == e_expected);

    std::cout << "[OK] decrypt paper Example 2 passed\n";
}

// Ejemplo 2 (paper §2.3): n=10, t=3, z=10; P no primo; ajuste gamma.
// c1=325910333959, c2=12; s=(c1+u*c2) mod g = 4693214982; TrEEA -> lambda=1771, omega=1572,
// gcd con P revela pos {1,7,8}; gamma=2 -> lambda=3542, omega=3144; soluciones e1=8, e7=3, e8=1.
// Fuente: paper, Sección 2.3, Ejemplo 1.  :contentReference[oaicite:1]{index=1}
static void test_paper_example1_decrypt() {
    auto prm = make_params(/*n=*/10, /*t=*/3, /*z=*/10);

    // Clave secreta del ejemplo
    gln::PrivateKey sk;
    sk.p = {
        BigInt(14), BigInt(19), BigInt(27), BigInt(37), BigInt(25),
        BigInt(41), BigInt(11), BigInt(23), BigInt(47), BigInt(43)
    };
    sk.g = BigInt(55411224877);
    sk.u = BigInt(33260733702);

    // Ciphertext del ejemplo
    gln::Ciphertext ct{ BigInt(325910333959), BigInt(12) };

    // Comprobación intermedia: s esperado
    BigInt s_expected(4693214982); // paper
    BigInt s = gln::mod(ct.c1 + sk.u * ct.c2, sk.g);
    assert(s == s_expected);

    // Decrypt
    std::vector<uint32_t> e = gln::decrypt(ct, sk, prm);

    // Plaintext esperado
    std::vector<uint32_t> e_expected = {8,0,0,0,0,0,3,1,0,0};
    assert(e.size() == e_expected.size());
    assert(e == e_expected);

    std::cout << "[OK] decrypt paper Example 1 passed\n";
}


// Ejemplo 2 (paper §2.3): n=7, t=2, z=5; P primo; sin ajuste.
// P=[29,67,19,5,59,71,7]; g=1529269632; u=1056683436;
// c1=7374415530, c2=6; s=1480359090; TrEEA -> lambda=469, omega=282;
// ecuaciones 7*e2=282 (mod 67), 67*e7=282 (mod 7) -> e2=2, e7=4.
// Fuente: paper, Sección 2.3, Ejemplo 2.  :contentReference[oaicite:2]{index=2}
static void test_paper_example2_decrypt() {
    auto prm = make_params(/*n=*/7, /*t=*/2, /*z=*/5);

    gln::PrivateKey sk;
    sk.p = { BigInt(29), BigInt(67), BigInt(19), BigInt(5), BigInt(59), BigInt(71), BigInt(7) };
    sk.g = BigInt(1529269632);
    sk.u = BigInt(1056683436);

    gln::Ciphertext ct{ BigInt(7374415530), BigInt(6) };

    BigInt s_expected(1480359090); // paper
    BigInt s = gln::mod(ct.c1 + sk.u * ct.c2, sk.g);
    assert(s == s_expected);

    std::vector<uint32_t> e = gln::decrypt(ct, sk, prm);
    std::vector<uint32_t> e_expected = {0,2,0,0,0,0,4};
    assert(e == e_expected);

    std::cout << "[OK] decrypt paper Example 2 passed\n";
}

// ====================== ROUND-TRIP ALEATORIO ======================

static std::vector<uint32_t> random_plaintext(gln::Random& rng,
                                              std::size_t n, std::size_t t, std::size_t z)
{
    std::vector<uint32_t> e(n, 0u);
    // Elige t posiciones distintas
    std::vector<std::size_t> idx(n);
    for (std::size_t i = 0; i < n; ++i) idx[i] = i;
    std::shuffle(idx.begin(), idx.end(), std::mt19937_64{rng.uniform_uint64(0, UINT64_MAX)});

    for (std::size_t j = 0; j < t; ++j) {
        std::size_t k = idx[j];
        uint64_t val = 1 + (rng.uniform_uint64(0, UINT64_MAX) % (z - 1)); // en [1, z-1]
        e[k] = static_cast<uint32_t>(val);
    }
    return e;
}

static void test_roundtrip_keygen_encrypt_decrypt() {
    gln::Random rng(123456789ULL); // reproducible

    // Parámetros pequeños pero válidos
    auto prm = make_params(/*n=*/32, /*t=*/6, /*z=*/256, /*beta=*/1);

    // Genera claves y prueba varios mensajes
    auto kp = gln::keygen(prm, rng);
    for (int rep = 0; rep < 8; ++rep) {
        auto e = random_plaintext(rng, prm.n, prm.t_w, prm.z);
        auto ct = gln::encrypt_checked(e, kp.pk, prm);
        auto d  = gln::decrypt(ct, kp.sk, prm);
        assert(d == e);
    }

    std::cout << "[OK] decrypt round-trip keygen/encrypt/decrypt passed\n";
}

int main() {
    test_memoria();
    test_paper_example1_decrypt();
    test_paper_example2_decrypt();
    test_roundtrip_keygen_encrypt_decrypt();
    return 0;
}