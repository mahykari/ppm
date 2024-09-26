from sympy import isprime, nextprime


def ok(p):
  return isprime(2 * p + 1)


if __name__ == '__main__':
  limit = 512
  safeprimes = []
  lb, ub = 1, 2
  for i in range(limit + 1):
    # We assume p is always a prime.
    # We only check if 2p + 1 is also a prime.
    p = nextprime(lb)
    while not ok(p) and p < ub:
      p = nextprime(p)
    if not ok(p) or p >= ub:
      safeprimes.append(-1)
    else:
      safeprimes.append(p)
    print(i, safeprimes[-1])
    lb, ub = ub, 2 * ub
