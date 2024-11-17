#ifndef DRD4lib_Calibrations_h
#define DRD4lib_Calibrations_h

#include <filesystem>
#include <vector>

namespace drs4 {
  class ChannelCalibrations {
  public:
    ChannelCalibrations() = default;

    const std::vector<double>& offMean() const { return off_mean_; }
    std::vector<double>& offMean() { return off_mean_; }
    const std::vector<double>& calibSample() const { return calib_sample_; }
    std::vector<double>& calibSample() { return calib_sample_; }

  private:
    std::vector<double> off_mean_;
    std::vector<double> calib_sample_;
  };

  class GroupCalibrations {
  public:
    explicit GroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values);

    void loadVoltageCalibrations(size_t num_adc_values, const std::string& postfix = "_cell.txt");
    void loadSampleCalibrations(size_t num_adc_values, const std::string& postfix = "_nsample.txt");
    void loadTimeCalibrations(size_t num_adc_values, const std::string& postfix = "_time.txt");

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
    explicit ModuleCalibrations(const std::string& path, size_t num_groups, size_t num_channels, size_t num_adc_values);

    void addGroupCalibrations(const std::string& path, size_t num_channels, size_t num_adc_values);
    const GroupCalibrations& groupCalibrations(size_t) const;

  private:
    std::vector<GroupCalibrations> groups_calibrations_;
  };

  class Calibrations {
  public:
    explicit Calibrations(const std::string& path, const std::string& filename_base_path = "Tables_");

    void addModuleCalibrations(size_t module_id, size_t num_groups, size_t num_channels, size_t num_adc_values);

  private:
    const std::filesystem::path base_path_;
    const std::string filename_base_path_;
    std::vector<ModuleCalibrations> modules_calibrations_;
  };
}  // namespace drs4

#endif
