// tests/test_treea.cpp
#include "gln/nt_utils.hpp"
#include <cassert>
#include <iostream>

void test_treea(const gln::BigInt& a, const gln::BigInt& b, 
                const gln::BigInt& expected_v, const gln::BigInt& expected_r) {
    std::cout << "Testing TrEEA with a = " << a << ", b = " << b << "\n";
    
    auto out = gln::treea(a, b);
    
    std::cout << "Result: v = " << out.v << ", r = " << out.r << "\n";
    std::cout << "Expected: v = " << expected_v << ", r = " << expected_r << "\n";
    
    assert(out.v == expected_v);
    assert(out.r == expected_r);
    
    // Comprobación adicional: |v|, |r| < sqrt(a)
    gln::BigInt sqrt_a = gln::BigInt::sqrt(a);
    assert(gln::BigInt::abs(out.v) < sqrt_a);
    assert(gln::BigInt::abs(out.r) < sqrt_a);
    
    std::cout << "Test passed!\n\n";
}

int main() {    
    // Caso del ejemplo que proporcionaste
    test_treea(gln::BigInt("55411224877"), gln::BigInt("4693214982"),
               gln::BigInt("1771"), gln::BigInt("1572"));

    test_treea(gln::BigInt("417331"), gln::BigInt("105480"),
               gln::BigInt("91"), gln::BigInt("67"));
    
    return 0;
}