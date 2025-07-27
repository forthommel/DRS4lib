#include <fstream>
#include <iostream>  //FIXME
#include <sstream>

#include "DRS4lib/GroupCalibrations.h"

using namespace drs4;

GroupCalibrations::GroupCalibrations(size_t num_channels, const std::string& path, bool legacy_format)
    : base_path_(path), legacy_format_(legacy_format), channels_calibrations_(num_channels + 1) {
  loadVoltageCalibrations();
  loadSampleCalibrations();
  loadTimeCalibrations();
}

void GroupCalibrations::loadVoltageCalibrations(const std::string& postfix) {
  std::ifstream file(base_path_ + postfix);
  std::string line;
  int ich, it, ioff;
  while (std::getline(file, line)) {
    if (legacy_format_) {  // legacy calibrations data format
      std::istringstream is(line);
      is >> ich >> it >> ioff;
      channels_calibrations_.at(ich).offMean()[it] = ioff;
    } else {
      if (line.empty())
        continue;
      if (line.find("Calibration") != std::string::npos) {  // skip until we reach the next block
        const auto channel_str_pos = line.find("channel");
        if (channel_str_pos == std::string::npos)
          throw std::runtime_error("Invalid header for cell calibration payload");
        const auto channel = line.substr(channel_str_pos + 7, line.find(":") + 1);
        ich = std::stoi(channel);
        it = 0;
      } else {
        std::istringstream is(line);
        for (size_t i = 0; i < 8; ++i, ++it) {
          is >> ioff;
          channels_calibrations_.at(ich).offMean()[it] = ioff;
        }
      }
    }
  }
}

void GroupCalibrations::loadSampleCalibrations(const std::string& postfix) {
  std::ifstream file(base_path_ + postfix);
  int ich, it, ioff;
  std::string line;
  while (std::getline(file, line)) {
    if (legacy_format_) {  // legacy calibrations data format
      std::istringstream is(line);
      is >> ich >> it >> ioff;
      channels_calibrations_.at(ich).calibSample()[it] = ioff;
    } else {
      if (line.empty())
        continue;
      if (line.find("Calibration") != std::string::npos) {  // skip until we reach the next block
        const auto channel_str_pos = line.find("channel");
        if (channel_str_pos == std::string::npos)
          throw std::runtime_error("Invalid header for cell calibration payload");
        const auto channel = line.substr(channel_str_pos + 7, line.find(":") + 1);
        ich = std::stoi(channel);
        it = 0;
      } else {
        std::istringstream is(line);
        for (size_t i = 0; i < 8; ++i, ++it) {
          is >> ioff;
          channels_calibrations_.at(ich).calibSample()[it] = ioff;
        }
      }
    }
  }
}

void GroupCalibrations::loadTimeCalibrations(const std::string& postfix) {
  std::ifstream file(base_path_ + postfix);
  int it;
  float y;
  tcal_dV_.clear();
  dV_sum_ = 0.;
  std::string line;
  while (std::getline(file, line)) {
    if (legacy_format_) {  // legacy calibrations data format
      std::istringstream is(line);
      is >> it >> y;
      tcal_dV_.emplace_back(y);
      dV_sum_ += y;
    } else {
      if (line.empty())
        continue;
      if (line.find("Calibration") != std::string::npos)  // skip until we reach the next block
        it = 0;
      else {
        std::istringstream is(line);
        for (size_t i = 0; i < 8; ++i, ++it) {
          is >> y;
          tcal_dV_.emplace_back(y);
          dV_sum_ += y;
        }
      }
    }
  }
  tcal_ = std::vector<double>{0.};
  for (size_t i = 1; i < tcal_dV_.size(); ++i)
    tcal_.emplace_back(tcal_dV_.at(i) - tcal_dV_.at(i - 1));
}

const ChannelCalibrations& GroupCalibrations::channelCalibrations(size_t ich) const {
  if (ich >= channels_calibrations_.size())
    throw std::runtime_error("Failed to retrieve calibration for channel '" + std::to_string(ich) + "'.");
  return channels_calibrations_.at(ich);
}

double GroupCalibrations::timeCalibration(size_t ismp) const {
  if (ismp >= tcal_.size())
    throw std::runtime_error("Failed to retrieve time calibration for sample '" + std::to_string(ismp) + "'.");
  return tcal_.at(ismp);
}

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const GroupCalibrations& calib) {
    os << "GroupCalibrations{dV_sum=" << calib.dV_sum_ << ", Channel=[";
    std::string sep;
    for (const auto& ch_calib : calib.channels_calibrations_)
      os << sep << ch_calib, sep = ", ";
    return os << "]}";
  }
}  // namespace drs4
