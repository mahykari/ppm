#ifndef CIIRCUIT_HH
#define CIIRCUIT_HH

#include <vector>
#include <memory>

// A `Driver` is an entity that can drive a gate.
// This can be either a gate, or an input wire to the circuit.
// All drivers are managed inside a `Circuit`;
// so, they can refer to each other by their ID's.
class Driver {
public:
  unsigned id;
  Driver(unsigned id);
  void drive(unsigned gateId);
  // A driver can 'drive' multiple gates,
  // ID's of which are stored in `drivenGates`.
  std::vector<unsigned> drivenGates;
};

typedef std::unique_ptr<Driver> DriverPtr;

class Gate : public Driver {
public:
  Gate(unsigned id, unsigned inputLeft, unsigned inputRight);
  unsigned inputLeft;
  unsigned inputRight;
};

class Circuit {
public:
  Circuit(unsigned inputLength, unsigned outputLength);
  void addGate(unsigned inputLeft, unsigned inputRight);
  std::vector<Driver*> shuffle();
  std::vector<Driver*> get();
  // Method size() returns the number of *drivers* in the circuit,
  // i.e., this->drivers.size().
  unsigned size();
private:
  // A particular gate can be picked with just an ID,
  // as all gates are NAND gates.
  unsigned inputLength;
  unsigned outputLength;
  unsigned counter;
  std::vector<DriverPtr> drivers;
};

#endif
