#ifndef DRS4lib_Reader_h
#define DRS4lib_Reader_h

#include <memory>
#include <unordered_map>

#include "DRS4lib/Calibrations.h"
#include "DRS4lib/ModuleFileReader.h"

namespace drs4 {
  class GlobalEvent;

  class Reader {
  public:
    Reader() = default;
    explicit Reader(const std::string& pattern, const std::vector<size_t>& modules_ids, const Calibrations&);

    void addModule(const std::string& filename, size_t module_id = 0, const ModuleCalibrations& = ModuleCalibrations());

    void setFilename(const std::string& filename, size_t module_id = 0);
    void reset();
    bool next(GlobalEvent&);

    inline const std::vector<size_t>& invalidEventIds() const { return invalid_event_ids_; }

  private:
    const std::vector<std::string> filenames_;

    std::unordered_map<size_t, ModuleFileReader> files_readers_;
    std::vector<size_t> invalid_event_ids_;
  };
}  // namespace drs4

#endif
