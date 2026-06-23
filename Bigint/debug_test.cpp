#include "bigint.h"
#include <iostream>
#include <sstream>

int main() {
    // Test 1: borrow chain
    BigInt a("1000000000000000000");
    BigInt b(1);
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';
    std::cout << "a - b = " << (a - b) << '\n';
    std::cout << "expected: 999999999999999999\n\n";

    // Test 2: mul large
    BigInt x("12345678901234567890");
    BigInt y("98765432109876543210");
    BigInt prod = x * y;
    std::cout << "x = " << x << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "prod = " << prod << '\n';
    std::cout << "prod / x = " << (prod / x) << '\n';
    std::cout << "y = " << y << '\n';
    std::cout << "equal? " << (prod / x == y) << '\n';
}
