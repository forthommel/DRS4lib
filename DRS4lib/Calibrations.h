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
    explicit GroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values);

    void loadVoltageCalibrations(size_t num_adc_values, const std::string& postfix = "_cell.txt");
    void loadSampleCalibrations(size_t num_adc_values, const std::string& postfix = "_nsample.txt");
    void loadTimeCalibrations(size_t num_adc_values, const std::string& postfix = "_time.txt");

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
    explicit ModuleCalibrations(const std::string& path = "",
                                size_t num_groups = 0,
                                size_t num_channels = 0,
                                size_t num_adc_values = 0);

    friend std::ostream& operator<<(std::ostream&, const ModuleCalibrations&);

    void addGroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values);
    const GroupCalibrations& groupCalibrations(size_t) const;

  private:
    std::vector<GroupCalibrations> groups_calibrations_;
  };

  class Calibrations {
  public:
    explicit Calibrations(const std::string& path, const std::string& filename_base_path = "Tables_");

    friend std::ostream& operator<<(std::ostream&, const Calibrations&);

    const ModuleCalibrations& addModuleCalibrations(size_t module_id,
                                                    size_t num_groups,
                                                    size_t num_channels,
                                                    size_t num_adc_values);

  private:
    const std::filesystem::path base_path_;
    const std::string filename_base_path_;
    std::vector<ModuleCalibrations> modules_calibrations_;
  };
}  // namespace drs4

#endif
