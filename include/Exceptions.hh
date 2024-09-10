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
