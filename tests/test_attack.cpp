#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "gln/keygen.hpp"
#include "gln/attack.hpp"
#include "gln/params.hpp"

using namespace gln;

static void test_basic() {
  Params prm = choose_params(/*n*/5, /*t_w*/3, /*z*/2, /*beta*/4);
  Random rng;
  KeyPair kp = keygen(prm, rng);

  AttackReport rep = attack_triples(kp.pk, prm.b, prm.beta);
  assert(rep.found_exact = true);
}

int main(){
  test_basic();
  return 0;
}
