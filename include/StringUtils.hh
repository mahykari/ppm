#ifndef STRING_UTILS_HH
#define STRING_UTILS_HH

#include <string>
#include "BigInt.hh"
#include "QuadraticResidueGroup.hh"

std::string toString(const BigInt& n, int base = 10);
std::string toString(const QuadraticResidueGroup& g);

#endif
