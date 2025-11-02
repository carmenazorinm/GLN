// tests/test_kem.cpp
#include <cassert>
#include <iostream>
#include "gln/kem.hpp"
#include "gln/params.hpp"
#include "gln/random.hpp"
#include "gln/keygen.hpp"

static std::size_t ceil_log2(std::size_t x){ if(x<=1) return 0; std::size_t r=0,v=x-1; while(v){v>>=1;++r;} return r; }
static gln::Params make_params(std::size_t n,std::size_t t,std::size_t z,std::size_t beta=2){
  gln::Params p{}; p.n=n;p.t_w=t;p.z=z;p.beta=beta; p.b=ceil_log2(z); p.ell=ceil_log2(t?t:1);
  std::size_t b=p.b, ell=p.ell; std::size_t bits1=2*t*(b+beta)+3, bits2=2*(b+ell+(t-1)*(b+beta))+3;
  p.bits_g = bits1>bits2?bits1:bits2; return p;
}

int main(){
  gln::Random rng(42);
  auto prm = make_params(32, 6, 256, 3);
  auto kp  = gln::keygen(prm, rng);

  auto enc = gln::kem_encap(kp.pk, prm, rng);
  auto key2= gln::kem_decap(enc.ct, kp.sk, kp.pk, prm);

  assert(enc.key == key2);
  std::cout << "[OK] KEM encap/decap matched\n";
  return 0;
}
