#include <iostream>
#include <zmq.h>
#include "BM.hh"
#include "LWY.hh"
#include "MathUtils.hh"
#include "MonitorableSystem.hh"
#include "Shake256YaoGarbler.hh"
#include "CommandLineInterface.hh"

namespace L = LWY;

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

int main(int argc, char* argv[]) {

  auto cli = CommandLineInterface(argc, argv);
  cli.parse();

  SetUp();

  auto messageHandler = MessageHandler(L::MONITOR_PORT, L::SYSTEM_PORT);

  auto params = cli.parameters;
  BigInt primeModulus = getSafePrime(params.securityParameter);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());

  // ONE-TIME MESSAGE:
  // System receives gateCount from Monitor,
  // formatted as a decimal number.
  unsigned gateCount = std::stoul(messageHandler.recv());
  printf("I: received gate count %d\n", gateCount);

  auto garbler = Shake256YaoGarbler();

  auto parameters = L::ParameterSet {
    // GateCount value from CLI is invalid;
    // only the value received from Monitor shall be used.
    .gateCount          = gateCount,
    .monitorStateLength = params.monitorStateLength,
    .systemStateLength  = params.systemStateLength,
    .group              = QuadraticResidueGroup(primeModulus),
    .garbler            = &garbler,
    .securityParameter  = params.securityParameter
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
