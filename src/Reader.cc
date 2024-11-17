#include "DRS4lib/DataFormat.h"
#include "DRS4lib/Reader.h"

using namespace drs4;

void Reader::addModule(const std::string& filename, const ModuleCalibrations& calibrations) {
  files_readers_.emplace_back(filename, calibrations);
}

bool Reader::next(Event& event) {
  for (auto& file_reader : files_readers_) {
    if (!file_reader.next(event))
      return false;
  }
  return true;
}

Reader::ModuleFileReader::ModuleFileReader(const std::string& filename, const ModuleCalibrations& calibrations)
    : calibrations_(calibrations) {
  //**********************************************************
  // Check if has valid input files, otherwise exit with error
  //**********************************************************
  if (std::ifstream ifile(filename); !ifile)
    throw std::runtime_error("!USAGE! Input file '" + filename + "' does not exist. Please enter valid file name");
  file_.reset(std::fopen(filename.data(), "r"));
}

bool Reader::ModuleFileReader::next(Event& event) {
  // check for end of files
  if (feof(file_.get()))
    return false;
  //if (iEvent % 1000 == 0)
  //  std::cout << "Event " << iEvent << "\n";

  std::array<uint32_t, 4> event_header_words;
  for (size_t i = 0; i < event_header_words.size(); ++i)
    const auto dummy = fread(&event_header_words.at(i), sizeof(uint32_t), 1, file_.get());
  const EventHeader event_header(event_header_words);
  event = Event(event_header);

  float channel[2][9][1024];
  float times[2][1024];

  float amplitude[18432];  // 2 * 9 * 1024
  float timevalue[2048];   // 2 * 1024

  // temp variables for data input
  uint32_t payload;
  uint32_t temp[3];
  uint16_t samples[9][1024];

  //************************************
  // Loop over channel groups
  //************************************
  for (size_t group = 0; group < event.header().groupMask().size(); ++group) {
    if (!event.header().groupMask().at(group))
      continue;
    {  // Read group header
      const auto dummy = fread(&payload, sizeof(uint32_t), 1, file_.get());
    }
    auto& group_info = event.addGroup(ChannelGroup(payload));
    // uint16_t tcn = (event_header >> 20) & 0xfff; // trigger counter bin
    const auto tcn = group_info.triggerCounter();

    // Check if all channels were active (if 8 channels active return 3072)
    const auto nsample = group_info.numSamples();
    // std::cout << " Group =  " << group << "   samples = " << nsample << std::endl;

    // Define time coordinate
    std::vector<float> group_times;
    group_times.emplace_back(0.);
    for (int i = 1; i < 1024; i++) {
      group_times.emplace_back(calibrations_.groupCalibrations(group).timeCalibrations().at((i - 1 + tcn) % 1024) *
                                   tscale_.at(group_info.frequency()) +
                               times[group][i - 1]);
    }

    //************************************
    // Read sample info for group
    //************************************

    std::array<uint32_t, 3> packed_sample_frame;
    std::vector<std::vector<uint16_t> > channel_samples(9, std::vector<uint16_t>(1024, 0));
    for (int i = 0; i < nsample; i++) {
      const auto dummy = fread(packed_sample_frame.data(), sizeof(uint32_t), 3, file_.get());
      size_t ich = 0;
      for (const auto& sample : wordsUnpacker(packed_sample_frame))
        channel_samples.at(ich++).at(i) = sample;
    }

    // Trigger channel
    if (group_info.triggerChannel())
      for (int j = 0; j < nsample / 8; j++) {
        const auto dummy = fread(packed_sample_frame.data(), sizeof(uint32_t), 3, file_.get());
        size_t ismp = 0;
        for (const auto& sample : wordsUnpacker(packed_sample_frame))
          channel_samples.at(8).at(j * 8 + ismp) = sample;
      }

    //************************************
    // Loop over channels 0-8
    //************************************

    for (int i = 0; i < 9; i++) {
      const auto& off_mean = calibrations_.groupCalibrations(group).channelCalibrations(i).offMean();
      const auto& calib_sample = calibrations_.groupCalibrations(group).channelCalibrations(i).calibSample();
      // Fill pulses
      for (int j = 0; j < 1024; j++) {
        channel[group][i][j] = (double)(samples[i][j]) - (double)(off_mean.at((j + tcn) % 1024));
        channel[group][i][j] -= (double)(calib_sample.at(j));
        channel[group][i][j] = 1000. * ((channel[group][i][j] - 0) / 4095. - 0.5) + 0;
      }
    }
    const auto dummy = fread(&payload, sizeof(uint32_t), 1, file_.get());
  }
  for (int igr = 0; igr < 2; igr++)
    for (int ich = 0; ich < 9; ich++)
      for (int it = 0; it < 1024; it++)
        amplitude[it + 1024 * ich + (1024 * 9) * igr] = channel[igr][ich][it];
  for (int igr = 0; igr < 2; igr++)
    for (int it = 0; it < 1024; it++)
      timevalue[it + 1024 * igr] = times[igr][it];
  return true;
}

std::vector<uint16_t> Reader::ModuleFileReader::wordsUnpacker(const std::array<uint32_t, 3>& words) {
  return std::vector<uint16_t>{uint16_t(words.at(0) & 0xfff),
                               uint16_t((words.at(0) >> 12) & 0xfff),
                               uint16_t((words.at(0) >> 24) | ((words.at(1) & 0xf) << 8)),
                               uint16_t((words.at(1) >> 4) & 0xfff),
                               uint16_t((words.at(1) >> 16) & 0xfff),
                               uint16_t((words.at(1) >> 28) | ((words.at(2) & 0xff) << 4)),
                               uint16_t((words.at(2) >> 8) & 0xfff),
                               uint16_t(words.at(2) >> 20)};
}
