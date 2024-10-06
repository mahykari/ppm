#include <cassert>
#include <openssl/evp.h>
#include "Exceptions.hh"
#include "MathUtils.hh"
#include "Sha512YaoGarbler.hh"

// Output of SHA-512 is 128 characters long,
// when represented in hexadecimal.
const size_t SHA512_HEX_SIZE = EVP_MAX_MD_SIZE * 2;
// The last character is an XOR checksum.
const size_t CHECK_HEX_SIZE = 1;

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

  uint8_t check = 0;
  for (size_t i = 0; i < out.size(); i++)
    check ^= hexValue(out[i]);
  auto out1 = out + HEX_ALPHABET[check];

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

  uint8_t check = 0;
  for (size_t i = 0; i < size - 1; i++)
    check ^= hexValue(label[i]);

  if (label[size - 1] != HEX_ALPHABET[check])
    throw InvalidCipher();
  return label.substr(0, size - 1);
}
