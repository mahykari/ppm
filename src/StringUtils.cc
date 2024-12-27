#include <sstream>
#include "StringUtils.hh"
#include "YaoGarbler.hh"

std::vector<std::string> split(const std::string& s) {
  std::vector<std::string> tokens;
  std::stringstream ss(s);
  std::string token;
  while (ss >> token)
    tokens.push_back(token);
  return tokens;
}

bool isNumber(const std::string& s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

std::string toString(const BigInt& n, int base) {
  return n.get_str(base);
}

std::string toString(const QuadraticResidueGroup& g) {
  return
    "QuadraticResidueGroup("
    + toString(g.primeModulus) + ")";
}

std::tuple<std::vector<BigInt>, std::string>
readBigInts(const std::string& message, int base, int count) {
  std::stringstream ss(message);
  std::vector<BigInt> bigInts;
  for (int i = 0; i < count; i++) {
    if (ss.eof())
      abort();
    std::string nStr;
    ss >> nStr;
    bigInts.push_back(BigInt(nStr, base));
  }
  std::string remaining = ss.eof() ? "" : ss.str().substr(ss.tellg());
  return { bigInts, remaining };
}

std::tuple<std::vector<GarbledGate>, std::string>
readGarbledGates(const std::string& message, int count) {
  std::vector<GarbledGate> garbledGates;
  std::stringstream ss(message);
  for (auto i = 0; i < count; i++) {
    GarbledGate gate;
    for (auto j = 0; j < 4; j++)
      ss >> gate[j];
    garbledGates.push_back(gate);
  }
  auto remaining = ss.str().substr(ss.tellg());
  return { garbledGates, remaining };
}
