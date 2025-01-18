#include <iostream>
#include "MathUtils.hh"
#include "MessageHandler.hh"
#include "BM.hh"
#include "LWY.hh"
#include "Y.hh"
#include "Circuit.hh"
#include "Shake256YaoGarbler.hh"
#include "CommandLineInterface.hh"
#include "SpecToCircuitConverter.hh"

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

namespace L = LWY;

int main (int argc, char *argv[]) {
  auto cli = CommandLineInterface(argc, argv);
  cli.parse();

  YosysConverter converter(cli.specFileName);
  auto circuit = converter.convert();

  SetUp();
  auto messageHandler = MessageHandler(L::SYSTEM_PORT, L::MONITOR_PORT);

  auto params = cli.parameters;
  BigInt primeModulus = getSafePrime(params.securityParameter);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());

  auto gateCount =
    circuit.size() - (params.monitorStateLength + params.systemStateLength);

  // ONE-TIME MESSAGE:
  // Monitor sends gateCount to System.
  messageHandler.send(std::to_string(gateCount));

  auto garbler = Shake256YaoGarbler();

  switch (params.protocol) {
    case ProtocolType::YAO: {
      auto monitorMemory = Y::MonitorMemory {
        .circuit = &circuit
      };
      auto parameters = Y::ParameterSet {
        .gateCount          = gateCount,
        .monitorStateLength = params.monitorStateLength,
        .systemStateLength  = params.systemStateLength,
        .garbler            = &garbler,
        .securityParameter  = params.securityParameter
      };
      auto interface = Y::MonitorInterface(
        &parameters, &monitorMemory, &messageHandler);
      interface.run();
      break;
    } case ProtocolType::LWY: {
      auto monitorMemory = L::MonitorMemory {
        .circuit = &circuit
      };
      auto parameters = L::ParameterSet {
        .gateCount          = gateCount,
        .monitorStateLength = params.monitorStateLength,
        .systemStateLength  = params.systemStateLength,
        .group              = QuadraticResidueGroup(primeModulus),
        .garbler            = &garbler,
        .securityParameter  = params.securityParameter
      };
      auto interface = L::MonitorInterface(
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
