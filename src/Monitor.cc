#include <iostream>
#include "MathUtils.hh"
#include "MessageHandler.hh"
#include "BellareMicaliOTProtocol.hh"
#include "LiuEtAlMonitoringProtocol.hh"
#include "Circuit.hh"

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

namespace L = LiuEtAlMonitoringProtocol;
namespace B = BellareMicaliOTProtocol;

int main (int argc, char *argv[]) {
  SetUp();
  auto messageHandler = MessageHandler(L::SYSTEM_PORT, L::MONITOR_PORT);

  unsigned securityParameter = 80;
  auto primeModulus = BigInt(SAFE_PRIMES[securityParameter]);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());

  auto parameters = L::ParameterSet {
    // .gateCount = 3,
    // .monitorStateLength = 0,
    .gateCount = 6,
    .monitorStateLength = 1,
    .systemStateLength = 4,
    .group = QuadraticResidueGroup(primeModulus),
    .garbler = Sha512YaoGarbler(),
    .securityParameter = securityParameter
  };

  // auto circuit = Circuit(4, 1);
  // circuit.addGate(0, 1);
  // circuit.addGate(2, 3);
  // circuit.addGate(4, 5);
  auto circuit = Circuit(5, 2);
  circuit.addGate(1, 2); //  5: ~ (x0 x1)
  circuit.addGate(3, 4); //  6: ~ (x2 x3)
  circuit.addGate(5, 6); //  7: x0 x1 + x2 x3
  circuit.addGate(0, 7); //  8: ~ (m (x0 x1 + x2 x3))
  circuit.addGate(0, 0); //  9: ~ m
  circuit.addGate(8, 8); // 10: m (x0 x1 + x2 x3)
  auto monitorMemory = L::MonitorMemory {
    .circuit = &circuit
  };

  auto interface = L::MonitorInterface(
    &parameters, &monitorMemory, &messageHandler);

  interface.run();

  std::cout << std::string(80, '=') << '\n';
  std::cout << "TESTING BELLARE-MICALI OT PROTOCOL" << '\n';
  std::cout << std::string(80, '=') << '\n';

  auto parameters1 = B::ParameterSet {
    .group = QuadraticResidueGroup(primeModulus)
  };

  auto chooserMemory = B::ChooserMemory { .sigma = 1 };
  auto interface1 = B::ChooserInterface(
    &parameters1, &chooserMemory, &messageHandler);
  interface1.run();

  exit(EXIT_SUCCESS);
}
