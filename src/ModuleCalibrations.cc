#include <iostream>
#include <stdexcept>

#include "DRS4lib/ModuleCalibrations.h"

using namespace drs4;

ModuleCalibrations::ModuleCalibrations(size_t num_groups, size_t num_channels, const std::string& path) {
  for (size_t i = 0; i < num_groups; ++i)
    addGroupCalibrations(num_channels, path + "gr" + std::to_string(i));
}

void ModuleCalibrations::addGroupCalibrations(size_t num_channels, const std::string& path) {
  groups_calibrations_.emplace_back(num_channels, path);
}

const GroupCalibrations& ModuleCalibrations::groupCalibrations(size_t igroup) const {
  if (igroup >= groups_calibrations_.size())
    throw std::runtime_error("Failed to retrieve calibration for group '" + std::to_string(igroup) + "'.");
  return groups_calibrations_.at(igroup);
}

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const ModuleCalibrations& calib) {
    os << "ModuleCalibrations{Groups=[";
    std::string sep;
    for (const auto& grp_calib : calib.groups_calibrations_)
      os << sep << grp_calib, sep = ", ";
    return os << "]}";
  }
}  // namespace drs4
