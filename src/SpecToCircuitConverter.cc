#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "StringUtils.hh"
#include "SpecToCircuitConverter.hh"
#include "Exceptions.hh"

SpecToCircuitConverter
::SpecToCircuitConverter(std::string specFileName)
  : specFileName(specFileName), circuit(Circuit(0, 0)) {}

BaseConverter::BaseConverter(std::string specFileName)
  : SpecToCircuitConverter(specFileName) {
    throw OutdatedClass("BaseConverter");
  }


Circuit BaseConverter::convert() {
  specToIR();
  return IRToCircuit();
}

void BaseConverter::specToIR() {
  printf("I: parsing spec file %s...\n", specFileName.c_str());
  std::string specParserName = "./SpecParser.sh";
  pid_t pid = fork();
  assert (pid >= 0);
  if (pid == 0) {
    char* argv[] = {
      (char*) specParserName.c_str(),
      (char*) specFileName.c_str(), NULL};
    execv((char*) specParserName.c_str(), argv);
    perror ("execv");
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    pid_t waitReturn;
    int status;
    do {
      waitReturn = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      assert (waitReturn >= 0);
      assert (WIFEXITED(status) and WEXITSTATUS(status) == 0);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

Circuit BaseConverter::IRToCircuit() {
  const std::string SEPLINE = std::string(8, '-');
  const std::string SEPEXPR = "$";
  const std::string SEPLIST = "#";
  class IncGenerator {
  public:
    IncGenerator(unsigned& n) : counter(n) {}
    unsigned operator() () {
      return this->counter++;
    }
  private:
    unsigned& counter;
  };

  std::string irFileName = specFileName + ".ir";
  printf("I: parsing IR file %s...\n", irFileName.c_str());

  // Setting up the neccessary variables for parsing the IR file.
  std::ifstream irFile(irFileName);
  std::string line;
  std::vector<std::string> tokens;

  // First line is ALWAYS the register sizes.
  std::getline(irFile, line);
  tokens = split(line);
  unsigned monitorStateLength = 0, systemStateLength = 0;
  assert (tokens[0] == "registerWordSizes");
  std::vector<unsigned> registerSizes;
  unsigned counter = 0;
  auto gen = IncGenerator(counter);
  for (unsigned i = 1; i < tokens.size(); i++) {
    auto wordSize = std::stoi(tokens[i]);
    registerSizes.push_back(wordSize);
    monitorStateLength += wordSize;
    auto word = Word(wordSize);
    std::generate(word.begin(), word.end(), gen);
    this->identMap["R[" + std::to_string(i-1) + "]"] = word;
  }

  // Next line is ALWAYS the input segment sizes.
  std::getline(irFile, line);
  tokens = split(line);
  assert (tokens[0] == "inputWordSizes");
  for (unsigned i = 1; i < tokens.size(); i++) {
    auto wordSize = std::stoi(tokens[i]);
    systemStateLength += wordSize;
    auto word = Word(wordSize);
    std::generate(word.begin(), word.end(), gen);
    this->identMap["I[" + std::to_string(i-1) + "]"] = word;
  }
  // Now we have enough information to create the circuit,
  // as the circuit constructor requires the input and output lengths.
  this->circuit = Circuit(
    monitorStateLength + systemStateLength,
    monitorStateLength);
  // We need this to run the monitor.
  this->monitorStateLength = monitorStateLength;
  this->systemStateLength = systemStateLength;
  // To represent constant numbers in the circuit,
  // we need to create constant values.
  // NOTE that argument to Zero and One is the driver of the constant;
  // every circuit has at least one driver, so we can use 0.
  auto zero = Zero(0);
  auto one = One(0);
  zero.build(this->circuit);
  one.build(this->circuit);
  this->circuitConstants = {zero, one};

  std::vector<unsigned> guards;
  std::getline(irFile, line);
  assert (line == SEPLINE);
  std::getline(irFile, line);
  tokens = split(line);
  assert (tokens[0] == "guards");
  unsigned guardCount = std::stoi(tokens[1]);
  guards.resize(guardCount);
  // 'Initial' guard is always false.
  auto prevGuard = this->circuitConstants[0];
  for (unsigned i = 0; i < guardCount; i++) {
    std::vector<std::string> guardSpec;
    while (true) {
      std::getline(irFile, line);
      if (line == SEPEXPR) break;
      guardSpec.push_back(line);
    }
    auto guard = this->readExpression(guardSpec);
    assert (guard.size() == 1);
    auto prevGuardInv = Inverter( { prevGuard } );
    prevGuardInv.build(this->circuit);
    auto guardActive = AndGate(guard, prevGuardInv);
    guardActive.build(this->circuit);
    guards[i] = guardActive;
    prevGuard = guardActive;
  }
  this->guards = guards;
  std::getline(irFile, line);
  assert (line == SEPLINE);
  // Skipping over guard updates.
  while (true) {
    std::getline(irFile, line);
    if (line == SEPLINE) break;
  }
  // Reading register updates.
  std::getline(irFile, line);
  tokens = split(line);
  assert (tokens[0] == "regactions");
  unsigned registerCount = std::stoi(tokens[1]);
  std::vector<RegisterUpdateVec> registerActions;
  for (unsigned i = 0; i < registerCount; i++) {
    std::getline(irFile, line);
    tokens = split(line);
    assert (tokens[0] == "updates");
    unsigned updateCount = std::stoi(tokens[1]);
    RegisterUpdateVec updates;
    for (unsigned j = 0; j < updateCount; j++) {
      std::getline(irFile, line);
      tokens = split(line);
      assert (tokens[0] == "guard");
      unsigned guardIdx = std::stoi(tokens[1]);
      auto guard = guards[guardIdx];
      // Guard is the wire carrying the guard expression value.
      std::vector<std::string> updateSpec;
      while (true) {
        std::getline(irFile, line);
        if (line == SEPEXPR) break;
        updateSpec.push_back(line);
      }
      auto update = this->readExpression(updateSpec);
      // If update was not an expression,
      // it can be larger than its destined register.
      // So, we resize it down to the register size.
      assert (update.size() >= registerSizes[i]);
      update.resize(registerSizes[i]);
      updates.push_back({guard, update});
    }
    registerActions.push_back(updates);
    std::getline(irFile, line);
    assert (line == SEPLIST);
  }
  WordVector registers;
  for (unsigned i = 0; i < registerCount; i++)
    registers.push_back(this->identMap["R[" + std::to_string(i) + "]"]);
  auto ts = TransitionSystem(registers, registerActions);
  ts.build(this->circuit);
  this->circuit.updateOutputs(ts);
  return std::move(this->circuit);
};

Word BaseConverter::readExpression(std::vector<std::string> expr) {
  const unsigned MAX_WORD_SZ = 64;
  if (expr.size() == 1 && split(expr[0]).size() == 1) {
    // This expression is just an identifier or a number.
    auto ident = expr[0];
    if (this->identMap.find(ident) != this->identMap.end()) {
      return this->identMap[ident];
    } else if (isNumber(ident)) {
      return this->readNumber(ident, MAX_WORD_SZ);
    } else throw std::invalid_argument("Unknown identifier: " + ident);
  }
  std::vector<std::string> tokens;
  std::string line;
  std::string op;
  std::vector<Word> operands;
  Word result;
  for (auto e : expr) {
    tokens = split(e);
    assert (tokens.size() >= 4);
    auto arrowIdx = tokens.size() - 2;
    // Every expression line is in the following form:
    // Operator op1 ... opN -> result;
    // so, there is always an arrow before the last token.
    assert (tokens[arrowIdx] == "->");
    std::vector<Word> operands;
    for (unsigned i = 0; i < arrowIdx; i++) {
      if (i == 0) continue;
      // All identifiers passed as operands should be in identMap.
      auto ident = tokens[i];
      if (this->identMap.find(ident) != this->identMap.end()) {
        operands.push_back(this->identMap[ident]);
      } else if (isNumber(ident)) {
        // ASSUMPTION: First operand is always an identifier;
        // so we can refer to its size to determine the size of this number.
        operands.push_back(this->readNumber(ident, operands[0].size()));
      } else throw std::invalid_argument("Unknown identifier: " + ident);
    }
    // A result-carrying operand should not already be in identMap.
    auto resultIdent = tokens[arrowIdx + 1];
    assert (this->identMap.find(resultIdent) == this->identMap.end());
    result = this->buildOperator(tokens[0], operands);
    this->identMap[resultIdent] = result;
  }
  return result;
}

Word BaseConverter::buildOperator(std::string op, std::vector<Word> operands) {
  auto isBinary = (op != "Inverter" and op != "Negator");
  assert (not isBinary or operands.size() == 2);
  if (op == "AndGate") {
    auto andGate = AndGate(operands[0], operands[1]);
    andGate.build(this->circuit);
    return andGate;
  } else if (op == "OrGate") {
    auto orGate = OrGate(operands[0], operands[1]);
    orGate.build(this->circuit);
    return orGate;
  } else if (op == "XorGate") {
    auto xorGate = XorGate(operands[0], operands[1]);
    xorGate.build(this->circuit);
    return xorGate;
  } else if (op == "EqChecker") {
    auto eqChecker = EqChecker(operands[0], operands[1]);
    eqChecker.build(this->circuit);
    return eqChecker;
  } else if (op == "LtChecker") {
    auto ltChecker = LtChecker(operands[0], operands[1]);
    ltChecker.build(this->circuit);
    return ltChecker;
  } else if (op == "Adder") {
    auto adder = Adder(operands[0], operands[1], this->circuitConstants[0]);
    adder.build(this->circuit);
    return adder.sum();
  } else if (op == "Inverter") {
    assert (operands.size() == 1);
    auto inverter = Inverter(operands[0]);
    inverter.build(this->circuit);
    return inverter;
  } else if (op == "Negator") {
    assert (operands.size() == 1);
    auto negator = Negator(operands[0]);
    negator.build(this->circuit);
    return negator;
  } else {
    throw std::invalid_argument("Unknown operator: " + op);
  }
}

Word BaseConverter::readNumber(std::string number, unsigned length) {
  // ASSUMPTION: number is an unsigned integer in base 10.
  Word word(length);
  auto n = std::stol(number);
  for (unsigned i = 0; i < length; i++) {
    bool bit = (n >> i) & 1;
    word[i] = this->circuitConstants[bit];
  }
  return word;
}

YosysConverter::YosysConverter(std::string specFileName)
  : SpecToCircuitConverter(specFileName) {}

Circuit YosysConverter::convert() {
  specToBlif();
  blifToCircuit();
  return std::move(this->circuit);
}

void YosysConverter::specToBlif() {
  // For now, specFileName doesn't matter.
  printf("I: parsing spec file %s...\n", this->specFileName.c_str());
  std::string specParserName = "./YosysParser.sh";
  pid_t pid = fork();
  assert (pid >= 0);
  if (pid == 0) {
    char* argv[] = {
      (char*) specParserName.c_str(),
      (char*) this->specFileName.c_str(),
      NULL};
    execv((char*) specParserName.c_str(), argv);
    perror ("execv");
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    pid_t waitReturn;
    int status;
    do {
      waitReturn = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      assert (waitReturn >= 0);
      assert (WIFEXITED(status) and WEXITSTATUS(status) == 0);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

void YosysConverter::blifToCircuit() {
  std::string blifFileName = "synth.blif";
  printf("I: parsing BLIF file %s...\n", blifFileName.c_str());
  BlifParser parser(blifFileName);
  parser.parse(this->circuit);
}

BlifParser::BlifParser(std::string blifFileName)
  : blifFileName(blifFileName) {}

void BlifParser::parse(Circuit& circuit) {
  std::ifstream blifFile(blifFileName);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(blifFile, line))
    lines.push_back(line);

  // First, we parse inputs and outputs.
  for (auto& line : lines) {
    if (line.starts_with(".inputs")) parseInputs(line);
    if (line.starts_with(".outputs")) parseOutputs(line);
  }
  printf("D: inputs and outputs parsed.\n");

  // Now we should reshape the circuit to be of proper size.
  circuit = Circuit(
    this->monitorStateLength + this->systemStateLength,
    this->outputLength);
  auto zero = Zero(0);
  auto one = One(0);
  zero.build(circuit);
  one.build(circuit);
  this->wireIndices["$false"] = zero;
  this->wireIndices["$true"] = one;
  printf("D: circuit initialized.\n");
  // Second, we parse connections.
  unsigned connCounter = 0;
  for (auto& line : lines) {
    if (line.starts_with(".conn")) { parseConnection(line); connCounter++; }
  }
  printf("D: connections (#%u) parsed.\n", connCounter);
  // Lastly, we parse gates.
  unsigned gateCounter = 0;
  for (auto& line : lines) {
    if (line.starts_with(".subckt")) { parseGate(line); gateCounter++; };
  }
  printf("D: gates (#%u) parsed.\n", gateCounter);
  // Now we can build the circuit.
  buildCircuit(circuit);
  // Now we update the circuit outputs.
  for (auto& key : this->graph)
    assert (this->wireIndices.find(key.first) != this->wireIndices.end());

  Word outputs(this->outputLength);
  for (unsigned i = 0; i < this->outputLength; i++) {
    auto wire = "out[" + std::to_string(i) + "]";
    assert (wireIndices.find(wire) != wireIndices.end());
    outputs[i] = wireIndices[wire];
  }
  // Outputs might be in any order,
  // since some of them also drive other wires.
  // To fix this, we add 'identity' gates to the circuit.
  printf("D: outputs: ");
  for (auto& out : outputs)
    printf("%d ", out);
  printf("\n");
  printf("D: circuit size: %d\n", circuit.size());
  printf("D: adding identity gates for outputs...\n");
  auto id = Identity(outputs);
  id.build(circuit);
  printf("D: outputs after identity: ");
  for (auto& out : Word(id))
    printf("%d ", out);
  printf("\n");
  printf("D: circuit size after identity: %d\n", circuit.size());
  circuit.updateOutputs(id);
}

void BlifParser::parseInputs(const std::string& line) {
  std::istringstream iss(line.substr(7));  // skip ".inputs "
  std::string input;
  unsigned inputIdx = 0;
  this->monitorStateLength = 0;
  this->systemStateLength = 0;
  while (iss >> input) {
    assert (input.starts_with("monitor[") or input.starts_with("system["));
    // Inputs are indexed manually.
    // ASSUMPTION: Monitor inputs always come before system inputs.
    // So, line looks like:
    // .inputs monitor[0] ... monitor[n] system[0] ... system[m]
    monitorStateLength += input.starts_with("monitor[");
    systemStateLength += input.starts_with("system[");
    this->graph[input] = {};
    this->wireIndices[input] = inputIdx++;
  }
}

void BlifParser::parseOutputs(const std::string& line) {
  std::istringstream iss(line.substr(8));  // skip ".outputs "
  std::string output;
  this->outputLength = 0;
  // At this stage, we can't do anything special with the outputs.
  // We only know that all output signals are named out[i], for some i.
  while (iss >> output) {
    assert (output.starts_with("out["));
    this->graph[output] = {};
    outputLength++;
  }
}

void BlifParser::parseConnection(const std::string& line) {
  // Every connection is a forward edge.
  std::istringstream iss(line.substr(5)); // skip ".conn "
  std::string first, second;
  iss >> first >> second;
  this->graph[second].push_back(first);
}

void BlifParser::parseGate(const std::string& line) {
  if (line.starts_with(".subckt $_NOT_")) {
    // Parse "A=input Y=output"
    size_t aPos = line.find("A=") + 2;
    size_t yPos = line.find("Y=") + 2;
    std::string input = line.substr(aPos, line.find(" ", aPos) - aPos);
    std::string output = line.substr(yPos);

    // NOT(x) = NAND(x,x)
    this->graph[output] = {input, input};
  } else if (line.starts_with(".subckt $_NAND_")) {
    // Parse "A=in1 B=in2 Y=output"
    size_t aPos = line.find("A=") + 2;
    size_t bPos = line.find("B=") + 2;
    size_t yPos = line.find("Y=") + 2;
    std::string in1 = line.substr(aPos, line.find(" ", aPos) - aPos);
    std::string in2 = line.substr(bPos, line.find(" ", bPos) - bPos);
    std::string output = line.substr(yPos);
    this->graph[output] = {in1, in2};
  }
}

void BlifParser::buildCircuit(Circuit& circuit) {
  // The goal is to build all output wires;
  // so we start from the outputs and traverse backwards.
  for (unsigned i = 0; i < this->outputLength; i++) {
    std::string output = "out[" + std::to_string(i) + "]";
    buildWire(output, circuit);
  }
}

void BlifParser::buildWire(std::string wire, Circuit& circuit) {
  if (wireIndices.find(wire) != wireIndices.end()) return;
  // If wire is not in the circuit, we should build it.
  // First, we build all the wires connected to this wire.
  for (auto& connected : this->graph[wire])
    buildWire(connected, circuit);
  // Now, we can build the wire.
  // Alias wires always have one incoming wire,
  // so we can distinguish them from other wires.
  if (this->graph[wire].size() == 1) {
    auto driver = graph[wire][0];
    wireIndices[wire] = wireIndices[driver];
  } else {
    // printf("D: building gate for %s, ", wire.c_str());
    // printf("inputs: ");
    // for (auto& in : this->graph[wire])
    //   printf("%s ", in.c_str());
    // printf("\n");
    assert (this->graph[wire].size() == 2);
    auto in1 = this->graph[wire][0];
    auto in2 = this->graph[wire][1];
    auto in1Idx = wireIndices[in1];
    auto in2Idx = wireIndices[in2];
    auto wireIdx = circuit.addGate(in1Idx, in2Idx);
    // printf("D: in1Idx = %d, in2Idx = %d, wireIdx = %d\n",
    // in1Idx, in2Idx, wireIdx);
    wireIndices[wire] = wireIdx;
  }
}
