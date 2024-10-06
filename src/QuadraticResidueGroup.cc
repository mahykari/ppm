#include <memory>
#include "MathUtils.hh"
#include "QuadraticResidueGroup.hh"
#include "BigInt.hh"

using QRGroup = QuadraticResidueGroup;

QRGroup::QuadraticResidueGroup(BigInt p)
  : primeModulus(p), smallPrime((p - 1) / 2) {}

QRGroup::~QuadraticResidueGroup() {}

BigInt QRGroup::randomGenerator() {
  // For any safe prime p,
  // 4 is a generator of the group of quadratic residues modulo p.
  // So, to get a random generator,
  // we can return a random exponentiation of 4 modulo p.
  BigInt g(4);

  gmp_randstate_t state;
  // TODO: use a better initialization;
  // Mersenne Twister is not cryptographically secure.
  gmp_randinit_mt(state);
  gmp_randseed_ui(state, timeBasedSeed());

  auto exp = this->randomExponent();
  mpz_powm(
    g.get_mpz_t(),
    g.get_mpz_t(),
    exp.get_mpz_t(),
    this->primeModulus.get_mpz_t());
  return g;
}

BigInt QRGroup::mul(const BigInt& a, const BigInt& b) {
  return (a * b) % this->primeModulus;
}

BigInt QRGroup::exp(const BigInt& a, const BigInt& n) {
  BigInt rop;
  mpz_powm(
    rop.get_mpz_t(),
    a.get_mpz_t(),
    n.get_mpz_t(),
    this->primeModulus.get_mpz_t());
  return rop;
}

BigInt QuadraticResidueGroup::inv(const BigInt& a) {
  BigInt rop;
  mpz_invert(
    rop.get_mpz_t(),
    a.get_mpz_t(),
    this->primeModulus.get_mpz_t());
  return rop;
}

BigInt QuadraticResidueGroup::randomExponent() {
  gmp_randstate_t state;
  gmp_randinit_mt(state);
  gmp_randseed_ui(state, timeBasedSeed());

  BigInt exp;
  // Exponent must be in {1, ..., p1 - 1}.
  BigInt upperBound = this->smallPrime;
  mpz_urandomm(
    exp.get_mpz_t(), state, upperBound.get_mpz_t());
  while (exp == 0)
    mpz_urandomm(
      exp.get_mpz_t(), state, upperBound.get_mpz_t());
  return exp;
}

BigInt QuadraticResidueGroup::order() {
  return this->smallPrime;
}
