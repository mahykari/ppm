#include <iostream>
#include "BigIntegerElement.hh"

using BigInt = BigIntegerElement;

BigInt::BigIntegerElement(Group* g, mpz_t n) : Element(g) {
  mpz_init(this->n);
  mpz_set(this->n, n);
}

BigInt::~BigIntegerElement() {
  mpz_clear(this->n);
}

std::string BigInt::toString() const {
  char* str = mpz_get_str(NULL, 10, this->n);
  std::string s(str);
  free(str);
  return s;
}
