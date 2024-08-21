#include <stdexcept>

class NoSuchGroupException : public std::runtime_error {
public:
  NoSuchGroupException()
  : std::runtime_error("No such group exists") {}
};

class GroupAlreadyExistsException : public std::runtime_error {
public:
  GroupAlreadyExistsException()
  : std::runtime_error("Group already exists") {}
};
