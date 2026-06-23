#include "bigint.h"
bool BigInt::isZero() const{
    return digit_.empty() || (digit_.size() == 1 && digit_[0] == 0);
}

void BigInt::trim(){ //空间收缩
    while(!digit_.empty() && digit_.back() == 0)
        digit_.pop_back();
    if(digit_.empty()) 
        negative_ = false;
}

BigInt::BigInt(long long value) {
    negative_ = false;
    unsigned long long uvalue;
    if (value < 0) {
        negative_ = true;
        uvalue = 0ULL - static_cast<unsigned long long>(value);
    } else {
        uvalue = static_cast<unsigned long long>(value);
    }
    while (uvalue) {
        digit_.push_back(static_cast<uint32_t>(uvalue % base));
        uvalue /= base;
    }
    trim();
}

BigInt::BigInt(const std::string &str) {
    //字符串构造检验
    if(str.length() == 0) {
        throw std::invalid_argument("invalid BigInt literal: Empty string");
    }
    if(str[0] != '+' && str[0] != '-' && str[0] < '0' && str[0] > '9') {
        throw std::invalid_argument("invalid BigInt literal: Invalid begin");
    }
    for(int i = 0;i < str.length();i++) {
        if(str[0] < '0' || str[0] > '9') {
            throw std::invalid_argument("invalid BigInt literal: Invalid body");
        }
    }

    negative_ = false;
    int len = static_cast<int>(str.length()); //强制类型转换
    if (len == 0) return;

    int pos = 0;
    if (str[0] == '-') {
        negative_ = true;
        pos = 1;
    } else if (str[0] == '+') {
        pos = 1;
    }

    for (int i = len; i > pos; i -= 9) {
        int start = i - 9;
        if (start < pos) start = pos;
        int chunkLen = i - start; //截取长度
        uint32_t chunk = static_cast<uint32_t>(std::stoul(str.substr(start, chunkLen)));//非法输入抛异常
        digit_.push_back(chunk);
    }
    trim();
}

int BigInt::absCompare(const BigInt &a,const BigInt &b) {
    if(a.digit_.size() > b.digit_.size()) return 1;
    else if(a.digit_.size() ==  b.digit_.size()) {
        if(a.digit_.size() == 0) return 0;
        for(int i = a.digit_.size() - 1;i >= 0;i--) {
            if(a.digit_[i] > b.digit_[i]) return 1;
            else if(a.digit_[i] < b.digit_[i]) return -1;
        }
        return 0;
    }
    else return -1;
}

BigInt BigInt::absAdd(const BigInt &a,const BigInt &b) { 
    BigInt sum; //逐位相加
    int lena = (int)a.digit_.size();
    int lenb = (int)b.digit_.size();
    int Min = std::min(lena,lenb);
    int Max = std::max(lena,lenb);
    if(Max == 0) return sum;
    uint64_t Mid = 0,carry = 0,now = 0;
    for(int i = 0;i < Min;i++) //对于每一位
    {

        Mid = a.digit_[i] + b.digit_[i] + carry;
        now = Mid % base;
        sum.digit_.push_back(now);
        carry = Mid / base;

    }

    if(lena == lenb) {
        if(carry) sum.digit_.push_back(carry);
        return sum;
    }

    else {
        const BigInt &longer = (lena > lenb) ? a : b; //顶层const
        for(int i = Min;i < Max;i++)
        {
            Mid = longer.digit_[i] + carry;
            now = Mid % base;
            sum.digit_.push_back(now);
            carry = Mid / base;
        }
        if(carry) sum.digit_.push_back(carry);
        return sum;
    }
}
BigInt BigInt::absSub(const BigInt &a, const BigInt &b) {
    BigInt diff;
    int borrow = 0;

    const BigInt *Bigger = &a;
    const BigInt *Smaller = &b;

    int Min,Max,cmp = absCompare(a,b);
    if(cmp == 1) {
        Min = (int)Smaller->digit_.size();
        Max = (int)Bigger->digit_.size();
    }
    else if(cmp == 0) return diff;
    else {
        Bigger = &b;
        Smaller = &a;
        Min = (int)Smaller->digit_.size();
        Max = (int)Bigger->digit_.size();
    }

    for(int i = 0;i < Min;i++) {
        int64_t bitdiff = static_cast<int64_t>(Bigger->digit_[i]) - static_cast<int64_t>(Smaller->digit_[i]) - borrow;
        if(bitdiff < 0) {
            diff.digit_.push_back(bitdiff + base);
            borrow = 1;
        }
        else {
            diff.digit_.push_back(bitdiff);
            borrow = 0;
        }
    }
    if(Smaller->digit_.size() == Bigger->digit_.size()) {
        diff.trim();
        return diff;
    }
    else {
        for(int i = Min;i < Max;i++)
        {
            int64_t bitdiff = static_cast<int64_t>(Bigger->digit_[i]) - borrow;
            if(bitdiff < 0) {
                diff.digit_.push_back(bitdiff + base);
                borrow = 1;
            }
            else {
                diff.digit_.push_back(bitdiff);
                borrow = 0;
            }
        }
        diff.trim();
        return diff;
    }
}
bool BigInt::operator==(const BigInt &other) const {
    return(this->negative_ == other.negative_ && absCompare(*this,other) == 0);
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt& other) const {
    if (negative_ != other.negative_) {
        return negative_;
    }
    int cmp = absCompare(*this, other);
    if (negative_) {
        return cmp > 0;
    }
    return cmp < 0;
}

bool BigInt::operator>(const BigInt& other) const {
    return other < *this;
}

bool BigInt::operator<=(const BigInt& other) const {
    return !(other < *this);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

BigInt BigInt::operator+(const BigInt &other) const {
    BigInt ans;
    if(this -> negative_ == other.negative_) {
        ans = absAdd(*this,other);
        ans.negative_ = this -> negative_;
        return ans;
    }
    else { //符号不同
        if(absCompare(*this,other) >= 0) {
            ans = absSub(*this,other);
            ans.negative_ = this -> negative_;
            ans.trim();
            return ans;
        }
        else {
            ans = absSub(*this,other);
            ans.negative_ = !(this -> negative_);
            ans.trim();
            return ans;
        }
    }
}

BigInt BigInt::operator-(const BigInt &other) const {
    BigInt ans;

    if(this->negative_ != other.negative_) {
        ans = absAdd(*this,other);
        ans.trim();
        ans.negative_ = this -> negative_;
        return ans;
    }
    else {
        if(absCompare(*this,other) >= 0) {
            ans = absSub(*this,other);
            ans.negative_ = this -> negative_;
            ans.trim();
            return ans;
        }
        else {
            ans = absSub(*this,other);
            ans.negative_ = !(this -> negative_);
            ans.trim();
            return ans;
        }
    }
}
BigInt BigInt::operator*(const BigInt &other) const {
    int lena = (int)((*this).digit_.size());
    int lenb = (int)other.digit_.size();
    BigInt ans;
    ans.negative_ = this->negative_ ^ other.negative_;
    ans.digit_.resize(lena + lenb + 1);
    uint64_t cur;
    for(int i = 0;i < lena;i++)
    {
        uint64_t carry = 0;
        for(int j = 0;j < lenb;j++)
        {
            cur = ans.digit_[i + j] + (uint64_t)this->digit_[i] * other.digit_[j] + carry; //已经存出好的和后面来的
            ans.digit_[i + j] = cur % base;
            carry = cur / base;
        }
        int k = i + lenb;
        while(carry > 0)
        {
            cur = ans.digit_[k] + carry;
            ans.digit_[k] = cur % base;
            carry = cur / base;
            k++;
        }
    }

    ans.trim();

    return ans;
}
BigInt BigInt::absMulUint(const BigInt& a, uint32_t m) {
    BigInt res;
    if (a.isZero() || m == 0) return res;

    uint64_t carry = 0;

    for (uint32_t d : a.digit_) {
        uint64_t cur = (uint64_t)d * m + carry;
        res.digit_.push_back(cur % base);
        carry = cur / base;
    }

    if (carry) {
        res.digit_.push_back(carry);
    }

    return res;
}

BigInt BigInt::operator/(const BigInt &other) const {
    if (other.isZero()) {
        throw std::domain_error("division by zero");
    }

    BigInt a = *this;
    BigInt b = other;

    a.negative_ = false;
    b.negative_ = false;

    if (absCompare(a, b) < 0) {
        return BigInt(0);
    }

    BigInt ans;
    BigInt rem;

    ans.digit_.assign(a.digit_.size(), 0);

    for (int i = (int)a.digit_.size() - 1; i >= 0; --i) {
        rem.digit_.insert(rem.digit_.begin(), a.digit_[i]);
        rem.trim();

        uint32_t l = 0, r = base - 1, best = 0;

        while (l <= r) {
            uint32_t mid = l + (r - l) / 2;
            BigInt prod = absMulUint(b, mid);

            if (absCompare(prod, rem) <= 0) {
                best = mid;
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }

        ans.digit_[i] = best;
        rem = absSub(rem, absMulUint(b, best));
    }

    ans.negative_ = negative_ ^ other.negative_;
    ans.trim();

    return ans;
}
std::ostream& operator<<(std::ostream& os, const BigInt& other) {
    if(other.isZero()) {
        os << 0;
        return os;
    }
    if(other.negative_) {
        os << '-';
    }
    int last = (int)other.digit_.size() - 1;
    os << other.digit_[last];

    for(int i = last - 1;i >= 0;i--) {
        os << std::setw(9) << std::setfill('0') << other.digit_[i];
    } //setw(int)不足位数默认用空格补齐，setfill设置补齐的字符

    return os;
}
BigInt BigInt::operator%(const BigInt &other) const {
    if (other.isZero()) {
        throw std::domain_error("modulo by zero");
    }

    BigInt a = *this;
    BigInt b = other;

    a.negative_ = false;
    b.negative_ = false;

    if (absCompare(a, b) < 0) {
        BigInt rem = a;
        rem.negative_ = negative_;
        rem.trim();
        return rem;
    }

    BigInt rem;

    for (int i = (int)a.digit_.size() - 1; i >= 0; --i) {
        rem.digit_.insert(rem.digit_.begin(), a.digit_[i]);
        rem.trim();

        uint32_t l = 0, r = base - 1, best = 0;

        while (l <= r) {
            uint32_t mid = l + (r - l) / 2;
            BigInt prod = absMulUint(b, mid);

            if (absCompare(prod, rem) <= 0) {
                best = mid;
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }

        rem = absSub(rem, absMulUint(b, best));
    }

    rem.negative_ = negative_;
    rem.trim();
    return rem;
}
BigInt BigInt::operator+=(const BigInt &other) {
    *this = (*this) + other;
    return *this;
}
BigInt BigInt::operator-=(const BigInt &other) {
    *this = (*this) - other;
    return *this;
}
BigInt BigInt::operator*=(const BigInt &other) {
    *this = (*this) * other;
    return *this;
}
BigInt BigInt::operator/=(const BigInt &other) {
    *this = (*this) / other;
    return *this;
}
BigInt BigInt::operator%=(const BigInt &other) {
    *this = (*this) % other;
    return *this;
}
std::istream& operator>>(std::istream& is, BigInt &other) {
    std::string s;
    is >> s;
    BigInt tmp(s);
    other = tmp;
    return is;
}
BigInt BigInt::operator-() const {
    BigInt ans(*this);
    if(!(this->isZero())) {
        ans.negative_ = !(*this).negative_;
    }
    else return ans;
}
BigInt BigInt::abs() const{
    BigInt ans = *this;
    ans.negative_ = false;
    return ans;
}
std::string BigInt::toString() const{
    std::ostringstream oss;
    oss << *this;
    return oss.str();
}
long long BigInt::toLongLong() const {
    unsigned long long ans = 0;
    for (int i = digit_.size() - 1; i >= 0; --i) {
        ans = ans * base + digit_[i];
        if (ans > LLONG_MAX) {
            throw std::overflow_error("The BigInt is too big to change into long long.");
        }
    }
    if (negative_) {
        if (ans == 0x8000000000000000ULL) return LLONG_MIN;
        return -static_cast<long long>(ans);
    }
    return static_cast<long long>(ans);
}
BigInt gcd(BigInt a, BigInt b) {
    BigInt tmp;
    while(!b.isZero()) { //总是b会先变为零
        a %= b;
        a.trim();
        std::swap(a,b);
    }
    return a;
}
BigInt lcm(BigInt a,BigInt b) {
    return a * b / gcd(a,b);
}
