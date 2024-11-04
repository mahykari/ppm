#include <string>
#include <vector>
#include "BigInt.hh"
#include "QuadraticResidueGroup.hh"
#include "Sha512YaoGarbler.hh"
#include "Circuit.hh"
#include "MonitorableSystem.hh"
#include "MessageHandler.hh"
#include "BellareMicaliOTProtocol.hh"

namespace LiuEtAlMonitoringProtocol {
  const unsigned SYSTEM_PORT = 5555;
  const unsigned MONITOR_PORT = 5556;
  const unsigned MAX_BUFFER_SIZE = 1 << 16;
  const int MSG_NUM_BASE = 16;

  class ParameterSet {
  public:
    // Parameters passed to this protocol
    // can be put together as follows:
    // * Circuit description
    unsigned gateCount;
    unsigned monitorStateLength;
    unsigned systemStateLength;
    // Output length is always monitorStateLength + 1.

    // * Encryption parameters
    QuadraticResidueGroup group;
    Sha512YaoGarbler garbler;
    unsigned securityParameter;
    unsigned inputLength();
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

  class SystemMemory {
  public:
    MonitorableSystem* system;
    std::vector<BigInt> driverLabels;
    std::vector<BigInt> inWireLabels;
    std::vector<GarbledGate> garbledGates;
    std::array<BigInt, 2> garblingExponents;
    std::array<BigInt, 2> nextRoundGarblingExponents;
    bool isFirstRound = true;
    std::string receivedMessage;
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
    std::vector<Driver*> shuffledCircuit;
    std::vector<BigInt> driverLabels;
    std::vector<BigInt> inWireKeys;
    std::vector<GarbledGate> garbledGates;
    std::vector<BigInt> evaluatedDriverLabels;
    std::array<BigInt, 2> flagBitLabels;
    bool isFirstRound = true;
    std::string receivedMessage;
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

  // The following is a list of all Monitor and System states,
  // with each State inheriting from its corresponding base class.

  class InitSystem : public SystemState {
  public:
    InitSystem(ParameterSet* parameters, SystemMemory* memory);
    StatePtr next() override;
  };

  class RecvLabels : public SystemState {
  public:
    RecvLabels(ParameterSet* parameters, SystemMemory* memory);
    bool isRecv() override;
    StatePtr next() override;
  private:
    void parseLabels();
    void generateFlagBitLabel();
  };

  class GenerateGarbledGates : public SystemState {
  public:
    GenerateGarbledGates(ParameterSet* parameters, SystemMemory* memory);
    StatePtr next() override;
  private:
    void generateGarblingExponents();
    std::array<BigInt, 2> expLabel(
      BigInt label,
      std::array<BigInt, 2> exponents);
    std::string padLabel(BigInt label);
    void garble();
  };

  class SendGarbledGates : public SystemState {
  public:
    SendGarbledGates(ParameterSet* parameters, SystemMemory* memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  };

  class SendSystemInputLabels : public SystemState {
  public:
    SendSystemInputLabels(ParameterSet* parameters, SystemMemory* memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    std::vector<BigInt> systemInputLabels();
    std::array<BigInt, 2> flagBitLabels();
  };

  class SendFlagBitLabels : public SystemState {
  public:
    SendFlagBitLabels(ParameterSet* parameters, SystemMemory* memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    std::array<BigInt, 2> flagBitLabels();
  };

  namespace B = BellareMicaliOTProtocol;

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
    std::unique_ptr<B::ParameterSet> OTParameters;
    std::unique_ptr<B::SenderMemory> senderMemory;
    B::StatePtr state;
    void setOTMessages();
    unsigned counter;
  };

  class RecvFlagBit : public SystemState {
  public:
    RecvFlagBit(ParameterSet* parameters, SystemMemory* memory);
    bool isRecv() override;
    StatePtr next() override;
  };

  class UpdateSystem : public SystemState {
  public:
    UpdateSystem(ParameterSet* parameters, SystemMemory* memory);
    StatePtr next() override;
  };

  class SystemDone : public SystemState {
  public:
    SystemDone(ParameterSet* parameters, SystemMemory* memory);
    StatePtr next() override;
  };

  class InitMonitor : public MonitorState {
  public:
    InitMonitor(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };

  class GenerateDriverLabels : public MonitorState {
  public:
    GenerateDriverLabels(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };

  class GenerateInWireKeys : public MonitorState {
  public:
    GenerateInWireKeys(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };

  class SendLabels : public MonitorState {
  public:
    SendLabels(ParameterSet* parameters, MonitorMemory* memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    std::vector<BigInt> generateInWireLabels();
    void shuffleCircuit();
  };

  class RecvGarbledGates : public MonitorState {
  public:
    RecvGarbledGates(ParameterSet* parameters, MonitorMemory* memory);
    bool isRecv() override;
    StatePtr next() override;
  };

  class RecvSystemInputLabels : public MonitorState {
  public:
    RecvSystemInputLabels(ParameterSet* parameters, MonitorMemory* memory);
    bool isRecv() override;
    StatePtr next() override;
  };

  class RecvFlagBitLabels : public MonitorState {
  public:
    RecvFlagBitLabels(ParameterSet* parameters, MonitorMemory* memory);
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
    std::unique_ptr<B::ParameterSet> OTParameters;
    std::unique_ptr<B::ChooserMemory> chooserMemory;
    B::StatePtr state;
    unsigned counter;
    void setSigma();
  };

  class EvaluateCircuit : public MonitorState {
  public:
    EvaluateCircuit(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  private:
    std::string padLabel(BigInt label);
    std::vector<unsigned> getUnshuffling();
    void evaluateDriverLabels();
  };

  class SendFlagBit : public MonitorState {
  public:
    SendFlagBit(ParameterSet* parameters, MonitorMemory* memory);
    bool isSend() override;
    std::string message() override;
    StatePtr next() override;
  private:
    bool getFlagBit();
  };

  class FaultObserved : public MonitorState {
  public:
    FaultObserved(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };

  class CopyMonitorStateLabels : public MonitorState {
  public:
    CopyMonitorStateLabels(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };

  class MonitorDone : public MonitorState {
  public:
    MonitorDone(ParameterSet* parameters, MonitorMemory* memory);
    StatePtr next() override;
  };
};
