#ifndef DRS4lib_Event_h
#define DRS4lib_Event_h

#include <unordered_map>

#include "DRS4lib/ChannelGroup.h"
#include "DRS4lib/EventHeader.h"

namespace drs4 {
  class Event {
  public:
    explicit Event(const EventHeader& = EventHeader{});

    inline const EventHeader& header() const { return header_; }

    inline size_t numGroups() const { return groups_.size(); }
    ChannelGroup& addGroup(const ChannelGroup&);
    const std::vector<ChannelGroup>& groups() const { return groups_; }
    const ChannelGroup& group(size_t) const;

  private:
    EventHeader header_;
    std::vector<ChannelGroup> groups_;
  };

  class GlobalEvent {
  public:
    GlobalEvent() = default;

    inline void clear() { module_events_.clear(); }
    inline void setModuleEvent(size_t module_id, const Event& event) { module_events_[module_id] = event; }
    inline const std::unordered_map<size_t, Event> moduleEvents() const { return module_events_; }

  private:
    std::unordered_map<size_t, Event> module_events_;
  };
}  // namespace drs4

#endif
