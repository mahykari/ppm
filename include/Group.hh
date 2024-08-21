#ifndef GROUP_HH
#define GROUP_HH

#include <iostream>
#include <map>
#include <memory>
#include <gmp.h>
#include "Element.hh"

// We assume that the program will only use
// at most one instance from any type of group.

enum GroupType {
  QUADRATIC_RESIDUE,
};

class Group {
protected:
  static std::map<GroupType, Group*> groups;
public:
  Group(GroupType type);
  Group* get(GroupType type);
  bool contains(Element* a);
  virtual std::unique_ptr<Element> mul(
    Element* a, Element* b) = 0;
  virtual std::unique_ptr<Element> exp(Element* a, mpz_t n) = 0;
};

#endif
