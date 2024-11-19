#ifndef MONITORABLE_SYSTEM_HH
#define MONITORABLE_SYSTEM_HH

#include <vector>

class MonitorableSystem {
public:
  virtual void next() = 0;
  virtual const std::vector<bool>& data() = 0;
};

class SweepSystem : public MonitorableSystem {
public:
  unsigned n = 0;
  std::vector<bool> x = {0, 0, 0, 0};
  void next() override {
    x[n % 4] = 1 - x[n % 4];
    n++;
  }
  const std::vector<bool>& data() override {
    return x;
  }
};

class JumpSweepSystem : public MonitorableSystem {
public:
  JumpSweepSystem(unsigned N) : N(N) {
    x = std::vector<bool> (N, 0);
  }

  unsigned N;
  unsigned cntr = 0;
  std::vector<bool> x;
  void next() override {
    x[cntr] = 1 - x[cntr];
    cntr += 2;
    if (cntr == N)
      cntr = 1;
    if (cntr == N + 1)
      cntr = 0;
  }
  const std::vector<bool>& data() override {
    return x;
  }
};

#endif
