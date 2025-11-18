// tests/test_gcd.cpp
#include "gln/nt_utils.hpp"
#include <cassert>
#include <iostream>

void test_gcd() {
    {
        gln::BigInt a("123456"), b("7890");
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt(6));
        std::cout << "Test 1 passed: gcd(123456, 7890) == 6\n";
    }

    {
        gln::BigInt a("104729"), b("104723"); 
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt(1));
        std::cout << "Test 2 passed: gcd(104729, 104723) == 1\n";
    }

    {
        gln::BigInt a("123456789012345678901234567890");
        gln::BigInt b("987654321098765432109876543210");
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt("9000000000900000000090"));
        std::cout << "Test 3 passed: gcd(large1, large2) == large1\n";
    }

    {
        gln::BigInt a("123456"), b("0");
        auto g = gln::gcd(a, b);
        assert(g == a);
        std::cout << "Test 4 passed: gcd(123456, 0) == 123456\n";
    }

    {
        gln::BigInt a("0"), b("0");
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt(0));
        std::cout << "Test 5 passed: gcd(0, 0) == 0\n";
    }

    {
        gln::BigInt a("123456789"), b("123456789");
        auto g = gln::gcd(a, b);
        assert(g == a);
        std::cout << "Test 6 passed: gcd(a, a) == a\n";
    }

    {
        gln::BigInt a("1000000000000000000001");
        gln::BigInt b("1000000000000000000000");
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt(1));
        std::cout << "Test 7 passed: gcd(consecutive_numbers) == 1\n";
    }

    {
        gln::BigInt a("36"), b("48");
        auto g = gln::gcd(a, b);
        assert(g == gln::BigInt(12));
        std::cout << "Test 8 passed: gcd(36, 48) == 12\n";
    }
}

int main() {
    try {
        test_gcd();
        std::cout << "All GCD tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << '\n';
        return 1;
    }
}
