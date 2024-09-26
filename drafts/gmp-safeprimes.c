#include <stdbool.h>
#include <stdio.h>
#include <gmp.h>

#define N_SAFEPRIMES 512

mpz_t safeprimes[N_SAFEPRIMES + 5];

bool ok(mpz_t p) {
  /* From GMP documentation for
  mpz_probab_prime_p(const mpz_t n, int reps):
  > ... Return 2 if n is definitely prime,
  > return 1 if n is probably prime (without being certain),
  > or return 0 if n is definitely non-prime. */
  return mpz_probab_prime_p(p, 40) >= 1;
}

int main() {
  for (int i = 0; i <= N_SAFEPRIMES; i++)
    mpz_init(safeprimes[i]);

  mpz_t lb, ub;
  mpz_init_set_ui(lb, 1);
  mpz_init_set_ui(ub, 2);
  for (int i = 0; i <= N_SAFEPRIMES; i++) {
    mpz_t p;
    mpz_init(p);
    mpz_nextprime(p, lb);
    while (!ok(p) && mpz_cmp(p, ub) < 0)
      mpz_nextprime(p, p);
    if (!ok(p) || mpz_cmp(p, ub) >= 0)
      mpz_set_si(safeprimes[i], -1);
    else
      mpz_set(safeprimes[i], p);
    printf("%d %s\n", i, mpz_get_str(NULL, 10, safeprimes[i]));
    mpz_set(lb, ub);
    mpz_mul_ui(ub, ub, 2);
  }
}
