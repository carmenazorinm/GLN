#include "gln/decrypt.hpp"
#include "gln/bigint.hpp"  // to_u32
#include "gln/nt_utils.hpp"    // mod, gcd, invmod, treea

#include <stdexcept>
#include <string>

namespace gln {

// namespace {
// // Conversión BigInt -> uint32 (vía decimal). Asume que cabe en 32 bits.
// static uint32_t to_u32(const BigInt& x) {
//     const std::string s = x.str();
//     // std::stoul maneja signos; aquí x es no negativo en uso normal
//     return static_cast<uint32_t>(std::stoul(s));
// }
// } // namespace

std::vector<uint32_t> decrypt(const Ciphertext& ct,
                              const PrivateKey& sk,
                              const Params& prm)
{
    const std::size_t n = sk.p.size();
    if (n == 0) throw std::invalid_argument("decrypt: empty key");
    if (prm.t_w == 0) throw std::invalid_argument("decrypt: t_w must be > 0");

    // 1) s = (c1 + u*c2) mod g
    BigInt s = mod(ct.c1 + (sk.u * ct.c2), sk.g);  // s in [0, g-1]

    // 2) (v, r) = TrEEA(g, s)  -> (aprox) (lambda, omega)
    TreeaOut out = treea(sk.g, s);
    BigInt lambda = out.v;   // si gcd(lambda,omega)=1, ya es la lambda real
    BigInt omega  = out.r;

    // 3) Encontrar posiciones no nulas: gcd(p_i, lambda) != 1
    std::vector<std::size_t> supp; supp.reserve(prm.t_w);
    for (std::size_t i = 0; i < n; ++i) {
        if (gcd(sk.p[i], lambda) != BigInt(1)) {
            supp.push_back(i);
        }
    }

    // 4) Ajuste general (caso compuesto): lambda <- prod p_ki ; omega <- omega * gamma
    //    (si ya estaba bien, gamma=1)
    BigInt lambda_exact(1);
    for (std::size_t idx : supp) lambda_exact *= sk.p[idx];

    if (!(lambda_exact == lambda)) {
        // gamma = lambda_exact / lambda  (debería dividir exactamente)
        BigInt gamma = lambda_exact / lambda;
        lambda = lambda_exact;
        omega  = omega * gamma;
    }

    // 5) Resolver (lambda/p_k) * e_k ≡ omega (mod p_k) para cada k en soporte
    std::vector<uint32_t> e(n, 0u);
    for (std::size_t idx : supp) {
        const BigInt& pk = sk.p[idx];
        BigInt A = lambda / pk;                       // lambda / p_k
        BigInt A_inv = invmod(mod(A, pk), pk);        // (A mod pk)^{-1} mod pk
        BigInt e_k = mod(omega * A_inv, pk);          // e_k en [0, pk-1]
        // opcional: validar rango < z
        // if (e_k >= BigInt(static_cast<long long>(prm.z))) {
        //     // Con parámetros correctos, no debería pasar; marcamos error claro
        //     throw std::runtime_error("decrypt: recovered symbol >= z");
        // }
        e[idx] = to_u32(e_k);
    }

    // (opcional) Validar peso
    // std::size_t weight = 0;
    // for (uint32_t x : e) if (x != 0) ++weight;
    // if (weight != prm.t_w) {
    //     // No lanzar necesariamente; pero ayuda a detectar inconsistencias en tests
    //     // throw std::runtime_error("decrypt: wrong weight in recovered plaintext");
    // }

    return e;
}

} // namespace gln
