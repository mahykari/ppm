#ifndef QUADRATIC_RESIDUE_GROUP_HH
#define QUADRATIC_RESIDUE_GROUP_HH

#include <gmp.h>
#include "CyclicGroup.hh"
#include "BigInt.hh"

class QuadraticResidueGroup
  : public CyclicGroup<BigInt> {
public:
  QuadraticResidueGroup(mpz_t p);
  ~QuadraticResidueGroup() override;

  BigInt randomGenerator() override;
  BigInt mul(const BigInt& a, const BigInt& b) override;
  BigInt exp(const BigInt& a, mpz_t n) override;

  std::string toString() const override;
private:
  mpz_t primeModulus;
  mpz_t smallPrime;
};

#endif
