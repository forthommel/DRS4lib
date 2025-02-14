#ifndef DRS4lib_ChannelCalibrations_h
#define DRS4lib_ChannelCalibrations_h

#include <iosfwd>
#include <unordered_map>

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
}  // namespace drs4

#endif
