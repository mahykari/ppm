#ifndef MESSAGE_HANDLER_HH
#define MESSAGE_HANDLER_HH

#include <string>
#include <memory>
#include <zmq.hpp>

class MessageHandler {
public:
  MessageHandler(
    unsigned sendPort,
    unsigned recvPort);
  virtual void send(std::string message);
  virtual std::string recv();
private:
  zmq::context_t context;
  zmq::socket_t sender;
  zmq::socket_t receiver;
};

#endif
