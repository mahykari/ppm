#ifndef CYCLIC_GROUP_HH
#define CYCLIC_GROUP_HH

#include "Group.hh"

// Refer to the definition of Group
// for an explanation of the template argument E.
template <class E>
class CyclicGroup : public Group<E> {
public:
  // A cyclic group has at least one generator;
  // Method randomGenerator() returns one of them at random.
  virtual E randomGenerator() = 0;
};

#endif
