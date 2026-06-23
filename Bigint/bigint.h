#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <sstream>

class BigInt {
public:
    BigInt() : negative_(false), digit_() {}
    BigInt(long long value);
    BigInt(const std::string &str);

    bool isZero() const; 

    void trim();         // 去除前导零

    static int absCompare(const BigInt &a, const BigInt &b);
    static BigInt absAdd(const BigInt &a, const BigInt &b);
    static BigInt absSub(const BigInt &a, const BigInt &b);
    bool   operator==(const BigInt& other) const;
    bool   operator!=(const BigInt& other) const;
    bool   operator<(const BigInt& other) const;
    bool   operator>(const BigInt& other) const;
    bool   operator<=(const BigInt& other) const;
    bool   operator>=(const BigInt& other) const;
    BigInt operator+(const BigInt &other) const;
    BigInt operator-(const BigInt &other) const;
    BigInt operator*(const BigInt &other) const;
    static BigInt absMulUint(const BigInt& a, uint32_t m);
    BigInt operator/(const BigInt &other) const;
    BigInt operator%(const BigInt &other) const;
    BigInt operator+=(const BigInt &other);
    BigInt operator-=(const BigInt &other);
    BigInt operator*=(const BigInt &other);
    BigInt operator/=(const BigInt &other);
    BigInt operator%=(const BigInt &other);
    friend std::ostream& operator<<(std::ostream& os, const BigInt& value);
    friend std::istream& operator>>(std::istream& is, BigInt& other);
    BigInt operator-() const;
    BigInt abs() const;
    std::string toString() const;
    long long toLongLong() const;
    friend BigInt gcd(BigInt a, BigInt b);
    friend BigInt lcm(BigInt a, BigInt b);
private:
    static constexpr uint32_t base = 1000000000;
    bool negative_;
    std::vector<uint32_t> digit_;
};
