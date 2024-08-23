#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"

using namespace std;

// TODO: integrate a testing framework.
void test_qr() {
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

int main() {
  test_qr();
}
