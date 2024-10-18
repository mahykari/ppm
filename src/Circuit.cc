#include <algorithm>
#include <random>
#include <assert.h>
#include "MathUtils.hh"
#include "Circuit.hh"

Driver::Driver(int id) : id(id) {}

void Driver::drive(int gateId) {
  this->drivenGates.push_back(gateId);
}

Gate::Gate(int id, int left, int right)
  : Driver(id), inputLeft(left), inputRight(right) {}

Circuit::Circuit(int inputLength, int outputLength)
  : inputLength(inputLength), outputLength(outputLength)
{
  this->counter = inputLength;
  for (int i = 0; i < this->inputLength; i++)
    this->drivers.push_back(std::make_unique<Driver>(i));
}

void Circuit::addGate(int inputLeft, int inputRight)
{
  DriverPtr gate = std::make_unique<Gate>(
    this->counter, inputLeft, inputRight);
  this->drivers[inputLeft]->drive(gate->id);
  this->drivers[inputRight]->drive(gate->id);
  this->counter++;
  this->drivers.push_back(std::move(gate));
}

std::vector<Driver*> Circuit::shuffle() {
  std::vector<Driver*> shuffledGates(this->counter, nullptr);
  for (int i = 0; i < this->counter; i++)
    shuffledGates[i] = this->drivers[i].get();
  assert (this->inputLength + this->outputLength < this->counter);
  // Only *some* gates need to be shuffled;
  // inputs and gates driving a circuit output wire
  // should stay in place. So, we start shuffling
  // from the first until the last 'internal' 'gate'.
  auto seed = timeBasedSeed();
  std::shuffle(
    shuffledGates.begin() + this->inputLength,
    shuffledGates.end() - this->outputLength,
    std::default_random_engine(seed));
  return shuffledGates;
}

int Circuit::size() {
  return this->counter - this->inputLength;
}
