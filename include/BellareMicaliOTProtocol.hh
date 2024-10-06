#ifndef BELLARE_MICALI_OT_PROTOCOL_HH
#define BELLARE_MICALI_OT_PROTOCOL_HH

#include "Protocol.hh"
#include "QuadraticResidueGroup.hh"

// This Oblivious Transfer (OT) protocol was
// first introduced by Bellare and Micali,
// and later amended by Naor and Pinkas
// in the following paper:
// https://dl.acm.org/doi/10.5555/365411.365502

enum class BellareMicaliOTStage {
  Init,
  SendPublicKey,
  SendEncryptedMessages,
  Done,
};

class BellareMicaliOTSender {
public:
  BigInt constant;
  BigInt publicKeys[2];
  std::string messages[2];
};

class BellareMicaliOTChooser {
public:
  BigInt senderConstant;
  BigInt key;
  bool sigma;
};

template <typename P>
class BellareMicaliOTProtocol
  : public Protocol<BellareMicaliOTStage, P>
{
public:
  BellareMicaliOTProtocol(
    QuadraticResidueGroup group,
    P chooser, P sender, P role);
  void updateChooser(bool sigma);
  void updateSender(std::string messages[2]);
  void next(std::string message) override;
  std::string currentMessage() override;
  bool isOver() override;
private:
  QuadraticResidueGroup group;
  P chooser, sender;
  std::unique_ptr<BellareMicaliOTSender> senderData;
  std::unique_ptr<BellareMicaliOTChooser> chooserData;
};

#endif
