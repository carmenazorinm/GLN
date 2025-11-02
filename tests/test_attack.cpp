#include <iostream>
#include "gln/keygen.hpp"
#include "gln/attack.hpp"
#include "gln/params.hpp"

using namespace gln;

int main(int argc, char** argv){
  Params prm = choose_params(/*n*/10, /*t_w*/2, /*z*/(16), /*beta*/3);
  Random rng; 
  KeyPair kp = keygen(prm, rng);

  // Ataca sobre los 3 primeros índices (i=0,j=1,k=2), y prueba 2000 candidatos
  AttackReport rep = attack_triples(kp.pk, {0,1,2}, prm.b, prm.beta, /*max_candidates*/2000);

  std::cout << "Combos probados: " << rep.combos_tested << "\n";
  std::cout << "g_guess: " << rep.g_guess << "\n";
  std::cout << "g real: " << kp.pk.g << "\n";
  std::cout << "found_exact? " << ((rep.g_guess == kp.pk.g || (rep.g_guess % kp.pk.g)==BigInt(0)) ? "YES":"NO") << "\n";
  return 0;

}
