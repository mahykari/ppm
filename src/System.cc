#include <iostream>
#include <zmq.h>
#include "BM.hh"
#include "LWY.hh"
#include "MathUtils.hh"
#include "MonitorableSystem.hh"
#include "CommandLineInterface.hh"

namespace L = LWY;

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

int main(int argc, char* argv[]) {

  auto cli = CommandLineInterface();
  cli.parse(argc, argv);

  SetUp();

  auto messageHandler = MessageHandler(L::MONITOR_PORT, L::SYSTEM_PORT);

  auto primeModulus = BigInt(SAFE_PRIMES[cli.securityParameter]);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());

  auto parameters = L::ParameterSet {
    .gateCount = cli.gateCount,
    .monitorStateLength = cli.monitorStateLength,
    .systemStateLength = cli.systemStateLength,
    .group = QuadraticResidueGroup(primeModulus),
    .garbler = Sha512YaoGarbler(),
    .securityParameter = cli.securityParameter
  };

  auto systemMemory = L::SystemMemory {
    .system = cli.system.get(),
  };

  auto interface = L::SystemInterface(
    &parameters, &systemMemory, &messageHandler);
  // printf("D: running system interface\n");
  interface.run();
  exit(EXIT_SUCCESS);
}
