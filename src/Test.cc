#include <cassert>
#include <iostream>
#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"
#include "Sha512YaoGarbler.hh"
#include "StringUtils.hh"

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

int main() {
  testQR();
  cout << string(70, '-') << '\n';
  testGarbler();
}
