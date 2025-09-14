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

    static constexpr std::array<double, 4> SAMPLING_FREQUENCIES = {1. / 5.e9, 1. / 2.5e9, 1. / 1.e9, 1. / 0.75e9};
    static void wordsUnpacker(const uint32_t*, const uint32_t*, const uint32_t*, std::array<uint16_t, 8>& samples);
    static void wordsUnpacker(const std::array<uint32_t, 3>& words, std::array<uint16_t, 8>& samples);

    void setFilename(const std::string&);
    void reset();
    void rewind(size_t = 1);
    bool next(Event& event);

    std::streampos currentPosition() const { return previous_position_; }

  private:
    const ModuleCalibrations calibrations_{};
    std::ifstream file_;
    std::streampos previous_position_;
    std::array<uint32_t, 4> event_header_words_;
    std::array<uint32_t, 3> packed_sample_frame_;
  };
}  // namespace drs4

#endif
