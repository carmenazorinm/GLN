#pragma once
#include "gln/bigint.hpp"

namespace gln {

BigInt mod(const BigInt& a, const BigInt& m);        // en [0, m-1]
BigInt addmod(const BigInt& a, const BigInt& b, const BigInt& m);
BigInt submod(const BigInt& a, const BigInt& b, const BigInt& m);
BigInt mulmod(const BigInt& a, const BigInt& b, const BigInt& m);

BigInt gcd(BigInt a, BigInt b);
void   egcd(const BigInt& a, const BigInt& b, BigInt& x, BigInt& y, BigInt& g);
BigInt invmod(const BigInt& a, const BigInt& m);     // lanza si no invertible

struct TreeaOut { BigInt v; BigInt r; };
TreeaOut treea(BigInt g, BigInt s);                  // TrEEA

bool   is_probable_prime(const BigInt& n, int rounds=32);
BigInt next_prime(const BigInt& start);

} // namespace gln
