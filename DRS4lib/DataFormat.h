#ifndef DRS4lib_DataFormat_h
#define DRS4lib_DataFormat_h

#include <array>
#include <cstdint>
#include <vector>

namespace drs4 {
  class EventHeader : private std::array<uint32_t, 4> {
  public:
    explicit EventHeader(const std::array<uint32_t, 4>& words = {0, 0, 0, 0}) : std::array<uint32_t, 4>(words) {}
    EventHeader(const EventHeader& oth) : std::array<uint32_t, 4>(oth) {}

    inline uint8_t init() const { return at(0) >> 28; }
    inline uint32_t size() const { return at(0) & 0xfffffff; }
    inline bool bf() const { return (at(1) >> 26) & 0x1; }
    inline std::vector<bool> groupMask() const {
      const auto mask = at(1) & 0x3;
      return std::vector<bool>{bool(mask & 0x1), bool((mask >> 1) & 0x1)};
    }
    inline uint32_t eventNumber() const { return at(2) & 0xffffff; }

    inline bool valid() const { return (init() == 0b1010) && (size() == 6920) && (bf() == 0); }
  };

  class ChannelGroup {
  public:
    explicit ChannelGroup(uint32_t first_word = 0) : first_word_(first_word) {}

    inline uint8_t controlBits() const {
      return first_word_ >> 30 + (first_word_ >> 18 & 0x3) + (first_word_ >> 13 & 0x7);
    }

    /// trigger counter bin
    inline uint8_t triggerCounter() const { return (first_word_ >> 22) & 0xff; }
    inline uint8_t frequency() const { return (first_word_ >> 16) & 0x3; }
    inline bool triggerChannel() const { return (first_word_ >> 12) & 0x1; }

    inline size_t numSamples() const { return (first_word_ & 0xfff) / 3; }

    inline void setTimes(const std::vector<float>& times) { times_ = times; }
    inline const std::vector<float> times() const { return times_; }

    inline void addSample(uint16_t sample) { samples_.emplace_back(sample); }
    inline const std::vector<uint16_t> samples() const { return samples_; }

    inline bool valid() const { return triggerChannel() && (controlBits() == 0); }

  private:
    const uint32_t first_word_;
    std::vector<float> times_;
    std::vector<uint16_t> samples_;
  };

  class Event {
  public:
    explicit Event(const EventHeader& header) : header_(header) {}

    void setHeader(const EventHeader& header) { header_ = header; }
    inline const EventHeader& header() const { return header_; }

    inline ChannelGroup& addGroup(const ChannelGroup& group) { return groups_.emplace_back(group); }
    inline const std::vector<ChannelGroup>& groups() const { return groups_; }

  private:
    EventHeader header_;
    std::vector<ChannelGroup> groups_;
  };
}  // namespace drs4

#endif
