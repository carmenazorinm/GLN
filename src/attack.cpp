#include "gln/attack.hpp"
#include "gln/nt_utils.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <iostream>
#include <cmath>

namespace gln {

std::vector<BigInt> generate_prime_candidates(std::size_t b,
                                              std::size_t beta,
                                              std::size_t max_candidates)
{
  std::vector<BigInt> P;
  if (b >= 63 || beta == 0) return P;

  BigInt lo = (BigInt)1 << b;
  BigInt hi = (BigInt)1 << (b + beta);
  if ((lo & BigInt(1ULL)) == BigInt(0)) lo=lo+BigInt(1); // impar

  for (BigInt x = lo; x < hi && P.size() < max_candidates; x += BigInt(2ULL)) {
    if (is_probable_prime(x)) P.emplace_back(x);
  }
  return P;
}

AttackReport attack_triples(const PublicKey& pk,
                            std::tuple<std::size_t,std::size_t,std::size_t> indices,
                            std::size_t b,
                            std::size_t beta,
                            std::size_t max_candidates)
{
  AttackReport rep;
  auto [i, j, k] = indices;
  if (i >= pk.t.size() || j >= pk.t.size() || k >= pk.t.size()) return rep;

  const BigInt& t1 = pk.t[i];
  const BigInt& t2 = pk.t[j];
  const BigInt& t3 = pk.t[k];

  std::vector<BigInt> P = generate_prime_candidates(b, beta, max_candidates);
  if (P.size() < 3) return rep;

  BigInt best_g(0);
  std::size_t tested = 0;
  const std::size_t N = P.size();
  for (std::size_t a = 0; a < N; ++a) {
    const BigInt& p1 = P[a];
    for (std::size_t b2 = a+1; b2 < N; ++b2) {
      const BigInt& p2 = P[b2];
      BigInt X12 = (p1 * p2) * (t1 - t2) - (p2 - p1);

      for (std::size_t c = b2+1; c < N; ++c) {
        const BigInt& p3 = P[c];
        BigInt X13 = (p1 * p3) * (t1 - t3) - (p3 - p1);

        BigInt g_guess = gcd(X12, X13);
        ++tested;

        if (g_guess > best_g) {
          best_g = g_guess;
        }
      }
    }
  }

  rep.g_guess = best_g;
  rep.combos_tested = tested;
  rep.found_exact = (best_g == pk.g) || (best_g % pk.g == BigInt(0));
  return rep;
}

AttackReport attack_triples(const PublicKey& pk, std::size_t b, std::size_t beta) {
  AttackReport rep;

  std::size_t max_candidates =  pow(2,(b+beta))/((b+beta)*log(2))-pow(2,b)/(b*log(2));
  std::vector<BigInt> P = generate_prime_candidates(b, beta, max_candidates);

  BigInt best_g(0);

  const std::size_t N = P.size();
  std::size_t tested = 0;

  BigInt t1 = pk.t[0];
  BigInt t2 = pk.t[1];
  BigInt t3 = pk.t[2];
  for(auto pos_t1 = 0; pos_t1 < pk.t.size() -3  && !rep.found_exact; pos_t1++) {
    for(auto pos_t2 = pos_t1+1; pos_t2 < pk.t.size() -2  && !rep.found_exact ; pos_t2++) {
      for(auto pos_t3 = pos_t2+1; pos_t3 < pk.t.size()-1  && !rep.found_exact ; pos_t3++) {
        t1 = pk.t[pos_t1];
        t2 = pk.t[pos_t2];
        t3 = pk.t[pos_t3];

        for (std::size_t a = 0; a < N  && !rep.found_exact; ++a) {
        const BigInt& p1 = P[a];
        for (std::size_t b2 = a+1; b2 < N  && !rep.found_exact ; ++b2) {
          const BigInt& p2 = P[b2];
          BigInt X12 = (p1 * p2) * (t1 - t2) - (p2 - p1);

          for (std::size_t c = b2+1; c < N  && !rep.found_exact; ++c) {
            const BigInt& p3 = P[c];
            BigInt X13 = (p1 * p3) * (t1 - t3) - (p3 - p1);

            BigInt g_guess = gcd(X12, X13);
            ++tested;

            if (g_guess > best_g) {
              best_g = g_guess;
            }
            rep.found_exact = (best_g == pk.g) || (best_g % pk.g == BigInt(0));
          }
        }
      }

      rep.g_guess = best_g;
      rep.combos_tested = tested;
      rep.found_exact = (best_g == pk.g) || (best_g % pk.g == BigInt(0));

      }
    }
  }
  return rep;
  
}


} // namespace gln
