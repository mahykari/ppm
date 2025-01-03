#ifndef SHAKE_256_YAO_GARBLER_HH
#define SHAKE_256_YAO_GARBLER_HH

#include "YaoGarbler.hh"

// The Shake256YaoGarbler class uses
// the Shake-256 extendable-output hash function
// for encryption and decryption.

class Shake256YaoGarbler : public YaoGarbler {
private:
  std::string encImpl(Label left, Label right, Label out) override;
  Label decImpl(Label left, Label right, Ciphertext cipher) override;
  bool checkLabels(std::vector<Label> labels) override;
};

#endif
