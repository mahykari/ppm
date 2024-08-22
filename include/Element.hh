#ifndef ELEMENT_HH
#define ELEMENT_HH

#include <string>

// Class Element is mainly a 'wrapper' class for group elements;
// when an element is more than just a number,
// wrapping its fields in an Element simplifies
// passing elements to methods.

class Element {
public:
  virtual ~Element() = default;

  // I/O methods
  virtual std::string toString() const = 0;
};

#endif
