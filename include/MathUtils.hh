#ifndef UTILS_HH
#define UTILS_HH

#include <string>
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

#endif
