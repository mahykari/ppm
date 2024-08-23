#ifndef BIG_INT_HH
#define BIG_INT_HH

#include <gmp.h>
#include "Element.hh"

class BigInt : public Element {
friend class QuadraticResidueGroup;
public:
  BigInt(mpz_t n);
  ~BigInt() override;
  std::string toString() const override;
private:
  mpz_t n;
};

#endif
