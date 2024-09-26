#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include <openssl/evp.h>
#include <gmp.h>

// class OpenSSLFree {
// public:
//   void operator()(void* ptr) {
//     EVP_MD_CTX_free((EVP_MD_CTX*) ptr);
//   }
// };

// template <class T>
// using OpenSSLPointer = std::unique_ptr<T, OpenSSLFree>;

// class HashingError : public std::runtime_error {
// public:
//   HashingError() : std::runtime_error("Hashing failed") {}
// };

// const char alphabet[] = "0123456789abcdef";

// static inline uint8_t value(char c) {
//   if (c >= '0' && c <= '9') return c - '0';     
//   if (c >= 'a' && c <= 'f') return c - 'a' + 10;
//   if (c >= 'A' && c <= 'F') return c - 'A' + 10;
//   return -1;
// }

namespace {
  class OpenSSLFree {
  public:
    void operator()(void* ptr) {
      EVP_MD_CTX_free((EVP_MD_CTX*) ptr);
    }
  };

  using OpenSSLPointer = std::unique_ptr<EVP_MD_CTX, OpenSSLFree>;

  class Sha512InternalError : public std::runtime_error {
  public:
    Sha512InternalError() : std::runtime_error("SHA-512 failed") {}
  };

  const char alphabet[] = "0123456789abcdef";

  inline uint8_t value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  }
};

std::string hashSha256(const std::string& s) {
  OpenSSLPointer context(EVP_MD_CTX_new());

  if(context.get() == nullptr)
    throw Sha512InternalError();

  if(!EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr))
    throw Sha512InternalError();

  if(!EVP_DigestUpdate(context.get(), s.c_str(), s.length()))
    throw Sha512InternalError();

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hashLength;

  if(!EVP_DigestFinal_ex(context.get(), hash, &hashLength))
    throw Sha512InternalError();

  std::stringstream outputStream;
  for(unsigned int i = 0; i < hashLength; i++)
    outputStream
      << std::hex << std::setw(2) << std::setfill('0') 
      << (int) hash[i];
  
  return outputStream.str();
}

int main() {
  char input[EVP_MAX_MD_SIZE];
  std::cin >> input;
  mpz_t a;
  mpz_init_set_str(a, input, 16);
  auto s = std::string(mpz_get_str(nullptr, 16, a));
  std::cout << "- s in base 16: " << s << '\n';
  auto h = hashSha256(s);
  std::cout
    << "- SHA-256(s): "
    << h.c_str() << '\n';
  assert (hashSha256(s).size() == 64);
  auto w = h.substr(0, s.size());
  std::string encYao(s.size(), 0);
  for (auto i = 0; i < s.size(); i++)
    encYao[i] = alphabet[value(s[i]) ^ value(w[i])];
  std::cout 
    << "- encrypted s: "
    << encYao << '\n';
  auto decYao = encYao;
  for (auto i = 0; i < s.size(); i++)
    decYao[i] = alphabet[value(encYao[i]) ^ value(w[i])];
  std::cout
    << "- decrypted s: "
    << decYao << '\n';
}
