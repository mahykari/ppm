#ifndef QUADRATIC_RESIDUE_GROUP_HH
#define QUADRATIC_RESIDUE_GROUP_HH

#include <gmp.h>
#include "CyclicGroup.hh"
#include "BigInt.hh"

class QuadraticResidueGroup
  : public CyclicGroup<BigInt> {
public:
  explicit QuadraticResidueGroup(BigInt p);
  ~QuadraticResidueGroup() override;

  BigInt randomGenerator() override;
  const BigInt baseGenerator = BigInt(4);
  BigInt mul(const BigInt& a, const BigInt& b) override;
  BigInt exp(const BigInt& a, const BigInt& n) override;
  BigInt inv(const BigInt& a);
  BigInt randomExponent();
  BigInt order() override;

  friend std::string toString(const QuadraticResidueGroup& g);

private:
  BigInt primeModulus;
  BigInt smallPrime;
};

#endif
