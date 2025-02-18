#include <stdexcept>

#include "DRS4lib/Event.h"

using namespace drs4;

Event::Event(const EventHeader& header) : header_(header) {}

ChannelGroup& Event::addGroup(const ChannelGroup& group) { return groups_.emplace_back(group); }

const ChannelGroup& Event::group(size_t igrp) const {
  if (igrp >= groups_.size())
    throw std::runtime_error("Failed to channel content for group '" + std::to_string(igrp) + "'.");
  return groups_.at(igrp);
}
