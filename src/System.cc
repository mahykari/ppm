#include <iostream>
#include <zmq.h>
#include "LiuEtAlMonitoringProtocol.hh"
#include "BellareMicaliOTProtocol.hh"
#include "MathUtils.hh"

namespace L = LiuEtAlMonitoringProtocol;
namespace B = BellareMicaliOTProtocol;

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
    // .gateCount = 3,
    // .monitorStateLength = 0,
    .gateCount = 6,
    .monitorStateLength = 1,
    .systemStateLength = 4,
    .group = QuadraticResidueGroup(primeModulus),
    .garbler = Sha512YaoGarbler(),
    .securityParameter = securityParameter
  };

  auto system = SweepSystem();
  auto systemMemory = L::SystemMemory {
    .system = &system
  };

  auto interface = L::SystemInterface(
    &parameters, &systemMemory, &messageHandler);
  printf("D: running system interface\n");
  interface.run();

  std::cout << std::string(80, '=') << '\n';
  std::cout << "TESTING BELLARE-MICALI OT PROTOCOL" << '\n';
  std::cout << std::string(80, '=') << '\n';

  auto parameters1 = B::ParameterSet {
    .group = QuadraticResidueGroup(primeModulus)
  };

  auto senderMemory = B::SenderMemory {
    .messages = {"12345", "67890"},
  };

  auto interface1 = B::SenderInterface(
    &parameters1, &senderMemory, &messageHandler);

  interface1.run();

  exit(EXIT_SUCCESS);
}
