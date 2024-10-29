#ifndef BELLARE_MICALI_OT_PROTOCOL_HH
#define BELLARE_MICALI_OT_PROTOCOL_HH

#include "Protocol.hh"
#include "QuadraticResidueGroup.hh"
#include "MessageHandler.hh"

// This Oblivious Transfer (OT) protocol was
// first introduced by Bellare and Micali,
// and later amended by Naor and Pinkas
// in the following paper:
// https://dl.acm.org/doi/10.5555/365411.365502

namespace BellareMicaliOTProtocol {
  const int MSG_NUM_BASE = 16;

  class ParameterSet {
  public:
    QuadraticResidueGroup group;
  };

  class State;
  typedef std::unique_ptr<State> StatePtr;

  class State {
  public:
    // Both 'isSend' and 'isRecv' return false by default;
    // child classes override only to return true.
    // Communication states are marked with 'Send' or 'Recv' prefixes.
    virtual bool isSend();
    virtual bool isRecv();
    virtual std::string message();
    virtual StatePtr next() = 0;
  };

  class SenderMemory {
  public:
    BigInt constant;
    BigInt publicKeys[2];
    std::string messages[2];
    std::string encryptionElements[2];
    std::string encryptedMessages[2];
    std::string receivedMessage;
  };

  class SenderInterface {
  public:
    SenderInterface(
      ParameterSet parameters,
      SenderMemory memory,
      MessageHandler& messageHandler);
    void sync();
    void next();
    void run();
  private:
    ParameterSet parameters;
    SenderMemory memory;
    MessageHandler& messageHandler;
    StatePtr state;
  };

  class ChooserMemory {
  public:
    BigInt senderConstant;
    BigInt key;
    bool sigma;
    std::string encryptionElement;
    std::string encryptedMessage;
    std::string chosenMessage;
    // Not to be confused with Sender's messages;
    // receivedMessage stores the message received
    // via a protocol `Recv` state.
    std::string receivedMessage;
  };

  class ChooserInterface {
  public:
    ChooserInterface(
      ParameterSet parameters,
      ChooserMemory memory,
      MessageHandler& messageHandler);
    void sync();
    void next();
    void run();
  private:
    ParameterSet parameters;
    ChooserMemory memory;
    MessageHandler& messageHandler;
    StatePtr state;
  };

  class SenderState : public State {
  public:
    SenderState(ParameterSet& parameters, SenderMemory& memory);
    virtual StatePtr next() = 0;
  protected:
    ParameterSet& parameters;
    SenderMemory& memory;
  };

  class ChooserState : public State {
  public:
    ChooserState(ParameterSet& parameters, ChooserMemory& memory);
    virtual StatePtr next() = 0;
  protected:
    ParameterSet& parameters;
    ChooserMemory& memory;
  };

  class InitSender : public SenderState {
  public:
    InitSender(ParameterSet& parameters, SenderMemory& memory);
    StatePtr next() override;
  };

  class GenerateConstant : public SenderState {
  public:
    GenerateConstant(ParameterSet& parameters, SenderMemory& memory);
    StatePtr next() override;
  };

  class SendConstant : public SenderState {
  public:
    SendConstant(ParameterSet& parameters, SenderMemory& memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class RecvPublicKey : public SenderState {
  public:
    RecvPublicKey(ParameterSet& parameters, SenderMemory& memory);
    bool isRecv() override;
    StatePtr next() override;
  private:
    void evaluatePublicKeys(BigInt receivedKey);
  };

  class EncryptMessages : public SenderState {
  public:
    EncryptMessages(ParameterSet& parameters, SenderMemory& memory);
    StatePtr next() override;
  private:
    std::string encrypt(const std::string& message, const std::string& key);
  };

  class SendEncryptedMessages : public SenderState {
  public:
    SendEncryptedMessages(ParameterSet& parameters, SenderMemory& memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class SenderDone : public SenderState {
  public:
    SenderDone(ParameterSet& parameters, SenderMemory& memory);
    StatePtr next() override;
  };

  class InitChooser : public ChooserState {
  public:
    InitChooser(ParameterSet& parameters, ChooserMemory& memory);
    StatePtr next() override;
  };

  class RecvConstant : public ChooserState {
  public:
    RecvConstant(ParameterSet& parameters, ChooserMemory& memory);
    bool isRecv() override;
    StatePtr next() override;
  };

  class GeneratePublicKey : public ChooserState {
  public:
    GeneratePublicKey(ParameterSet& parameters, ChooserMemory& memory);
    StatePtr next() override;
  };

  class SendPublicKey : public ChooserState {
  public:
    SendPublicKey(ParameterSet& parameters, ChooserMemory& memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class RecvEncryptedMessages : public ChooserState {
  public:
    RecvEncryptedMessages(ParameterSet& parameters, ChooserMemory& memory);
    bool isRecv() override;
    StatePtr next() override;
  };

  class DecryptChosenMessage : public ChooserState {
  public:
    DecryptChosenMessage(ParameterSet& parameters, ChooserMemory& memory);
    StatePtr next() override;
  private:
    std::string decrypt(const std::string& message, const std::string& key);
  };

  class ChooserDone : public ChooserState {
  public:
    ChooserDone(ParameterSet& parameters, ChooserMemory& memory);
    StatePtr next() override;
  };
}

#endif
