#include <assert.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <zmq.h>
#include "InsecureParityProtocol.hh"
#include "Party.hh"

const char *SYSTEM_ENDPOINT = "tcp://*:5555";
const char *MONITOR_ENDPOINT = "tcp://localhost:5556";

using C = MonitoringComponent;

int main() {
  // For all return codes
  int rc = 0;
  printf("I: binding System...\n");
  void* context = zmq_ctx_new();
  void* responder = zmq_socket(context, ZMQ_REP);
  rc = zmq_bind(responder, SYSTEM_ENDPOINT);
  assert (rc == 0);

  printf("I: connecting to Monitor...\n");
  void* requester = zmq_socket(context, ZMQ_REQ);
  rc = zmq_connect(requester, MONITOR_ENDPOINT);
  assert (rc == 0);

  auto protocol = InsecureParityProtocol(
    C::System, C::Monitor, C::System);
  while (not protocol.isOver()) {
    char buffer [10] = {0};
    if (protocol.isSender()) {
      auto message = protocol.currentMessage();
      zmq_send (requester, message.c_str(), message.size(), 0);
      zmq_recv (requester, buffer, 10, 0);
    } else {
      zmq_recv(responder, buffer, 10, 0);
      zmq_send(responder, "", 0, 0);
    }
    protocol.next(std::string(buffer));
  }
  zmq_close (requester);
  zmq_close(responder);
  zmq_ctx_destroy (context);
}
