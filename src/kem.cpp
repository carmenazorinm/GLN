#include "gln/kem.hpp"

#include <algorithm>
#include <limits>
#include <string>

namespace gln {

static std::size_t ceil_log2(std::size_t x) {
    if (x <= 1) return 0;
    std::size_t r = 0, v = x - 1;
    while (v) { v >>= 1; ++r; }
    return r;
}

static void u32_to_be(uint32_t v, std::vector<uint8_t>& out) {
    out.push_back(uint8_t((v >> 24) & 0xFF));
    out.push_back(uint8_t((v >> 16) & 0xFF));
    out.push_back(uint8_t((v >> 8) & 0xFF));
    out.push_back(uint8_t((v >> 0) & 0xFF));
}

static void u16_to_be(uint16_t v, std::vector<uint8_t>& out) {
    out.push_back(uint8_t((v >> 8) & 0xFF));
    out.push_back(uint8_t((v >> 0) & 0xFF));
}

static void write_symbol_be(uint32_t ei, std::size_t b, std::vector<uint8_t>& out) {
    for (std::size_t j = 0; j < b; ++j) {
        std::size_t shift = (b - 1 - j) * 8;
        out.push_back(uint8_t((ei >> shift) & 0xFF));
    }
}

static std::vector<uint8_t> encode_e_bytes(const std::vector<uint32_t>& e, const Params& prm) {
    const std::size_t n = e.size();
    const std::size_t b = (prm.b + 7) / 8; 

    std::vector<uint8_t> out;
    out.reserve(8 + 2 + n * b);

    u32_to_be(static_cast<uint32_t>(n), out);
    u32_to_be(static_cast<uint32_t>(prm.t_w), out);
    u16_to_be(static_cast<uint16_t>(b), out);

    for (uint32_t ei : e) write_symbol_be(ei, b, out);
    return out;
}

static std::vector<uint8_t> encode_ct_bytes(const Ciphertext& ct) {
    std::string s = ct.c1.str();
    s.push_back('|');
    std::string s2 = ct.c2.str();
    s += s2;
    return std::vector<uint8_t>(s.begin(), s.end());
}

static std::vector<uint8_t> encode_sk_secret_bytes(const PrivateKey& sk) {
    std::string s = sk.g.str();
    s.push_back('|');
    s += sk.u.str();
    s.push_back('|');
    for (std::size_t i = 0; i < sk.p.size(); ++i) {
        if (i) s.push_back(',');
        s += sk.p[i].str();
    }
    return std::vector<uint8_t>(s.begin(), s.end());
}

SKey default_kdf(uint8_t domain_tag, const std::vector<uint8_t>& payload) {
    std::hash<std::string> H;
    std::string seed;
    seed.push_back(char(domain_tag));
    seed.append(reinterpret_cast<const char*>(payload.data()), payload.size());

    SKey out{};
    uint64_t acc = H(seed);
    for (size_t i = 0; i < out.size(); ++i) {
        acc = H(std::string(reinterpret_cast<const char*>(&acc), sizeof(acc)));
        out[i] = static_cast<uint8_t>(acc & 0xFFu);
        acc >>= 1;
        if (acc == 0) acc = 0x9e3779b97f4a7c15ull ^ H(seed) ^ (i * 0x10001u);
    }
    return out;
}

static std::vector<uint32_t> random_plaintext(Random& rng,
                                              const Params& prm)
{
    const std::size_t n = prm.n, t = prm.t_w, z = prm.z;
    std::vector<uint32_t> e(n, 0u);

    std::vector<std::size_t> idx(n);
    for (std::size_t i = 0; i < n; ++i) idx[i] = i;

    auto seed = rng.uniform_uint64(0, std::numeric_limits<uint64_t>::max());
    std::mt19937_64 gen(seed);
    std::shuffle(idx.begin(), idx.end(), gen);

    for (std::size_t j = 0; j < t; ++j) {
        const std::size_t k = idx[j];
        uint64_t val = 1 + (rng.uniform_uint64(0, std::numeric_limits<uint64_t>::max()) % (z - 1)); // [1,z-1]
        e[k] = static_cast<uint32_t>(val);
    }
    return e;
}

KemEncapOut kem_encap(const PublicKey& pk,
                      const Params& prm,
                      Random& rng,
                      KdfFunc kdf)
{
    auto e = random_plaintext(rng, prm);

    Ciphertext ct = encrypt_checked(e, pk, prm);

    auto e_bytes  = encode_e_bytes(e, prm);
    auto ct_bytes = encode_ct_bytes(ct);

    std::vector<uint8_t> payload;
    payload.reserve(e_bytes.size() + 1 + ct_bytes.size());
    payload.insert(payload.end(), e_bytes.begin(), e_bytes.end());
    payload.push_back(0xFF);
    payload.insert(payload.end(), ct_bytes.begin(), ct_bytes.end());

    SKey K = kdf(/*domain_tag=*/1, payload);

    return KemEncapOut{ KemCiphertext{ct}, K };
}

SKey kem_decap(const KemCiphertext& c,
               const PrivateKey& sk,
               const PublicKey& pk,
               const Params& prm,
               KdfFunc kdf)
{
    const Ciphertext& ct = c.ct;

    std::vector<uint32_t> e = decrypt(ct, sk, prm);

    Ciphertext check = encrypt(e, pk);
    bool ok = (check.c1 == ct.c1) && (check.c2 == ct.c2);

    if (ok) {
        auto e_bytes  = encode_e_bytes(e, prm);
        auto ct_bytes = encode_ct_bytes(ct);

        std::vector<uint8_t> payload;
        payload.reserve(e_bytes.size() + 1 + ct_bytes.size());
        payload.insert(payload.end(), e_bytes.begin(), e_bytes.end());
        payload.push_back(0xFF);
        payload.insert(payload.end(), ct_bytes.begin(), ct_bytes.end());

        return kdf(/*domain_tag=*/1, payload);
    } else {
        auto sk_secret = encode_sk_secret_bytes(sk);
        auto ct_bytes  = encode_ct_bytes(ct);

        std::vector<uint8_t> payload;
        payload.reserve(sk_secret.size() + 1 + ct_bytes.size());
        payload.insert(payload.end(), sk_secret.begin(), sk_secret.end());
        payload.push_back(0xEE);
        payload.insert(payload.end(), ct_bytes.begin(), ct_bytes.end());

        return kdf(/*domain_tag=*/0, payload);
    }
}

} // namespace gln
