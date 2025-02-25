#include "MonitorableSystem.hh"

void SweepSystem::next() {
  x[n % 4] = 1 - x[n % 4];
  n++;
}

const std::vector<bool>& SweepSystem::data() {
  return x;
}

JumpSweepSystem::JumpSweepSystem(unsigned N) : N(N) {
  x = std::vector<bool> (N, 0);
}

void JumpSweepSystem::next()  {
  x[cntr] = 1 - x[cntr];
  cntr += 2;
  if (cntr == N)
    cntr = 1;
  if (cntr == N + 1)
    cntr = 0;
}

const std::vector<bool>& JumpSweepSystem::data() {
  return x;
}

Timekeeper::Timekeeper(unsigned NDOORS, unsigned WORDLEN)
: NDOORS(NDOORS), WORDLEN(WORDLEN) {
  x.resize(NDOORS);
}

Timekeeper::Timekeeper() : Timekeeper(3, 10) {}

void Timekeeper::next() {
  if (NDOORS < 3 or WORDLEN < 8)
    throw ConfigBehaviorUndefined();
  cntr++;
  if (cntr < 2) {
    x[0].enteredA = 1;

    x[1].exitedA = 1;
    x[1].enteredB = 1;

    x[2].exitedB = 1;
  } else {
    x[0].enteredA = 5;
    x[0].exitedA = 3;
    x[0].enteredB = 7;
  }
}

const std::vector<bool>& Timekeeper::data() {
  std::vector<std::vector<bool>> doors;
  for (auto& door : x) {
    doors.push_back(
      flatten(std::vector<std::vector<bool>> {
        toBinary(door. exitedB, WORDLEN),
        toBinary(door.enteredB, WORDLEN),
        toBinary(door. exitedA, WORDLEN),
        toBinary(door.enteredA, WORDLEN),
      } ) );
  }
  dataVec = flatten(doors);
  return dataVec;
}

TimekeeperPlus::TimekeeperPlus(unsigned N_EX, unsigned N_IN, unsigned WORDLEN)
: Timekeeper(N_EX + N_IN, WORDLEN), N_EX(N_EX), N_IN(N_IN) {}

TimekeeperPlus::TimekeeperPlus() : TimekeeperPlus(3, 3, 10) {}

void TimekeeperPlus::next() {
  if (N_EX < 3 or WORDLEN < 8)
    throw ConfigBehaviorUndefined();

  cntr++;
  // if (cntr < 2) {
  if (cntr < 1) {
    // External doors
    x[0].enteredA = 1;

    x[1].exitedA = 1;
    x[1].enteredB = 1;

    x[2].exitedB = 1;

    // Internal doors
    // Note that there might be no internal doors.
    if (N_IN > 0) {
      x[N_EX].enteredA = 2;
      x[N_EX].exitedA = 1;

      x[N_EX + 1].enteredB = 3;
      x[N_EX + 1].exitedB = 2;
    }
  } else {
    // External doors
    x[0].enteredA = 5;
    x[1].exitedA = 3;
    x[2].enteredB = 7;

    // Internal doors
    // Note that there might be no internal doors.
    if (N_IN > 0) {
      x[N_EX].enteredA = 4;
      x[N_EX].exitedA = 3;

      x[N_EX + 1].enteredB = 6;
      x[N_EX + 1].exitedB = 5;
    }
  }
}

Locks::Locks(unsigned NLOCKS) : NLOCKS(NLOCKS) {
  x.resize(NLOCKS);
}

void Locks::next() {
  if (NLOCKS < 1)
    throw ConfigBehaviorUndefined();
  cntr++;
  x[0].skip = false;
  x[0].lock = true;
}

const std::vector<bool>& Locks::data() {
  std::vector<std::vector<bool>> locks;
  for (auto& lock : x) {
    locks.push_back(
      flatten(std::vector<std::vector<bool>> {
        toBinary(lock.lock, 1),
        toBinary(lock.skip, 1),
      } ) );
  }
  dataVec = flatten(locks);
  return dataVec;
}
