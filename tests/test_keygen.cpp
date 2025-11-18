// tests/test_keygen.cpp
#include <cassert>
#include <set>
#include <string>
#include <iostream>

#include "gln/keygen.hpp"
#include "gln/nt_utils.hpp"   // gcd, invmod, mulmod, is_probable_prime
#include "gln/random.hpp"
#include "gln/params.hpp"

using gln::BigInt;

static std::size_t ceil_log2(std::size_t x) {
    if (x <= 1) return 0;
    std::size_t r = 0;
    std::size_t v = x - 1;
    while (v > 0) { v >>= 1; ++r; }
    return r;
}

static gln::Params make_params(std::size_t n, std::size_t t, std::size_t z, std::size_t beta) {
    gln::Params prm{};
    prm.n = n;
    prm.t_w = t;
    prm.z = z;
    prm.beta = beta;
    prm.b = ceil_log2(z);
    prm.ell = ceil_log2(t == 0 ? 1 : t);

    const std::size_t b = prm.b;
    const std::size_t ell = prm.ell;
    const std::size_t bits1 = 2 * t * (b + beta) + 3;
    const std::size_t bits2 = 2 * (b + ell + (t - 1) * (b + beta)) + 3;
    prm.bits_g = (bits1 > bits2) ? bits1 : bits2;

    return prm;
}

static void run_one_test(const gln::Params& prm) {
    std::cout << "[*] Testing keygen with n=" << prm.n
              << ", t=" << prm.t_w
              << ", z=" << prm.z
              << ", beta=" << prm.beta
              << ", bits_g=" << prm.bits_g << "\n";

    gln::Random rng;

    gln::KeyPair kp = gln::keygen(prm, rng);
    const auto& pk = kp.pk;
    const auto& sk = kp.sk;

    assert(pk.t.size() == prm.n);
    assert(sk.p.size() == prm.n);
    assert(pk.g == sk.g);

    assert(pk.g > BigInt(1));

    std::set<std::string> seen_p;
    for (const auto& pi : sk.p) {
        assert(pi >= BigInt(static_cast<long long>(prm.z)));
        assert(gln::is_probable_prime(pi));
        assert(gln::gcd(pk.g, pi) == BigInt(1));
        assert(seen_p.insert(pi.str()).second);
    }

    for (const auto& ti : pk.t) {
        assert(ti >= BigInt(0));
        assert(ti < pk.g);
    }

    for (std::size_t i = 0; i < prm.n; ++i) {
        BigInt hi = gln::invmod(sk.p[i], pk.g);
        assert(gln::mulmod(sk.p[i], hi, pk.g) == BigInt(1));
        BigInt lhs = gln::mod(gln::submod(hi, sk.u, pk.g), pk.g);
        assert(lhs == pk.t[i]);
    }

    std::cout << "[OK] keygen invariants passed.\n";
}

int main() {
    run_one_test(make_params(/*n=*/32, /*t=*/6, /*z=*/256, /*beta=*/1));

    run_one_test(make_params(/*n=*/64, /*t=*/8, /*z=*/256, /*beta=*/2));

    return 0;
}
