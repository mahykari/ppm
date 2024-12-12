#ifndef STRING_UTILS_HH
#define STRING_UTILS_HH

#include <string>
#include <tuple>
#include "BigInt.hh"
#include "QuadraticResidueGroup.hh"
#include "YaoGarbler.hh"

std::vector<std::string> split(const std::string& s);
bool isNumber(const std::string& s);

std::string toString(const BigInt& n, int base = 10);
std::string toString(const QuadraticResidueGroup& g);

std::tuple<std::vector<BigInt>, std::string>
readBigInts(const std::string& message, int base, int count);

std::tuple<std::vector<GarbledGate>, std::string>
readGarbledGates(const std::string& message, int count);

#endif
