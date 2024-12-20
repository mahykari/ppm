#ifndef MODULE_HH
#define MODULE_HH

#include <vector>
#include "Circuit.hh"

// A *word* is a vector of wires.
// Each wire is represented as the identifier of its driver.
typedef std::vector<unsigned> Word;
typedef std::vector<Word> WordVector;

class Module {
public:
  void build(Circuit&);
  virtual void buildImpl(Circuit&) = 0;
  Word output();
  operator Word();
  operator unsigned();
  unsigned operator[](unsigned);
protected:
  Word outputWord;
};

class Broadcaster : public Module {
public:
  Broadcaster(unsigned input, unsigned count);
  void buildImpl(Circuit&) override;
private:
  unsigned input;
  unsigned count;
};

class Inverter : public Module {
public:
  Inverter(Word input);
  void buildImpl(Circuit&) override;
private:
  Word input;
};

class Identity : public Module {
public:
  Identity(Word input);
  void buildImpl(Circuit&) override;
private:
  Word input;
};

class BinaryGate : public Module {
public:
  BinaryGate(Word inputLeft, Word inputRight);
  BinaryGate(unsigned inputLeft, unsigned inputRight);
  void buildImpl(Circuit&) = 0;
protected:
  Word inputLeft;
  Word inputRight;
};

class AndGate : public BinaryGate {
public:
  using BinaryGate::BinaryGate;
  void buildImpl(Circuit&) override;
};

class OrGate : public BinaryGate {
public:
  using BinaryGate::BinaryGate;
  void buildImpl(Circuit&) override;
};

class XorGate : public BinaryGate {
public:
  using BinaryGate::BinaryGate;
  void buildImpl(Circuit&) override;
};

class XnorGate : public BinaryGate {
public:
  using BinaryGate::BinaryGate;
  void buildImpl(Circuit&) override;
};

class Selector : public Module {
public:
  Selector(WordVector input, Word select);
  void buildImpl(Circuit&) override;
private:
  WordVector input;
  Word select;
};

class HalfAdder : public Module {
public:
  HalfAdder(unsigned inputLeft, unsigned inputRight);
  void buildImpl(Circuit&) override;
  unsigned sum();
  unsigned carry();
private:
  unsigned inputLeft;
  unsigned inputRight;
};

class FullAdder : public Module {
public:
  FullAdder(unsigned inputLeft, unsigned inputRight, unsigned carryIn);
  void buildImpl(Circuit&) override;
  unsigned sum();
  unsigned carry();
private:
  unsigned inputLeft;
  unsigned inputRight;
  unsigned carryIn;
};

class Adder : public Module {
public:
  Adder(Word inputLeft, Word inputRight, unsigned carryIn);
  void buildImpl(Circuit&) override;
  Word sum();
  unsigned carry();
protected:
  Word inputLeft;
  Word inputRight;
  unsigned carryIn;
};

class EqChecker : public Module {
public:
  EqChecker(Word inputLeft, Word inputRight);
  void buildImpl(Circuit&) override;
private:
  Word inputLeft;
  Word inputRight;
};

class LtChecker : public Module {
public:
  LtChecker(Word inputLeft, Word inputRight);
  void buildImpl(Circuit&) override;
private:
  Word inputLeft;
  Word inputRight;
};

class Zero : public Module {
public:
  Zero(unsigned source);
  void buildImpl(Circuit&) override;
private:
  unsigned source;
};

class One : public Module {
public:
  One(unsigned source);
  void buildImpl(Circuit&) override;
private:
  unsigned source;
};

class Negator : public Module {
public:
  Negator(Word input);
  void buildImpl(Circuit&) override;
private:
  Word input;
};

typedef std::pair<unsigned, Word> RegisterUpdate;
typedef std::vector<RegisterUpdate> RegisterUpdateVec;

class TransitionSystem : public Module {
public:
  TransitionSystem(
    WordVector registers,
    std::vector<RegisterUpdateVec> registerUpdates);
  void buildImpl(Circuit&) override;
private:
  WordVector registers;
  std::vector<RegisterUpdateVec> registerUpdates;
};

#endif
