#include <algorithm>
#include <random>
#include <assert.h>
#include "MathUtils.hh"
#include "Circuit.hh"

Driver::Driver(unsigned id) : id(id) {}

void Driver::drive(unsigned gateId) {
  this->drivenGates.push_back(gateId);
}

Gate::Gate(unsigned id, unsigned left, unsigned right)
  : Driver(id), inputLeft(left), inputRight(right) {}

Circuit::Circuit(unsigned inputLength, unsigned outputLength)
  : inputLength(inputLength), outputLength(outputLength)
{
  this->counter = inputLength;
  for (unsigned i = 0; i < this->inputLength; i++)
    this->drivers.push_back(std::make_unique<Driver>(i));
}

void Circuit::addGate(unsigned inputLeft, unsigned inputRight)
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
  for (unsigned i = 0; i < this->counter; i++)
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

std::vector<Driver*> Circuit::get() {
  std::vector<Driver*> drivers(this->counter, nullptr);
  for (unsigned i = 0; i < this->counter; i++)
    drivers[i] = this->drivers[i].get();
  return drivers;
}

unsigned Circuit::size() {
  assert (this->counter == this->drivers.size());
  return this->counter;
}
