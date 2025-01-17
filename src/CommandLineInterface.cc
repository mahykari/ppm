#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <string>
#include "CommandLineInterface.hh"

typedef CommandLineInterface CLI;

CLI::CommandLineInterface(int argc, char* argv[])
: argc(argc), argv(argv) {}

void CLI::usage() {
  printf(
    "Usage: %s -security k -mslen m -sslen s -ngates n "
    "[-sys sys_name] [-spec spec_name]\n", argv[0]);
  exit(EXIT_SUCCESS);
}

void CLI::parse() {
  if (argc < 2) {
    printf("Error: too few arguments\n");
    printf("Try %s -h for help\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (std::string(argv[1]) == "-h") {
    usage();
    exit(EXIT_SUCCESS);
  }

  auto args = argMap();
  if (args.contains("-h")) {
    usage();
    exit(EXIT_SUCCESS);
  }

  assert (args.contains("-security"));
  assert (args.contains("-mslen"));
  assert (args.contains("-sslen"));

  parameters.securityParameter = std::stoul(args["-security"]);
  parameters.monitorStateLength = std::stoul(args["-mslen"]);
  parameters.systemStateLength = std::stoul(args["-sslen"]);
  auto protocolStr = args["-protocol"];
  parameters.protocol =
    protocolStr == "yao" ? ProtocolType::YAO : ProtocolType::LWY;

  if (args.contains("-spec"))
    specFileName = args["-spec"];
  if (args.contains("-sys")) {
    std::string sysName = args["-sys"];
    if (sysName == "sweep") {
      system = std::make_unique<SweepSystem>();
    } else if (sysName == "jump") {
      if (args.contains("-n")) {
        unsigned N = std::stoul(args["-n"]);
        system = std::make_unique<JumpSweepSystem>(N);
      } else {
        system = std::make_unique<JumpSweepSystem>(10);
      }
    } else if (sysName == "timekeeper") {
      // Timekeeper may require additional arguments.
      if (args.contains("-ndoors") and args.contains("-wordlen")) {
        unsigned nDoors = std::stoul(args["-ndoors"]);
        unsigned wordLen = std::stoul(args["-wordlen"]);
        system = std::make_unique<Timekeeper>(nDoors, wordLen);
      } else {
        system = std::make_unique<Timekeeper>();
      }
    } else if (sysName == "timekeeper+") {
      // Timekeeper+ may require additional arguments.
      if (  args.contains("-nex")
        and args.contains("-nin")
        and args.contains("-wordlen"))
      {
        unsigned nEx = std::stoul(args["-nex"]);
        unsigned nIn = std::stoul(args["-nin"]);
        unsigned wordLen = std::stoul(args["-wordlen"]);
        system = std::make_unique<TimekeeperPlus>(nEx, nIn, wordLen);
      } else {
        system = std::make_unique<TimekeeperPlus>();
      }
    } else {
      printf("Error: invalid system name\n");
      exit(EXIT_FAILURE);
    }
  }
}

std::map<std::string, std::string> CLI::argMap() {
  assert (argc > 2);
  std::map<std::string, std::string> result;
  for (auto s = argv + 1; s < argv + argc - 1; s++) {
    if (std::string(*s) == "-h") {
      result["-h"] = "";
      s++;
      continue;
    }
    std::string key = *s, value = *(s + 1);
    result[key] = value;
  }
  return result;
}
