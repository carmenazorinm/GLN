#pragma once
#include <vector>
#include <tuple>
#include <cstddef>
#include "gln/bigint.hpp"
#include "gln/keygen.hpp"
#include "gln/params.hpp"

namespace gln {

struct AttackReport {
  BigInt g_guess;          // mejor candidato a g
  bool   found_exact = false; // true si coincide con pk.g 
  std::size_t combos_tested = 0; // n de combinaciones probadas
};

AttackReport attack_triples(const PublicKey& pk,
                            std::tuple<std::size_t,std::size_t,std::size_t> indices,
                            std::size_t b,
                            std::size_t beta,
                            std::size_t max_candidates = 10000);

AttackReport attack_triples(const PublicKey& pk, std::size_t b, std::size_t beta);

std::vector<BigInt> generate_prime_candidates(std::size_t b,
                                              std::size_t beta,
                                              std::size_t max_candidates);

} // namespace gln
