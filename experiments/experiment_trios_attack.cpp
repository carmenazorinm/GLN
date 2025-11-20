// benchmark_main.cpp
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <tuple>
#include <sstream>
#include <random>

// includes del proyecto GLN
#include "gln/bigint.hpp"
#include "gln/random.hpp"
#include "gln/params.hpp"
#include "gln/keygen.hpp"
#include "gln/encrypt.hpp"
#include "gln/decrypt.hpp"
#include "gln/attack.hpp"  

using gln::BigInt;

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

static void print_usage(const char* prog) {
    std::cerr <<
    "Usage: " << prog << " [options]\n\n"
    "Core options:\n"
    "  --n <int>           length n (default 32)\n"
    "  --t <int>           weight t (default 6)\n"
    "  --z <int>           alphabet size z (default 256)\n"
    "  --beta <int>        beta for params (default 3)\n"
    "  --seed <uint64>     RNG seed (default 42)\n"
    "  --csv               print one CSV line with results\n"
    "  --quiet             less verbose\n"
    "\nAttack (triples) options:\n"
    "  --attack            run attack_triples from gln/attacks.hpp\n"
    "\nExample:\n"
    "  " << prog << " --n 100 --t 10 --z 1024 --beta 3 --seed 12345 --attack --attack-tries 50 --csv\n";
}

int main(int argc, char** argv) {
    // Defaults
    std::size_t n = 10;
    std::size_t t = 2;
    std::size_t z = 16;
    std::size_t beta = 3;
    uint64_t seed = 42;
    bool want_csv = false;
    bool quiet = false;

    // Ataque (por tríos)
    bool do_attack = false;
    bool validate_g = true;

    // parse args
    for (int iarg=1;iarg<argc;++iarg) {
        std::string a(argv[iarg]);
        if (a=="--n" && iarg+1<argc) { n = std::stoul(argv[++iarg]); }
        else if (a=="--t" && iarg+1<argc) { t = std::stoul(argv[++iarg]); }
        else if (a=="--z" && iarg+1<argc) { z = std::stoul(argv[++iarg]); }
        else if (a=="--beta" && iarg+1<argc) { beta = std::stoul(argv[++iarg]); }
        else if (a=="--seed" && iarg+1<argc) { seed = std::stoull(argv[++iarg]); }
        else if (a=="--csv") { want_csv = true; }
        else if (a=="--quiet") { quiet = true; }

        else if (a=="--attack") { do_attack = true; }

        else if (a=="--help") { print_usage(argv[0]); return 0; }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // Parámetros del esquema
    auto prm = gln::choose_params(/*n=*/n, /*t=*/t, /*z=*/z, /*beta=*/beta);
    gln::Random rng(seed);

    if (!quiet) {
        std::cout << "Parameters: n=" << prm.n
                  << " t=" << prm.t_w
                  << " z=" << prm.z
                  << " beta=" << beta
                  << " |g|≈" << prm.bits_g << " bits"
                  << " seed=" << seed << "\n";
    }

    using clk = std::chrono::high_resolution_clock;

    // KeyGen
    auto t0 = clk::now();
    auto kp = gln::keygen(prm, rng);
    auto t1 = clk::now();
    std::chrono::duration<double> dt_keygen = t1 - t0;
    if (!quiet) {
        std::cout << "KeyGen completed in " << std::fixed << std::setprecision(6)
                  << dt_keygen.count() << " s\n";
    }
    // std::cout << "g of pubkey: " << kp.pk.g.str() << "g of privkey: " << kp.sk.g.str() << std::endl;

    // Plaintext aleatorio
    auto e = random_plaintext(rng, prm);
    if (!quiet) {
        std::cout << "Plaintext non-zero entries: [";
        bool first=true;
        for (std::size_t i=0;i<e.size();++i){
            if (e[i]!=0){
                if (!first) std::cout << ", ";
                std::cout << i << ":" << e[i];
                first=false;
            }
        }
        std::cout << "]\n";
    }

    // Encrypt
    auto t2 = clk::now();
    auto ct = gln::encrypt_checked(e, kp.pk, prm);
    auto t3 = clk::now();
    std::chrono::duration<double> dt_encrypt = t3 - t2;
    if (!quiet) {
        std::cout << "Encryption completed in " << std::fixed << std::setprecision(6)
                  << dt_encrypt.count() << " s\n";
        std::cout << "Ciphertext: c1 = " << ct.c1 << "  c2 = " << ct.c2 << "\n";
    }

    // Decrypt
    auto t4 = clk::now();
    auto d = gln::decrypt(ct, kp.sk, prm);
    auto t5 = clk::now();
    std::chrono::duration<double> dt_decrypt = t5 - t4;
    if (!quiet) {
        std::cout << "Decryption completed in " << std::fixed << std::setprecision(6)
                  << dt_decrypt.count() << " s\n";
    }

    bool ok = (d == e);
    if (!quiet) {
        if (ok) std::cout << "[OK] decrypt(encrypt(e)) == e ✅\n";
        else    std::cout << "[ERROR] decrypt(encrypt(e)) != e ❌\n";
    }

    double dt_attack_total = 0.0;
    bool attack_success = false;
    std::size_t attack_total_combos = 0;

    if (do_attack /*&& !attack_success*/) {
        auto ta0 = clk::now();
        gln::AttackReport repA = gln::attack_triples(kp.pk, prm.b, prm.beta);
        auto ta1 = clk::now();
        std::chrono::duration<double> dt_attack = ta1 - ta0;

        dt_attack_total += dt_attack.count();
        attack_total_combos += repA.combos_tested;

        // evaluar éxito
        attack_success = attack_success || repA.found_exact;
    }

    if (want_csv) {
        std::cout << "CSV,"
                  << prm.n << "," << prm.t_w << "," << prm.z << "," << beta << "," << seed << ","
                  << std::fixed << std::setprecision(6)
                  << dt_keygen.count() << "," << dt_encrypt.count() << "," << dt_decrypt.count() << ","
                  << (ok ? "1" : "0") << ","
                  << (do_attack ? "1" : "0") << ","
                  << std::fixed << std::setprecision(6) << (do_attack ? dt_attack_total : 0.0) << ","
                  << (attack_success ? "1" : "0") << ","
                  << attack_total_combos
                  << "\n";
    } else {
        std::cout << "SUMMARY: n=" << prm.n << " t=" << prm.t_w << " z=" << prm.z 
                  << " keygen_s=" << std::fixed << std::setprecision(6) << dt_keygen.count()
                  << " enc_s=" << dt_encrypt.count()
                  << " dec_s=" << dt_decrypt.count()
                  << " ok=" << (ok? "1":"0");
        if (do_attack) {
            std::cout << " | attack_s=" << dt_attack_total
                      << " found=" << (attack_success? "1":"0")
                      << " combos=" << attack_total_combos;
        }
        std::cout << "\n";
    }

    // código de salida: si decrypt OK pero ataque opcional fallido, devolvemos 0 igualmente.
    // Si quieres que un ataque fallido marque exit!=0, cambia aquí.
    return ok ? 0 : 2;
}
