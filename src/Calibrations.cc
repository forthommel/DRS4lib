#include "DRS4lib/Calibrations.h"

using namespace drs4;

Calibrations::Calibrations(const std::string& path, const std::string& filename_base_path)
    : base_path_(path), filename_base_path_(filename_base_path) {}

const ModuleCalibrations& Calibrations::addModuleCalibrations(size_t module_id,
                                                              size_t num_groups,
                                                              size_t num_channels,
                                                              size_t num_adc_values) {
  return modules_calibrations_.emplace_back(
      base_path_ / std::to_string(module_id) / filename_base_path_, num_groups, num_channels, num_adc_values);
}

ModuleCalibrations::ModuleCalibrations(const std::string& path,
                                       size_t num_groups,
                                       size_t num_channels,
                                       size_t num_adc_values) {
  for (size_t i = 0; i < num_groups; ++i)
    addGroupCalibrations(path + "gr" + std::to_string(i), num_channels, num_adc_values);
}

void ModuleCalibrations::addGroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values) {
  groups_calibrations_.emplace_back(path, num_channels, num_adc_values);
}

const GroupCalibrations& ModuleCalibrations::groupCalibrations(size_t igroup) const {
  if (igroup >= groups_calibrations_.size())
    throw std::runtime_error("Failed to retrieve calibration for group '" + std::to_string(igroup) + "'.");
  return groups_calibrations_.at(igroup);
}

GroupCalibrations::GroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values)
    : base_path_(path), channels_calibrations_(num_channels) {
  loadVoltageCalibrations(num_adc_values);
  loadSampleCalibrations(num_adc_values);
  loadTimeCalibrations(num_adc_values);
}

void GroupCalibrations::loadVoltageCalibrations(size_t num_adc_values, const std::string& postfix) {
  auto* fp1 = fopen((base_path_ + postfix).data(), "r");
  size_t dummy;
  int ich, it, ioff;
  for (int k = 0; k < num_adc_values; k++) {
    for (size_t j = 0; j < channels_calibrations_.size(); ++j) {
      dummy = fscanf(fp1, "%i", &ich);
      dummy = fscanf(fp1, "%i", &it);
      dummy = fscanf(fp1, "%i", &ioff);
      channels_calibrations_.at(j).offMean()[it] = ioff;
    }
  }
  fclose(fp1);
}

void GroupCalibrations::loadSampleCalibrations(size_t num_adc_values, const std::string& postfix) {
  auto* fp1 = fopen((base_path_ + postfix).data(), "r");
  size_t dummy;
  int ich, it, ioff;
  for (int k = 0; k < num_adc_values; k++) {
    for (size_t j = 0; j < channels_calibrations_.size(); ++j) {
      dummy = fscanf(fp1, "%i", &ich);
      dummy = fscanf(fp1, "%i", &it);
      dummy = fscanf(fp1, "%i", &ioff);
      channels_calibrations_.at(j).calibSample()[it] = ioff;
    }
  }
  fclose(fp1);
}

void GroupCalibrations::loadTimeCalibrations(size_t num_adc_values, const std::string& postfix) {
  auto* fp1 = fopen((base_path_ + postfix).data(), "r");
  size_t dummy;
  int it;
  float y;
  tcal_dV_.clear();
  dV_sum_ = 0.;
  for (size_t k = 0; k < num_adc_values; k++) {
    dummy = fscanf(fp1, "%i", &it);
    dummy = fscanf(fp1, "%f", &y);
    tcal_dV_.emplace_back(y);
    dV_sum_ += y;
  }
  fclose(fp1);
  tcal_.emplace_back(0.);
  for (size_t i = 1; i < num_adc_values; ++i)
    tcal_.emplace_back(tcal_dV_.at(i) - tcal_dV_.at(i - 1));
}

const ChannelCalibrations& GroupCalibrations::channelCalibrations(size_t ich) const {
  if (ich >= channels_calibrations_.size())
    throw std::runtime_error("Failed to retrieve calibration for channel '" + std::to_string(ich) + "'.");
  return channels_calibrations_.at(ich);
}

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const Calibrations& calib) {
    os << "Calibrations{path=" << calib.base_path_ << ", Modules=[";
    std::string sep;
    for (const auto& mod_calib : calib.modules_calibrations_)
      os << sep << mod_calib, sep = ", ";
    return os << "]}";
  }

  std::ostream& operator<<(std::ostream& os, const ModuleCalibrations& calib) {
    os << "ModuleCalibrations{Groups=[";
    std::string sep;
    for (const auto& grp_calib : calib.groups_calibrations_)
      os << sep << grp_calib, sep = ", ";
    return os << "]}";
  }

  std::ostream& operator<<(std::ostream& os, const GroupCalibrations& calib) {
    os << "GroupCalibrations{dV_sum=" << calib.dV_sum_ << ", Channel=[";
    std::string sep;
    for (const auto& ch_calib : calib.channels_calibrations_)
      os << sep << ch_calib, sep = ", ";
    return os << "]}";
  }

  std::ostream& operator<<(std::ostream& os, const ChannelCalibrations& calib) {
    return os << "ChannelCalibrations{offMean(" << calib.offMean().size() << "), calibSample("
              << calib.calibSample().size() << ")}";
  }
}  // namespace drs4
