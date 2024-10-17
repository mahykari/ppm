#include <assert.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <zmq.h>

#include "BellareMicaliOTProtocol.hh"
#include "Party.hh"
#include "MathUtils.hh"
#include "StringUtils.hh"

const char *SYSTEM_ENDPOINT = "tcp://*:5555";
const char *MONITOR_ENDPOINT = "tcp://localhost:5556";

using C = MonitoringComponent;

class SetUp {
public:
  SetUp() {
    initPrimes();
  }
};

int main() {
  SetUp();
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

  BigInt primeModulus(SAFE_PRIMES[80]);
  printf(
    "I: using safe prime p= %s\n",
    toString(primeModulus).c_str());
  auto protocol = BellareMicaliOTProtocol(
    QuadraticResidueGroup(primeModulus),
    C::Monitor, C::System, C::System);
  std::string messages[2] = {
    std::string(10, '0') + std::string(10, '1'),
    std::string(10, '2') + std::string(10, '3')
  };
  protocol.updateSender(messages);
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
