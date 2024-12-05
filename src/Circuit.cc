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
    this->inputs.push_back(std::make_unique<Driver>(i));
}

unsigned Circuit::addGate(unsigned inputLeft, unsigned inputRight)
{
  DriverPtr gate = std::make_unique<Gate>(
    this->counter, inputLeft, inputRight);
  this->findDriver(inputLeft)->drive(gate->id);
  this->findDriver(inputRight)->drive(gate->id);
  this->internals.push_back(std::move(gate));
  return this->counter++;
}

std::vector<Driver*> Circuit::shuffle() {
  auto shuffledGates = this->get();
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
  for (auto& input : this->inputs)
    drivers.push_back(input.get());
  for (auto& gate : this->internals)
    drivers.push_back(gate.get());
  for (auto& output : this->outputs)
    drivers.push_back(output.get());
  return drivers;
}

void Circuit::updateOutputs(Word outputIds) {
  assert (outputIds.size() == this->outputLength);
  // ASSUMPTION: outputIds are sorted in ascending order.
  // ASSUMPTION: internals are sorted in ascending order of their ID's.
  // printf("D: internals = { ");
  // for (auto& i : this->internals)
  //   printf("%u ", i->id);
  // printf("}\n");
  auto idsIt = outputIds.begin();
  auto internalsIt = this->internals.begin();
  while (idsIt != outputIds.end()) {
    if (internalsIt->get()->id == *idsIt) {
      this->outputs.push_back(std::move(*internalsIt));
      idsIt++;
    }
    internalsIt++;
  }
  printf("D: outputs.size() = %lu\n", this->outputs.size());

  auto removeIt = std::remove_if(
    this->internals.begin(), this->internals.end(),
    [] (const DriverPtr& driver) { return driver == nullptr; });
  this->internals.erase(removeIt, this->internals.end());
  assert (this->outputs.size() == this->outputLength);
  assert (
    this->internals.size() + this->inputs.size()
    == this->counter - this->outputLength);
}

unsigned Circuit::size() {
  return this->counter;
}

ValueWord Circuit::evaluate(ValueWord input) {
  Word probed;
  for (auto& o : this->outputs)
    probed.push_back(o->id);
  auto circuitOutput = this->probe(input, probed);
  printf("D: circuitOutput.size() = %lu\n", circuitOutput.size());
  assert (circuitOutput.size() == this->outputLength);
  return circuitOutput;
}

ValueWord Circuit::probe(ValueWord input, Word probed) {
  auto driverVals = this->evaluateInternal(input);
  ValueWord probedVals;
  for (auto p : probed)
    probedVals.push_back(driverVals[p]);
  return probedVals;
}

Driver* Circuit::findDriver(unsigned id) {
  // printf("D: -- findDriver(%u) --\n", id);
  assert (id < this->counter);
  if (id < this->inputLength)
    return this->inputs[id].get();
  auto foundDriver = std::find_if(
    this->internals.begin(), this->internals.end(),
    [id] (const DriverPtr& driver) { return driver->id == id; });
  if (foundDriver != this->internals.end())
    return foundDriver->get();

  foundDriver = std::find_if(
    this->outputs.begin(), this->outputs.end(),
    [id] (const DriverPtr& driver) { return driver->id == id; });
  // If a driver is not an input or an internal gate,
  // it must be an output gate.
  assert (foundDriver != this->outputs.end());
  return foundDriver->get();
}

ValueWord Circuit::evaluateInternal(ValueWord input) {
  assert (input.size() == this->inputLength);
  ValueWord driverVals (this->counter, false);
  auto evalGate = [&] (Gate* gate) {
    auto left = driverVals[gate->inputLeft];
    auto right = driverVals[gate->inputRight];
    return !(left and right);
  };
  for (unsigned i = 0; i < this->inputLength; i++)
    driverVals[i] = input[i];
  for (auto& i : this->internals)
    driverVals[i->id] = evalGate( static_cast<Gate*> ( i.get() ) );
  printf("D: internals evaluated\n");

  for (auto& o : this->outputs)
    driverVals[o->id] = evalGate( static_cast<Gate*> ( o.get() ) );
  printf("D: outputs evaluated\n");
  return driverVals;
}
