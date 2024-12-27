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
  if (NDOORS != 3 or WORDLEN != 10)
    throw ConfigBehaviorUndefined();
  x[0].enteredA = 5;
  x[1].exitedA = 3;
  x[2].enteredB = 7;
  return;
}

const std::vector<bool>& Timekeeper::data() {
  std::vector<std::vector<bool>> doors;
  for (auto& door : x) {
    doors.push_back(
      flatten(std::vector<std::vector<bool>> {
        toBinary(door.enteredA, WORDLEN),
        toBinary(door. exitedA, WORDLEN),
        toBinary(door.enteredB, WORDLEN),
        toBinary(door. exitedB, WORDLEN)
      } ) );
  }
  dataVec = flatten(doors);
  return dataVec;
}
