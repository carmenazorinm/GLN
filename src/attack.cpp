#include "gln/attack.hpp"
#include "gln/nt_utils.hpp"
#include <algorithm>
#include <chrono>
#include <random>
#include <iostream>

namespace gln {

std::vector<BigInt> generate_prime_candidates(std::size_t b,
                                              std::size_t beta,
                                              std::size_t max_candidates)
{
  // Rango [2^b, 2^(b+beta)) – empezamos en el primer impar >= 2^b+1
  // Nota: aquí uso uint64_t para demo. Si tus b,beta superan 63 bits de rango,
  // reemplaza por tu generador de primos en BigInt (nt_utils).
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


AttackReport attack_pairs_demo(const std::vector<BigInt>& h,
                               const std::vector<BigInt>& p_true_optional)
{
  AttackReport rep;
  if (h.size() < 2) return rep;

  // Para demo: si nos pasan p_true_optional del mismo tamaño, usamos (h_i, p_i) correctos.
  // Si no, asumimos que h y p_true_optional[0..] están alineados para la demo.
  std::vector<BigInt> p = p_true_optional;
  if (p.size() < h.size()) p.resize(h.size(), BigInt(0));

  // Toma dos índices i != j
  std::size_t i = 0, j = 1;
  if (gln::BigInt::sgn(p[i]) == 0 || gln::BigInt::sgn(p[j]) == 0){
    // Sin p_i reales no podemos formar h_i * p_i - 1. Este ataque solo es para demo.
    return rep;
  }

  BigInt X1 = h[i] * p[i] - BigInt(1);
  BigInt X2 = h[j] * p[j] - BigInt(1);
  rep.g_guess = gcd(X1, X2);
  rep.combos_tested = 1;
  // rep.found_exact lo puedes marcar en tests comparando con pk.g
  return rep;
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

  // 1) Genera candidatos de primos (puedes sustituir por tu nt_utils)
  // std::cout << "Genero primos candidatos: " << std::endl;
  std::vector<BigInt> P = generate_prime_candidates(b, beta, max_candidates);
  // std::cout << "Primos candidatos generados: " << P.size() << std::endl;
  if (P.size() < 3) return rep;

  BigInt best_g(0);
  std::size_t tested = 0;

  // 2) Bucle sobre triadas. Para pruebas, limítalo (p.ej. primeros 2000 primos)
  const std::size_t N = P.size();
  for (std::size_t a = 0; a < N; ++a) {
    const BigInt& p1 = P[a];
    for (std::size_t b2 = a+1; b2 < N; ++b2) {
      const BigInt& p2 = P[b2];
      // Precalcula parte X12
      BigInt X12 = (p1 * p2) * (t1 - t2) - (p2 - p1);

      for (std::size_t c = b2+1; c < N; ++c) {
        const BigInt& p3 = P[c];
        BigInt X13 = (p1 * p3) * (t1 - t3) - (p3 - p1);

        BigInt g_guess = gcd(X12, X13);
        // std::cout << "gcd: " << g_guess.str() << " primes: "<< p1 << ", " << p2 << ", " << p3 << std::endl;
        ++tested;

        // Conserva el gcd más grande observado
        if (g_guess > best_g) {
          best_g = g_guess;
          // Heurística: si best_g tiene ~bits similares a pk.g (si lo conoces), puedes parar.
          // Aquí no comparamos con pk.g para no depender de tener la clave privada.
        }
      }
    }
  }

  rep.g_guess = best_g;
  rep.combos_tested = tested;
  // Si estás ejecutando benchmarks donde sí conoces pk.g (modo validación), puedes:
  rep.found_exact = (best_g == pk.g) || (best_g % pk.g == BigInt(0));
  return rep;
}


} // namespace gln
