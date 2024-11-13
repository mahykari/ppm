#ifndef PROT_STATE_HH
#define PROT_STATE_HH

#include <memory>

class State;
typedef std::unique_ptr<State> StatePtr;

class State {
public:
  virtual bool isSend();
  virtual bool isRecv();
  virtual std::string message();
  virtual StatePtr next() = 0;
};

#endif
