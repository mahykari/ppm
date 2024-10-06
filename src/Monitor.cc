#include <iostream>
#include <memory>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

#include "BellareMicaliOTProtocol.hh"
#include "QuadraticResidueGroup.hh"
#include "Party.hh"
#include "MathUtils.hh"
#include "StringUtils.hh"

auto SYSTEM_ENDPOINT = "tcp://localhost:5555";
auto MONITOR_ENDPOINT = "tcp://*:5556";

using C = MonitoringComponent;

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

int main (int argc, char *argv[]) {
  SetUp();

  printf ("Connecting to System...\n");
  void *context = zmq_ctx_new ();
  void *requester = zmq_socket (context, ZMQ_REQ);
  zmq_connect(requester, SYSTEM_ENDPOINT);

  printf("Binding Monitor...\n");
  void *responder = zmq_socket (context, ZMQ_REP);
  zmq_bind(responder, "tcp://*:5556");

  BigInt primeModulus(SAFE_PRIMES[80]);
  printf(
    "Using safe prime p=%s\n",
    toString(primeModulus).c_str());
  auto protocol = BellareMicaliOTProtocol(
    QuadraticResidueGroup(primeModulus),
    C::Monitor, C::System, C::Monitor);
  protocol.updateChooser(1);
  while (not protocol.isOver()) {
    char buffer [512] = {};
    if (protocol.isSender()) {
      auto message = protocol.currentMessage();
      zmq_send (requester, message.c_str(), message.size(), 0);
      zmq_recv (requester, buffer, 0, 0);
    } else {
      zmq_recv(responder, buffer, sizeof buffer, 0);
      zmq_send(responder, buffer, 0, 0);
    }
    protocol.next(std::string(buffer));
  }
  zmq_close (requester);
  zmq_close(responder);
  zmq_ctx_destroy (context);
}
