#include "gln/bigint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <utility>
#include <string>

namespace gln {

using boost::multiprecision::cpp_int;

struct BigInt::Impl {
  boost::multiprecision::cpp_int v;
  Impl() : v(0) {}
  explicit Impl(long long x) : v(x) {}
  explicit Impl(const std::string& dec) : v(0) {
    bool neg = false;
    std::size_t i = 0;
    if (!dec.empty() && dec[0] == '-') { neg = true; i = 1; }
    for (; i < dec.size(); ++i) {
      char c = dec[i];
      if (c >= '0' && c <= '9') { v *= 10; v += (c - '0'); }
    }
    if (neg) v = -v;
  }
  Impl(const Impl& o) : v(o.v) {}

  explicit Impl(const uint8_t* buf, size_t len) : v(0) {
    for (size_t i = 0; i < len; ++i) {
        v *= 256;
        v += static_cast<unsigned long long>(buf[i]);
    }
  }
};

BigInt::BigInt() : p_(new Impl()) {}
BigInt::BigInt(long long x) : p_(new Impl(x)) {}
BigInt::BigInt(const std::string& s) : p_(new Impl(s)) {}
BigInt::BigInt(const uint8_t* buf, size_t len) : p_(new Impl(buf,len)){}

BigInt::BigInt(const BigInt& o) : p_(new Impl(*o.p_)) {}
BigInt::BigInt(BigInt&& o) noexcept : p_(o.p_) { o.p_ = nullptr; }


BigInt& BigInt::operator=(const BigInt& o) {
  if (this != &o) { Impl* np = new Impl(*o.p_); delete p_; p_ = np; }
  return *this;
}
BigInt& BigInt::operator=(BigInt&& o) noexcept {
  if (this != &o) { delete p_; p_ = o.p_; o.p_ = nullptr; }
  return *this;
}
BigInt::~BigInt() { delete p_; }

BigInt operator&(const BigInt& a, const BigInt& b) {
    std::string sa = to_string(a);
    std::string sb = to_string(b);

    cpp_int Ai = 0;
    cpp_int Bi = 0;

    if (!sa.empty()) Ai = cpp_int(0), Ai = cpp_int(0); 
    Ai = 0;
    Bi = 0;

    Ai = cpp_int(0);
    for (char c : sa) {
        if (c >= '0' && c <= '9') {
            Ai *= 10;
            Ai += cpp_int(c - '0');
        }
    }
    Bi = cpp_int(0);
    for (char c : sb) {
        if (c >= '0' && c <= '9') {
            Bi *= 10;
            Bi += cpp_int(c - '0');
        }
    }

    cpp_int Ci = Ai & Bi;
    std::string sc = Ci.convert_to<std::string>();

    return BigInt(sc);
}

BigInt& BigInt::operator+=(const BigInt& o){ p_->v += o.p_->v; return *this; }
BigInt& BigInt::operator-=(const BigInt& o){ p_->v -= o.p_->v; return *this; }
BigInt& BigInt::operator*=(const BigInt& o){ p_->v *= o.p_->v; return *this; }
BigInt& BigInt::operator/=(const BigInt& o){ p_->v /= o.p_->v; return *this; }
BigInt& BigInt::operator%=(const BigInt& o){ p_->v %= o.p_->v; return *this; }
BigInt BigInt::operator-() const { BigInt r(*this); r.p_->v = -r.p_->v; return r; }

bool operator<(const BigInt& a, const BigInt& b){ return a.p_->v < b.p_->v; }
bool operator>(const BigInt& a, const BigInt& b){ return a.p_->v > b.p_->v; }
bool operator<=(const BigInt& a, const BigInt& b){ return a.p_->v <= b.p_->v; }
bool operator>=(const BigInt& a, const BigInt& b){ return a.p_->v >= b.p_->v; }
bool operator==(const BigInt& a, const BigInt& b){ return a.p_->v == b.p_->v; }
bool operator!=(const BigInt& a, const BigInt& b){ return a.p_->v != b.p_->v; }

BigInt BigInt::abs(const BigInt& x){ BigInt r(x); if(r.p_->v<0) r.p_->v = -r.p_->v; return r; }
int BigInt::sgn(const BigInt& x){ return (x.p_->v>0) - (x.p_->v<0); }

BigInt BigInt::sqrt(const BigInt& x)  {
    if (x.p_->v < 0) {
        throw std::runtime_error("BigInt::sqrt: negative argument");
    }
    BigInt result;
    result.p_->v = boost::multiprecision::sqrt(x.p_->v);
    return result;
}

std::ostream& operator<<(std::ostream& os, const BigInt& x) {
    return os << x.p_->v.str();
}

std::string BigInt::str() const { return p_->v.str(); }
std::string to_string(const BigInt& x) { return x.p_->v.convert_to<std::string>(); }

BigInt& BigInt::operator<<=(unsigned long k) {
    p_->v <<= k; 
    return *this;
}

} // namespace gln
