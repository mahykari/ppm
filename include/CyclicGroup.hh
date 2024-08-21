#ifndef CYCLIC_GROUP_HH
#define CYCLIC_GROUP_HH

#include "Group.hh"
#include "Element.hh"

class CyclicGroup : public Group {
public:
  CyclicGroup(GroupType type);
  virtual std::unique_ptr<Element> randomGenerator() = 0;
};

#endif
