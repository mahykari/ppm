#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>
#include <gmp.h>
#include <openssl/evp.h>
#include "Utils.hh"

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
