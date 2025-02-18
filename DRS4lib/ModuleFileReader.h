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

    void setFilename(const std::string&);
    void reset();
    void rewind(size_t = 1);
    bool next(Event& event);

  private:
    static constexpr std::array<double, 4> tscale_ = {1., 2., 5., 6.6667};
    static std::vector<uint16_t> wordsUnpacker(const std::array<uint32_t, 3>& words);

    const ModuleCalibrations calibrations_{};
    std::ifstream file_;
    std::streampos previous_position_;
  };
}  // namespace drs4

#endif
