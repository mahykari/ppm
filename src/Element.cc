#include "Element.hh"

Element::Element(Group* g) : group(g) {}

bool Element::belongsTo(Group* g) const {
  return this->group == g;
}
