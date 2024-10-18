#ifndef SEMIHONEST_LIU_ET_AL_PROTOCOL_HH
#define SEMIHONEST_LIU_ET_AL_PROTOCOL_HH

#include <memory>
#include <vector>
#include <tuple>
#include "BigInt.hh"
#include "QuadraticResidueGroup.hh"
#include "Circuit.hh"
#include "Party.hh"
#include "Protocol.hh"
#include "Sha512YaoGarbler.hh"

// This is an infinite-round version of
// the Liu et al. protocol for
// semi-honest private function evaluation:
// https://eprint.iacr.org/2021/1682

enum class SemiHonestLiuEtAlStage {
  FirstSendOutWireLabels,
  FirstSendInWireLabels,
  // Differentiating between
  // FirstSendGarbledTables and SendGarbledTables,
  // as the former is only used in the first round
  // and is succeeded by FirstOT rather than Continue.
  FirstSendGarbledTables,
  FirstOT,
  SendGarbledTables,
  Continue,
  Done,
};

class LiuEtAlDataHolder {
public:
  std::vector<BigInt> outWireLabels;
  std::vector<BigInt> inWireLabels;
  std::vector<GarbledGate> garbledGates;
  std::array<BigInt, 2> garblingExponents;
  void fillGarbledGates(
    const int gateCount,
    QuadraticResidueGroup& group,
    YaoGarbler& garbler);
};

class LiuEtAlCircuitHolder {
public:
  Circuit* circuit;
  std::vector<BigInt> outWireLabels;
  std::vector<BigInt> inWireKeys;
  std::vector<Driver*> shuffledCircuit;
  std::vector<GarbledGate> garbledGates;
  std::vector<BigInt> evaluatedOutWireLabels;
  std::array<BigInt, 2> outputLabels;
  bool isDone;
};

class SemiHonestLiuEtAlProtocol
  : public Protocol<SemiHonestLiuEtAlStage, MonitoringComponent>
{
public:
  SemiHonestLiuEtAlProtocol(
    QuadraticResidueGroup group,
    int gateCount,
    int monitorStateLength,
    int systemStateLength,
    MonitoringComponent role);
  void next(std::string message) override;
  std::string currentMessage() override;
  bool isOver() override;
private:
  std::unique_ptr<LiuEtAlDataHolder> dataHolderMemory;
  std::unique_ptr<LiuEtAlCircuitHolder> circuitHolderMemory;
  QuadraticResidueGroup group;
  unsigned gateCount;
  unsigned monitorStateLength;
  unsigned systemStateLength;
  unsigned OTCounter;
  Sha512YaoGarbler garbler;
  std::tuple<std::vector<GarbledGate>, std::string>
    readGarbledTables(std::string message, int count);
  std::tuple<std::vector<BigInt>, std::string>
    readBigInts(std::string message, int count);
};

#endif
