#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>

#include "gln/random.hpp"
#include "gln/params.hpp"
#include "gln/keygen.hpp"
#include "gln/kem.hpp"

static std::size_t ceil_log2(std::size_t x){
    if (x <= 1) return 0;
    std::size_t r=0, v=x-1; while(v){ v>>=1; ++r; } return r;
}

static void print_key(const gln::SKey& K, const char* label){
    std::cout << label << " ";
    for (auto byte : K){
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    std::cout << std::dec << "\n";
}

int main(){
    // Parámetros “sanos”
    auto prm = gln::choose_params(/*n=*/32, /*t=*/6, /*z=*/256, /*beta=*/3);

    gln::Random rng(42);

    // 1) KeyGen
    auto kp = gln::keygen(prm, rng);

    // 2) Encap
    auto enc = gln::kem_encap(kp.pk, prm, rng);
    std::cout << "Ciphertext:\n  c1 = " << enc.ct.ct.c1 << "\n  c2 = " << enc.ct.ct.c2 << "\n";
    print_key(enc.key, "K_encap =");

    // 3) Decap correcto
    auto K2 = gln::kem_decap(enc.ct, kp.sk, kp.pk, prm);
    print_key(K2, "K_decap =");
    if (K2 == enc.key) std::cout << "[OK] KEM encap/decap coinciden ✅\n";
    else               std::cout << "[ERROR] KEM encap/decap NO coinciden ❌\n";

    // 4) Comprobación de fallback (CCA): corrompemos ct y decap no debería igualar
    gln::KemCiphertext tampered = enc.ct;
    tampered.ct.c1 = tampered.ct.c1 + gln::BigInt(1); // cambia el ciphertext

    auto K_bad = gln::kem_decap(tampered, kp.sk, kp.pk, prm);
    print_key(K_bad, "K_decap(tampered) =");
    if (K_bad != enc.key) std::cout << "[OK] Fallback distinto cuando el CT se corrompe ✅\n";
    else                  std::cout << "[WARN] La clave no cambió con CT corrupto ⚠️\n";

    return 0;
}
