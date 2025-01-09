#include <iostream>
#include "MathUtils.hh"
#include "MessageHandler.hh"
#include "BM.hh"
#include "LWY.hh"
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

  auto monitorMemory = L::MonitorMemory {
    .circuit = &circuit
  };

  auto gateCount =
    circuit.size() - (params.monitorStateLength + params.systemStateLength);

  // ONE-TIME MESSAGE:
  // Monitor sends gateCount to System.
  messageHandler.send(std::to_string(gateCount));

  auto garbler = Shake256YaoGarbler();

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
  exit(EXIT_SUCCESS);
}
