#ifndef DRD4lib_Calibrations_h
#define DRD4lib_Calibrations_h

#include <filesystem>
#include <iosfwd>
#include <unordered_map>
#include <vector>

namespace drs4 {
  class ChannelCalibrations {
  public:
    ChannelCalibrations() = default;

    const std::unordered_map<size_t, double>& offMean() const { return off_mean_; }
    std::unordered_map<size_t, double>& offMean() { return off_mean_; }
    const std::unordered_map<size_t, double>& calibSample() const { return calib_sample_; }
    std::unordered_map<size_t, double>& calibSample() { return calib_sample_; }

    friend std::ostream& operator<<(std::ostream&, const ChannelCalibrations&);

  private:
    std::unordered_map<size_t, double> off_mean_;
    std::unordered_map<size_t, double> calib_sample_;
  };

  class GroupCalibrations {
  public:
    explicit GroupCalibrations(size_t num_channels, const std::string& path);

    void loadVoltageCalibrations(const std::string& postfix = "_cell.txt");
    void loadSampleCalibrations(const std::string& postfix = "_nsample.txt");
    void loadTimeCalibrations(const std::string& postfix = "_time.txt");

    friend std::ostream& operator<<(std::ostream&, const GroupCalibrations&);

    inline const std::vector<double> timeCalibrations() const { return tcal_; }
    const ChannelCalibrations& channelCalibrations(size_t) const;

  private:
    const std::string base_path_;

    std::vector<ChannelCalibrations> channels_calibrations_;
    std::vector<double> tcal_dV_;
    std::vector<double> tcal_;
    double dV_sum_{0.};
  };

  class ModuleCalibrations {
  public:
    explicit ModuleCalibrations(size_t num_groups = 0, size_t num_channels = 0, const std::string& path = "");

    friend std::ostream& operator<<(std::ostream&, const ModuleCalibrations&);

    void addGroupCalibrations(size_t num_channels, const std::string& path);
    const GroupCalibrations& groupCalibrations(size_t) const;

  private:
    std::vector<GroupCalibrations> groups_calibrations_;
  };

  class Calibrations {
  public:
    explicit Calibrations(const std::string& path, const std::string& filename_base_path = "Tables_");

    friend std::ostream& operator<<(std::ostream&, const Calibrations&);

    const ModuleCalibrations& addModuleCalibrations(size_t module_id, size_t num_groups = 2, size_t num_channels = 8);
    const ModuleCalibrations& moduleCalibrations(size_t module_id) const;

  private:
    const std::filesystem::path base_path_;
    const std::string filename_base_path_;
    std::unordered_map<size_t, ModuleCalibrations> modules_calibrations_;
  };
}  // namespace drs4

#endif
