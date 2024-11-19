#include "State.hh"
#include "Exceptions.hh"

bool State::isSend() {
  return false;
}

bool State::isRecv() {
  return false;
}

std::string State::message() {
  throw NonSendStateHasNoMessage();
}
