#ifndef DRS4lib_DataFormat_h
#define DRS4lib_DataFormat_h

#include <array>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <vector>

namespace drs4 {
  using Waveform = std::vector<float>;

  class EventHeader : private std::array<uint32_t, 4> {
  public:
    explicit EventHeader(const std::array<uint32_t, 4>& words = {0, 0, 0, 0}) : std::array<uint32_t, 4>(words) {}
    EventHeader(const EventHeader& oth) : std::array<uint32_t, 4>(oth) {}

    inline uint8_t init() const { return at(0) >> 28; }
    inline uint32_t eventSize() const { return at(0) & 0xfffffff; }
    inline bool boardFail() const { return (at(1) >> 26) & 0x1; }
    inline std::vector<bool> groupMask() const {
      const auto mask = at(1) & 0x3;
      return std::vector<bool>{bool(mask & 0x1), bool((mask >> 1) & 0x1)};
    }
    inline uint32_t eventNumber() const { return at(2) & 0xffffff; }
    inline uint32_t eventTimeTag() const { return at(3) & 0x7fffffff; }
    inline bool eventTimeOverflow() const { return bool((at(3) >> 31) & 0x1); }
  };

  class ChannelGroup {
  public:
    explicit ChannelGroup(uint32_t first_word = 0) : first_word_(first_word) {}

    inline uint8_t controlBits() const {
      return first_word_ >> 30 + (first_word_ >> 18 & 0x3) + (first_word_ >> 13 & 0x7);
    }

    /// trigger counter bin
    inline uint16_t triggerCounter() const { return (first_word_ >> 20) & 0xfff; }
    inline uint8_t frequency() const { return (first_word_ >> 16) & 0x3; }
    inline bool triggerChannel() const { return (first_word_ >> 12) & 0x1; }

    inline size_t numSamples() const { return (first_word_ & 0xfff) / 3; }

    inline void setTimes(const std::vector<float>& times) { times_ = times; }
    inline const std::vector<float> times() const { return times_; }

    inline void addChannelWaveform(size_t channel_id, const Waveform& waveform) {
      channel_waveforms_[channel_id] = waveform;
    }
    inline const std::map<size_t, Waveform> waveforms() const { return channel_waveforms_; }

  private:
    uint32_t first_word_;
    std::vector<float> times_;
    std::map<std::size_t, Waveform> channel_waveforms_;
  };

  class Event {
  public:
    explicit Event(const EventHeader& header = EventHeader{}) : header_(header) {}

    void setHeader(const EventHeader& header) { header_ = header; }
    inline const EventHeader& header() const { return header_; }

    inline ChannelGroup& addGroup(const ChannelGroup& group) { return groups_.emplace_back(group); }
    inline const std::vector<ChannelGroup>& groups() const { return groups_; }

  private:
    EventHeader header_;
    std::vector<ChannelGroup> groups_;
  };

  class GlobalEvent {
  public:
    GlobalEvent() = default;

    inline void clear() { module_events_.clear(); }
    inline void addModuleEvent(size_t module_id, const Event& event) { module_events_[module_id] = event; }
    inline const std::unordered_map<size_t, Event> moduleEvents() const { return module_events_; }

  private:
    std::unordered_map<size_t, Event> module_events_;
  };
}  // namespace drs4

#endif
