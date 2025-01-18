#include <sstream>
#include <iomanip>
#include "StringUtils.hh"
#include "YaoGarbler.hh"
#include "Timer.hh"

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

std::tuple<std::vector<std::string>, std::string>
readStrings(const std::string& message, int count) {
  std::vector<std::string> strings(count);
  std::stringstream ss(message);
  for (auto i = 0; i < count; i++) {
    ss >> strings[i];
  }
  auto remaining = ss.str().substr(ss.tellg());
  return { strings, remaining };
}

HexGeneratorState::HexGeneratorState() {
  urandom.open("/dev/urandom", std::ios::in | std::ios::binary);
  if (!urandom)
    throw std::runtime_error("Failed to open /dev/urandom");
  buffer.resize(1 << 20);
  bufferPos = buffer.size();
}

HexGeneratorState::~HexGeneratorState() {
  urandom.close();
}

HexGeneratorState SingletonHexGeneratorState;

std::string randomHexString(unsigned size) {
  std::ostringstream hexStream;
  hexStream << std::hex << std::setfill('0');

  HexGeneratorState& state = SingletonHexGeneratorState;

  size_t remaining = size;
  while (remaining > 0) {
    if (state.bufferPos == state.buffer.size()) {
      Timer timer;
      timer.start();
      state.urandom.read(
        reinterpret_cast<char*>(state.buffer.data()), state.buffer.size());
      printf(
        "D: refilling hex generator buffer (took %f ms)\n", timer.display());
      state.bufferPos = 0;
    }

    size_t bytesToRead =
      std::min(remaining, state.buffer.size() - state.bufferPos);
    for (size_t i = 0; i < bytesToRead; ++i) {
      hexStream
        << std::setw(2)
        << static_cast<int>(state.buffer[state.bufferPos + i]);
    }

    state.bufferPos += bytesToRead;
    remaining -= bytesToRead;
  }

  return hexStream.str();
}
