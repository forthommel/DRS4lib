#ifndef DRS4lib_ModuleFileReader_h
#define DRS4lib_ModuleFileReader_h

#include <array>
#include <fstream>

#include "DRS4lib/ModuleCalibrations.h"

namespace drs4 {
  class Event;

  class ModuleFileReader {
  public:
    explicit ModuleFileReader(const std::string& filename, const ModuleCalibrations&);
    ModuleFileReader() = default;

    static constexpr std::array<double, 4> SAMPLING_FREQUENCIES = {1. / 1.e9, 1. / 2.e9, 1. / 5.e9, 1. / 6.6667e9};
    static std::vector<uint16_t> wordsUnpacker(const std::array<uint32_t, 3>& words);

    void setFilename(const std::string&);
    void reset();
    void rewind(size_t = 1);
    bool next(Event& event);

    std::streampos currentPosition() const { return previous_position_; }

  private:
    static constexpr double coeff_ = 1. / 4095.;

    const ModuleCalibrations calibrations_{};
    std::ifstream file_;
    std::streampos previous_position_;
  };
}  // namespace drs4

#endif
