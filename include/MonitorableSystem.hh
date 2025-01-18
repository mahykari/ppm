#ifndef MONITORABLE_SYSTEM_HH
#define MONITORABLE_SYSTEM_HH

#include <vector>
#include "MathUtils.hh"
#include "Exceptions.hh"

class MonitorableSystem {
public:
  virtual void next() = 0;
  virtual const std::vector<bool>& data() = 0;
};

class SweepSystem : public MonitorableSystem {
public:
  unsigned n = 0;
  std::vector<bool> x = {0, 0, 0, 0};
  void next() override;
  const std::vector<bool>& data() override;
};

class JumpSweepSystem : public MonitorableSystem {
public:
  JumpSweepSystem(unsigned N);

  unsigned N;
  unsigned cntr = 0;
  std::vector<bool> x;
  void next() override;
  const std::vector<bool>& data() override;
};

class Timekeeper : public MonitorableSystem {
public:
  struct DoorUpdate {
    unsigned enteredA = 0;
    unsigned  exitedA = 0;
    unsigned enteredB = 0;
    unsigned  exitedB = 0;
  };

  Timekeeper(unsigned NDOORS, unsigned WORDLEN);
  Timekeeper();

  unsigned NDOORS;
  unsigned WORDLEN;
  unsigned cntr = 0;
  std::vector<DoorUpdate> x;
  std::vector<bool> dataVec;
  virtual void next() override;
  virtual const std::vector<bool>& data() override;
};

class TimekeeperPlus : public Timekeeper {
public:
  TimekeeperPlus(unsigned N_EX, unsigned N_IN, unsigned WORDLEN);
  TimekeeperPlus();

  unsigned N_EX, N_IN;
  void next() override;
};

#endif
