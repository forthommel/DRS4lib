#ifndef DRS4lib_ChannelCalibrations_h
#define DRS4lib_ChannelCalibrations_h

#include <cstdint>
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

    void calibrate(uint16_t start_index_cell,
                   const std::vector<uint16_t>& input_raw_samples,
                   std::vector<double>& output_samples) const;

    friend std::ostream& operator<<(std::ostream&, const ChannelCalibrations&);

  private:
    static constexpr double coeff_ = 1. / 4095.;
    std::unordered_map<size_t, double> off_mean_;
    std::unordered_map<size_t, double> calib_sample_;
  };
}  // namespace drs4

#endif
