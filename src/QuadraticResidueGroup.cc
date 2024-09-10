#include <memory>
#include "Utils.hh"
#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"

using QRGroup = QuadraticResidueGroup;

QRGroup::QuadraticResidueGroup(mpz_t p) {
  mpz_init(this->primeModulus);
  mpz_set(this->primeModulus, p);

  mpz_init(this->smallPrime);
  mpz_sub_ui(this->smallPrime, p, 1);
  mpz_divexact_ui(this->smallPrime, this->smallPrime, 2);
}

QRGroup::~QuadraticResidueGroup() {
  mpz_clear(this->primeModulus);
  mpz_clear(this->smallPrime);
}

BigInt QRGroup::randomGenerator() {
  // For any safe prime p,
  // 4 is a generator of the group of quadratic residues modulo p.
  // So, to get a random generator,
  // we can return a random exponentiation of 4 modulo p.
  mpz_t g;
  mpz_init(g);
  mpz_set_ui(g, 4);

  gmp_randstate_t state;
  // TODO: use a better initialization;
  // Mersenne Twister is not cryptographically secure.
  gmp_randinit_mt(state);
  gmp_randseed_ui(state, timeBasedSeed());

  mpz_t exp;
  mpz_init(exp);
  // Exponent must be in {1, ..., p1 - 1}.
  mpz_urandomm(exp, state, this->smallPrime);
  while (mpz_sgn(exp) == 0)
    mpz_urandomm(exp, state, this->smallPrime);
  mpz_powm(g, g, exp, this->primeModulus);
  return {g};
}

BigInt QRGroup::mul(const BigInt& a, const BigInt& b) {
  mpz_t rop;
  mpz_init(rop);
  mpz_mul(rop, a.n, b.n);
  mpz_mod(rop, rop, this->primeModulus);
  return {rop};
}

BigInt QRGroup::exp(const BigInt& a, mpz_t n) {
  mpz_t rop;
  mpz_init(rop);
  mpz_powm(rop, a.n, n, this->primeModulus);
  return {rop};
}

std::string QRGroup::toString() const {
  mpz_t p;
  mpz_init_set(p, this->primeModulus);
  return "QuadraticResidueGroup(" 
    + BigInt(p).toString() + ")";
}
