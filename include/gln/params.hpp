#pragma once
#include <cstddef>

namespace gln {
struct Params {
  std::size_t n, t_w, z;
  std::size_t b, ell, beta;   // b=ceil(log2 z), ell=ceil(log2 t)
  std::size_t bits_g;
};
Params choose_params(std::size_t n, std::size_t t_w, std::size_t z, std::size_t beta);
} // namespace gln
