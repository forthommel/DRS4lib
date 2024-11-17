#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>

#include <iostream>

#include "DRS4lib/Calibrations.h"
#include "DRS4lib/DataFormat.h"
#include "DRS4lib/Reader.h"

int main() {
  drs4::Calibrations calibrations("../data");
  const auto& module1_calibrations = calibrations.addModuleCalibrations(13118, 2, 8, 1024);
  const auto& module2_calibrations = calibrations.addModuleCalibrations(21333, 2, 8, 1024);

  std::cout << calibrations << std::endl;

  const std::string base_path =
      "/eos/experiment/iucmsdata/Data/2024/202410-H6-RADiCAL/era2/run_4_particle_e-_energy_100-GeV_chunk_1";

  drs4::Reader reader(base_path, {13118, 21333}, calibrations);
  /*reader.addModule(base_path + "0.dat", 13118, module1_calibrations);
  reader.addModule(base_path + "1.dat", 21333, module2_calibrations);*/

  drs4::GlobalEvent global_event;
  std::vector<TGraph*> graphs;  // not deleted, "trust" ROOT for GC
  std::vector<TMultiGraph> channel_graphs(2 * 2 * 9);
  size_t event_id = 0;
  while (reader.next(global_event)) {
    if (event_id++ > 20)
      break;
    size_t icg = 0;
    for (const auto& [module_id, module_event] : global_event.moduleEvents()) {
      for (const auto& group_info : module_event.groups()) {
        const auto& time_values = group_info.times();
        for (const auto& [channel_id, waveform] : group_info.waveforms()) {
          auto* graph = graphs.emplace_back(new TGraph(time_values.size(), time_values.data(), waveform.data()));
          channel_graphs.at(icg++).Add(graph);
        }
      }
    }
  }
  {
    TCanvas c;
    c.DivideSquare(channel_graphs.size());
    for (size_t i = 0; i < channel_graphs.size(); ++i) {
      c.cd(i + 1);
      channel_graphs.at(i).Draw("ap");
    }
    c.SaveAs("event_display.png");
  }
  return 0;
}
