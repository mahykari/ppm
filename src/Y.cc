#include "Y.hh"
#include "StringUtils.hh"
#include "MathUtils.hh"
#include "QuadraticResidueGroup.hh"

unsigned Y::ParameterSet::inputLength() {
  return this->monitorStateLength + this->systemStateLength;
}

Y::SystemInterface::SystemInterface(
  ParameterSet* parameters,
  SystemMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    messageHandler(messageHandler),
    memory(memory) {
  state = std::make_unique<InitSystem>(this->parameters, this->memory);
}

Y::MonitorInterface::MonitorInterface(
  ParameterSet* parameters,
  MonitorMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    messageHandler(messageHandler),
    memory(memory) {
  state = std::make_unique<InitMonitor>(this->parameters, this->memory);
}

Y::SystemState::SystemState(
  ParameterSet* parameters,
  SystemMemory* memory)
  : parameters(parameters), memory(memory) {}

Y::MonitorState::MonitorState(
  ParameterSet* parameters,
  MonitorMemory* memory)
  : parameters(parameters), memory(memory) {}

void Y::SystemInterface::sync() {
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void Y::SystemInterface::next() {
  this->memory->timer.pause();
  this->sync();
  this->memory->timer.resume();
  this->state = this->state->next();
}

void Y::SystemInterface::run() {
  this->memory->timer.start();
  while (this->state)
    this->next();
}

StatePtr Y::InitSystem::next() {
  printf("I: InitSystem::next\n");
  fflush(stdout);
  return std::make_unique<RecvCircuit>(this->parameters, this->memory);
}

bool Y::RecvCircuit::isRecv() {
  return true;
}

void Y::RecvCircuit::parseCircuit(const std::string& circuitString) {
  auto circuit = this->memory->circuit;
  assert (circuit != nullptr);
  auto inputLength = this->parameters->inputLength();
  // The circuit should not contain any gates at this stage;
  // so, there are just as many drivers as circuit inputs.
  assert (circuit->size() == inputLength);
  std::vector<std::string> gateSpecs;
  auto gateCount = this->parameters->gateCount;
  std::tie(gateSpecs, std::ignore) =
    readStrings(circuitString, 3 * gateCount);
  for (unsigned i = 0; i < gateCount; i++) {
    auto inputLeft = std::stoul(gateSpecs[3 * i]);
    auto inputRight = std::stoul(gateSpecs[3 * i + 1]);
    auto id = std::stoul(gateSpecs[3 * i + 2]);
    // Since the circuit uses an increment counter for gate ID's,
    // Each received gate ID should be equal to the current counter value.
    assert (id == i + inputLength);
    circuit->addGate(inputLeft, inputRight);
    // No need to set output for the circuit,
    // as it is not evaluated on System side.
  }
}

StatePtr Y::RecvCircuit::next() {
  printf("I: RecvCircuit::next\n");
  fflush(stdout);
  auto message = this->memory->receivedMessage;

  parseCircuit(message);
  return std::make_unique<InitMonitorStateLabels>
    (this->parameters, this->memory);
}

StatePtr Y::InitMonitorStateLabels::next() {
  printf("I: InitMonitorStateLabels::next\n");
  fflush(stdout);
  auto secParam = this->parameters->securityParameter;
  auto secLen = secParam >> 2;
  auto& driverLabels = this->memory->driverLabels;
  assert (driverLabels.empty());
  for (unsigned i = 0; i < parameters->monitorStateLength; i++) {
    driverLabels.push_back(
      { randomHexString(secLen), randomHexString(secLen) } );
  }

  return std::make_unique<GenerateGarbledGates>
    (this->parameters, this->memory);
}

void Y::GenerateGarbledGates::fillDriverLabels() {
  auto& driverLabels = this->memory->driverLabels;
  // Only monitor state labels should be filled at this stage.
  auto monitorStateLength = this->parameters->monitorStateLength;
  assert (driverLabels.size() == monitorStateLength);
  auto circuit = this->memory->circuit;
  auto driverCount = circuit->size();
  driverLabels.resize(driverCount);
  auto secParam = this->parameters->securityParameter;
  auto secLen = secParam >> 2;
  for (unsigned i = monitorStateLength; i < driverCount; i++)
    driverLabels[i] =
      { randomHexString(secLen), randomHexString(secLen) };
}

void Y::GenerateGarbledGates::garble() {
  auto& driverLabels = this->memory->driverLabels;
  auto& garbledGates = this->memory->garbledGates;
  auto gateCount = this->parameters->gateCount;
  garbledGates.resize(gateCount);
  auto offset =
    this->parameters->systemStateLength +
    this->parameters->monitorStateLength;
  auto drivers = this->memory->circuit->get();
  for (unsigned i = 0; i < gateCount; i++) {
    // Generate garbled gates using the garbler
    auto gate = static_cast<Gate*>(drivers[offset + i]);
    auto leftIdx = gate->inputLeft;
    auto rightIdx = gate->inputRight;
    // printf("D:   garbling gate %d\n", i);
    // printf("D:     left  labels (%d): %s %s\n", leftIdx,
    //   driverLabels[leftIdx][0].c_str(),
    //   driverLabels[leftIdx][1].c_str());
    // printf("D:     right labels (%d): %s %s\n", rightIdx,
    //   driverLabels[rightIdx][0].c_str(),
    //   driverLabels[rightIdx][1].c_str());
    // fflush(stdout);
    garbledGates[i] =
      this->parameters->garbler->enc(
        driverLabels[leftIdx],
        driverLabels[rightIdx],
        driverLabels[offset + i] );
  }
}

StatePtr Y::GenerateGarbledGates::next() {
  printf("I: GenerateGarbledGates::next\n");
  fflush(stdout);
  this->fillDriverLabels();
  // printf("D:   driver labels:\n");
  // for (unsigned i = 0; i < this->memory->driverLabels.size(); i++) {
  //   printf("D:     %d: %s %s\n", i,
  //     this->memory->driverLabels[i][0].c_str(),
  //     this->memory->driverLabels[i][1].c_str());
  // }
  this->garble();
  return std::make_unique<SendGarbledGates>(this->parameters, this->memory);
}

bool Y::SendGarbledGates::isSend() {
  return true;
}

std::string Y::SendGarbledGates::message() {
  std::stringstream ss;
  for (auto& gate : this->memory->garbledGates)
    for (auto& label : gate)
      ss << label << ' ';
  return ss.str();
}

StatePtr Y::SendGarbledGates::next() {
  printf("I: SendGarbledGates::next\n");
  fflush(stdout);
  return std::make_unique<SendSystemInputLabels>
    (this->parameters, this->memory);
}

bool Y::SendSystemInputLabels::isSend() {
  return true;
}

std::vector<Label> Y::SendSystemInputLabels::systemInputLabels() {
  auto& systemData = this->memory->system->data();
  auto& driverLabels = this->memory->driverLabels;
  std::vector<Label> labels;
  labels.reserve(systemData.size());
  auto offset = this->parameters->monitorStateLength;
  for (unsigned i = 0; i < systemData.size(); i++) {
    auto index = systemData[i];
    auto driverLabelPair = driverLabels[offset + i];
    labels.push_back( driverLabelPair[index] );
  }
  return labels;
}

std::string Y::SendSystemInputLabels::message() {
  std::stringstream ss;
  for (auto& label : this->systemInputLabels())
    ss << label << ' ';
  return ss.str();
}

StatePtr Y::SendSystemInputLabels::next() {
  printf("I: SendSystemInputLabels::next\n");
  fflush(stdout);
  return std::make_unique<SendFlagBitLabels>(this->parameters, this->memory);
}

bool Y::SendFlagBitLabels::isSend() {
  return true;
}

LabelPair Y::SendFlagBitLabels::flagBitLabels() {
  // ASSUMPTION: flag bit is always output from the last gate (driver).
  auto& driverLabels = this->memory->driverLabels;
  return driverLabels.back();
}

std::string Y::SendFlagBitLabels::message() {
  std::stringstream ss;
  for (auto& label : this->flagBitLabels())
    ss << label << ' ';
  return ss.str();
}

StatePtr Y::SendFlagBitLabels::next() {
  printf("I: SendFlagBitLabels::next\n");
  fflush(stdout);
  if (this->memory->isFirstRound) {
    this->memory->isFirstRound = false;
    return std::make_unique<SystemObliviousTransfer>
      (this->parameters, this->memory);
  }
  return std::make_unique<RecvFlagBit> (this->parameters, this->memory);
}

Y::SystemObliviousTransfer::SystemObliviousTransfer(
  ParameterSet* parameters, SystemMemory* memory)
  : SystemState(parameters, memory)
{
  this->counter = 0;
  auto secParam = this->parameters->securityParameter;
  this->OTParameters = std::make_unique<BM::ParameterSet>(
    BM::ParameterSet {
      .securityParameter= secParam,
      .group = QuadraticResidueGroup(getSafePrime(secParam))
    }
  );
  this->senderMemory = std::make_unique<BM::SenderMemory>();
  this->setOTMessages();
  this->state = std::make_unique<BM::InitSender>
    (this->OTParameters.get(), this->senderMemory.get());
}

bool Y::SystemObliviousTransfer::isRecv() {
  if (this->state)
    return this->state->isRecv();
  return false;
}

bool Y::SystemObliviousTransfer::isSend() {
  if (this->state)
    return this->state->isSend();
  return false;
}

std::string Y::SystemObliviousTransfer::message() {
  return this->state->message();
}

void Y::SystemObliviousTransfer::setOTMessages() {
  auto& memory = this->memory;
  auto& senderMemory = this->senderMemory;
  LabelPair inputDriverLabels = memory->driverLabels[this->counter];
  for (unsigned i = 0; i < 2; i++)
    senderMemory->messages[i] = inputDriverLabels[i];
}

StatePtr Y::SystemObliviousTransfer::next() {
  printf("I: SystemObliviousTransfer::next\n");
  fflush(stdout);
  if (this->counter == this->parameters->monitorStateLength)
    return std::make_unique<RecvFlagBit>(this->parameters, this->memory);

  if (not this->state) {
    this->counter++;
    this->setOTMessages();
    this->state = std::make_unique<BM::InitSender>
      (this->OTParameters.get(), this->senderMemory.get());
  } else {
    this->senderMemory->receivedMessage = this->memory->receivedMessage;
    this->state = this->state->next();
  }
  return std::make_unique<SystemObliviousTransfer> (std::move(*this));
}

bool Y::RecvFlagBit::isRecv() {
  return true;
}

StatePtr Y::RecvFlagBit::next() {
  printf("I: RecvFlagBit::next\n");
  fflush(stdout);
  auto& message = this->memory->receivedMessage;
  bool flagBit = std::stoi(message);
  auto& timer = this->memory->timer;
  printf("D: ==== round duration: %f ms ====\n", timer.display());
  fflush(stdout);
  timer.reset();
  timer.start();
  if (flagBit)
    return std::make_unique<SystemDone>(this->parameters, this->memory);
  return std::make_unique<UpdateSystem>(this->parameters, this->memory);
}

StatePtr Y::UpdateSystem::next() {
  printf("I: UpdateSystem::next\n");
  fflush(stdout);
  this->memory->system->next();
  return std::make_unique<Y::SystemCopyMonitorStateLabels>(
    this->parameters, this->memory);
}

StatePtr Y::SystemCopyMonitorStateLabels::next() {
  printf("I: SystemCopyMonitorStateLabels::next\n");
  fflush(stdout);
  auto& driverLabels = this->memory->driverLabels;
  auto monitorStateLength = this->parameters->monitorStateLength;
  auto offset =
    this->parameters->systemStateLength + this->parameters->gateCount - 1;
  for (unsigned i = 0; i < monitorStateLength; i++)
    driverLabels[i] = driverLabels[offset + i];
  driverLabels.resize(monitorStateLength);
  return std::make_unique<GenerateGarbledGates>
    (this->parameters, this->memory);
}

StatePtr Y::SystemDone::next() {
  printf("I: SystemDone::next\n");
  fflush(stdout);
  return nullptr;
}

void Y::MonitorInterface::sync() {
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void Y::MonitorInterface::next() {
  this->memory->timer.pause();
  this->sync();
  this->memory->timer.resume();
  this->state = this->state->next();
}

void Y::MonitorInterface::run() {
  this->memory->timer.start();
  while (this->state)
    this->next();
}

StatePtr Y::InitMonitor::next() {
  printf("I: InitMonitor::next\n");
  fflush(stdout);
  return std::make_unique<SendCircuit>(this->parameters, this->memory);
}

bool Y::SendCircuit::isSend() {
  return true;
}

std::string Y::SendCircuit::message() {
  auto offset =
    this->parameters->monitorStateLength +
    this->parameters->systemStateLength;
  auto drivers = this->memory->circuit->get();
  std::stringstream ss;
  for (unsigned i = 0; i < this->parameters->gateCount; i++) {
    auto gate = static_cast<Gate*>( drivers[offset + i] );
    ss << gate->inputLeft << ' ';
    ss << gate->inputRight << ' ';
    ss << gate->id << ' ';
  }
  return ss.str();
}

StatePtr Y::SendCircuit::next() {
  printf("I: SendCircuit::next\n");
  fflush(stdout);
  return std::make_unique<RecvGarbledGates> (this->parameters, this->memory);
}

bool Y::RecvGarbledGates::isRecv() {
  return true;
}

StatePtr Y::RecvGarbledGates::next() {
  printf("I: RecvGarbledGates::next\n");
  fflush(stdout);
  auto message = this->memory->receivedMessage;
  auto gateCount = this->parameters->gateCount;
  auto& garbledGates = this->memory->garbledGates;
  std::tie(garbledGates, message) = readGarbledGates(message, gateCount);
  return std::make_unique<RecvSystemInputLabels>
    (this->parameters, this->memory);
}

bool Y::RecvSystemInputLabels::isRecv() {
  return true;
}

StatePtr Y::RecvSystemInputLabels::next() {
  printf("I: RecvSystemInputLabels::next\n");
  fflush(stdout);
  auto message = this->memory->receivedMessage;
  std::vector<Label> systemInputLabels;
  std::tie(systemInputLabels, message) =
    readStrings(message, this->parameters->systemStateLength);
  // Store the received system input labels in memory
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto offset = this->parameters->monitorStateLength;
  evaluatedDriverLabels.resize(this->memory->circuit->size());
  for (unsigned i = 0; i < systemInputLabels.size(); i++)
    evaluatedDriverLabels[offset + i] = systemInputLabels[i];
  return std::make_unique<RecvFlagBitLabels>(this->parameters, this->memory);
}

bool Y::RecvFlagBitLabels::isRecv() {
  return true;
}

StatePtr Y::RecvFlagBitLabels::next() {
  printf("I: RecvFlagBitLabels::next\n");
  fflush(stdout);
  auto message = this->memory->receivedMessage;
  std::vector<Label> flagBitLabels;
  std::tie(flagBitLabels, message) = readStrings(message, 2);
  this->memory->flagBitLabels[0] = flagBitLabels[0];
  this->memory->flagBitLabels[1] = flagBitLabels[1];
  if (this->memory->isFirstRound) {
    this->memory->isFirstRound = false;
    return std::make_unique<Y::MonitorObliviousTransfer>
      (this->parameters, this->memory);
  }
  return std::make_unique<Y::EvaluateCircuit>(this->parameters, this->memory);
}

Y::MonitorObliviousTransfer::MonitorObliviousTransfer(
  ParameterSet* parameters, MonitorMemory* memory)
  : MonitorState(parameters, memory)
{
  this->counter = 0;
  auto secParam = this->parameters->securityParameter;
  this->OTParameters = std::make_unique<BM::ParameterSet>
    (BM::ParameterSet {
      .securityParameter = secParam,
      .group = QuadraticResidueGroup(getSafePrime(secParam)) });
  this->chooserMemory = std::make_unique<BM::ChooserMemory>();
  this->state = std::make_unique<BM::InitChooser>
    (this->OTParameters.get(), this->chooserMemory.get());
}

bool Y::MonitorObliviousTransfer::isRecv() {
  if (this->state)
    return this->state->isRecv();
  return false;
}

bool Y::MonitorObliviousTransfer::isSend() {
  if (this->state)
    return this->state->isSend();
  return false;
}

std::string Y::MonitorObliviousTransfer::message() {
  return this->state->message();
}

void Y::MonitorObliviousTransfer::setSigma() {
  // ASSUMPTION: monitor starts in an all-zero state.
  this->chooserMemory->sigma = 0;
}

StatePtr Y::MonitorObliviousTransfer::next() {
  printf("I: MonitorObliviousTransfer::next\n");
  fflush(stdout);
  if (this->counter == this->parameters->monitorStateLength)
    return std::make_unique<Y::EvaluateCircuit>(this->parameters, this->memory);

  if (not this->state) {
    this->memory->evaluatedDriverLabels[this->counter] =
      this->chooserMemory->chosenMessage;
    this->counter++;
    this->setSigma();
    this->state = std::make_unique<BM::InitChooser>
      (this->OTParameters.get(), this->chooserMemory.get());
  } else {
    this->chooserMemory->receivedMessage = this->memory->receivedMessage;
    this->state = this->state->next();
  }
  return std::make_unique<Y::MonitorObliviousTransfer> (std::move(*this));
}

void Y::EvaluateCircuit::evaluateDriverLabels() {
  auto drivers = this->memory->circuit->get();
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto& garbledGates = this->memory->garbledGates;
  auto offset =
    this->parameters->monitorStateLength +
    this->parameters->systemStateLength;

  for (unsigned i = 0; i < this->parameters->gateCount; i++) {
    // printf("D:   evaluating gate %d\n", i);
    auto gate = static_cast<Gate*>(drivers[offset + i]);
    auto  leftLabel = evaluatedDriverLabels[gate->inputLeft];
    auto rightLabel = evaluatedDriverLabels[gate->inputRight];
    // printf("     left  label (%d): %s\n", gate->inputLeft, leftLabel.c_str());
    // printf("     right label (%d): %s\n", gate->inputRight, rightLabel.c_str());
    // fflush(stdout);
    auto outLabel = this->parameters->garbler->dec(
      leftLabel, rightLabel, garbledGates[gate->id - offset]);
    // printf("     out   label (%d): %s\n", gate->id, outLabel.c_str());
    evaluatedDriverLabels[gate->id] = outLabel;
  }
}

StatePtr Y::EvaluateCircuit::next() {
  printf("I: EvaluateCircuit::next\n");
  fflush(stdout);
  this->evaluateDriverLabels();
  return std::make_unique<Y::SendFlagBit> (this->parameters, this->memory);
}

bool Y::SendFlagBit::isSend() {
  return true;
}

bool Y::SendFlagBit::getFlagBit() {
  auto& flagBitLabels = this->memory->flagBitLabels;
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto flagBitLabel = evaluatedDriverLabels.back();
  assert (
    flagBitLabel == flagBitLabels[0] or
    flagBitLabel == flagBitLabels[1]   );
  return flagBitLabel == flagBitLabels[1];
}

std::string Y::SendFlagBit::message() {
  return std::to_string(this->getFlagBit());
}

StatePtr Y::SendFlagBit::next() {
  printf("I: SendFlagBit::next\n");
  fflush(stdout);
  auto& timer = this->memory->timer;
  printf("I: ==== round duration: %f ms ====\n", timer.display());
  timer.reset();
  timer.start();
  fflush(stdout);
  if (this->getFlagBit())
    return std::make_unique<Y::FaultObserved> (
      this->parameters, this->memory);
  else
    return std::make_unique<Y::MonitorCopyMonitorStateLabels> (
      this->parameters, this->memory);
}

StatePtr Y::FaultObserved::next() {
  printf("I: FaultObserved::next\n");
  fflush(stdout);
  return std::make_unique<MonitorDone> (this->parameters, this->memory);
}

StatePtr Y::MonitorCopyMonitorStateLabels::next() {
  printf("I: MonitorCopyMonitorStateLabels::next\n");
  fflush(stdout);
  auto monitorStateLength = this->parameters->monitorStateLength;
  auto& evaluatedDriverLabels = this->memory->evaluatedDriverLabels;
  auto offset =
    this->parameters->systemStateLength + this->parameters->gateCount - 1;
  for (unsigned i = 0; i < monitorStateLength; i++)
    evaluatedDriverLabels[i] = evaluatedDriverLabels[offset + i];
  return std::make_unique<Y::RecvGarbledGates> (this->parameters, this->memory);
}

StatePtr Y::MonitorDone::next() {
  printf("I: MonitorDone::next\n");
  fflush(stdout);
  return nullptr;
}
