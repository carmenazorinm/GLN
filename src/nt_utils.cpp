#include "gln/nt_utils.hpp"
#include <stdexcept>
#include <iostream>

namespace gln {

BigInt mod(const BigInt& a, const BigInt& m){
  BigInt r = a % m;
  if (BigInt::sgn(r) < 0) r += m;
  return r;
}
BigInt addmod(const BigInt& a, const BigInt& b, const BigInt& m){
  BigInt r = a + b; r = r % m; if (BigInt::sgn(r) < 0) r += m; return r;
}
BigInt submod(const BigInt& a, const BigInt& b, const BigInt& m){
  BigInt r = a - b; r = r % m; if (BigInt::sgn(r) < 0) r += m; return r;
}
BigInt mulmod(const BigInt& a, const BigInt& b, const BigInt& m){
  BigInt r = (a * b) % m; if (BigInt::sgn(r) < 0) r += m; return r;
}

BigInt gcd(BigInt a, BigInt b){
  while (BigInt::sgn(b) != 0){ BigInt r = a % b; a = b; b = r; }
  return BigInt::abs(a);
}

void egcd(const BigInt& a, const BigInt& b, BigInt& x, BigInt& y, BigInt& g){
  // Iterativo (EEA): devuelve x,y,g con ax + by = g = gcd(a,b)
  BigInt old_r = a, r = b;
  BigInt old_s = BigInt(1), s = BigInt(0);
  BigInt old_t = BigInt(0), t = BigInt(1);
  while (BigInt::sgn(r) != 0){
    BigInt q = old_r / r;
    BigInt tmp;

    tmp = old_r - q*r; old_r = r; r = tmp;
    tmp = old_s - q*s; old_s = s; s = tmp;
    tmp = old_t - q*t; old_t = t; t = tmp;
  }
  x = old_s; y = old_t; g = old_r;
}

BigInt invmod(const BigInt& a, const BigInt& m){
  if (m < BigInt(0)) throw std::runtime_error("invmod: m must be positive");
  BigInt x,y,g; egcd(mod(a,m), m, x, y, g);
  if (!(g == BigInt(1))) throw std::runtime_error("invmod: no invertible");
  return mod(x, m);
}

// TrEEA:
TreeaOut treea(BigInt g, BigInt s){
  if (g < s || s <= BigInt(0) || g <= BigInt(0)) throw std::runtime_error("treea: requirement a >= b > 0");
  BigInt r0 = g, r1 = s;
  BigInt v0 = BigInt(0), v1 = BigInt(1);

  // Primer paso “pre”
  BigInt c = r0 / r1;
  BigInt r = r0 - c*r1; r0 = r1; r1 = r;
  BigInt v = BigInt(0) - c; v0 = v1; v1 = v;

  while ((r1*r1) >= g){
    // std::cout << "v1 = " << v1 << " r1 = " << r1 << std::endl;
    c = r0 / r1;
    r = r0 - c*r1; r0 = r1; r1 = r;
    v = v0 - c*v1; v0 = v1; v1 = v;
  }

  // std::cout << "v1 = " << v1 << " r1 = " << r1 << std::endl;
  return { v1, r1 };
}

// Miller–Rabin muy básico (para empezar)
static BigInt powmod(BigInt a, BigInt e, const BigInt& m){
  BigInt res = BigInt(1);
  a = mod(a,m);
  while (BigInt::sgn(e) > 0){
    if ((e % BigInt(2)) == BigInt(1)) res = mulmod(res, a, m);
    e /= BigInt(2);
    a = mulmod(a, a, m);
  }
  return res;
}

bool is_probable_prime(const BigInt& n, int rounds){
  // Rechazos rápidos
  if (n <= BigInt(1)) return false;
  if (n == BigInt(2) || n == BigInt(3)) return true;
  if ((n % BigInt(2)) == BigInt(0)) return false;

  // n-1 = d * 2^s
  BigInt d = n - BigInt(1);
  int s = 0;
  while ((d % BigInt(2)) == BigInt(0)){ d /= BigInt(2); ++s; }

  // bases chicas deterministas para empezar
  const int bases_small[] = {2,3,5,7,11,13,17,19,23,29,31};
  int use = std::min(rounds, (int)(sizeof(bases_small)/sizeof(int)));
  for (int i=0;i<use;i++){
    BigInt a = BigInt(bases_small[i]);
    if (a >= n- BigInt(2)) continue;
    BigInt x = powmod(a, d, n);
    if (x == BigInt(1) || x == n - BigInt(1)) continue;
    bool maybe = false;
    for (int r=1; r<s; ++r){
      x = mulmod(x, x, n);
      if (x == n - BigInt(1)){ maybe = true; break; }
    }
    if (!maybe) return false;
  }
  return true;
}

BigInt next_prime(const BigInt& start){
  BigInt p = start;
  if ((p % BigInt(2)) == BigInt(0)) p += BigInt(1);
  while (!is_probable_prime(p)) p += BigInt(2);
  return p;
}

} // namespace gln
