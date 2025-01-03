#include <cassert>
#include "Shake256YaoGarbler.hh"
#include "MathUtils.hh"
#include "Exceptions.hh"

const unsigned CHECK_HEX_SIZE = 25;

bool Shake256YaoGarbler::checkLabels(std::vector<Label> labels) {
  bool valid = true;
  assert (labels.size() == 3 * 2);
  auto size = labels[0].size();
  for (auto label : labels)
    valid &= label.size() == size;
  return valid;
}

Ciphertext
Shake256YaoGarbler::encImpl(
  Label left, Label right, Label out
) {
  // 1 ^ {4 * CHECK_HEX_SIZE} in binary.
  auto check = std::string(CHECK_HEX_SIZE, 'f');
  auto out1 = out + check;

  auto s = left + right;
  auto h = hashShake256(s, out1.size());

  Ciphertext cipher(out1.size(), 0);
  for (size_t i = 0; i < out1.size(); i++)
    cipher[i] = HEX_ALPHABET[hexValue(out1[i]) ^ hexValue(h[i])];
  return cipher;
}

Label
Shake256YaoGarbler::decImpl(
  Label left, Label right, Ciphertext cipher
) {
  auto s = left + right;
  auto size = cipher.size();
  auto h = hashShake256(s, size);
  auto label = Label(size, 0);
  for (size_t i = 0; i < size; i++)
    label[i] = HEX_ALPHABET[hexValue(cipher[i]) ^ hexValue(h[i])];

  auto check = std::string(CHECK_HEX_SIZE, 'f');
  // printf(
  //   "D: Sha512YaoGarbler::decImpl: label %s, check: %s\n",
  //   label.c_str(),
  //   check.c_str());
  if (label.substr(size - CHECK_HEX_SIZE) != check)
    throw InvalidCipher();
  return label.substr(0, size - CHECK_HEX_SIZE);
}
