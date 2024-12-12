#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <string>
#include "CommandLineInterface.hh"

typedef CommandLineInterface CLI;

void CLI::usage(char* execname) {
  printf(
    "Usage: %s -security k -mslen m -sslen s -ngates n "
    "[-sys sys_name] [-spec spec_name]\n", execname);
  exit(EXIT_SUCCESS);
}

void CLI::parse(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Error: too few arguments\n");
    printf("Try %s -h for help\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (std::string(argv[1]) == "-h") {
    usage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  for (int i = 1; i < argc; i += 2) {
    auto arg = std::string(argv[i]);
    if (arg == "-security")
      this->securityParameter = atoi(argv[i + 1]);
    else if (arg == "-mslen")
      this->monitorStateLength = atoi(argv[i + 1]);
    else if (arg == "-sslen")
      this->systemStateLength = atoi(argv[i + 1]);
    else if (arg == "-ngates")
      this->gateCount = atoi(argv[i + 1]);
    else if (arg == "-sys") {
      auto sys_name = std::string(argv[i + 1]);
      if (sys_name == "sweep")
        this->system = std::make_unique<SweepSystem>();
      else if (sys_name == "jump")
        this->system = std::make_unique<JumpSweepSystem>(systemStateLength);
      else {
        printf("Error: unknown system name\n");
        exit(EXIT_FAILURE);
      }
    }
    else if (arg == "-spec")
      this->specFileName = std::string(argv[i + 1]);
    else {
      printf("Error: unknown argument %s\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }
}
