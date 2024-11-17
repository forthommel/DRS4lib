#ifndef DRS4lib_Reader_h
#define DRS4lib_Reader_h

#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>

#include "DRS4lib/Calibrations.h"

namespace drs4 {
  class Event;
  class GlobalEvent;

  class Reader {
  public:
    Reader() = default;

    void addModule(const std::string& filename, size_t module_id = 0, const ModuleCalibrations& = ModuleCalibrations());

    bool next(GlobalEvent&);

  private:
    static constexpr std::array<double, 4> tscale_ = {1., 2., 5., 6.6667};

    const std::vector<std::string> filenames_;

    class ModuleFileReader {
    public:
      explicit ModuleFileReader(const std::string& filename, const ModuleCalibrations&);

      bool next(Event& event);

    private:
      static std::vector<uint16_t> wordsUnpacker(const std::array<uint32_t, 3>& words);

      const ModuleCalibrations calibrations_;
      struct file_deleter {
        void operator()(std::FILE* fp) { std::fclose(fp); }
      };
      std::unique_ptr<std::FILE, file_deleter> file_;
    };
    std::unordered_map<size_t, ModuleFileReader> files_readers_;
  };
}  // namespace drs4

#endif
