#include "gln/decrypt.hpp"
#include "gln/bigint.hpp"  
#include "gln/nt_utils.hpp"  

#include <stdexcept>
#include <string>

namespace gln {

std::vector<uint32_t> decrypt(const Ciphertext& ct,
                              const PrivateKey& sk,
                              const Params& prm)
{
    const std::size_t n = sk.p.size();
    if (n == 0) throw std::invalid_argument("decrypt: empty key");
    if (prm.t_w == 0) throw std::invalid_argument("decrypt: t_w must be > 0");

    // s = (c1 + u*c2) mod g
    BigInt s = mod(ct.c1 + (sk.u * ct.c2), sk.g);  // s in [0, g-1]

    // (v, r) = TrEEA(g, s)  
    TreeaOut out = treea(sk.g, s);
    BigInt lambda = out.v;  
    BigInt omega  = out.r;

    // gcd(p_i, lambda) != 1
    std::vector<std::size_t> supp; supp.reserve(prm.t_w);
    for (std::size_t i = 0; i < n; ++i) {
        if (gcd(sk.p[i], lambda) != BigInt(1)) {
            supp.push_back(i);
        }
    }

    // lambda <- prod p_ki ; omega <- omega * gamma
    BigInt lambda_exact(1);
    for (std::size_t idx : supp) lambda_exact *= sk.p[idx];

    if (!(lambda_exact == lambda)) {
        // gamma = lambda_exact / lambda 
        BigInt gamma = lambda_exact / lambda;
        lambda = lambda_exact;
        omega  = omega * gamma;
    }

    // resolver (lambda/p_k) * e_k ≡ omega (mod p_k) para cada k en soporte
    std::vector<uint32_t> e(n, 0u);
    for (std::size_t idx : supp) {
        const BigInt& pk = sk.p[idx];
        BigInt A = lambda / pk;                       // lambda / p_k
        BigInt A_inv = invmod(mod(A, pk), pk);        // (A mod pk)^{-1} mod pk
        BigInt e_k = mod(omega * A_inv, pk);          // e_k en [0, pk-1]
        e[idx] = to_u32(e_k);
    }
    return e;
}

} // namespace gln
