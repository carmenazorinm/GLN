#include "gln/bigint.hpp"
#include <iostream>
#include <cassert>

int main(){
    using gln::BigInt;
    BigInt a("123456789012345678901234567890");
    BigInt b("98765432109876543210987654321");
    BigInt c = a & b;
    std::cout << "a & b = " << c.str() << std::endl;
    // Puedes calcular el resultado con boost interactivo o una calculadora big-int para compararlo.
    return 0;
}
