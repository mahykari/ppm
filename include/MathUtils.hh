#ifndef UTILS_HH
#define UTILS_HH

#include <string>
#include <vector>
#include <gmp.h>

const char HEX_ALPHABET[] = "0123456789abcdef";

uint8_t hexValue(char c);
std::string hashSha512(const std::string& s);

uint64_t timeBasedSeed();

const size_t N_SAFEPRIMES = 256;
// Sophie-Germain primes
inline mpz_t SG_PRIMES[N_SAFEPRIMES + 5];
// Safe primes
inline mpz_t SAFE_PRIMES[N_SAFEPRIMES + 5];

void initPrimes();

// Binary representation of n.
// Note that the in the resulting vector,
// LSB is at index 0, and MSB is at index length - 1,
// hence a reverted representation.
std::vector<bool> toBinary(unsigned n, unsigned length);

// Copy from https://stackoverflow.com/a/17299623/15279018.
template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
  std::size_t total_size = 0;
  for (const auto& sub : v)
    total_size += sub.size();
  std::vector<T> result;
  result.reserve(total_size);
  for (const auto& sub : v)
    result.insert(result.end(), sub.begin(), sub.end());
  return result;
}

#endif
