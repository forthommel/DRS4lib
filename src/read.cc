#include <iostream>

#include "DRS4lib/Calibrations.h"
#include "DRS4lib/Reader.h"

int main() {
  drs4::Calibrations calibrations("../data");
  calibrations.addModuleCalibrations(13118, 2, 8, 1024);

  std::cout << calibrations << std::endl;

  drs4::Reader reader;

  return 0;
}
