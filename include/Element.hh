#ifndef ELEMENT_HH
#define ELEMENT_HH

#include <string>

// An element can be in exactly one group;
// so, it keeps a reference to it.
// This reference is checked later
// when the element is used in a group.

class Group;

class Element {
public:
  Element(Group* g);
  bool belongsTo(Group* g) const;
  virtual ~Element() = default;
  virtual std::string toString() const = 0;
protected:
  Group* group;
};

#endif
