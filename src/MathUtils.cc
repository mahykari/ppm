#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <gmp.h>
#include <openssl/evp.h>
#include "MathUtils.hh"

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
  for (size_t i = 0; i <= N_SAFEPRIMES; i++) {
    mpz_t p;
    mpz_init(p);
    mpz_nextprime(p, lb);
    while (not isSophieGermain(p) and mpz_cmp(p, ub) < 0)
      mpz_nextprime(p, p);
    if (not isSophieGermain(p) or mpz_cmp(p, ub) >= 0) {
      mpz_set_si(SG_PRIMES[i], -1);
      mpz_set_si(SAFE_PRIMES[i], -1);
    }
    else {
      mpz_set(SG_PRIMES[i], p);
      mpz_mul_ui(SAFE_PRIMES[i], p, 2);
      mpz_add_ui(SAFE_PRIMES[i], SAFE_PRIMES[i], 1);
    }
    mpz_set(lb, ub);
    mpz_mul_ui(ub, ub, 2);
  }
}
