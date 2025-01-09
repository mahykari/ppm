#ifndef COMMAND_LINE_INTERFACE_HH
#define COMMAND_LINE_INTERFACE_HH

#include <memory>
#include <map>
#include "MonitorableSystem.hh"

struct ParameterSet {
  unsigned securityParameter;
  unsigned monitorStateLength;
  unsigned systemStateLength;
};

struct CommandLineInterface {
  CommandLineInterface(int argc, char* argv[]);
  int argc;
  char** argv;

  ParameterSet parameters;
  std::unique_ptr<MonitorableSystem> system;
  std::string specFileName;

  void usage();
  void parse();
  std::map<std::string, std::string> argMap();
};

#endif
