#include <algorithm>
#include "Exceptions.hh"
#include "YaoGarbler.hh"

GarbledGate YaoGarbler::enc(
  LabelPair left,
  LabelPair right,
  LabelPair out)
{
  std::vector<Label> labels;
  labels.reserve(3 * 2);
  labels.insert(labels.end(), left.begin(), left.end());
  labels.insert(labels.end(), right.begin(), right.end());
  labels.insert(labels.end(), out.begin(), out.end());
  if (not checkLabels(labels))
    throw InvalidLabels();
  
  GarbledGate gate;
  // ****************************************************************
  // * ASSUMPTION: Every gate is a 2-input NAND gate.               *
  // * TODO: Generalize to arbitrary gates.                         *
  // ****************************************************************
  gate[0] = this->encImpl(left[0], right[0], out[1]);
  gate[1] = this->encImpl(left[0], right[1], out[0]);
  gate[2] = this->encImpl(left[1], right[0], out[0]);
  gate[3] = this->encImpl(left[1], right[1], out[0]);

  // Garbled gate is a random permutation of the encrypted values.
  // ****************************************************************
  // * TODO: Use a provably random permutation generator.           *
  // ****************************************************************
  std::random_shuffle(gate.begin(), gate.end());
  return gate;
}

Label YaoGarbler::dec(
  Label left,
  Label right,
  GarbledGate gate)
{
  // ****************************************************************
  // * ASSUMPTION: Labels and ciphertexts are of appropriate sizes. *
  // ****************************************************************
  for (auto i = 0; i < 4; i++) {
    try {
      return this->decImpl(left, right, gate[i]);
    } catch(const InvalidCipher& e) {
      continue;
    }
  }
  throw InvalidCipher();
}
