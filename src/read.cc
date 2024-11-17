#include <iostream>

#include "DRS4lib/Calibrations.h"
#include "DRS4lib/DataFormat.h"
#include "DRS4lib/Reader.h"

int main() {
  drs4::Calibrations calibrations("../data");
  const auto& module1_calibrations = calibrations.addModuleCalibrations(13118, 2, 8, 1024);
  const auto& module2_calibrations = calibrations.addModuleCalibrations(21333, 2, 8, 1024);

  std::cout << calibrations << std::endl;

  drs4::Reader reader;
  reader.addModule("input0.dat", 13118, module1_calibrations);
  reader.addModule("input1.dat", 21333, module2_calibrations);

  drs4::GlobalEvent global_event;
  while (reader.next(global_event)) {
  }

  return 0;
}
