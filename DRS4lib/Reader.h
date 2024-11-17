#ifndef DRS4lib_Reader_h
#define DRS4lib_Reader_h

#include <array>
#include <fstream>
#include <memory>

#include "DRS4lib/Calibrations.h"

namespace drs4 {
  class Event;

  class Reader {
  public:
    Reader() = default;

    void addModule(const std::string& filename, const ModuleCalibrations&);

    bool next(Event&);

  private:
    static constexpr std::array<double, 4> tscale_ = {1., 2., 5., 6.6667};

    const std::vector<std::string> filenames_;

    class ModuleFileReader {
    public:
      explicit ModuleFileReader(const std::string& filename, const ModuleCalibrations&);

      bool next(Event& event);

    private:
      const ModuleCalibrations calibrations_;
      struct file_deleter {
        void operator()(std::FILE* fp) { std::fclose(fp); }
      };
      std::unique_ptr<std::FILE, file_deleter> file_;
    };
    std::vector<ModuleFileReader> files_readers_;
  };
}  // namespace drs4

#endif
