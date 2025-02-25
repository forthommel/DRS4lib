#include <iostream>

#include "DRS4lib/Event.h"
#include "DRS4lib/Reader.h"

using namespace drs4;
using namespace std::string_literals;

Reader::Reader(const std::string& pattern, const std::vector<size_t>& modules_ids, const Calibrations& calibrations) {
  size_t i = 0;
  for (const auto& module_id : modules_ids)
    addModule(pattern + "_" + std::to_string(i++) + ".dat", module_id, calibrations.moduleCalibrations(module_id));
}

void Reader::addModule(const std::string& filename, size_t module_id, const ModuleCalibrations& calibrations) {
  files_readers_.insert(std::make_pair(module_id, ModuleFileReader(filename, calibrations)));
}

void Reader::setFilename(const std::string& filename, size_t module_id) {
  files_readers_[module_id].setFilename(filename);
}

void Reader::reset() {
  for (auto& [module_id, file_reader] : files_readers_)
    file_reader.reset();
}

bool Reader::next(GlobalEvent& event) {
  ssize_t other_event_number = -999;
  for (auto& [module_id, file_reader] : files_readers_) {
    Event module_event;
    if (!file_reader.next(module_event))
      return false;
    if (other_event_number < 0)
      other_event_number = module_event.header().eventNumber();
    else if (module_event.header().eventNumber() != other_event_number) {
      if (module_event.header().eventNumber() != other_event_number + 1) {
        std::string readers_positions;
        for (const auto& [other_module_id, other_file_reader] : files_readers_)
          readers_positions +=
              " "s + std::to_string(other_module_id) + ":" + std::to_string(other_file_reader.currentPosition());
        throw std::runtime_error("Lost synchronisation between the module readers. Other readers event number=" +
                                 std::to_string(other_event_number) +
                                 " != this event number=" + std::to_string(module_event.header().eventNumber()) +
                                 ". Reader positions:" + readers_positions + ".");
      }
      std::cout << "!WARNING! Off-by-one event id mismatch between module readers (this module: event #"
                << module_event.header().eventNumber() << ", other module: event #" << other_event_number
                << "). Rewinding module #" << module_id << " stream." << std::endl;
      file_reader.rewind();
      invalid_event_ids_.emplace_back(other_event_number);
      return next(event);  // off-by-one trigger, we skip it and move to the next one
    }
    event.setModuleEvent(module_id, module_event);
  }
  return true;
}
