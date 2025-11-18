#pragma once
#include <string>
#include <cstdint>
namespace gln {

class BigInt {
  struct Impl;         // Declaración de la implementación oculta
  Impl* p_;            // Puntero a la implementación
public:
  BigInt();                                  // 0
  explicit BigInt(long long x);              // desde entero
  explicit BigInt(const std::string& dec);   // desde cadena decimal
  explicit BigInt(const uint8_t* buf, size_t len); // desde buffer de bytes
  BigInt(const BigInt&);                     // copia
  BigInt(BigInt&&) noexcept;                 // movimiento
  BigInt& operator=(const BigInt&);          // asignación copia
  BigInt& operator=(BigInt&&) noexcept;      // asignación movimiento
  ~BigInt();                                 // destructor

  friend std::string to_string(const BigInt&);

  // Aritmética
  BigInt& operator+=(const BigInt&);
  BigInt& operator-=(const BigInt&);
  BigInt& operator*=(const BigInt&);
  BigInt& operator/=(const BigInt&);
  BigInt& operator%=(const BigInt&);

  friend BigInt operator+(BigInt a, const BigInt& b){ return a+=b; }
  friend BigInt operator-(BigInt a, const BigInt& b){ return a-=b; }
  friend BigInt operator*(BigInt a, const BigInt& b){ return a*=b; }
  friend BigInt operator/(BigInt a, const BigInt& b){ return a/=b; }
  friend BigInt operator%(BigInt a, const BigInt& b){ return a%=b; }
  friend BigInt operator&(const BigInt& a, const BigInt& b);
  BigInt operator-() const;

  // Comparaciones
  friend bool operator<(const BigInt&, const BigInt&);
  friend bool operator>(const BigInt&, const BigInt&);
  friend bool operator<=(const BigInt&, const BigInt&);
  friend bool operator>=(const BigInt&, const BigInt&);
  friend bool operator==(const BigInt&, const BigInt&);
  friend bool operator!=(const BigInt&, const BigInt&);
  friend std::ostream& operator<<(std::ostream& os, const BigInt& x);

  // Utilidades
  static BigInt abs(const BigInt&);
  static int    sgn(const BigInt&);
  static BigInt sqrt(const BigInt&);

  // Output operator
  std::string str() const;

  // Shift a la izquierda por k bits
  BigInt& operator<<=(unsigned long k);
  friend BigInt operator<<(BigInt a, unsigned long k) { a <<= k; return a; }

  
};

std::string to_string(const BigInt&);

} // namespace gln
