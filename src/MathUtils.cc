#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cassert>
#include <gmp.h>
#include <openssl/evp.h>
#include "MathUtils.hh"
#include "Exceptions.hh"

namespace {
  class OpenSSLFree {
  public:
    void operator()(void* ptr) {
      EVP_MD_CTX_free((EVP_MD_CTX*) ptr);
    }
  };

  using OpenSSLPointer = std::unique_ptr<EVP_MD_CTX, OpenSSLFree>;

  class Sha512Error : public std::runtime_error {
  public:
    Sha512Error() : std::runtime_error("SHA-512 failed") {}
  };

  class Shake256Error : public std::runtime_error {
  public:
    Shake256Error() : std::runtime_error("SHAKE-256 failed") {}
  };

  class InvalidHexCharacter : public std::runtime_error {
  public:
    InvalidHexCharacter()
      : std::runtime_error("Invalid hex character") {}
  };
};

uint8_t hexValue(char c) {
  if (c >= '0' and c <= '9')
    return c - '0';
  if (c >= 'a' and c <= 'f')
    return c - 'a' + 10;
  throw InvalidHexCharacter();
}

std::string hashSha512(const std::string& s) {
  OpenSSLPointer context(EVP_MD_CTX_new());

  if(context.get() == nullptr)
    throw Sha512Error();

  if(!EVP_DigestInit_ex(context.get(), EVP_sha512(), nullptr))
    throw Sha512Error();

  if(!EVP_DigestUpdate(context.get(), s.c_str(), s.length()))
    throw Sha512Error();

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hashLength;

  if(!EVP_DigestFinal_ex(context.get(), hash, &hashLength))
    throw Sha512Error();

  std::stringstream outputStream;
  for(unsigned int i = 0; i < hashLength; i++)
    outputStream
      << std::hex << std::setw(2) << std::setfill('0')
      << (int) hash[i];

  return outputStream.str();
}

std::string hashShake256(const std::string& s, size_t len) {
  OpenSSLPointer context(EVP_MD_CTX_new());

  if(context.get() == nullptr)
    throw Sha512Error();

  if (!EVP_DigestInit_ex(context.get(), EVP_shake256(), nullptr))
    throw Shake256Error();

  if (!EVP_DigestUpdate(context.get(), s.c_str(), s.length()))
    throw Shake256Error();

  size_t outputLength = (len == 0) ? s.length() : len;
  // Overallocating to prevent ending character problems.
  auto hash = std::make_unique<unsigned char[]>(outputLength + 5);
  if (!EVP_DigestFinalXOF(context.get(), hash.get(), outputLength))
    throw Shake256Error();

  std::stringstream outputStream;
  for (size_t i = 0; i < outputLength; i++)
    outputStream
      << std::hex << std::setw(2) << std::setfill('0')
      << (int) hash[i];

  return outputStream.str();
}

uint64_t timeBasedSeed() {
  // The following comments are quoted
  // from the CPlusPlus.com reference.
  return
    // "... Specifically, system_clock is
    // a system-wide realtime clock."
    std::chrono::system_clock
    // "Returns the current time_point
    // in the frame of the system_clock."
    ::now()
    // "Returns a duration object with
    // the time span value between the epoch and the time point."
    .time_since_epoch()
    // "Returns the internal count (i.e., the representation value)
    // of the duration object."
    .count();
}

bool isSophieGermain(mpz_t p) {
  /* From GMP documentation for
  mpz_probab_prime_p(const mpz_t n, int reps):
  > ... Return 2 if n is definitely prime,
  > return 1 if n is probably prime (without being certain),
  > or return 0 if n is definitely non-prime. */
  mpz_t p1;
  mpz_init_set(p1, p);
  mpz_mul_ui(p1, p1, 2);
  mpz_add_ui(p1, p1, 1);
  return mpz_probab_prime_p(p1, 40) >= 1;
}

void initPrimes() {
  printf("I: initializing safe primes...\n");
  for (size_t i = 0; i <= N_SAFEPRIMES; i++) {
    mpz_init(SG_PRIMES[i]);
    mpz_init_set_si(SAFE_PRIMES[i], -1);
  }
  mpz_t lb, ub;
  mpz_init_set_ui(lb, 1);
  mpz_init_set_ui(ub, 2);
  for (size_t i = 1; i <= N_SAFEPRIMES; i++) {
    mpz_t p;
    mpz_init(p);
    mpz_nextprime(p, lb);
    while (not isSophieGermain(p) and mpz_cmp(p, ub) < 0)
      mpz_nextprime(p, p);
    if (not isSophieGermain(p) or mpz_cmp(p, ub) >= 0) {
      mpz_set_si(SG_PRIMES[i], -1);
      mpz_set_si(SAFE_PRIMES[i + 1], -1);
    }
    else {
      mpz_set(SG_PRIMES[i], p);
      mpz_mul_ui(SAFE_PRIMES[i + 1], p, 2);
      mpz_add_ui(SAFE_PRIMES[i + 1], SAFE_PRIMES[i + 1], 1);
    }
    mpz_clear(p);
    mpz_set(lb, ub);
    mpz_mul_ui(ub, ub, 2);
  }
}

mpz_class getSafePrime(size_t i) {
  if (i <= N_SAFEPRIMES) {
    auto p = mpz_class(SAFE_PRIMES[i]);
    assert (p != -1);
    return p;
  } else {
    switch (i) {
      case  768: return mpz_class( MODP_768_PRIME, 16);
      case 1024: return mpz_class(MODP_1024_PRIME, 16);
      case 1536: return mpz_class(MODP_1536_PRIME, 16);
      case 2048: return mpz_class(MODP_2048_PRIME, 16);
      case 3072: return mpz_class(MODP_3072_PRIME, 16);
      case 4096: return mpz_class(MODP_4096_PRIME, 16);
      default: throw InvalidPrimeIndex();
    }
  }
}

std::vector<bool> toBinary(unsigned n, unsigned length) {
  std::vector<bool> word(length);
  for (unsigned i = 0; i < length; i++) {
    word[i] = (n >> i) & 1;
  }
  return word;
}
