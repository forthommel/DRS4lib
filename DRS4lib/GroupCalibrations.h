#ifndef DRS4lib_GroupCalibrations_h
#define DRS4lib_GroupCalibrations_h

#include <string>
#include <vector>

#include "DRS4lib/ChannelCalibrations.h"

namespace drs4 {
  class GroupCalibrations {
  public:
    explicit GroupCalibrations(size_t num_channels, const std::string& path);

    void loadVoltageCalibrations(const std::string& postfix = "_cell.txt");
    void loadSampleCalibrations(const std::string& postfix = "_nsample.txt");
    void loadTimeCalibrations(const std::string& postfix = "_time.txt");

    friend std::ostream& operator<<(std::ostream&, const GroupCalibrations&);

    double timeCalibration(size_t) const;
    const ChannelCalibrations& channelCalibrations(size_t) const;

  private:
    const std::string base_path_;

    std::vector<ChannelCalibrations> channels_calibrations_;
    std::vector<double> tcal_dV_;
    std::vector<double> tcal_;
    double dV_sum_{0.};
  };
}  // namespace drs4

#endif
