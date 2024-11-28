#include <cassert>
#include <iostream>
#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"
#include "Sha512YaoGarbler.hh"
#include "StringUtils.hh"
#include "Module.hh"

using namespace std;

// TODO: integrate a testing framework.
void testQR() {
  BigInt p(11);
  cout << "Using prime modulus p="
    <<  toString(p) << '\n';
  auto group = QuadraticResidueGroup(p);
  cout << "Using " << toString(group) << '\n';
  auto g = group.randomGenerator();
  cout << "Using generator g = " << toString(g) << '\n';

  for (int i = 1; i <= 5; i++) {
    BigInt e(i);
    auto h = group.exp(g, e);
    printf("g^%d = %s\n", i, toString(h).c_str());
  }
  BigInt a(9);
  BigInt b(3);
  BigInt result = group.mul(a, b);
  printf(
    "Using a=9, b=3, a*b = %s\n",
    toString(result).c_str());
}

void testGarbler() {
  auto garbler = Sha512YaoGarbler();
  LabelPair leftp = {"012", "345"};
  LabelPair rightp = {"678", "9ab"};
  LabelPair outp = {"cde", "f01"};

  printf(
    "Using leftp=[%s,%s], rightp=[%s,%s], outp=[%s,%s]:\n",
    leftp[0].c_str(), leftp[1].c_str(),
    rightp[0].c_str(), rightp[1].c_str(),
    outp[0].c_str(), outp[1].c_str()
  );

  auto gate =
    garbler.enc(leftp, rightp, outp);
  printf(
    "- gate=[%s,%s,%s,%s]\n",
    gate[0].c_str(), gate[1].c_str(),
    gate[2].c_str(), gate[3].c_str()
  );

  auto label =
    garbler.dec(leftp[1], rightp[0], gate);
  assert(label == outp[1]);
  printf("- label=%s\n", label.c_str());
}

struct IncGenerator {
  static unsigned current_;
  IncGenerator() = default;
  unsigned operator()() { return current_++; }
};

unsigned IncGenerator::current_ = 0;

void testModule() {
  vector<bool> valA = {0, 1, 0, 1, 0, 1, 1, 0}; // 106
  vector<bool> valB = {1, 1, 1, 1, 0, 0, 1, 0}; //  79
  vector<bool> valC = {1, 1, 1, 1, 1, 1, 0, 1}; // 191

  auto wordLength = valA.size();
  auto circuit = Circuit(3 * wordLength, 2);
  Word inA(wordLength);
  Word inB(wordLength);
  Word inC(wordLength);
  IncGenerator gen;
  generate(inA.begin(), inA.end(), gen);
  generate(inB.begin(), inB.end(), gen);
  generate(inC.begin(), inC.end(), gen);

  printf("D: circuit size before build = %u\n", circuit.size());
  auto zero = Zero(0);
  zero.build(circuit);
  printf("D: circuit size after zero = %u\n", circuit.size());
  auto adder = Adder(inA, inB, zero);
  adder.build(circuit);
  printf("D: adder built\n");
  printf("D: circuit size after adder = %u\n", circuit.size());
  auto eq = EqChecker(adder.sum(), inC);
  auto lt = LtChecker(adder.sum(), inC);
  eq.build(circuit);
  printf("D: eq built\n");
  printf("D: circuit size after eq = %u\n", circuit.size());
  lt.build(circuit);
  printf("D: lt built\n");
  printf("D: circuit size after lt = %u\n", circuit.size());
  circuit.updateOutputs({eq, lt});
  auto vals = valA;
  vals.insert(vals.end(), valB.begin(), valB.end());
  vals.insert(vals.end(), valC.begin(), valC.end());
  auto output = circuit.evaluate(vals);
  cout << "output: { ";
  for (auto o : output)
    cout << o << ' ';
  cout << "}\n";
  assert (output == (vector<bool> {0, 1}));
  cout << "circuit size: " << circuit.size() << '\n';
}

void sep() {
  cout << string(70, '-') << '\n';
}

int main() {
  testQR();
  sep();
  testGarbler();
  sep();
  testModule();
}
