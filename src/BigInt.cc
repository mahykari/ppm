#include <iostream>
#include "BigInt.hh"

BigInt::BigInt(mpz_t n) {
  mpz_init(this->n);
  mpz_set(this->n, n);
}

BigInt::~BigInt() {
  mpz_clear(this->n);
}

std::string BigInt::toString() const {
  char* str = mpz_get_str(NULL, 10, this->n);
  std::string s(str);
  free(str);
  return s;
}
