#ifndef GROUP_HH
#define GROUP_HH

#include <iostream>
#include <map>
#include <memory>
#include <gmp.h>

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
  virtual E exp(const E& a, mpz_t n) = 0;

  // I/O methods
  virtual std::string toString() const = 0;
};

#endif
