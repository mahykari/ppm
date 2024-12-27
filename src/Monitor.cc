#include <iostream>
#include "MathUtils.hh"
#include "MessageHandler.hh"
#include "BM.hh"
#include "LWY.hh"
#include "Circuit.hh"
#include "CommandLineInterface.hh"

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

namespace L = LWY;

int main (int argc, char *argv[]) {
  printf("E: Monitor is outdated\n");
  exit(EXIT_FAILURE);
  auto cli = CommandLineInterface();
  cli.parse(argc, argv);

  SetUp();
  auto messageHandler = MessageHandler(L::SYSTEM_PORT, L::MONITOR_PORT);

  auto primeModulus = BigInt(SAFE_PRIMES[cli.securityParameter]);
  printf("I: using prime modulus %s\n", primeModulus.get_str(10).c_str());

  // CAUTION: systemStateLength should be a perfect square.
  unsigned inputLength = cli.monitorStateLength + cli.systemStateLength;
  unsigned outputLength = cli.monitorStateLength + 1;
  auto circuit = Circuit(inputLength, outputLength);
  std::vector<unsigned> gateIds;
  for (unsigned i = 0; i < cli.systemStateLength; i++)
    gateIds.push_back(cli.monitorStateLength + i);
  unsigned n = gateIds.size();
  // printf("D: n = %u\n", n);
  for (unsigned i = 0; i < n - 1; i++) {
    unsigned inputLeft = gateIds[2*i];
    unsigned inputRight = gateIds[2*i + 1];
    gateIds.push_back(circuit.addGate(inputLeft, inputRight));
  }
  // Last gate computes SUM(x_{2i} x_{2i + 1}).
  auto sumGateId = gateIds.back();
  // ~ (m SUM(x_{2i} x_{2i + 1}))
  auto notFlagGateId = circuit.addGate(0, sumGateId);
  circuit.addGate(0, 0); // ~ m
  circuit.addGate(notFlagGateId, notFlagGateId); // m SUM(x_{2i} x_{2i + 1})
  auto monitorMemory = L::MonitorMemory {
    .circuit = &circuit
  };

  printf(
    "D: circuit size = %u, input length: %u\n",
    circuit.size(), inputLength);

  auto parameters = L::ParameterSet {
    .gateCount = circuit.size() - inputLength,
    .monitorStateLength = cli.monitorStateLength,
    .systemStateLength = cli.systemStateLength,
    .group = QuadraticResidueGroup(primeModulus),
    .garbler = Sha512YaoGarbler(),
    .securityParameter = cli.securityParameter
  };

  auto interface = L::MonitorInterface(
    &parameters, &monitorMemory, &messageHandler);

  interface.run();

  // std::cout << std::string(80, '=') << '\n';
  // std::cout << "TESTING BELLARE-MICALI OT PROTOCOL" << '\n';
  // std::cout << std::string(80, '=') << '\n';

  // auto parameters1 = B::ParameterSet {
  //   .group = QuadraticResidueGroup(primeModulus)
  // };

  // auto chooserMemory = B::ChooserMemory { .sigma = 1 };
  // auto interface1 = B::ChooserInterface(
  //   &parameters1, &chooserMemory, &messageHandler);
  // interface1.run();

  exit(EXIT_SUCCESS);
}
