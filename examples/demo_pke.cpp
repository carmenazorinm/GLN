#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

#include "gln/bigint.hpp"
#include "gln/random.hpp"
#include "gln/params.hpp"
#include "gln/keygen.hpp"
#include "gln/encrypt.hpp"
#include "gln/decrypt.hpp"

using gln::BigInt;

static std::size_t ceil_log2(std::size_t x){
    if (x <= 1) return 0;
    std::size_t r=0, v=x-1; while(v){ v>>=1; ++r; } return r;
}

// Mensaje aleatorio válido: peso t_w, símbolos en [1, z-1]
static std::vector<uint32_t> random_plaintext(gln::Random& rng, const gln::Params& prm) {
    const std::size_t n = prm.n, t = prm.t_w, z = prm.z;
    std::vector<uint32_t> e(n, 0u);

    // elige t posiciones distintas
    std::vector<std::size_t> idx(n);
    for (std::size_t i=0;i<n;++i) idx[i]=i;
    auto seed = rng.uniform_uint64(0, std::numeric_limits<uint64_t>::max());
    std::mt19937_64 gen(seed);
    std::shuffle(idx.begin(), idx.end(), gen);

    for (std::size_t j=0;j<t;++j) {
        std::size_t k = idx[j];
        uint64_t val = 1 + (rng.uniform_uint64(0, std::numeric_limits<uint64_t>::max()) % (z-1));
        e[k] = static_cast<uint32_t>(val);
    }
    return e;
}

int main() {
    // Parámetros “sanos” para que KeyGen sea rápido (muchos primos disponibles)
    // Cambia si quieres otro tamaño:
    auto prm = gln::choose_params(/*n=*/32, /*t=*/6, /*z=*/256, /*beta=*/3);

    gln::Random rng(42); // semilla fija para reproducibilidad

    // 1) KeyGen
    auto kp = gln::keygen(prm, rng);
    std::cout << "KeyGen listo. n=" << prm.n
              << " t=" << prm.t_w
              << " z=" << prm.z
              << " |g|≈" << prm.bits_g << " bits\n";

    // 2) Mensaje e válido
    auto e = random_plaintext(rng, prm);
    std::cout << "e = [";
    for (std::size_t i=0;i<e.size();++i) { if(i) std::cout<<","; std::cout<<e[i]; }
    std::cout << "]\n";

    // 3) Encrypt
    auto ct = gln::encrypt_checked(e, kp.pk, prm);
    std::cout << "Ciphertext:\n  c1 = " << ct.c1 << "\n  c2 = " << ct.c2 << "\n";

    // 4) Decrypt
    auto d = gln::decrypt(ct, kp.sk, prm);
    std::cout << "Dec( ct ) = [";
    for (std::size_t i=0;i<d.size();++i) { if(i) std::cout<<","; std::cout<<d[i]; }
    std::cout << "]\n";

    // 5) Verificación
    if (d == e) {
        std::cout << "[OK] decrypt(encrypt(e)) == e ✅\n";
        return 0;
    } else {
        std::cerr << "[ERROR] decrypt(encrypt(e)) != e ❌\n";
        return 1;
    }
}
