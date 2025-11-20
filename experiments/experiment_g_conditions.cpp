#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <tuple>
#include <sstream>
#include <random>
#include <cmath>

#include "gln/bigint.hpp"
#include "gln/random.hpp"
#include "gln/params.hpp"
#include "gln/keygen.hpp"
#include "gln/encrypt.hpp"
#include "gln/decrypt.hpp"
#include "gln/attack.hpp"

using gln::BigInt;

static std::size_t ceil_log2(std::size_t x){
    if (x <= 1) return 0;
    std::size_t r=0, v=x-1; while(v){ v>>=1; ++r; } return r;
}

static std::size_t bitlen(const BigInt& x) {
  return gln::to_string(x).length();
}

static std::size_t bitlen(std::vector<BigInt> x) {
    std::size_t size = 0;
    for (auto x_i: x) {
        size += bitlen(x_i);
    }
  return size;
}

static float density(std::size_t n, std::size_t bits_g) {
    return n*1.0/bits_g;
}


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
    "Options:\n"
    "  --n <int>         length n (default 32)\n"
    "  --t <int>         weight t (default 6)\n"
    "  --z <int>         alphabet size z (default 256)\n"
    "  --beta <int>      beta parameter for primes (default 3)\n"
    "  --seed <int>    RNG seed (default 42)\n"
    "\nAttack (triples) options:\n"
    "  --attack            run attack_triples from gln/attacks.hpp\n"
    "\nOutput options:\n"
    "  --csv             print a machine-friendly CSV result line (one-liner)\n"
    "  --quiet           reduce human-readable output (still prints CSV if --csv)\n"
    "  --help            show this help\n\n"
    "Example:\n"
    "  " << prog << " --n 100 --t 10 --z 1024 --beta 3 --seed 12345 --csv\n";
}


int main(int argc, char** argv) {
    std::size_t n = 32;
    std::size_t t = 6;
    std::size_t z = 256;
    std::size_t beta = 3;
    uint64_t seed = 42;
    bool want_csv = false;
    bool quiet = false;
    bool want_json = false;
    bool do_attack = false;

    // parseo sencillo de args
    for (int i=1;i<argc;++i) {
        std::string a(argv[i]);
        if (a=="--n" && i+1<argc) { n = std::stoul(argv[++i]); }
        else if (a=="--t" && i+1<argc) { t = std::stoul(argv[++i]); }
        else if (a=="--z" && i+1<argc) { z = std::stoul(argv[++i]); }
        else if (a=="--beta" && i+1<argc) { beta = std::stoul(argv[++i]); }
        else if (a=="--seed" && i+1<argc) { seed = std::stoull(argv[++i]); }
        else if (a=="--attack") { do_attack = true; }
        else if (a=="--csv") { want_csv = true; }
        else if (a=="--json") { want_json = true; }
        else if (a=="--quiet") { quiet = true; }
        else if (a=="--help") { print_usage(argv[0]); return 0; }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    gln::Params prm = gln::choose_params(/*n=*/n, /*t=*/t, /*z=*/z, /*beta=*/beta);
    gln::Random rng(seed);

    if (!quiet) {
        std::cout << "Parameters: n=" << prm.n
                  << " t=" << prm.t_w
                  << " z=" << prm.z
                  << " beta=" << beta
                  << " |g|≈" << prm.bits_g << " bits"
                  << " seed=" << seed << "\n";
    }

    // ========== Tiempo de keygen ==========
    using clk = std::chrono::high_resolution_clock;
    auto t0 = clk::now();
    auto kp = gln::keygen(prm, rng);
    auto t1 = clk::now();
    std::chrono::duration<double> dt_keygen = t1 - t0;

    float densidad = density(prm.n, prm.bits_g);

    if (!quiet) {
        std::cout << "KeyGen completed in " << std::fixed << std::setprecision(6)
                  << dt_keygen.count() << " s\n";
    }

    // ========== Generar random plaintext ==========
    auto e = random_plaintext(rng, prm);

    if (!quiet) {
        std::cout << "Plaintext (sparse) generated (showing non-zero entries): [";
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

    // ========== Tiempo de encriptación ==========
    auto t2 = clk::now();
    auto ct = gln::encrypt_checked(e, kp.pk, prm);
    auto t3 = clk::now();
    std::chrono::duration<double> dt_encrypt = t3 - t2;

    if (!quiet) {
        std::cout << "Encryption completed in " << std::fixed << std::setprecision(6)
                  << dt_encrypt.count() << " s\n";
        std::cout << "Ciphertext: c1 = " << ct.c1 << "  c2 = " << ct.c2 << "\n";
    }

    // ========== Tiempo de desencriptación ==========
    auto t4 = clk::now();
    auto d = gln::decrypt(ct, kp.sk, prm);
    auto t5 = clk::now();
    std::chrono::duration<double> dt_decrypt = t5 - t4;

    if (!quiet) {
        std::cout << "Decryption completed in " << std::fixed << std::setprecision(6)
                  << dt_decrypt.count() << " s\n";
        std::cout << "Recovered plaintext non-zero entries: [";
        bool first=true;
        for (std::size_t i=0;i<d.size();++i){
            if (d[i]!=0){
                if (!first) std::cout << ", ";
                std::cout << i << ":" << d[i];
                first=false;
            }
        }
        std::cout << "]\n";
    }

    // ========== Verificar ==========
    bool ok = (d == e);

    if (!quiet) {
        if (ok) std::cout << "[OK] decrypt(encrypt(e)) == e \n";
        else   std::cout << "[ERROR] decrypt(encrypt(e)) != e \n";
    }

    // ========== Tiempo de ataque ==========
    double dt_attack_total = 0.0;
    bool attack_success = false;
    std::size_t attack_total_combos = 0;

    if (do_attack ) {
        auto ta0 = clk::now();
        gln::AttackReport repA = gln::attack_triples(kp.pk, prm.b, prm.beta);
        auto ta1 = clk::now();
        std::chrono::duration<double> dt_attack = ta1 - ta0;

        dt_attack_total += dt_attack.count();
        attack_total_combos += repA.combos_tested;

        // evaluar éxito
        attack_success = attack_success || repA.found_exact;

        if (!quiet) {
            std::cout << "Attack completed in " << std::fixed << std::setprecision(6)
                  << dt_attack.count() << " s\n";
            std::cout << "Expected g = " << kp.pk.g.str() << "\n";
            std::cout << "Guessed g = " << repA.g_guess << "\n";
            if(attack_success && repA.g_guess != kp.pk.g) {
                std::cout << "g_guessed/g_expected " << repA.g_guess / kp.pk.g << "\n";
            }
        }
    }

    

    // ========== Print output ==========
    if (want_csv) {
        std::cout << "CSV,"
                  << prm.n << "," << prm.t_w << "," << prm.z << "," << beta << "," << seed << "," << bitlen(kp.pk.g) << "," << bitlen(ct.c1) << "," << bitlen(ct.c2) << "," << bitlen(kp.pk.t) << ","
                  << std::fixed << std::setprecision(6)
                  << dt_keygen.count() << "," << dt_encrypt.count() << "," << dt_decrypt.count() << ","
                  << (ok ? "1" : "0")
                  << "\n";
    } else if(want_json) {
        std::cout << "{" << "\"h\": [" << kp.pk.t[0].str();
        for(int pos = 1; pos < kp.pk.t.size(); pos++) {
            std::cout << "," << kp.pk.t[pos].str();
        }
        std::cout << "], \"ciphertext\": { \"c1\": \"" << ct.c1.str() << "\", \"c2\": " << ct.c2.str() << "},";
        std::cout << "\"plaintext\": [" << e[0];
        for(int i = 1; i < e.size(); i++){
            std::cout << "," << e[i];
        }
        std::cout << "],";
        std::cout << "\"d\": " << densidad;
        std::cout << "}";
    }
    else {
        float primos = pow(2,(prm.b+prm.beta))/((prm.b+prm.beta)*log(2))-pow(2,prm.b)/(prm.b*log(2));
        std::cout << "SUMMARY: "
          << "n=" << prm.n << " t=" << prm.t_w << " z=" << prm.z
          << " g_bits=" << bitlen(kp.pk.g)
          << " c1_bits=" << bitlen(ct.c1)
          << " c2_bits=" << bitlen(ct.c2)
          << " pubkey_bits=" << bitlen(kp.pk.t)
          << " keygen_s=" << std::fixed << std::setprecision(6) << dt_keygen.count()
          << " enc_s=" << dt_encrypt.count()
          << " dec_s=" << dt_decrypt.count()
          << " ok=" << (ok? "1":"0")
          << " n_primes=" << primos
          << " density=" << densidad << "\n";
        if (do_attack) {
            std::cout << " | attack_s=" << dt_attack_total
                      << " found=" << (attack_success? "1":"0")
                      << " combos=" << attack_total_combos;
        }
    }

    return ok ? 0 : 2;
}

