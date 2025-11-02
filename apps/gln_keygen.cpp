#include "gln/bigint.hpp"
#include <iostream>

int main() {
  gln::BigInt g("123456789012345678901234567890");
  std::cout << "GLN keygen stub. g = " << gln::to_string(g) << "\n";
  return 0;
}
