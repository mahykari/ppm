#include <cassert>
#include <openssl/evp.h>
#include "Exceptions.hh"
#include "MathUtils.hh"
#include "Sha512YaoGarbler.hh"

// Output of SHA-512 is 128 characters long,
// when represented in hexadecimal.
const size_t SHA512_HEX_SIZE = EVP_MAX_MD_SIZE * 2;
// The last character is an XOR checksum.
const size_t CHECK_HEX_SIZE = 16;

bool Sha512YaoGarbler::checkLabels(std::vector<Label> labels) {
  bool valid = true;
  assert (labels.size() == 3 * 2);
  auto size = labels[0].size();
  for (auto label : labels)
    valid &= label.size() == size;
  valid &= size <= SHA512_HEX_SIZE - CHECK_HEX_SIZE;
  return valid;
}

Ciphertext Sha512YaoGarbler::encImpl(
  Label left, Label right, Label out)
{
  auto s = left + right;
  auto h = hashSha512(s);

  auto check = hashSha512(out);
  check = check.substr(0, CHECK_HEX_SIZE);
  auto out1 = out + check;

  Ciphertext cipher(out1.size(), 0);
  for (size_t i = 0; i < out1.size(); i++)
    cipher[i] = HEX_ALPHABET[hexValue(out1[i]) ^ hexValue(h[i])];
  return cipher;
}

Label Sha512YaoGarbler::decImpl(
  Label left, Label right, Ciphertext cipher)
{
  auto s = left + right;
  auto h = hashSha512(s);
  auto size = cipher.size();
  Label label(size, 0);
  for (size_t i = 0; i < size; i++)
    label[i] = HEX_ALPHABET[hexValue(cipher[i]) ^ hexValue(h[i])];

  auto check = hashSha512(label.substr(0, size - CHECK_HEX_SIZE));
  check = check.substr(0, CHECK_HEX_SIZE);
  // printf(
  //   "D: Sha512YaoGarbler::decImpl: label %s, check: %s\n",
  //   label.c_str(),
  //   check.c_str());
  if (label.substr(size - CHECK_HEX_SIZE) != check)
    throw InvalidCipher();
  return label.substr(0, size - CHECK_HEX_SIZE);
}
