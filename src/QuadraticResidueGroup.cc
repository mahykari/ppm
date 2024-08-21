#include <memory>
#include "QuadraticResidueGroup.hh"
#include "BigIntegerElement.hh"

using QRGroup = QuadraticResidueGroup;
using BigInt = BigIntegerElement;
template <typename T> using UPtr = std::unique_ptr<T>;

QRGroup::QuadraticResidueGroup(mpz_t p) 
  : CyclicGroup(GroupType::QUADRATIC_RESIDUE)
{
  mpz_init(this->primeModulus);
  mpz_set(this->primeModulus, p);

  mpz_init(this->smallPrime);
  mpz_sub_ui(this->smallPrime, p, 1);
  mpz_divexact_ui(this->smallPrime, this->smallPrime, 2);
}

QRGroup::~QuadraticResidueGroup() {
  mpz_clear(this->primeModulus);
}

UPtr<Element> QRGroup::randomGenerator() {
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
  gmp_randseed_ui(state, time(NULL));

  mpz_t exp;
  mpz_init(exp);
  // Exponent must be in {1, ..., p1 - 1}.
  mpz_urandomm(exp, state, this->smallPrime);
  while (mpz_sgn(exp) == 0)
    mpz_urandomm(exp, state, this->smallPrime);
  mpz_powm(g, g, exp, this->primeModulus);
  return std::make_unique<BigInt>(this, g);
}

UPtr<Element> QRGroup::mul(Element* a, Element* b) {
  mpz_t rop;
  mpz_init(rop);
  auto a_ = dynamic_cast<BigInt*>(a);
  auto b_ = dynamic_cast<BigInt*>(b);
  mpz_mul(rop, a_->n, b_->n);
  mpz_mod(rop, rop, this->primeModulus);
  return std::make_unique<BigInt>(this, rop);
}

UPtr<Element> QRGroup::exp(Element* a, mpz_t n) {
  mpz_t rop;
  mpz_init(rop);
  auto a_ = dynamic_cast<BigInt*>(a);
  mpz_powm(rop, a_->n, n, this->primeModulus);
  return std::make_unique<BigInt>(this, rop);
}
