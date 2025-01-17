#ifndef GROUP_HH
#define GROUP_HH

#include <iostream>
#include <map>
#include <memory>
#include <gmp.h>

  #include "BigInt.hh"

// Using a template class as the group element
// allows for tying the group to a specific element type;
// e.g., the group of quadratic residues modulo prime p
// only contains integer elements (of arbitrary size).

// Elements of a concrete group are instances of class E
// for a concrete E.
template <class E>
class Group {
public:
  virtual ~Group() = default;

  // Group operations
  virtual E mul(const E& a, const E& b) = 0;
  virtual E exp(const E& a, const E& n) = 0;
  virtual BigInt order() = 0;
};

#endif
