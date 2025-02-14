#include <iostream>

#include "DRS4lib/ChannelCalibrations.h"

namespace drs4 {
  std::ostream& operator<<(std::ostream& os, const ChannelCalibrations& calib) {
    return os << "ChannelCalibrations{offMean(" << calib.offMean().size() << "), calibSample("
              << calib.calibSample().size() << ")}";
  }
}  // namespace drs4
