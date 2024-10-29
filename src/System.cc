#include <iostream>
#include <zmq.h>
#include "LiuEtAlMonitoringProtocol.hh"
#include "MathUtils.hh"

namespace L = LiuEtAlMonitoringProtocol;

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
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

int main() {
  SetUp();
  auto messageHandler = MessageHandler(L::MONITOR_PORT, L::SYSTEM_PORT);

  unsigned securityParameter = 80;
  auto primeModulus = BigInt(SAFE_PRIMES[securityParameter]);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());
  auto parameters = L::ParameterSet {
    .gateCount = 3,
    .monitorStateLength = 0,
    .systemStateLength = 4,
    .group = QuadraticResidueGroup(primeModulus),
    .garbler = Sha512YaoGarbler(),
    .securityParameter = securityParameter
  };

  auto system = SweepSystem();
  auto systemMemory = L::SystemMemory {
    .system = system
  };

  auto interface = L::SystemInterface(
    parameters, systemMemory, messageHandler);

  interface.run();
  exit(EXIT_SUCCESS);
}
