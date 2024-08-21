#ifndef QUADRATIC_RESIDUE_GROUP_HH
#define QUADRATIC_RESIDUE_GROUP_HH

#include <gmp.h>
#include "CyclicGroup.hh"

class QuadraticResidueGroup : public CyclicGroup {
public:
  QuadraticResidueGroup(mpz_t p);
  ~QuadraticResidueGroup();
  std::unique_ptr<Element> randomGenerator();
  std::unique_ptr<Element> mul(Element* a, Element* b);
  std::unique_ptr<Element> exp(Element* a, mpz_t n);
private:
  mpz_t primeModulus;
  mpz_t smallPrime;
};

#endif
