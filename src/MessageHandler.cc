#include "MessageHandler.hh"

MessageHandler::MessageHandler(unsigned sendPort, unsigned recvPort) {
  this->context = zmq::context_t {1};

  this->sender = zmq::socket_t {context, ZMQ_REQ};
  sender.connect("tcp://localhost:" + std::to_string(sendPort));

  this->receiver = zmq::socket_t {context, ZMQ_REP};
  receiver.bind("tcp://*:" + std::to_string(recvPort));
}

void MessageHandler::send(std::string message) {
  printf("D: MessageHandler::send\n");
  printf("D:   sending message: %s\n", message.c_str());
  zmq::message_t zmqMessage(message);
  this->sender.send(zmqMessage, zmq::send_flags::none);
  auto result = this->sender.recv(zmqMessage, zmq::recv_flags::none);
  assert (result);
}

std::string MessageHandler::recv() {
  printf("D: MessageHandler::recv\n");
  zmq::message_t zmqMessage;
  auto result = this->receiver.recv(zmqMessage, zmq::recv_flags::none);
  assert (result);
  this->receiver.send(zmq::buffer(""), zmq::send_flags::none);
  std::string message = zmqMessage.to_string();
  printf("D:   received message: %s\n", message.c_str());
  return message;
}
