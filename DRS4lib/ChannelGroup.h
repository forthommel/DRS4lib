#ifndef DRS4lib_ChannelGroup_h
#define DRS4lib_ChannelGroup_h

#include <cstdint>
#include <map>
#include <vector>

namespace drs4 {
  using Waveform = std::vector<float>;

  class ChannelGroup {
  public:
    explicit ChannelGroup(uint32_t = 0);

    uint8_t controlBits() const;

    /// trigger counter bin
    inline uint16_t startIndexCell() const { return (group_event_description_ >> 20) & 0x3ff; }
    inline uint8_t frequency() const { return (group_event_description_ >> 16) & 0x3; }
    inline bool triggerChannel() const { return (group_event_description_ >> 12) & 0x1; }

    inline size_t numSamples() const { return (group_event_description_ & 0xfff) / 3; }

    inline void setTriggerTimeTag(uint32_t trigger_time_tag) { trigger_time_tag_ = trigger_time_tag; }
    inline uint32_t triggerTimeTag() const { return trigger_time_tag_; }
    inline double triggerTime() const { return trigger_time_tag_multiplier_ * triggerTimeTag(); }

    inline void setTimes(const std::vector<float>& times) { times_ = times; }
    inline const std::vector<float> times() const { return times_; }

    void addChannelWaveform(size_t channel_id, const Waveform& waveform);
    inline const std::map<size_t, Waveform> waveforms() const { return channel_waveforms_; }

  private:
    static constexpr double trigger_time_tag_multiplier_ = 8.5e-9;  ///< trigger time multiplier (in s)

    uint32_t group_event_description_;
    uint32_t trigger_time_tag_{0};
    std::vector<float> times_;
    std::map<std::size_t, Waveform> channel_waveforms_;
  };
}  // namespace drs4

#endif
