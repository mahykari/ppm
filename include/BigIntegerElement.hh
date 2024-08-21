#ifndef BIG_INTEGER_ELEMENT_HH
#define BIG_INTEGER_ELEMENT_HH

#include <gmp.h>
#include "Element.hh"

class BigIntegerElement : public Element {
friend class QuadraticResidueGroup;
public:
  BigIntegerElement(Group* g, mpz_t n);
  ~BigIntegerElement();
  std::string toString() const;
private:
  mpz_t n;
};

#endif
