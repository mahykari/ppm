#include <algorithm>
#include "InsecureParityProtocol.hh"
#include "Party.hh"

template <class P>
InsecureParityProtocol<P>::InsecureParityProtocol(
  P bitHolder, P evaluator, P party)
  : bitHolder(bitHolder), evaluator(evaluator)
{
  this->party = party;
  this->currentStage = UBPStage::SendBitset;
  this->currentSender = bitHolder;
}

template <class P>
void InsecureParityProtocol<P>::next(std::string message) {
  switch (this->currentStage) {
    case UBPStage::SendBitset:
      if (not this->isSender()) {
        printf("I: received bitset %s\n", message.c_str());
        this->parity = std::count(
          message.begin(), message.end(), '1') % 2;
      }
      this->currentStage = UBPStage::SendParity;
      this->currentSender = this->evaluator;
      break;
    case UBPStage::SendParity:
      if (not this->isSender()) {
        printf("I: received parity bit %d\n", std::stoi(message));
      }
      this->currentStage = UBPStage::Done;
      break;
    default: break;
  }
}

// https://xkcd.com/221/
const std::string RAND_STRING = "01011";

template <class P>
std::string InsecureParityProtocol<P>::currentMessage() {
  switch (this->currentStage) {
    case UBPStage::SendBitset:
      if (this->isSender())
        return RAND_STRING;
    case UBPStage::SendParity:
      if (this->isSender())
        return this->parity ? "1" : "0";
    default: return "";
  }
}

template <class P>
bool InsecureParityProtocol<P>::isOver() {
  return this->currentStage == UBPStage::Done;
}

template class InsecureParityProtocol<MonitoringComponent>;
