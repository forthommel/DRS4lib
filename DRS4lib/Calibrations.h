#ifndef DRS4lib_Calibrations_h
#define DRS4lib_Calibrations_h

#include <filesystem>

#include "DRS4lib/ModuleCalibrations.h"

namespace drs4 {
  class Calibrations {
  public:
    explicit Calibrations(const std::string& path, const std::string& filename_base_path = "Tables_");

    friend std::ostream& operator<<(std::ostream&, const Calibrations&);

    const ModuleCalibrations& addModuleCalibrations(size_t module_id,
                                                    size_t num_groups = 2,
                                                    size_t num_channels = 8,
                                                    bool legacy_format = false);
    const ModuleCalibrations& moduleCalibrations(size_t module_id) const;

  private:
    const std::filesystem::path base_path_;
    const std::string filename_base_path_;
    std::unordered_map<size_t, ModuleCalibrations> modules_calibrations_;
  };
}  // namespace drs4

#endif
