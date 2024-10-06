#ifndef INSECURE_PARITY_PROTOCOL_HH
#define INSECURE_PARITY_PROTOCOL_HH

#include "Protocol.hh"

enum class UBPStage {
  SendBitset,
  SendParity,
  Done,
};

template <class P>
class InsecureParityProtocol
  : public Protocol<UBPStage, P> {
public:
  // Any concrete constructor must know
  // the role of each engaging party.
  // As the same protocol is instantiated for two parties,
  // the constructor also gets the party
  // that is running the protocol.
  InsecureParityProtocol(
    P bitHolder, P evaluator, P party);
  void next(std::string message) override;
  std::string currentMessage() override;
  bool isOver() override;
private:
  // Parties' data are stored in the protocol,
  // as the protocol is the entity that requires them.
  bool parity = false;
  P bitHolder, evaluator;
};

#endif
