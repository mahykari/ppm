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
  int id;
  Driver(int id);
  void drive(int gateId);
  // A driver can 'drive' multiple gates,
  // ID's of which are stored in `drivenGates`.
  std::vector<int> drivenGates;
};

typedef std::unique_ptr<Driver> DriverPtr;

class Gate : public Driver {
public:
  Gate(int id, int inputLeft, int inputRight);
  int inputLeft;
  int inputRight;
};

class Circuit {
public:
  Circuit(int inputLength, int outputLength);
  void addGate(int inputLeft, int inputRight);
  std::vector<Driver*> shuffle();
  // Method size() returns the number of *drivers* in the circuit,
  // i.e., this->drivers.size().
  int size();
private:
  // A particular gate can be picked with just an ID,
  // as all gates are NAND gates.
  int inputLength;
  int outputLength;
  int counter;
  std::vector<DriverPtr> drivers;
};

#endif
