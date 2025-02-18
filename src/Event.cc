#include <stdexcept>

#include "DRS4lib/Event.h"

using namespace drs4;

Event::Event(const EventHeader& header) : header_(header) {}

ChannelGroup& Event::setGroup(size_t group_id, const ChannelGroup& group) {
  if (group_id >= groups_.size())
    groups_.resize(group_id + 1);
  groups_[group_id] = group;
  return groups_[group_id];
}

const ChannelGroup& Event::group(size_t igrp) const {
  if (igrp >= groups_.size())
    throw std::runtime_error("Failed to channel content for group '" + std::to_string(igrp) + "'.");
  return groups_.at(igrp);
}
