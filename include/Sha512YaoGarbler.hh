#ifndef SHA_512_YAO_GARBLER_HH
#define SHA_512_YAO_GARBLER_HH

#include "YaoGarbler.hh"

// The Sha512YaoGarbler class uses the SHA-512 hash function
// for encryption and decryption.

class Sha512YaoGarbler : public YaoGarbler {
private:
  std::string encImpl(Label left, Label right, Label out) override;
  Label decImpl(Label left, Label right, Ciphertext cipher) override;
  bool checkLabels(std::vector<Label> labels) override;
};

#endif
