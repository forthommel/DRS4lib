#include <iostream>

#include "DRS4lib/ChannelCalibrations.h"

using namespace drs4;

void ChannelCalibrations::calibrate(uint16_t start_index_cell,
                                    const std::vector<uint16_t>& input_raw_samples,
                                    std::vector<double>& output_samples) const {
  output_samples.resize(input_raw_samples.size());
  for (size_t sample_id = 0; sample_id < input_raw_samples.size(); ++sample_id) {
    if (calib_sample_.count(sample_id) == 0)
      throw std::runtime_error("Unpacked a " + std::to_string(input_raw_samples.size()) +
                               "-sample raw waveform while only " + std::to_string(calib_sample_.size()) +
                               " are allowed.");
    if (const auto index = (sample_id + start_index_cell) % input_raw_samples.size(); off_mean_.count(index) > 0)
      output_samples[sample_id] =
          coeff_ * (input_raw_samples.at(sample_id) - off_mean_.at(index) - calib_sample_.at(sample_id)) - 0.5;
    else
      throw std::runtime_error("Invalid index for off-mean computation: " + std::to_string(index) +
                               " is not part of calibration values.");
  }
}

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const ChannelCalibrations& calib) {
    return os << "ChannelCalibrations{offMean(" << calib.offMean().size() << "), calibSample("
              << calib.calibSample().size() << ")}";
  }
}  // namespace drs4
