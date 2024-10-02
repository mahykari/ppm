#ifndef PROTOCOL_HH
#define PROTOCOL_HH

#include <string>

// ******************************************************************
// ASSUMPTION: protocols have exactly two parties.                  *
// ******************************************************************
template <class S, class P>
class Protocol {
public:
  virtual ~Protocol() = default;
  virtual void next(std::string message) = 0;
  virtual std::string currentMessage() = 0;
  bool isSender() { return this->party == this->currentSender; }
  virtual bool isOver() = 0;
protected:
  // Inherited classes can only be instantiated
  // using their own public constructors.
  Protocol() = default;
  S currentStage;
  P currentSender;
  P party;
};

#endif
