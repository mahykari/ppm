#include "StringUtils.hh"

std::string toString(const BigInt& n, int base) {
  return n.get_str(base);
}

std::string toString(const QuadraticResidueGroup& g) {
  return
    "QuadraticResidueGroup("
    + toString(g.primeModulus) + ")";
}

