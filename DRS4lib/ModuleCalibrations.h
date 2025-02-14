#ifndef DRS4lib_ModuleCalibrations_h
#define DRS4lib_ModuleCalibrations_h

#include "DRS4lib/GroupCalibrations.h"

namespace drs4 {
  class ModuleCalibrations {
  public:
    explicit ModuleCalibrations(size_t num_groups = 0, size_t num_channels = 0, const std::string& path = "");

    friend std::ostream& operator<<(std::ostream&, const ModuleCalibrations&);

    void addGroupCalibrations(size_t num_channels, const std::string& path);
    const GroupCalibrations& groupCalibrations(size_t) const;

  private:
    std::vector<GroupCalibrations> groups_calibrations_;
  };
}  // namespace drs4

#endif
