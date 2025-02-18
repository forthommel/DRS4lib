#include <iostream>

#include "DRS4lib/DataFormat.h"
#include "DRS4lib/ModuleFileReader.h"

using namespace drs4;

ModuleFileReader::ModuleFileReader(const std::string& filename, const ModuleCalibrations& calibrations)
    : calibrations_(calibrations) {
  setFilename(filename);
}

void ModuleFileReader::setFilename(const std::string& filename) {
  //**********************************************************
  // Check if has valid input files, otherwise exit with error
  //**********************************************************
  if (file_ = std::ifstream(filename, std::fstream::in | std::fstream::binary); !file_.is_open())
    throw std::runtime_error("!USAGE! Input file '" + filename + "' does not exist. Please enter valid file name");
}

void ModuleFileReader::reset() { file_.clear(); }

void ModuleFileReader::rewind(size_t num_payloads) {
  if (num_payloads != 1) {
    std::cout << "!WARNING! assuming all payloads have the same length, while rewinging " << num_payloads
              << " payloads." << std::endl;
    file_.seekg(num_payloads * (previous_position_ - file_.tellg()), std::ios::cur);
  } else
    file_.seekg(previous_position_);
}

bool ModuleFileReader::next(Event& event) {
  // check for end of files
  if (file_.eof())
    return false;

  previous_position_ = file_.tellg();
  std::array<uint32_t, 4> event_header_words;
  file_.read((char*)event_header_words.data(), sizeof(event_header_words));
  const EventHeader event_header(event_header_words);
  event = Event(event_header);

  //************************************
  // Loop over channel groups
  //************************************
  for (size_t group = 0; group < event.header().groupMask().size(); ++group) {
    if (!event.header().groupMask().at(group))
      continue;
    // Read group header
    uint32_t header_payload;
    file_.read((char*)&header_payload, sizeof(uint32_t));
    auto& group_info = event.addGroup(ChannelGroup(header_payload));
    const auto tcn = group_info.startIndexCell();

    // Check if all channels were active (if 8 channels active return 3072)
    const auto nsample = group_info.numSamples();
    //std::cout << " Group =  " << group << "   samples = " << nsample << std::endl;

    // Define time coordinate
    if (group_info.times().empty()) {  // only initialise it once
      std::vector<float> group_times;
      for (size_t i = 0; i < nsample; ++i)
        group_times.emplace_back(
            i * (calibrations_.groupCalibrations(group).timeCalibrations().at((i - 1 + tcn) % nsample) *
                 tscale_.at(group_info.frequency())));
      group_info.setTimes(group_times);
    }

    //************************************
    // Read sample info for group
    //************************************

    std::array<uint32_t, 3> packed_sample_frame;
    std::vector<std::vector<uint16_t> > channel_samples(8, std::vector<uint16_t>(nsample, 0));
    for (size_t i = 0; i < nsample; ++i) {
      file_.read((char*)packed_sample_frame.data(), sizeof(packed_sample_frame));
      size_t ich = 0;
      for (const auto& sample : wordsUnpacker(packed_sample_frame))
        channel_samples.at(ich++).at(i) = sample;
    }

    // Trigger channel
    if (group_info.triggerChannel()) {
      auto& trigger_samples = channel_samples.emplace_back(std::vector<uint16_t>(nsample, 0));
      for (size_t i = 0; i < nsample / 8; ++i) {
        file_.read((char*)packed_sample_frame.data(), sizeof(packed_sample_frame));
        size_t ismp = 0;
        for (const auto& sample : wordsUnpacker(packed_sample_frame))
          trigger_samples.at(i * 8 + ismp) = sample;
      }
    }

    //************************************
    // Loop over channels 0-8
    //************************************

    for (size_t i = 0; i < channel_samples.size(); ++i) {
      const auto& channel_calibrations = calibrations_.groupCalibrations(group).channelCalibrations(i);
      const auto& off_mean = channel_calibrations.offMean();
      const auto& calib_sample = channel_calibrations.calibSample();
      // Fill pulses
      const auto& channel_raw_waveform = channel_samples.at(i);
      Waveform channel_waveform(channel_raw_waveform.size());
      for (size_t j = 0; j < channel_raw_waveform.size(); ++j)
        channel_waveform.at(j) =
            1000. * (((double(channel_raw_waveform.at(j)) - double(off_mean.at((j + tcn) % nsample))) -
                      double(calib_sample.at(j))) /
                         4095. -
                     0.5);
      group_info.addChannelWaveform(i, channel_waveform);
    }
    {  // Read group trailer (unused)
      uint32_t trailer_payload;
      file_.read((char*)&trailer_payload, sizeof(uint32_t));
      group_info.setTriggerTimeTag(trailer_payload & 0x7fffffff);
    }
  }
  return true;
}

std::vector<uint16_t> ModuleFileReader::wordsUnpacker(const std::array<uint32_t, 3>& words) {
  return std::vector<uint16_t>{uint16_t(words.at(0) & 0xfff),
                               uint16_t((words.at(0) >> 12) & 0xfff),
                               uint16_t((words.at(0) >> 24) | ((words.at(1) & 0xf) << 8)),
                               uint16_t((words.at(1) >> 4) & 0xfff),
                               uint16_t((words.at(1) >> 16) & 0xfff),
                               uint16_t((words.at(1) >> 28) | ((words.at(2) & 0xff) << 4)),
                               uint16_t((words.at(2) >> 8) & 0xfff),
                               uint16_t(words.at(2) >> 20)};
}
