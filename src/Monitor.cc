#include <iostream>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
#include "InsecureParityProtocol.hh"
#include "Party.hh"

auto SYSTEM_ENDPOINT = "tcp://localhost:5555";
auto MONITOR_ENDPOINT = "tcp://*:5556";

using C = MonitoringComponent;

int main (int argc, char *argv[]) {
  printf ("Connecting to System...\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect(requester, SYSTEM_ENDPOINT);

  printf("Binding Monitor...\n");
  void *responder = zmq_socket (context, ZMQ_REP);
  zmq_bind(responder, "tcp://*:5556");

  auto protocol = InsecureParityProtocol(
    C::System, C::Monitor, C::Monitor);
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
