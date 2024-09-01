#ifndef YAO_GARBLER_HH
#define YAO_GARBLER_HH

#include <array>
#include <string>
#include <vector>

using Label = std::string;
using Ciphertext = std::string;

// As the garbler does encryption and decryption,
// it suffices to define its interface using only strings
// and convert wire labels to strings outside the garbler.
// For this reason, both LabelPair and GarbledGate are
// defined as containers of strings.

// A LabelPair is an array of [exactly] 2 strings.
using LabelPair = std::array<Label, 2>;

// A GarbledGate is an array of [exactly] 4 strings;
// each string is the encrypted form of an output wire label.
// This vector should be thought of as 
// a random permutation of the encrypted values
// (e.g., similar to an unordered_set).
using GarbledGate = std::array<Ciphertext, 4>;

class YaoGarbler {
public:
  GarbledGate enc(
    LabelPair left,
    LabelPair right,
    LabelPair out);

  // For decryption, only the 'correct' keys are necessary.
  // With these keys, the garbler tries to decrypt the gate
  // and returns at the first succesful decryption.
  Label dec(
    Label left,
    Label right,
    GarbledGate gate);
protected:
  virtual bool checkLabels(std::vector<Label> labels) = 0;

  // As the exact implementation of {en,de}-cryption
  // is left to the children of YaoGarbler,
  // the following methods are suffixed with 'Impl'
  // and defined as protected pure virtual methods.
  virtual Ciphertext encImpl(
    Label left, Label right, Label out) = 0;
  virtual Label decImpl(
    Label left, Label right, Ciphertext cipher) = 0;
};

#endif
