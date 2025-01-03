#ifndef EXCEPTIONS_HH
#define EXCEPTIONS_HH

#include <stdexcept>

class InvalidLabels : public std::runtime_error {
public:
  InvalidLabels()
    : std::runtime_error("Invalid labels") {}
};

class InvalidCipher : public std::runtime_error {
public:
  InvalidCipher()
    : std::runtime_error("Invalid cipher") {}
};

class NonSendStateHasNoMessage : public std::runtime_error {
public:
  NonSendStateHasNoMessage()
    : std::runtime_error("Non-send state has no message") {}
};

class UnbuiltModuleHasNoOutput : public std::runtime_error {
public:
  UnbuiltModuleHasNoOutput()
    : std::runtime_error("Unbuilt module has no output") {}
};

class OutdatedClass : public std::runtime_error {
public:
  OutdatedClass(std::string className)
    : std::runtime_error("Outdated class: " + className) {}
};

class ConfigBehaviorUndefined : public std::runtime_error {
public:
  ConfigBehaviorUndefined()
    : std::runtime_error("Config behavior undefined") {}
};

class InvalidPrimeIndex : public std::runtime_error {
public:
  InvalidPrimeIndex()
    : std::runtime_error("Invalid prime index") {}
};

class BadTimerCallSequence : public std::runtime_error {
public:
  BadTimerCallSequence()
    : std::runtime_error("Bad timer call sequence") {}
};

#endif
