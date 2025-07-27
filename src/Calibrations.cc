#include "DRS4lib/Calibrations.h"

using namespace drs4;

Calibrations::Calibrations(const std::string& path, const std::string& filename_base_path)
    : base_path_(path), filename_base_path_(filename_base_path) {}

const ModuleCalibrations& Calibrations::addModuleCalibrations(size_t module_id,
                                                              size_t num_groups,
                                                              size_t num_channels,
                                                              bool legacy_format) {
  modules_calibrations_.insert(std::make_pair(
      module_id,
      ModuleCalibrations(
          num_groups, num_channels, base_path_ / std::to_string(module_id) / filename_base_path_, legacy_format)));
  return moduleCalibrations(module_id);
}

const ModuleCalibrations& Calibrations::moduleCalibrations(size_t module_id) const {
  if (modules_calibrations_.count(module_id) == 0)
    throw std::runtime_error("Failed to retrieve calibration for module '" + std::to_string(module_id) + "'.");
  return modules_calibrations_.at(module_id);
}

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const Calibrations& calib) {
    os << "Calibrations{path=" << calib.base_path_ << ", Modules=[";
    std::string sep;
    for (const auto& [module_id, module_calibrations] : calib.modules_calibrations_)
      os << sep << module_id << ":" << module_calibrations, sep = ", ";
    return os << "]}";
  }
}  // namespace drs4
