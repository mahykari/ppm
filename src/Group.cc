#include "Group.hh"
#include "Exceptions.hh"

std::map<GroupType, Group*> Group::groups;

Group::Group(GroupType type) {
  if (groups.find(type) != groups.end())
    GroupAlreadyExistsException();
  groups[type] = this;
}

Group* Group::get(GroupType type) {
  if (groups.find(type) == groups.end())
    throw NoSuchGroupException();
  return groups[type];
}

bool Group::contains(Element* a) {
  return a->belongsTo(this);
}
