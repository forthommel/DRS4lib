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

  //*************************
  // Parse group mask into channels
  //*************************

  const auto grM = event.header().groupMask();
  bool _isGR_On[2];
  _isGR_On[0] = (grM & 0x01);
  _isGR_On[1] = (grM & 0x02);

  int activeGroupsN = 0;
  int realGroup[2] = {-1, -1};
  for (int l = 0; l < 2; l++) {
    if (_isGR_On[l]) {
      realGroup[activeGroupsN] = l;
      activeGroupsN++;
    }
  }

  //************************************
  // Loop over channel groups
  //************************************
  for (int group = 0; group < activeGroupsN; group++) {
    {  // Read group header
      const auto dummy = fread(&payload, sizeof(uint32_t), 1, file_.get());
    }
    auto& group_info = event.addGroup(ChannelGroup(payload));
    // uint16_t tcn = (event_header >> 20) & 0xfff; // trigger counter bin
    const auto tcn = group_info.triggerCounter();
    const auto FREQ = group_info.frequency();
    const auto TR = group_info.triggerChannel();
    // std::cout << " Group =  " << group << "   TR0 = " << TR << std::endl;

    // Check if all channels were active (if 8 channels active return 3072)
    const auto nsample = group_info.numSamples();
    // std::cout << " Group =  " << group << "   samples = " << nsample << std::endl;

    // Define time coordinate
    times[realGroup[group]][0] = 0.0;
    for (int i = 1; i < 1024; i++)
      times[realGroup[group]][i] =
          float(calibrations_.groupCalibrations(realGroup[group]).timeCalibrations().at((i - 1 + tcn) % 1024) *
                    tscale_.at(FREQ) +
                times[realGroup[group]][i - 1]);

    //************************************
    // Read sample info for group
    //************************************

    for (int i = 0; i < nsample; i++) {
      const auto dummy = fread(&temp, sizeof(uint32_t), 3, file_.get());
      samples[0][i] = temp[0] & 0xfff;
      samples[1][i] = (temp[0] >> 12) & 0xfff;
      samples[2][i] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
      samples[3][i] = (temp[1] >> 4) & 0xfff;
      samples[4][i] = (temp[1] >> 16) & 0xfff;
      samples[5][i] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
      samples[6][i] = (temp[2] >> 8) & 0xfff;
      samples[7][i] = temp[2] >> 20;
    }

    // Trigger channel
    if (TR) {
      for (int j = 0; j < nsample / 8; j++) {
        fread(&temp, sizeof(uint32_t), 3, file_.get());
        samples[8][j * 8 + 0] = temp[0] & 0xfff;
        samples[8][j * 8 + 1] = (temp[0] >> 12) & 0xfff;
        samples[8][j * 8 + 2] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
        samples[8][j * 8 + 3] = (temp[1] >> 4) & 0xfff;
        samples[8][j * 8 + 4] = (temp[1] >> 16) & 0xfff;
        samples[8][j * 8 + 5] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
        samples[8][j * 8 + 6] = (temp[2] >> 8) & 0xfff;
        samples[8][j * 8 + 7] = temp[2] >> 20;
      }
    } else {
      for (int j = 0; j < nsample / 8; j++) {
        samples[8][j * 8 + 0] = 0;
        samples[8][j * 8 + 1] = 0;
        samples[8][j * 8 + 2] = 0;
        samples[8][j * 8 + 3] = 0;
        samples[8][j * 8 + 4] = 0;
        samples[8][j * 8 + 5] = 0;
        samples[8][j * 8 + 6] = 0;
        samples[8][j * 8 + 7] = 0;
      }
    }

    //************************************
    // Loop over channels 0-8
    //************************************

    for (int i = 0; i < 9; i++) {
      const auto& off_mean = calibrations_.groupCalibrations(realGroup[group]).channelCalibrations(i).offMean();
      const auto& calib_sample = calibrations_.groupCalibrations(realGroup[group]).channelCalibrations(i).calibSample();
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
