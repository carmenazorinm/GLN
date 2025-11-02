#include "gln/params.hpp"
#include <cmath>
#include <algorithm>

namespace gln {

Params choose_params(std::size_t n, std::size_t t_w, std::size_t z, std::size_t beta) {
    Params P;
    P.n = n;
    P.t_w = t_w;
    P.z = z;
    P.beta = beta;

    // b = ceil(log2(z))
    P.b = (z <= 1) ? 1 : static_cast<std::size_t>(std::ceil(std::log2(static_cast<double>(z))));

    // ell = ceil(log2(t_w))
    P.ell = (t_w <= 1) ? 1 : static_cast<std::size_t>(std::ceil(std::log2(static_cast<double>(t_w))));

    // bits_g segun formula del paper
    std::size_t term1 = 2 * t_w * (P.b + P.beta) + 3;
    std::size_t term2 = 2 * (t_w * (P.b + P.beta) + P.ell - P.beta) + 3;
    P.bits_g = std::max(term1, term2);

    return P;
}

} // namespace gln
