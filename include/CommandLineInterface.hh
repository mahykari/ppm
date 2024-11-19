#ifndef COMMAND_LINE_INTERFACE_HH
#define COMMAND_LINE_INTERFACE_HH

#include <memory>
#include "MonitorableSystem.hh"

struct CommandLineInterface {
  unsigned securityParameter;
  unsigned monitorStateLength;
  unsigned systemStateLength;
  unsigned gateCount;
  std::unique_ptr<MonitorableSystem> system;

  void usage(char* execname);
  void parse(int argc, char* argv[]);
};

#endif
