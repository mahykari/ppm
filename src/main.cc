#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"
#include "Sha512YaoGarbler.hh"

using namespace std;

// TODO: integrate a testing framework.
void testQR() {
  mpz_t p;
  mpz_init(p);
  mpz_set_ui(p, 11);
  cout << "Using prime modulus p="
    <<  mpz_get_str(NULL, 10, p) << '\n';
  auto group = QuadraticResidueGroup(p);
  cout << "Using " << group.toString() << '\n';
  auto g = group.randomGenerator();
  printf("Using generator g = %s\n", g.toString().c_str());
  
  for (int i = 1; i <= 5; i++) {
    mpz_t exp;
    mpz_init_set_ui(exp, i);
    auto h = group.exp(g, exp);
    printf("g^%d = %s\n", i, h.toString().c_str());
  } 

  mpz_t a, b, result;
  mpz_inits(a, b, result, NULL);
  mpz_set_ui(a, 9);
  mpz_set_ui(b, 3);

  auto a_ = BigInt(a);
  auto b_ = BigInt(b);
  auto c_ = group.mul(a, b);
  printf("Using a=9, b=3, a*b = %s\n", c_.toString().c_str());
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

  auto gate = garbler.enc(leftp, rightp, outp);
  printf(
    "- gate=[%s,%s,%s,%s]\n",
    gate[0].c_str(), gate[1].c_str(),
    gate[2].c_str(), gate[3].c_str()
  );

  auto label = garbler.dec(leftp[1], rightp[0], gate);
  printf("- label=%s\n", label.c_str());
}

int main() {
  testQR();
  cout << string(80, '-') << '\n';
  testGarbler();
}
