#pragma once
#include <vector>
#include <tuple>
#include <cstddef>
#include "gln/bigint.hpp"
#include "gln/keygen.hpp"
#include "gln/params.hpp"

namespace gln {

// Resultado genérico de un ataque
struct AttackReport {
  BigInt g_guess;          // mejor candidato a g
  bool   found_exact = false; // true si coincide con pk.g 
  std::size_t combos_tested = 0; // nº de combinaciones probadas
};

// (A) Ataque por parejas (si se publicaran h_i). Útil como prueba/demostración.
AttackReport attack_pairs_demo(const std::vector<BigInt>& h,
                               const std::vector<BigInt>& p_true_optional = {});

// (B) Ataque por tríos usando t_i:
// - pk.t: vector público de t_i
// - indices: el trío de posiciones (i,j,k) sobre el que atacar (p.ej. {0,1,2})
// - b, beta: definen el rango de primos candidatos [2^b, 2^(b+beta))
// - max_candidates: limita cuántos primos probar (para tests/benchmarks)
// Devuelve el mejor g_guess observado (gcd más grande encontrado).
AttackReport attack_triples(const PublicKey& pk,
                            std::tuple<std::size_t,std::size_t,std::size_t> indices,
                            std::size_t b,
                            std::size_t beta,
                            std::size_t max_candidates = 10000);

// Helper para generar candidatos de primos (simple). Sustituible por tus nt_utils.
std::vector<BigInt> generate_prime_candidates(std::size_t b,
                                              std::size_t beta,
                                              std::size_t max_candidates);

} // namespace gln
