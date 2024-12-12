#ifndef SPEC_TO_CIRCUIT_CONVERTER_HH
#define SPEC_TO_CIRCUIT_CONVERTER_HH

#include <string>
#include "Circuit.hh"
#include "Module.hh"

class SpecToCircuitConverter {
public:
  Word guards;
  unsigned monitorStateLength, systemStateLength;
  SpecToCircuitConverter(std::string specFileName);
  Circuit convert();
private:
  std::string specFileName;
  // IdentMap is a map from all the identifiers in the IR file
  // to their corresponding 'Words' in the circuit.
  std::map<std::string, Word> identMap;
  Circuit circuit;
  std::array<unsigned, 2> circuitConstants;
  void specToIR();
  Circuit IRToCircuit();
  Word readExpression(std::vector<std::string> expr);
  Word buildOperator(std::string op, std::vector<Word> operands);
  Word readNumber(std::string number, unsigned length);
};

#endif
