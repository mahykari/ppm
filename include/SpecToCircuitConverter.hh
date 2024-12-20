#ifndef SPEC_TO_CIRCUIT_CONVERTER_HH
#define SPEC_TO_CIRCUIT_CONVERTER_HH

#include <string>
#include "Circuit.hh"
#include "Module.hh"

class SpecToCircuitConverter {
public:
  SpecToCircuitConverter(std::string specFileName);
  virtual Circuit convert() = 0;
protected:
  std::string specFileName;
  Circuit circuit;
  std::array<unsigned, 2> circuitConstants;
};

// This converter reads a spec file written in a custom language.
// NOTE: this converter is outdated. Use another converter instead.
class BaseConverter : public SpecToCircuitConverter {
public:
  Word guards;
  unsigned monitorStateLength, systemStateLength;
  BaseConverter(std::string specFileName);
  Circuit convert() override;
private:
  // IdentMap is a map from all the identifiers in the IR file
  // to their corresponding 'Words' in the circuit.
  std::map<std::string, Word> identMap;
  void specToIR();
  Circuit IRToCircuit();
  Word readExpression(std::vector<std::string> expr);
  Word buildOperator(std::string op, std::vector<Word> operands);
  Word readNumber(std::string number, unsigned length);
};

typedef std::pair<std::string, std::string> Connection;
typedef std::unordered_map<std::string, std::vector<std::string>> CircuitGraph;

class BlifParser {
public:
  BlifParser(std::string blifFileName);
  void parse(Circuit&);
private:
  std::string blifFileName;
  unsigned monitorStateLength, systemStateLength;
  unsigned outputLength;
  std::unordered_map<std::string, unsigned> wireIndices;
  CircuitGraph graph;
  void parseInputs(const std::string& line);
  void parseOutputs(const std::string& line);
  void parseConnection(const std::string& line);
  void parseGate(const std::string& line);
  void buildCircuit(Circuit& circuit);
  void buildWire(std::string wire, Circuit& circuit);
};

class YosysConverter : public SpecToCircuitConverter {
public:
  YosysConverter(std::string specFileName);
  Circuit convert() override;
private:
  void specToBlif();
  void blifToCircuit();
};

#endif
