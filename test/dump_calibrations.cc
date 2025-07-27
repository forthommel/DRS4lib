#include <iostream>

#include "DRS4lib/Calibrations.h"

int main() {
  bool new_format = true;

  drs4::Calibrations calibrations("../data");
  calibrations.addModuleCalibrations(13118, 2, 8, true);
  calibrations.addModuleCalibrations(0, 2, 8, false);

  std::cout << calibrations << std::endl;
  return 0;
}
