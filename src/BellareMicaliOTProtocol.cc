#include "BellareMicaliOTProtocol.hh"
#include "Exceptions.hh"
#include "MathUtils.hh"
#include "StringUtils.hh"

namespace P = BellareMicaliOTProtocol;

bool P::State::isSend() {
  return false;
}

bool P::State::isRecv() {
  return false;
}

std::string P::State::message() {
  throw NonSendStateHasNoMessage();
}

P::SenderInterface::SenderInterface(
  P::ParameterSet* parameters,
  P::SenderMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    memory(memory),
    messageHandler(messageHandler) {
  this->state = std::make_unique<InitSender>(
    this->parameters, this->memory);
}

void P::SenderInterface::sync() {
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void P::SenderInterface::next() {
  this->sync();
  this->state = this->state->next();
}

void P::SenderInterface::run() {
  while (this->state)
    this->next();
}

P::ChooserInterface::ChooserInterface(
  ParameterSet* parameters,
  ChooserMemory* memory,
  MessageHandler* messageHandler)
  : parameters(parameters),
    memory(memory),
    messageHandler(messageHandler) {
  this->state = std::make_unique<InitChooser>
    (this->parameters, this->memory);
}

void P::ChooserInterface::sync() {
  if (this->state->isSend())
    this->messageHandler->send(this->state->message());
  else if (this->state->isRecv())
    this->memory->receivedMessage = this->messageHandler->recv();
}

void P::ChooserInterface::next() {
  this->sync();
  this->state = this->state->next();
}

void P::ChooserInterface::run() {
  while (this->state)
    this->next();
}

P::SenderState::SenderState(
  ParameterSet* parameters, SenderMemory* memory)
  : parameters(parameters), memory(memory) {}

P::InitSender::InitSender(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

P::StatePtr P::InitSender::next() {
  printf("I: InitSender::next\n");
  return std::make_unique<GenerateConstant> (
    this->parameters, this->memory);
}

P::GenerateConstant::GenerateConstant(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

P::StatePtr P::GenerateConstant::next() {
  printf("I: GenerateConstant::next\n");
  this->memory->constant = this->parameters->group.randomGenerator();
  printf("D:   generated constant: %s\n",
    toString(this->memory->constant, P::MSG_NUM_BASE).c_str());
  return std::make_unique<SendConstant> (
    this->parameters, this->memory);
}

P::SendConstant::SendConstant(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

bool P::SendConstant::isSend() {
  return true;
}

std::string P::SendConstant::message() {
  return toString(this->memory->constant, P::MSG_NUM_BASE);
}

P::StatePtr P::SendConstant::next() {
  printf("I: SendConstant::next\n");
  return std::make_unique<RecvPublicKey> (
    this->parameters, this->memory);
}

P::RecvPublicKey::RecvPublicKey(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

bool P::RecvPublicKey::isRecv() {
  return true;
}

void P::RecvPublicKey::evaluatePublicKeys(BigInt receivedKey) {
  auto group = this->parameters->group;
  this->memory->publicKeys[0] = receivedKey;
  this->memory->publicKeys[1] = group.mul(
    this->memory->constant,
    group.inv(receivedKey)
  );
}

P::StatePtr P::RecvPublicKey::next() {
  printf("I: RecvPublicKey::next\n");
  auto& message = this->memory->receivedMessage;
  auto receivedKey = BigInt(message, P::MSG_NUM_BASE);
  this->evaluatePublicKeys(receivedKey);
  return std::make_unique<EncryptMessages> (
    this->parameters, this->memory);
}

P::EncryptMessages::EncryptMessages(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

std::string P::EncryptMessages::encrypt(
  const std::string& message, const std::string& key)
{
  size_t textLen = message.length();
  std::string encryptedMessage(textLen, '0');
  for (size_t i = 0; i < textLen; i++) {
    encryptedMessage[i] = HEX_ALPHABET[
      hexValue(message[i]) ^ hexValue(key[i])];
  }
  return encryptedMessage;
}

P::StatePtr P::EncryptMessages::next() {
  printf("I: EncryptMessages::next\n");
  auto group = this->parameters->group;
  for (size_t i = 0; i < 2; i++) {
    auto randomExponent = this->parameters->group.randomExponent();
    this->memory->encryptionElements[i] = toString(
      group.exp(group.baseGenerator, randomExponent), P::MSG_NUM_BASE);
    auto expdPubKey = toString(
      group.exp(this->memory->publicKeys[i], randomExponent), P::MSG_NUM_BASE);
    auto hashedExpdPubKey = hashSha512(expdPubKey);
    auto message = this->memory->messages[i];
    this->memory->encryptedMessages[i] =
      this->encrypt(message, hashedExpdPubKey);
  }
  return std::make_unique<SendEncryptedMessages> (
    this->parameters, this->memory);
}

P::SendEncryptedMessages::SendEncryptedMessages(
  ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

bool P::SendEncryptedMessages::isSend() {
  return true;
}

std::string P::SendEncryptedMessages::message() {
  return
    this->memory->encryptionElements[0] + ' '
    + this->memory->encryptedMessages[0] + ' '
    + this->memory->encryptionElements[1] + ' '
    + this->memory->encryptedMessages[1];
}

P::StatePtr P::SendEncryptedMessages::next() {
  printf("I: SendEncryptedMessages::next\n");
  return std::make_unique<SenderDone> (
    this->parameters, this->memory);
}

P::SenderDone::SenderDone(ParameterSet* parameters, SenderMemory* memory)
  : SenderState(parameters, memory) {}

P::StatePtr P::SenderDone::next() {
  printf("I: SenderDone::next\n");
  return nullptr;
}

P::ChooserState::ChooserState(
  ParameterSet* parameters, ChooserMemory* memory)
  : parameters(parameters), memory(memory) {}

P::InitChooser::InitChooser(ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

P::StatePtr P::InitChooser::next() {
  printf("I: InitChooser::next\n");
  return std::make_unique<RecvConstant> (
    this->parameters, this->memory);
}

P::RecvConstant::RecvConstant(ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

bool P::RecvConstant::isRecv() {
  return true;
}

P::StatePtr P::RecvConstant::next() {
  printf("I: RecvConstant::next\n");
  std::cout << "D: received message: " << this->memory->receivedMessage << '\n';
  auto& message = this->memory->receivedMessage;
  this->memory->senderConstant = BigInt(message, P::MSG_NUM_BASE);
  return std::make_unique<GeneratePublicKey> (
    this->parameters, this->memory);
}

P::GeneratePublicKey::GeneratePublicKey(
  ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

P::StatePtr P::GeneratePublicKey::next() {
  printf("I: GeneratePublicKey::next\n");
  this->memory->key = this->parameters->group.randomExponent();
  return std::make_unique<SendPublicKey> (
    this->parameters, this->memory);
}

P::SendPublicKey::SendPublicKey(
  ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

bool P::SendPublicKey::isSend() {
  return true;
}

std::string P::SendPublicKey::message() {
  auto group = this->parameters->group;
  BigInt pubKeys[2];
  const bool sigma = this->memory->sigma;
  pubKeys[sigma] = group.exp(
    group.baseGenerator, this->memory->key);
  pubKeys[not sigma] = group.mul(
    this->memory->senderConstant,
    group.inv(pubKeys[sigma]));
  return toString(pubKeys[0], P::MSG_NUM_BASE);
}

P::StatePtr P::SendPublicKey::next() {
  printf("I: SendPublicKey::next\n");
  return std::make_unique<RecvEncryptedMessages> (
    this->parameters, this->memory);
}

P::RecvEncryptedMessages::RecvEncryptedMessages(
  ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

bool P::RecvEncryptedMessages::isRecv() {
  return true;
}

P::StatePtr P::RecvEncryptedMessages::next() {
  printf("I: RecvEncryptedMessages::next\n");
  auto& message = this->memory->receivedMessage;
  std::string parsedMessage[4];
  std::stringstream ss(message);
  for (size_t i = 0; i < 4; i++)
    ss >> parsedMessage[i];
  auto elementIndex = 2 * this->memory->sigma;
  this->memory->encryptionElement = parsedMessage[elementIndex];
  this->memory->encryptedMessage = parsedMessage[elementIndex + 1];
  return std::make_unique<DecryptChosenMessage> (
    this->parameters, this->memory);
}

P::DecryptChosenMessage::DecryptChosenMessage(
  ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

std::string P::DecryptChosenMessage::decrypt(
  const std::string& message, const std::string& key)
{
  size_t textLen = message.length();
  std::string decryptedMessage(textLen, '0');
  for (size_t i = 0; i < textLen; i++) {
    decryptedMessage[i] = HEX_ALPHABET[
      hexValue(message[i]) ^ hexValue(key[i])];
  }
  return decryptedMessage;
}

P::StatePtr P::DecryptChosenMessage::next() {
  printf("I: DecryptChosenMessage::next\n");
  auto group = this->parameters->group;
  auto encryptionElement = BigInt(
    this->memory->encryptionElement, P::MSG_NUM_BASE);
  auto encryptionKey = group.exp(encryptionElement, this->memory->key);
  auto hashedEncKey = hashSha512(toString(encryptionKey, P::MSG_NUM_BASE));
  auto& encryptedMessage = this->memory->encryptedMessage;
  this->memory->chosenMessage = this->decrypt(encryptedMessage, hashedEncKey);
  printf("D: decrypted message %s\n", this->memory->chosenMessage.c_str());
  return std::make_unique<ChooserDone> (this->parameters, this->memory);
}

P::ChooserDone::ChooserDone(ParameterSet* parameters, ChooserMemory* memory)
  : ChooserState(parameters, memory) {}

P::StatePtr P::ChooserDone::next() {
  printf("I: ChooserDone::next\n");
  return nullptr;
}
