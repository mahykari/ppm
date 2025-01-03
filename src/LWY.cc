#include <cassert>
#include <sstream>
#include "LWY.hh"
#include "Exceptions.hh"
#include "StringUtils.hh"

namespace P = LWY;

unsigned P::ParameterSet::inputLength() {
  return this->monitorStateLength + this->systemStateLength;
}

P::SystemState::SystemState(ParameterSet* parameters, SystemMemory* memory)
  : parameters(parameters), memory(memory) {}

P::SystemInterface::SystemInterface(
  P::ParameterSet* parameters,
  P::SystemMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    messageHandler(messageHandler),
    memory(memory) {
  state = std::make_unique<P::InitSystem>(this->parameters, this->memory);
}

void P::SystemInterface::sync() {
  // printf("D: SystemInterface::sync\n");
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void P::SystemInterface::next() {
  // printf("D: SystemInterface::next\n");
  this->sync();
  this->state = this->state->next();
}

void P::SystemInterface::run() {
  while (this->state)
    this->next();
}

P::MonitorState::MonitorState(
  P::ParameterSet* parameters, P::MonitorMemory* memory)
  : parameters(parameters), memory(memory) {}

P::MonitorInterface::MonitorInterface(
  P::ParameterSet* parameters,
  P::MonitorMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    messageHandler(messageHandler),
    memory(memory) {
  state = std::make_unique<P::InitMonitor>(this->parameters, this->memory);
}

void P::MonitorInterface::sync() {
  // printf("D: MonitorInterface::sync\n");
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void P::MonitorInterface::next() {
  // printf("D: MonitorInterface::next\n");
  this->sync();
  this->state = this->state->next();
}

void P::MonitorInterface::run() {
  while (this->state)
    this->next();
}

P::InitSystem::InitSystem(ParameterSet* parameters, SystemMemory* memory)
  : SystemState(parameters, memory) {
    this->memory->timer.reset();
}

StatePtr P::InitSystem::next() {
  printf("I: InitSystem::next\n");
  return std::make_unique<P::RecvLabels>(this->parameters, this->memory);
}

void P::RecvLabels::parseLabels() {
  auto driverCount =
    this->parameters->gateCount
    + this->parameters->monitorStateLength
    + this->parameters->systemStateLength;
  auto gateCount = this->parameters->gateCount;
  // Message is formatted as follows:
  // DriverLabel1 ... DriverLabelN InWireLabel1 ... InWireLabelM
  // (N = driverCount - 1, M = 2 * gateCount).
  auto& message = this->memory->receivedMessage;
  std::tie(this->memory->driverLabels, message) =
    readBigInts(message, P::MSG_NUM_BASE, driverCount - 1);
  std::tie(this->memory->inWireLabels, message) =
    readBigInts(message, P::MSG_NUM_BASE, 2 * gateCount);
  assert (this->memory->driverLabels.size() == driverCount - 1);
  assert (this->memory->inWireLabels.size() == 2 * gateCount);
}

void P::RecvLabels::generateFlagBitLabel() {
  auto flagBitLabel = this->parameters->group.randomGenerator();
  this->memory->driverLabels.push_back(flagBitLabel);
}

bool P::RecvLabels::isRecv() {
  return true;
}

StatePtr P::RecvLabels::next() {
  printf("I: RecvLabels::next\n");
  this->memory->timer.start();
  printf("D:   timer started\n");
  assert (not this->memory->receivedMessage.empty());
  this->parseLabels();
  printf("D:   labels parsed\n");
  this->generateFlagBitLabel();
  printf("D:   flag bit label generated\n");
  // printf("D:   labels parsed, flag bit generated\n");
  this->memory->timer.pause();
  printf("D:   labels parsed, exponents generated in %f ms\n",
         this->memory->timer.display());
  return std::make_unique<P::GenerateGarbledGates>(
    this->parameters, this->memory);
}

void P::GenerateGarbledGates::generateGarblingExponents() {
  auto& exponents = this->memory->garblingExponents;
  auto& nextRoundExponents = this->memory->nextRoundGarblingExponents;
  auto& group = this->parameters->group;
  if (this->memory->isFirstRound) {
    exponents[0] = group.randomExponent();
    exponents[1] = group.randomExponent();
    nextRoundExponents[0] = group.randomExponent();
    nextRoundExponents[1] = group.randomExponent();
  } else {
    std::swap(exponents, nextRoundExponents);
    nextRoundExponents[0] = group.randomExponent();
    nextRoundExponents[1] = group.randomExponent();
  }
}

std::array<BigInt, 2> P::GenerateGarbledGates::expLabel(
  BigInt label, std::array<BigInt, 2> exponents)
{
  auto group = this->parameters->group;
  return {
    group.exp(label, exponents[0]),
    group.exp(label, exponents[1])
  };
}

std::string P::GenerateGarbledGates::padLabel(BigInt label) {
  auto labelStr = toString(label, P::MSG_NUM_BASE);
  auto targetLength = this->parameters->securityParameter / 4;
  return std::string(targetLength - labelStr.size(), '0') + labelStr;
}

void P::GenerateGarbledGates::garble() {
  auto group = this->parameters->group;
  auto garbler = this->parameters->garbler;

  auto exponents = this->memory->garblingExponents;
  auto& inWireLabels = this->memory->inWireLabels;
  auto& driverLabels = this->memory->driverLabels;

  auto& garbledGates = this->memory->garbledGates;
  auto gateCount = this->parameters->gateCount;
  auto offset =
    this->parameters->monitorStateLength
    + this->parameters->systemStateLength;
  garbledGates.resize(gateCount);
  auto outputLength = this->parameters->monitorStateLength + 1;
  // For all output gates (including the one for the flag bit),
  // exponents picked for the next round are used.
  for (unsigned i = 0; i < gateCount; i++) {
    auto leftLabels = this->expLabel(
      inWireLabels[2 * i], this->memory->garblingExponents);
    auto rightLabels = this->expLabel(
      inWireLabels[2 * i + 1], this->memory->garblingExponents);
    auto outExponents = (i < gateCount - outputLength)
      ? this->memory->garblingExponents
      : this->memory->nextRoundGarblingExponents;
    auto outLabels = this->expLabel(driverLabels[offset + i], outExponents);
    // printf("D:   shuffled gate %d\n", i);
    // printf("D:     left labels:  { %s, %s }\n",
    //   this->padLabel(leftLabels[0]).c_str(),
    //   this->padLabel(leftLabels[1]).c_str());
    // printf("D:     right labels: { %s, %s }\n",
    //   this->padLabel(rightLabels[0]).c_str(),
    //   this->padLabel(rightLabels[1]).c_str());
    // printf("D:     out labels:   { %s, %s }\n",
    //   this->padLabel(outLabels[0]).c_str(),
    //   this->padLabel(outLabels[1]).c_str());
    garbledGates[i] = garbler->enc(
      { this->padLabel(leftLabels[0]),
        this->padLabel(leftLabels[1]) },
      { this->padLabel(rightLabels[0]),
        this->padLabel(rightLabels[1]) },
      { this->padLabel(outLabels[0]),
        this->padLabel(outLabels[1]) }
    );
  }
}

StatePtr P::GenerateGarbledGates::next() {
  printf("I: GenerateGarbledGates::next\n");
  // All rounds, other than the first round,
  // start from this state.
  // So, this timer is not reset until the next round;
  // i.e., the next time this state is reached.
  auto& timer = this->memory->timer;
  timer.reset();
  timer.start();
  this->generateGarblingExponents();
  this->garble();
  memory->garbledGates.resize(this->parameters->gateCount);
  // A pause is required to exclude message passing time.
  timer.pause();
  return std::make_unique<P::SendGarbledGates>(this->parameters, this->memory);
}

bool P::SendGarbledGates::isSend() {
  return true;
}

std::string P::SendGarbledGates::message() {
  std::stringstream ss;
  for (auto& gate : this->memory->garbledGates)
    for (auto& label : gate)
      ss << label << ' ';
  return ss.str();
}

StatePtr P::SendGarbledGates::next() {
  printf("I: SendGarbledGates::next\n");
  return std::make_unique<P::SendSystemInputLabels>(
    this->parameters, this->memory);
}

bool P::SendSystemInputLabels::isSend() {
  return true;
}

std::vector<BigInt> P::SendSystemInputLabels::systemInputLabels_Timed() {
  auto& systemData = this->memory->system->data();
  auto& timer = this->memory->timer;
  // printf("D:   system data: { ");
  // for (unsigned i = 0; i < systemData.size(); i++)
  //   printf("%d ", systemData[i]);
  // std::string s(systemData.size(), '0');
  // for (unsigned i = 0; i < systemData.size(); i++)
  //   s[i] = systemData[i] + '0';
  // printf("%s", BigInt(s, 2).get_str(10).c_str());
  // printf(" }\n");
  timer.resume();
  std::vector<BigInt> labels;
  assert (systemData.size() == this->parameters->systemStateLength);
  labels.resize(systemData.size());
  auto offset = this->parameters->monitorStateLength;
  auto group = this->parameters->group;
  for (unsigned i = 0; i < systemData.size(); i++) {
    auto exponent = this->memory->garblingExponents[ systemData[i] ];
    auto driverLabel = this->memory->driverLabels[offset + i];
    labels[i] = group.exp(driverLabel, exponent);
  }
  timer.pause();
  return labels;
}

std::string P::SendSystemInputLabels::message() {
  std::stringstream ss;
  for (auto& label : this->systemInputLabels_Timed())
    ss << toString(label, P::MSG_NUM_BASE) << ' ';
  return ss.str();
}

StatePtr P::SendSystemInputLabels::next() {
  printf("I: SendSystemInputLabels::next\n");
  return std::make_unique<P::SendFlagBitLabels> (
    this->parameters, this->memory);
}

bool P::SendFlagBitLabels::isSend() {
  return true;
}

std::array<BigInt, 2> P::SendFlagBitLabels::flagBitLabels_Timed() {
  auto& timer = this->memory->timer;
  auto& group = this->parameters->group;
  auto& driverLabels = this->memory->driverLabels;
  auto flagBitLabel = driverLabels.back();
  // For all output gates (including the one for the flag bit),
  // exponents picked for the next round are used.
  timer.resume();
  auto& exponents = this->memory->nextRoundGarblingExponents;
  std::array<BigInt, 2> labels = {
    group.exp(flagBitLabel, exponents[0]),
    group.exp(flagBitLabel, exponents[1])
  };
  timer.pause();
  return labels;
}

std::string P::SendFlagBitLabels::message() {
  auto labels = this->flagBitLabels_Timed();
  return toString(labels[0], P::MSG_NUM_BASE) + ' '
    + toString(labels[1], P::MSG_NUM_BASE);
}

StatePtr P::SendFlagBitLabels::next() {
  printf("I: SendFlagBitLabels::next\n");
  if (this->memory->isFirstRound) {
    this->memory->isFirstRound = false;
    return std::make_unique<P::SystemObliviousTransfer>
      (this->parameters, this->memory);
  }
  return std::make_unique<P::RecvFlagBit> (this->parameters, this->memory);
}

P::SystemObliviousTransfer::SystemObliviousTransfer(
  ParameterSet* parameters, SystemMemory* memory)
  : SystemState(parameters, memory)
{
  this->counter = 0;
  this->OTParameters = std::make_unique<BM::ParameterSet>(
    BM::ParameterSet {
      .securityParameter= this->parameters->securityParameter,
      .group = this->parameters->group
    }
  );
  this->senderMemory = std::make_unique<BM::SenderMemory>();
  this->setOTMessages_Timed();
  this->state = std::make_unique<BM::InitSender>
    (this->OTParameters.get(), this->senderMemory.get());
}

void P::SystemObliviousTransfer::setOTMessages_Timed() {
  auto& timer = this->memory->timer;
  timer.resume();
  auto& memory = this->memory;
  auto& senderMemory = this->senderMemory;
  auto& group = this->OTParameters->group;
  for (unsigned i = 0; i < 2; i++)
    senderMemory->messages[i] = toString(
      group.exp(
        memory->driverLabels[this->counter],
        memory->garblingExponents[i]),
      BM::MSG_NUM_BASE);
  timer.pause();
}

bool P::SystemObliviousTransfer::isSend() {
  if (this->state)
    return this->state->isSend();
  return false;
}

bool P::SystemObliviousTransfer::isRecv() {
  if (this->state)
    return this->state->isRecv();
  return false;
}

std::string P::SystemObliviousTransfer::message() {
  return this->state->message();
}

StatePtr P::SystemObliviousTransfer::next() {
  printf("I: SystemObliviousTransfer::next\n");
  if (this->counter == this->parameters->monitorStateLength)
    return std::make_unique<P::RecvFlagBit>(this->parameters, this->memory);

  if (not this->state) {
    this->counter++;
    this->setOTMessages_Timed();
    this->state = std::make_unique<BM::InitSender>
      (this->OTParameters.get(), this->senderMemory.get());
  } else {
    this->senderMemory->receivedMessage = this->memory->receivedMessage;
    // printf("D:   &senderMemory: %p\n", (void*) this->senderMemory.get());
    this->state = this->state->next();
    // printf("D:   generated constant: %s\n",
    //   this->senderMemory->constant.get_str(16).c_str());
  }
  return std::make_unique<P::SystemObliviousTransfer> (std::move(*this));
}

bool P::RecvFlagBit::isRecv() {
  return true;
}

StatePtr P::RecvFlagBit::next() {
  printf("I: RecvFlagBit::next\n");
  auto& message = this->memory->receivedMessage;
  bool flagBit = std::stoi(message);
  auto& timer = this->memory->timer;
  printf("D: ==== round duration: %f ms ====\n", timer.display());
  if (flagBit)
    return std::make_unique<P::SystemDone>(this->parameters, this->memory);
  else
    return std::make_unique<P::UpdateSystem>(
      this->parameters, this->memory);
}

StatePtr P::UpdateSystem::next() {
  printf("I: UpdateSystem::next\n");
  this->memory->system->next();
  return std::make_unique<P::GenerateGarbledGates>(
    this->parameters, this->memory);
}

StatePtr P::SystemDone::next() {
  printf("I: SystemDone::next\n");
  return nullptr;
}

P::InitMonitor::InitMonitor(ParameterSet* parameters, MonitorMemory* memory)
  : MonitorState(parameters, memory) {
    this->memory->timer.reset();
}

StatePtr P::InitMonitor::next() {
  printf("I: InitMonitor::next\n");
  return std::make_unique<P::GenerateDriverLabels>(
    this->parameters, this->memory);
}

StatePtr P::GenerateDriverLabels::next() {
  printf("I: GenerateDriverLabels::next\n");
  auto& timer = this->memory->timer;
  timer.start();
  auto& group = this->parameters->group;
  auto& driverLabels = this->memory->driverLabels;
  Circuit* circuit = this->memory->circuit;
  unsigned driverCount = circuit->size();
  unsigned monitorStateLength = this->parameters->monitorStateLength;
  driverLabels.resize(driverCount - 1);
  // Input and output monitor states receive the same labels.
  // First, all but the last (monitorStateLength) drivers are labelled.
  for (unsigned i = 0; i < driverCount - monitorStateLength - 1; i++)
    driverLabels[i] = group.randomGenerator();
  auto offset = driverCount - monitorStateLength - 1;
  for (unsigned i = 0; i < monitorStateLength; i++)
    driverLabels[offset + i] = driverLabels[i];
  timer.pause();
  printf("D:   driver labels generated in %f ms\n", timer.display());
  return std::make_unique<P::GenerateInWireKeys>(
    this->parameters, this->memory);
}

StatePtr P::GenerateInWireKeys::next() {
  printf("I: GenerateInWireKeys::next\n");
  auto& timer = this->memory->timer;
  timer.reset();
  timer.start();
  auto inWireCount = 2 * this->parameters->gateCount;
  this->memory->inWireKeys.resize(inWireCount);
  for (unsigned i = 0; i < inWireCount; i++)
    this->memory->inWireKeys[i] = this->parameters->group.randomExponent();
  timer.pause();
  printf("D:   in-wire keys generated in %f ms\n", timer.display());
  return std::make_unique<P::SendLabels>(
    this->parameters, this->memory);
}

bool P::SendLabels::isSend() {
  return true;
}

template <class T>
void testIdx(std::vector<T> vec, unsigned idx) {
  assert (idx < vec.size());
}

std::vector<BigInt> P::SendLabels::generateInWireLabels_Timed() {
  auto& timer = this->memory->timer;
  timer.reset();
  timer.start();

  std::vector<BigInt> inWireLabels;
  unsigned gateCount = this->parameters->gateCount;
  inWireLabels.resize(2 * gateCount);
  auto drivers = this->memory->circuit->get();
  auto offset = this->parameters->inputLength();
  // For gate G, ingoing wires are labelled as follows:
  // Left: (Driver label of G.leftInput) ^ (Key of G.leftInput)
  // Right: Same, but for G.rightInput.

  for (unsigned i = 0; i < gateCount; i++) {
    auto gate = static_cast<Gate*>(drivers[offset + i]);
    assert (gate != nullptr);
    auto indexLeft = 2 * i;
    auto indexRight = 2 * i + 1;

    testIdx(this->memory->driverLabels, gate->inputLeft);
    testIdx(this->memory->driverLabels, gate->inputRight);
    testIdx(this->memory->inWireKeys, indexLeft);
    testIdx(this->memory->inWireKeys, indexRight);

    inWireLabels[indexLeft] = this->parameters->group.exp(
      this->memory->driverLabels[gate->inputLeft],
      this->memory->inWireKeys[indexLeft]);

    inWireLabels[indexRight] = this->parameters->group.exp(
      this->memory->driverLabels[gate->inputRight],
      this->memory->inWireKeys[indexRight]);
  }
  timer.pause();
  printf("D:   in-wire labels generated in %f ms\n", timer.display());
  return inWireLabels;
}

void P::SendLabels::shuffleCircuit_Timed() {
  auto& timer = this->memory->timer;
  timer.resume();
  this->memory->shuffledCircuit = this->memory->circuit->shuffle();
  timer.pause();
}

std::string P::SendLabels::message() {
  printf("D: SendLabels::message\n");
  auto inWireLabels = this->generateInWireLabels_Timed();
  // printf("D:   in-wire labels generated\n");
  this->shuffleCircuit_Timed();
  std::stringstream ss;
  auto driverCount = this->memory->circuit->size();
  for (unsigned i = 0; i < driverCount - 1; i++) {
    auto driver = this->memory->shuffledCircuit[i];
    auto label = this->memory->driverLabels[driver->id];
    ss << toString(label, P::MSG_NUM_BASE) << ' ';
  }
  auto offset =
    this->parameters->monitorStateLength
    + this->parameters->systemStateLength;
  printf("D:  gateCount: %u\n", this->parameters->gateCount);
  for (unsigned i = 0; i < this->parameters->gateCount; i++) {
    auto driver = this->memory->shuffledCircuit[offset + i];
    auto index = 2 * (driver->id - offset);
    auto labelLeft = inWireLabels[index];
    auto labelRight = inWireLabels[index + 1];
    ss << toString(labelLeft, P::MSG_NUM_BASE) << ' ';
    ss << toString(labelRight, P::MSG_NUM_BASE) << ' ';
  }
  return ss.str();
}

StatePtr P::SendLabels::next() {
  printf("I: SendLabels::next\n");
  return std::make_unique<P::RecvGarbledGates>(this->parameters, this->memory);
}

bool P::RecvGarbledGates::isRecv() {
  return true;
}

StatePtr P::RecvGarbledGates::next() {
  printf("I: RecvGarbledGates::next\n");
  auto& timer = this->memory->timer;
  timer.reset();
  timer.start();
  auto message = this->memory->receivedMessage;
  auto gateCount = this->parameters->gateCount;
  auto& garbledGates = this->memory->garbledGates;
  std::tie(garbledGates, message) = readGarbledGates(message, gateCount);
  timer.pause();
  return std::make_unique<P::RecvSystemInputLabels>(
    this->parameters, this->memory);
}

bool P::RecvSystemInputLabels::isRecv() {
  return true;
}

StatePtr P::RecvSystemInputLabels::next() {
  printf("I: RecvSystemInputLabels::next\n");
  auto& timer = this->memory->timer;
  timer.resume();
  auto message = this->memory->receivedMessage;
  std::vector <BigInt> systemInputLabels;
  std::tie(systemInputLabels, message) =
    readBigInts(message, P::MSG_NUM_BASE, this->parameters->systemStateLength);
  // printf("D:   read system input labels\n");
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto offset = this->parameters->monitorStateLength;
  evaluatedDriverLabels.resize(this->memory->circuit->size());
  for (unsigned i = 0; i < systemInputLabels.size(); i++)
    evaluatedDriverLabels[offset + i] = systemInputLabels[i];
  timer.pause();
  return std::make_unique<P::RecvFlagBitLabels>(this->parameters, this->memory);
}

bool P::RecvFlagBitLabels::isRecv() {
  return true;
}

StatePtr P::RecvFlagBitLabels::next() {
  printf("I: RecvFlagBitLabels::next\n");
  auto& timer = this->memory->timer;
  timer.resume();
  auto message = this->memory->receivedMessage;
  std::vector<BigInt> flagBitLabels;
  std::tie(flagBitLabels, message) = readBigInts(message, P::MSG_NUM_BASE, 2);
  this->memory->flagBitLabels[0] = flagBitLabels[0];
  this->memory->flagBitLabels[1] = flagBitLabels[1];
  timer.pause();
  if (this->memory->isFirstRound) {
    this->memory->isFirstRound = false;
    return std::make_unique<P::MonitorObliviousTransfer>
      (this->parameters, this->memory);
  }
  return std::make_unique<P::EvaluateCircuit>(this->parameters, this->memory);
}

P::MonitorObliviousTransfer::MonitorObliviousTransfer(
  ParameterSet* parameters, MonitorMemory* memory)
  : MonitorState(parameters, memory),
    counter(0) {
  auto& timer = this->memory->timer;
  timer.resume();
  this->OTParameters = std::make_unique<BM::ParameterSet>
    (BM::ParameterSet {
      .securityParameter = this->parameters->securityParameter,
      .group = parameters->group });
  this->chooserMemory = std::make_unique<BM::ChooserMemory>();
  this->state = std::make_unique<BM::InitChooser>
    (this->OTParameters.get(), this->chooserMemory.get());
  timer.pause();
}

void P::MonitorObliviousTransfer::setSigma() {
  this->chooserMemory->sigma = 0;
}

bool P::MonitorObliviousTransfer::isSend() {
  if (this->state)
    return this->state->isSend();
  return false;
}

bool P::MonitorObliviousTransfer::isRecv() {
  if (this->state)
    return this->state->isRecv();
  return false;
}

std::string P::MonitorObliviousTransfer::message() {
  return this->state->message();
}

StatePtr P::MonitorObliviousTransfer::next() {
  printf("I: MonitorObliviousTransfer::next\n");
  if (this->counter == this->parameters->monitorStateLength)
    return std::make_unique<P::EvaluateCircuit>
      (this->parameters, this->memory);
  // printf("D:   counter: %d\n", this->counter);
  auto& timer = this->memory->timer;
  timer.resume();
  if (not this->state) {
    this->memory->evaluatedDriverLabels[this->counter] =
      BigInt(this->chooserMemory->chosenMessage, BM::MSG_NUM_BASE);
    this->counter++;
    this->setSigma();
    this->state = std::make_unique<BM::InitChooser>
      (this->OTParameters.get(), this->chooserMemory.get());
  } else {
    this->chooserMemory->receivedMessage = this->memory->receivedMessage;
    this->state = this->state->next();
  }
  timer.pause();
  return std::make_unique<P::MonitorObliviousTransfer> (std::move(*this));
}

std::string P::EvaluateCircuit::padLabel(BigInt label) {
  auto labelStr = toString(label, P::MSG_NUM_BASE);
  auto targetLength = this->parameters->securityParameter / 4;
  return std::string(targetLength - labelStr.size(), '0') + labelStr;
}

std::vector<unsigned> P::EvaluateCircuit::getUnshuffling_Timed() {
  auto& timer = this->memory->timer;
  timer.resume();
  auto& shuffledCircuit = this->memory->shuffledCircuit;
  auto drivers = this->memory->circuit->get();
  // Result[i] = j iff
  // Driver with ID i is at position j in the shuffled circuit.
  std::vector<unsigned> result;
  result.resize(drivers.size());
  for (unsigned i = 0; i < drivers.size(); i++) {
    auto& driver = shuffledCircuit[i];
    result[driver->id] = i;
  }
  timer.pause();
  return result;
}

void P::EvaluateCircuit::evaluateDriverLabels() {
  auto drivers = this->memory->circuit->get();
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto& garbledGates = this->memory->garbledGates;
  auto& inWireKeys = this->memory->inWireKeys;
  auto offset = this->parameters->inputLength();
  auto unshuffled = this->getUnshuffling_Timed();
  // printf("D:   shuffled IDs:   [ ");
  // for (auto driver : this->memory->shuffledCircuit)
  //   printf("%d ", driver->id);
  // printf("]\n");
  // printf("D:   unshuffled IDs: [ ");
  // for (auto id : unshuffled)
  //   printf("%d ", id);
  // printf("]\n");
  auto& timer = this->memory->timer;
  timer.resume();
  for (unsigned i = 0; i < this->parameters->gateCount; i++) {
    auto gate = static_cast<Gate*>(drivers[offset + i]);
    auto leftLabel = this->parameters->group.exp(
      evaluatedDriverLabels[gate->inputLeft],
      inWireKeys[2 * i]);
    auto rightLabel = this->parameters->group.exp(
      evaluatedDriverLabels[gate->inputRight],
      inWireKeys[2 * i + 1]);
    // printf("D:   evaluating gate ID %d\n", gate->id);
    // printf("D:   unshuffled index:  %d\n", unshuffled[gate->id]);
    // printf("D:     left label:   %s\n", this->padLabel(leftLabel).c_str());
    // printf("D:     right label:  %s\n", this->padLabel(rightLabel).c_str());
    auto outLabel = this->parameters->garbler->dec(
      this->padLabel(leftLabel),
      this->padLabel(rightLabel),
      garbledGates[unshuffled[gate->id] - offset]);
    // printf("D:     output label: %s\n", outLabel.c_str());
    evaluatedDriverLabels[gate->id] = BigInt(outLabel, P::MSG_NUM_BASE);
  }
  timer.pause();
}

StatePtr P::EvaluateCircuit::next() {
  printf("I: EvaluateCircuit::next\n");
  this->evaluateDriverLabels();
  return std::make_unique<P::SendFlagBit> (this->parameters, this->memory);
}

bool P::SendFlagBit::isSend() {
  return true;
}

bool P::SendFlagBit::getFlagBit() {
  auto& flagBitLabels = this->memory->flagBitLabels;
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto flagBitLabel = evaluatedDriverLabels.back();
  assert (
    flagBitLabel == flagBitLabels[0]
    or flagBitLabel == flagBitLabels[1]);
  return flagBitLabel == flagBitLabels[1];
}

std::string P::SendFlagBit::message() {
  return std::to_string(this->getFlagBit());
}

StatePtr P::SendFlagBit::next() {
  printf("I: SendOutputBit::next\n");
  auto& timer = this->memory->timer;
  printf("I: ==== round duration: %f ms ====\n", timer.display());
  timer.reset();
  if (this->getFlagBit())
    return std::make_unique<P::FaultObserved> (
      this->parameters, this->memory);
  else
    return std::make_unique<P::CopyMonitorStateLabels> (
      this->parameters, this->memory);
}

StatePtr P::FaultObserved::next() {
  printf("I: FaultObserved::next\n");
  return std::make_unique<P::MonitorDone> (this->parameters, this->memory);
}

StatePtr P::CopyMonitorStateLabels::next() {
  printf("I: CopyMonitorStateLabels::next\n");
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto monitorStateLength = this->parameters->monitorStateLength;
  auto offset =
    this->parameters->systemStateLength + this->parameters->gateCount - 1;
  // printf("D: offset = %d, # eval. labels = %lu\n", offset, evaluatedDriverLabels.size());
  for (unsigned i = 0; i < monitorStateLength; i++)
    evaluatedDriverLabels[i] = evaluatedDriverLabels[offset + i];
  return std::make_unique<P::RecvGarbledGates> (this->parameters, this->memory);
}

StatePtr P::MonitorDone::next() {
  printf("I: MonitorDone::next\n");
  return nullptr;
}
