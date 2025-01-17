#include <iostream>
#include <zmq.h>
#include "BM.hh"
#include "LWY.hh"
#include "Y.hh"
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

  switch (params.protocol) {
    case ProtocolType::YAO: {
      auto circuit = Circuit(
        params.monitorStateLength + params.systemStateLength,
        params.monitorStateLength + 1);

      auto monitorMemory = Y::SystemMemory {
        .circuit = &circuit,
        .system = cli.system.get(),
      };
      auto parameters = Y::ParameterSet {
        .gateCount          = gateCount,
        .monitorStateLength = params.monitorStateLength,
        .systemStateLength  = params.systemStateLength,
        .garbler            = &garbler,
        .securityParameter  = params.securityParameter
      };
      auto interface = Y::SystemInterface(
        &parameters, &monitorMemory, &messageHandler);
      interface.run();
      break;
    } case ProtocolType::LWY: {
      auto monitorMemory = L::SystemMemory {
        .system = cli.system.get(),
      };
      auto parameters = L::ParameterSet {
        .gateCount          = gateCount,
        .monitorStateLength = params.monitorStateLength,
        .systemStateLength  = params.systemStateLength,
        .group              = QuadraticResidueGroup(primeModulus),
        .garbler            = &garbler,
        .securityParameter  = params.securityParameter
      };
      auto interface = L::SystemInterface(
        &parameters, &monitorMemory, &messageHandler);
      interface.run();
      break;
    } default: {
      printf("E: unknown protocol\n");
      exit(EXIT_FAILURE);
    }
  }

  exit(EXIT_SUCCESS);
}
