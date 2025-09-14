#include <iostream>

#include "DRS4lib/Event.h"
#include "DRS4lib/ModuleFileReader.h"

using namespace drs4;

ModuleFileReader::ModuleFileReader(const std::string& filename, const ModuleCalibrations& calibrations)
    : calibrations_(calibrations) {
  setFilename(filename);
}

void ModuleFileReader::setFilename(const std::string& filename) {
  // check if input file is valid, otherwise throw runtime error exception
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

  file_.read(reinterpret_cast<char*>(event_header_words_.data()), sizeof(event_header_words_));
  event.setHeader(EventHeader(event_header_words_));
  Waveform channel_waveform;

  //************************************
  // Loop over channel groups
  //************************************
  for (size_t group = 0; group < event.header().groupMask().size(); ++group) {
    if (!event.header().groupMask().at(group))
      continue;
    // Read group header
    uint32_t header_payload;
    file_.read(reinterpret_cast<char*>(&header_payload), sizeof(uint32_t));
    auto& group_info = event.setGroup(group, ChannelGroup(header_payload));
    const auto tcn = group_info.startIndexCell();

    // Check if all channels were active (if 8 channels active return 3072)
    const auto nsample = group_info.numSamples();
    const auto& group_calibrations = calibrations_.groupCalibrations(group);

    // Define time coordinate
    if (group_info.times().empty()) {  // only initialise it once
      if (group_info.frequency() >= SAMPLING_FREQUENCIES.size())
        throw std::runtime_error("Invalid frequency index for group " + std::to_string(group) + ": " +
                                 std::to_string(group_info.frequency()));
      std::vector<double> group_times;
      for (size_t sample_id = 0; sample_id < nsample; ++sample_id) {
        const auto index = (tcn + sample_id - 1) % nsample;
        group_times.emplace_back(
            sample_id * (group_calibrations.timeCalibration(index) * SAMPLING_FREQUENCIES.at(group_info.frequency())));
      }
      group_info.setTimes(group_times);
    }

    //************************************
    // Read sample info for group
    //************************************

    std::vector<std::vector<uint16_t> > channel_samples(8, std::vector<uint16_t>(nsample, 0));
    std::array<uint16_t, 8> samples;
    for (size_t sample_id = 0; sample_id < nsample; ++sample_id) {
      file_.read(reinterpret_cast<char*>(packed_sample_frame_.data()), sizeof(packed_sample_frame_));
      size_t channel_id = 0;
      wordsUnpacker(packed_sample_frame_, samples);
      for (const auto& sample : samples) {
        if (channel_id >= channel_samples.size())
          throw std::runtime_error("Trying to fill sample #" + std::to_string(sample_id) + " for channel " +
                                   std::to_string(channel_id) + ", while only " +
                                   std::to_string(channel_samples.size()) + " are allowed.");
        channel_samples.at(channel_id++).at(sample_id) = sample;
      }
    }

    // Trigger channel
    auto& trigger_samples = channel_samples.emplace_back(std::vector<uint16_t>(nsample, 0));
    if (group_info.triggerChannel()) {
      for (size_t i = 0; i < nsample / 8; ++i) {
        file_.read(reinterpret_cast<char*>(packed_sample_frame_.data()), sizeof(packed_sample_frame_));
        size_t ismp = 0;
        wordsUnpacker(packed_sample_frame_, samples);
        for (const auto& sample : samples)
          trigger_samples.at(i * 8 + (ismp++)) = sample;
      }
    }

    //************************************
    // Loop over channels 0-8
    //************************************

    for (size_t channel_id = 0; channel_id < channel_samples.size(); ++channel_id) {
      // Fill pulses
      group_calibrations.channelCalibrations(channel_id)
          .calibrate(tcn, channel_samples.at(channel_id), channel_waveform);
      group_info.setChannelWaveform(channel_id, channel_waveform);
    }
    {  // Read group trailer (unused)
       //FIXME handle extended group trigger time tag on 60-bit word
      uint32_t trailer_payload;
      file_.read(reinterpret_cast<char*>(&trailer_payload), sizeof(uint32_t));
      group_info.setTriggerTimeTag(trailer_payload & 0x3fffffff);
    }
  }
  return true;
}

void ModuleFileReader::wordsUnpacker(const std::array<uint32_t, 3>& words, std::array<uint16_t, 8>& samples) {
  return wordsUnpacker(&words[0], &words[1], &words[2], samples);
}

void ModuleFileReader::wordsUnpacker(const uint32_t* word1,
                                     const uint32_t* word2,
                                     const uint32_t* word3,
                                     std::array<uint16_t, 8>& samples) {
  samples[0] = *word1 & 0xfff;
  samples[1] = (*word1 >> 12) & 0xfff;
  samples[2] = (*word1 >> 24) | ((*word2 & 0xf) << 8);
  samples[3] = (*word2 >> 4) & 0xfff;
  samples[4] = (*word2 >> 16) & 0xfff;
  samples[5] = (*word2 >> 28) | ((*word3 & 0xff) << 4);
  samples[6] = (*word3 >> 8) & 0xfff;
  samples[7] = *word3 >> 20;
}
