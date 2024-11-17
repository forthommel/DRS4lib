// C++ includes
#include <iostream>

// ROOT includes
#include <TFile.h>
#include <TTree.h>

#include "DRS4lib/Calibrations.h"
#include "DRS4lib/DataFormat.h"
#include "DRS4lib/InputParser.h"
#include "DRS4lib/Reader.h"

using namespace std;

int main(int argc, char** argv) {
  string inputFileName = "output0.dat";
  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h")) {
    cout << " Calibrates DRS dat files and saves it in ROOT format " << endl;
    cout << " Usage: " << argv[0] << " -h" << endl;
    cout << "                 prints this text " << endl;
    cout << "        " << argv[0] << " -drs N  " << endl;
    cout << "                 reads  outputN.dat  " << endl;
    cout << "        " << argv[0] << " -drs N -r XXX  " << endl;
    cout << "                 reads  RUNXXX_outputN.dat " << endl;
    cout << "        " << argv[0] << " -drs N -r XXX  -p PATH/ " << endl;
    cout << "                 reads PATH/RUNXXX_outputN.dat " << endl;
    cout << " ROOT file is outputN_tmp.root " << endl;
    exit(0);
  }

  int idrs = 0;
  int irun = 0;
  const std::string& sdrs = input.getCmdOption("-drs");
  if (!sdrs.empty()) {
    idrs = atoi(sdrs.c_str());
    if (idrs != 0)
      idrs = 1;
  }
  const std::string& srun = input.getCmdOption("-r");
  if (!srun.empty()) {
    irun = atoi(srun.c_str());
    inputFileName = "RUN" + srun + "_output0.dat";
    if (idrs == 1)
      inputFileName = "RUN" + srun + "_output1.dat";
  }
  const std::string& pathname = input.getCmdOption("-p");
  if (!pathname.empty())
    inputFileName = pathname + inputFileName;
  string outputFileName = "output0_tmp.root";
  if (idrs == 1)
    outputFileName = "output1_tmp.root";
  cout << inputFileName << endl;
  cout << outputFileName << endl;

  int drsID = 13118;
  if (idrs == 1)
    drsID = 21333;
  drs4::Calibrations calibrations("../data");
  const auto& module_calibrations = calibrations.addModuleCalibrations(drsID, 2, 8);

  //**************************************
  // Define output
  //**************************************

  TFile* file = new TFile(outputFileName.c_str(), "RECREATE", "CAEN V1742");
  TTree* tree = new TTree("pulse", "Digitized waveforms");

  int event;
  float amplitude[18432];  // 2 * 9 * 1024
  float timevalue[2048];   // 2 * 1024
  tree->Branch("run", &irun, "run/I");
  tree->Branch("event", &event, "event/I");
  tree->Branch("amplitude", amplitude, "amplitude[18432]/F");
  tree->Branch("timevalue", timevalue, "timevalue[2048]/F");

  //*************************
  // Open Input File
  //*************************

  drs4::Reader reader;
  reader.addModule(inputFileName, drsID, module_calibrations);

  //*************************
  //Event Loop
  //*************************

  cout << "\n=== Processing input data ===\n" << endl;
  int nGoodEvents = 0;
  int maxEvents = 999999;

  drs4::GlobalEvent global_event;

  for (int iEvent = 0; iEvent < maxEvents; iEvent++) {
    if (iEvent % 1000 == 0)
      cout << "Event " << iEvent << "\n";
    if (!reader.next(global_event))
      break;
    event = nGoodEvents;  // for output tree

    const auto& module_event = global_event.moduleEvents().begin()->second;  // single module
    for (size_t igr = 0; igr < module_event.groups().size(); ++igr) {
      const auto& group_event = module_event.groups().at(igr);
      for (const auto& [ich, waveform] : group_event.waveforms()) {
        for (size_t it = 0; it < waveform.size(); ++it) {
          size_t indx = it + 1024 * ich + (1024 * 9) * igr;
          amplitude[indx] = waveform.at(it);
        }
      }
      for (size_t it = 0; it < group_event.times().size(); ++it) {
        size_t indx = it + 1024 * igr;
        timevalue[indx] = group_event.times().at(it);
      }
    }
    tree->Fill();
    nGoodEvents++;
  }

  cout << "\nProcessed total of " << nGoodEvents << " events\n";

  file->Write();
  file->Close();

  return 0;
}
