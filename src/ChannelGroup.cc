#include "DRS4lib/ChannelGroup.h"

using namespace drs4;

ChannelGroup::ChannelGroup(uint32_t group_event_description) : group_event_description_(group_event_description) {}

uint8_t ChannelGroup::controlBits() const {
  return ((group_event_description_ >> 30) & 0x3) + ((group_event_description_ >> 18) & 0x3) +
         ((group_event_description_ >> 13) & 0x7);
}

void ChannelGroup::setChannelWaveform(size_t channel_id, const Waveform& waveform) {
  channel_waveforms_[channel_id] = waveform;
}

void ChannelGroup::setTriggerTimeTag(uint32_t trigger_time_tag) { trigger_time_tag_ = trigger_time_tag & 0x3fffffff; }
