// tests/test_invmod.cpp
#include "gln/nt_utils.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_invmod_positive_cases() {
    std::cout << "Testing positive cases...\n";
    
    // Caso básico
    {
        gln::BigInt m("101");
        auto inv = gln::invmod(gln::BigInt(3), m);
        assert(gln::to_string(inv) == "34"); // 3*34 = 102 ≡ 1 mod 101
        std::cout << "Test 1 passed: 3⁻¹ mod 101 = 34\n";
    }

    // Números más grandes
    {
        gln::BigInt m("1000000007"); // Un primo grande
        auto inv = gln::invmod(gln::BigInt(123456), m);
        assert(gln::to_string(inv) == "78351802"); // 123456 * 78351802 ≡ 1 mod 1000000007
        std::cout << "Test 2 passed: large prime modulus\n";
    }

    // Caso con m = 1 (cualquier número es congruente con 0 mod 1)
    {
        auto inv = gln::invmod(gln::BigInt(5), gln::BigInt(1));
        assert(gln::to_string(inv) == "0");
        std::cout << "Test 3 passed: modulus 1 case\n";
    }

    // Caso con a y m coprimos pero a > m
    {
        gln::BigInt m("17");
        try {
            auto inv = gln::invmod(gln::BigInt(34), m);
            std::cerr << "Error: Test 4 failed - expected exception not thrown\n";
            assert(false); // No debería llegar aquí
        } catch (const std::exception& e) {
            std::cout << "Test 4 passed: correctly caught non-coprime case\n";
        }
    }
}

void test_invmod_negative_cases() {
    std::cout << "\nTesting negative cases (should throw exceptions)...\n";
    
    // a y m no coprimos
    try {
        auto inv = gln::invmod(gln::BigInt(2), gln::BigInt(4));
        std::cerr << "Error: Test 5 failed - expected exception not thrown\n";
        assert(false); // No debería llegar aquí
    } catch (const std::exception& e) {
        std::cout << "Test 5 passed: correctly caught non-coprime case\n";
    }

    // m = 0
    // try {
    //     auto inv = gln::invmod(gln::BigInt(1), gln::BigInt(0));
    //     std::cerr << "Error: Test 6 failed - expected exception not thrown\n";
    //     assert(false);
    // } catch (const std::exception& e) {
    //     std::cout << "Test 6 passed: correctly caught modulus 0 case\n";
    // }

    // m negativo
    try {
        auto inv = gln::invmod(gln::BigInt(3), gln::BigInt(-101));
        std::cerr << "Error: Test 7 failed - expected exception not thrown\n";
        assert(false);
    } catch (const std::exception& e) {
        std::cout << "Test 7 passed: correctly caught negative modulus case\n";
    }
}

void test_invmod_edge_cases() {
    std::cout << "\nTesting edge cases...\n";
    
    // a = 1 (inverso es siempre 1)
    {
        gln::BigInt m("123456789");
        auto inv = gln::invmod(gln::BigInt(1), m);
        assert(gln::to_string(inv) == "1");
        std::cout << "Test 8 passed: a = 1 case\n";
    }

    // a = m - 1 (inverso es él mismo)
    {
        gln::BigInt m("101");
        auto inv = gln::invmod(gln::BigInt(100), m);
        assert(gln::to_string(inv) == "100"); // 100 * 100 = 10000 ≡ 1 mod 101
        std::cout << "Test 9 passed: a = m-1 case\n";
    }

    // a = 0
    try {
        auto inv = gln::invmod(gln::BigInt(0), gln::BigInt(101));
        std::cerr << "Error: Test 10 failed - expected exception not thrown\n";
        assert(false);
    } catch (const std::exception& e) {
        std::cout << "Test 10 passed: correctly caught a = 0 case\n";
    }
}

int main() {
    try {
        test_invmod_positive_cases();
        test_invmod_negative_cases();
        test_invmod_edge_cases();
        
        std::cout << "\nAll tests completed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
