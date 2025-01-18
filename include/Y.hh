#ifndef YAO_PROTOCOL_HH
#define YAO_PROTOCOL_HH

#include "Circuit.hh"
#include "State.hh"
#include "YaoGarbler.hh"
#include "MonitorableSystem.hh"
#include "MessageHandler.hh"
#include "Timer.hh"
#include "BM.hh"

namespace Y {
  class ParameterSet {
  public:
    // Circuit description
    unsigned gateCount;
    unsigned monitorStateLength;
    unsigned systemStateLength;
    // Output length is always monitorStateLength + 1.

    // Encryption parameters
    YaoGarbler* garbler;
    unsigned securityParameter;
    unsigned inputLength();
  };

  class SystemMemory {
  public:
    Circuit* circuit;
    MonitorableSystem* system;
    std::vector<LabelPair> driverLabels;
    std::vector<GarbledGate> garbledGates;
    bool isFirstRound = true;
    // Whenever the current protocol state is a 'Recv' state,
    // receivedMessage stores the message received for that state.
    std::string receivedMessage;
    // for timing purposes.
    Timer timer;
  };

  class SystemState : public State {
  public:
    SystemState(ParameterSet* parameters, SystemMemory* memory);
    virtual StatePtr next() = 0;
  protected:
    ParameterSet* parameters;
    SystemMemory* memory;
  };

  class MonitorMemory {
  public:
    Circuit* circuit;
    std::vector<GarbledGate> garbledGates;
    std::vector<Label> evaluatedDriverLabels;
    LabelPair flagBitLabels;
    bool isFirstRound = true;
    std::string receivedMessage;
    Timer timer;
  };

  class MonitorState : public State {
  public:
    MonitorState(ParameterSet* parameters, MonitorMemory* memory);
    virtual StatePtr next() = 0;
  protected:
    ParameterSet* parameters;
    MonitorMemory* memory;
  };

  class SystemInterface {
  public:
    SystemInterface(
      ParameterSet* parameters,
      SystemMemory* memory,
      MessageHandler* messageHandler);
    void sync();
    void next();
    void run();
  private:
    ParameterSet* parameters;
    MessageHandler* messageHandler;
    SystemMemory* memory;
    StatePtr state;
  };

  class MonitorInterface {
  public:
    MonitorInterface(
      ParameterSet* parameters,
      MonitorMemory* memory,
      MessageHandler* messageHandler);
    void sync();
    void next();
    void run();
  private:
    ParameterSet* parameters;
    MessageHandler* messageHandler;
    MonitorMemory* memory;
    StatePtr state;
  };

  class InitSystem : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  };

  class RecvCircuit : public SystemState {
  public:
    using SystemState::SystemState;
    bool isRecv() override;
    StatePtr next() override;
  private:
    void parseCircuit(const std::string& circuitString);
  };

  class InitMonitorStateLabels : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  };

  class GenerateGarbledGates : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  private:
    void garble();
    void fillDriverLabels();
  };

  class SendGarbledGates : public SystemState {
  public:
    using SystemState::SystemState;
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class SendSystemInputLabels : public SystemState {
  public:
    using SystemState::SystemState;
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    // Certain methods, such as the following,
    // are annotated with a '_Timed' suffix.
    // This is a *TEMPORARY* measure to signal
    // these methods are being timed.
    std::vector<Label> systemInputLabels();
    LabelPair flagBitLabels();
  };

  class SendFlagBitLabels : public SystemState {
  public:
    using SystemState::SystemState;
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    LabelPair flagBitLabels();
  };

  class SystemObliviousTransfer : public SystemState {
  public:
    SystemObliviousTransfer(ParameterSet* parameters, SystemMemory* memory);
    SystemObliviousTransfer(const SystemObliviousTransfer& other) = delete;
    SystemObliviousTransfer(SystemObliviousTransfer&& other) = default;
    bool isSend() override;
    bool isRecv() override;
    std::string message() override;
    StatePtr next() override;
  private:
    std::unique_ptr<BM::ParameterSet> OTParameters;
    std::unique_ptr<BM::SenderMemory> senderMemory;
    StatePtr state;
    Timer OTTimer;
    void setOTMessages();
    unsigned counter;
  };

  class RecvFlagBit : public SystemState {
  public:
    using SystemState::SystemState;
    bool isRecv() override;
    StatePtr next() override;
  };

  class UpdateSystem : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  };

  class SystemCopyMonitorStateLabels : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  };

  class SystemDone : public SystemState {
  public:
    using SystemState::SystemState;
    StatePtr next() override;
  };

  class InitMonitor : public MonitorState {
  public:
    using MonitorState::MonitorState;
    StatePtr next() override;
  };

  class SendCircuit : public MonitorState {
  public:
    using MonitorState::MonitorState;
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class RecvGarbledGates : public MonitorState {
  public:
    using MonitorState::MonitorState;
    bool isRecv() override;
    StatePtr next() override;
  };

  class RecvSystemInputLabels : public MonitorState {
  public:
    using MonitorState::MonitorState;
    bool isRecv() override;
    StatePtr next() override;
  };

  class RecvFlagBitLabels : public MonitorState {
  public:
    using MonitorState::MonitorState;
    bool isRecv() override;
    StatePtr next() override;
  };

  class MonitorObliviousTransfer : public MonitorState {
  public:
    MonitorObliviousTransfer(ParameterSet* parameters, MonitorMemory* memory);
    MonitorObliviousTransfer(const MonitorObliviousTransfer& other) = delete;
    MonitorObliviousTransfer(MonitorObliviousTransfer&& other) = default;
    bool isSend() override;
    bool isRecv() override;
    std::string message() override;
    StatePtr next() override;
  private:
    std::unique_ptr<BM::ParameterSet> OTParameters;
    std::unique_ptr<BM::ChooserMemory> chooserMemory;
    StatePtr state;
    unsigned counter;
    void setSigma();
  };

  class EvaluateCircuit : public MonitorState {
  public:
    using MonitorState::MonitorState;
    StatePtr next() override;
  private:
    std::string padLabel(BigInt label);
    std::vector<unsigned> getUnshuffling_Timed();
    void evaluateDriverLabels();
  };

  class SendFlagBit : public MonitorState {
  public:
    using MonitorState::MonitorState;
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    bool getFlagBit();
  };

  class FaultObserved : public MonitorState {
  public:
    using MonitorState::MonitorState;
    StatePtr next() override;
  };

  class MonitorCopyMonitorStateLabels : public MonitorState {
  public:
    using MonitorState::MonitorState;
    StatePtr next() override;
  };

  class MonitorDone : public MonitorState {
  public:
    using MonitorState::MonitorState;
    StatePtr next() override;
  };
}

#endif
