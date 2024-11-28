#include <cassert>
#include "Module.hh"
#include "Exceptions.hh"

void Module::build(Circuit& circuit) {
  if (this->outputWord.empty())
    this->buildImpl(circuit);
}

Word Module::output() {
  if (this->outputWord.empty())
    throw UnbuiltModuleHasNoOutput();
  return this->outputWord;
}

Module::operator Word() {
  return this->output();
}

Module::operator unsigned() {
  auto out = this->output();
  assert (out.size() == 1U);
  return out[0];
}

unsigned Module::operator[](unsigned index) {
  if (this->outputWord.empty())
    throw UnbuiltModuleHasNoOutput();
  assert (index < this->outputWord.size());
  return this->outputWord[index];
}

// A Broadcaster does not need to build anything;
// it simply repeats its input.

Broadcaster::Broadcaster(unsigned input, unsigned count)
: input(input), count(count) {}

void Broadcaster::buildImpl(Circuit& circuit) {
  this->outputWord = Word(this->count, this->input);
}

Inverter::Inverter(Word input) : input(input) {}

void Inverter::buildImpl(Circuit& circuit) {
  this->outputWord.resize(this->input.size());
  for (unsigned i = 0; i < this->input.size(); i++) {
    auto bit = this->input[i];
    this->outputWord[i] = circuit.addGate(bit, bit);
  }
}

BinaryGate::BinaryGate(Word inputLeft, Word inputRight)
: inputLeft(inputLeft), inputRight(inputRight) {}

BinaryGate::BinaryGate(unsigned inputLeft, unsigned inputRight)
: BinaryGate(Word { inputLeft }, Word { inputRight }) {}

void AndGate::buildImpl(Circuit& circuit) {
  auto length = this->inputLeft.size();
  this->outputWord.resize(length);
  for (unsigned i = 0; i < length; i++) {
    auto left = this->inputLeft[i];
    auto right = this->inputRight[i];
    auto nandOut = circuit.addGate(left, right);
    auto nandOutInv = circuit.addGate(nandOut, nandOut);
    this->outputWord[i] = nandOutInv;
  }
}

void OrGate::buildImpl(Circuit& circuit) {
  auto length = this->inputLeft.size();
  this->outputWord.resize(length);
  auto leftInv = Inverter(this->inputLeft);
  auto rightInv = Inverter(this->inputRight);
  leftInv.build(circuit);
  rightInv.build(circuit);

  auto leftInvOut = leftInv.output();
  auto rightInvOut = rightInv.output();

  for (unsigned i = 0; i < length; i++) {
    outputWord[i] =
      circuit.addGate(leftInvOut[i], rightInvOut[i]);
  }
}

void XorGate::buildImpl(Circuit& circuit) {
  auto length = this->inputLeft.size();
  this->outputWord.resize(length);

  // An expression of the form A + B can be written as ~((~A) (~B)).
  // We use the same strategy to implement XOR ((~a)b + a(~b)).
  for (unsigned i = 0; i < length; i++) {
    // Using the auxiliary NAND,
    // we don't need to invert both input bits.
    // ~(lr) = ~l + ~r
    auto nandAux = circuit.addGate(inputLeft[i], inputRight[i]);
    // ~(l (~l + ~r)) = ~(l(~l) + r(~r)) = ~(l(~r))
    auto nandLAux = circuit.addGate(inputLeft[i], nandAux);
    // ~(r(~l))
    auto nandRAux = circuit.addGate(inputRight[i], nandAux);
    // l(~r) + r(~l)
    outputWord[i] = circuit.addGate(nandLAux, nandRAux);
  }
}

void XnorGate::buildImpl(Circuit& circuit) {
  // a xnor b = ~(a xor b)
  auto length = this->inputLeft.size();
  this->outputWord.resize(length);
  // a xor b
  auto xorLR = XorGate(this->inputLeft, this->inputRight);
  xorLR.build(circuit);
  auto inv = Inverter(xorLR);
  inv.build(circuit);
  this->outputWord = inv;
}

Selector::Selector(WordVector input, Word select)
: input(input), select(select) {
  assert (input.size() == (1U << select.size()));
}

void Selector::buildImpl(Circuit& circuit) {
  auto selectLength = select.size();
  auto wordLength = input[0].size();

  if (selectLength == 1) {
    // Base case: `select` is a single bit.
    // We can construct the 2-to-1 selector directly,
    // using simpler modules.
    auto selectBC = Broadcaster(select[0], wordLength);
    selectBC.buildImpl(circuit);

    auto selectBCInv = Inverter(selectBC);
    selectBCInv.buildImpl(circuit);

    auto sel0 = AndGate(input[0], selectBCInv);
    auto sel1 = AndGate(input[1], selectBC);
    sel0.buildImpl(circuit);
    sel1.buildImpl(circuit);

    auto sel = OrGate(sel0[0], sel1[0]);
    sel.buildImpl(circuit);
    this->outputWord = sel;
  } else {
    // We split the input in two halves,
    // and pass each half to a smaller selector,
    // along with `select[1:]` (i.e., `select` without its MSB).
    // Each smaller selector will output a word.
    // To select output for the current selector,
    // we use another 2-to-1 selector
    // with the MSB of `select` as its select input.
    auto halfSelect = Word(select.begin() + 1, select.end());
    auto mid = 1 << (selectLength - 1);
    auto input0 = WordVector(input.begin()      , input.begin() + mid);
    auto input1 = WordVector(input.begin() + mid, input.end()        );
    auto selector0 = Selector(input0, halfSelect);
    auto selector1 = Selector(input1, halfSelect);
    selector0.buildImpl(circuit);
    selector1.buildImpl(circuit);

    auto selector2to1 = Selector(
      { selector0, selector1 },
      { select[0] } );
    selector2to1.buildImpl(circuit);
    this->outputWord = selector2to1;
  }
}

HalfAdder::HalfAdder(unsigned inputLeft, unsigned inputRight)
: inputLeft(inputLeft), inputRight(inputRight) {}

void HalfAdder::buildImpl(Circuit& circuit) {
  auto sum = XorGate(this->inputLeft, this->inputRight);
  auto carry = AndGate(this->inputLeft, this->inputRight);
  sum.build(circuit);
  carry.build(circuit);
  auto sumOut = sum.output();
  auto carryOut = carry.output();
  this->outputWord = { sum, carry };
}

unsigned HalfAdder::sum() { return (*this)[0]; }

unsigned HalfAdder::carry() { return (*this)[1]; }

FullAdder::FullAdder(
  unsigned inputLeft, unsigned inputRight, unsigned carryIn)
: inputLeft(inputLeft), inputRight(inputRight), carryIn(carryIn) {}

void FullAdder::buildImpl(Circuit& circuit) {
  auto ha1 = HalfAdder(this->inputLeft, this->inputRight);
  ha1.build(circuit);
  auto ha2 = HalfAdder(ha1.sum(), this->carryIn);
  ha2.build(circuit);
  auto carryOut = OrGate(ha1.carry(), ha2.carry());
  carryOut.build(circuit);
  this->outputWord = { ha2.sum(), carryOut };
}

unsigned FullAdder::sum() { return (*this)[0]; }

unsigned FullAdder::carry() { return (*this)[1]; }

Adder::Adder(Word inputLeft, Word inputRight, unsigned carryIn)
: inputLeft(inputLeft), inputRight(inputRight), carryIn(carryIn) {}

void Adder::buildImpl(Circuit& circuit) {
  auto length = this->inputLeft.size();
  this->outputWord.resize(length + 1);
  auto carryBit = this->carryIn;
  for (unsigned i = 0; i < length; i++) {
    auto fa = FullAdder(this->inputLeft[i], this->inputRight[i], carryBit);
    fa.build(circuit);
    this->outputWord[i] = fa.sum();
    carryBit = fa.carry();
  }
  this->outputWord[length] = carryBit;
}

Word Adder::sum() {
  // Last bit (MSB) is reserved for carry.
  auto out = this->output();
  return Word(out.begin(), out.end() - 1);
}

unsigned Adder::carry() { return this->output().back(); }

EqChecker::EqChecker(Word inputLeft, Word inputRight)
: inputLeft(inputLeft), inputRight(inputRight) {
  assert (inputLeft.size() == inputRight.size());
}

void EqChecker::buildImpl(Circuit& circuit) {
  // REMINDER: the bit with highest index is
  // the most significant bit.
  auto bitEq = XnorGate(this->inputLeft, this->inputRight);
  bitEq.build(circuit);
  printf("D: bitEq built\n");
  auto bitEqOut = bitEq.output();

  auto length = this->inputLeft.size();
  // partialEq[i] = bitEqOut[i] and partialEq[i + 1]
  Word partialEq(length, -1);
  partialEq[length - 1] = bitEqOut[length - 1];
  for (unsigned i = 2; i <= length; i++) {
    auto idx = length - i;
    auto aggregator = AndGate(bitEqOut[idx], partialEq[idx + 1]);
    aggregator.build(circuit);
    partialEq[idx] = aggregator;
  }
  outputWord = { partialEq[0] };
  printf("D: EqChecker::buildImpl finished\n");
}

LtChecker::LtChecker(Word inputLeft, Word inputRight)
: inputLeft(inputLeft), inputRight(inputRight) {
  assert (inputLeft.size() == inputRight.size());
}

void LtChecker::buildImpl(Circuit& circuit) {
  // REMINDER: the bit with highest index is the most significant bit.
  // First step. Compare pairs of bits for LT and EQ.
  // We know LT(a, b) = ~a b,
  // so we need to compute ~a b.
  auto leftInv = Inverter(this->inputLeft);
  leftInv.build(circuit);
  auto bitEq = XnorGate(this->inputLeft, this->inputRight);
  auto bitLt = AndGate(leftInv, this->inputRight);
  bitEq.build(circuit);
  bitLt.build(circuit);
  auto bitEqOut = bitEq.output();
  auto bitLtOut = bitLt.output();

  auto length = this->inputLeft.size();
  auto partialEq = Word(length, -1);
  auto partialLt = Word(length, -1);
  partialEq[length - 1] = bitEqOut[length - 1];
  partialLt[length - 1] = bitLtOut[length - 1];
  for (unsigned i = 2; i <= length; i++) {
    unsigned idx = length - i;
    auto aggregatorEq = AndGate(bitEqOut[idx], partialEq[idx + 1]);
    aggregatorEq.build(circuit);
    partialEq[idx] = aggregatorEq;
    auto extenderLt = AndGate(bitLtOut[idx], partialEq[idx + 1]);
    extenderLt.build(circuit);
    auto aggregatorLt = OrGate(extenderLt, partialLt[idx + 1]);
    aggregatorLt.build(circuit);
    partialLt[idx] = aggregatorLt;
  }
  this->outputWord = { partialLt[0] };
}

Zero::Zero(unsigned source) : source(source) {}

void Zero::buildImpl(Circuit& circuit) {
  auto one = One(this->source);
  one.build(circuit);
  auto inv = Inverter(one);
  inv.build(circuit);
  this->outputWord = inv;
}

One::One(unsigned source) : source(source) {}

void One::buildImpl(Circuit& circuit) {
  auto inv = Inverter({ this->source });
  inv.build(circuit);
  auto out = circuit.addGate(this->source, inv);
  this->outputWord = { out };
}
