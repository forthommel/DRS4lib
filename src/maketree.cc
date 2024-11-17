// C++ includes
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ROOT includes
#include <TCanvas.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>

#include "DRS4lib/InputParser.h"

using namespace std;

int main(int argc, char **argv) {
  std::string inputFileName = "output0.dat";
  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h")) {
    std::cout << " Calibrates DRS dat files and saves it in ROOT format " << std::endl;
    std::cout << " Usage:  ./maketree  -h" << std::endl;
    std::cout << "                 prints this text " << std::endl;
    std::cout << "        ./maketree -drs N  " << std::endl;
    std::cout << "                 reads  outputN.dat  " << std::endl;
    std::cout << "        ./maketree -drs N -r XXX  " << std::endl;
    std::cout << "                 reads  RUNXXX_outputN.dat " << std::endl;
    std::cout << "        ./maketree -drs N -r XXX  -p PATH/ " << std::endl;
    std::cout << "                 reads PATH/RUNXXX_outputN.dat " << std::endl;
    std::cout << " ROOT file is outputN_tmp.root " << std::endl;
    exit(0);
  }

  int idrs = 0;
  int irun = 0;
  const std::string &sdrs = input.getCmdOption("-drs");
  if (!sdrs.empty()) {
    idrs = atoi(sdrs.c_str());
    if (idrs != 0)
      idrs = 1;
  }
  const std::string &srun = input.getCmdOption("-r");
  if (!srun.empty()) {
    irun = atoi(srun.c_str());
    inputFileName = "RUN" + srun + "_output0.dat";
    if (idrs == 1)
      inputFileName = "RUN" + srun + "_output1.dat";
  }
  const std::string &pathname = input.getCmdOption("-p");
  if (!pathname.empty()) {
    inputFileName = pathname + inputFileName;
  }
  std::string outputFileName = "output0_tmp.root";
  if (idrs == 1) {
    outputFileName = "output1_tmp.root";
  }
  std::cout << inputFileName << std::endl;
  std::cout << outputFileName << std::endl;

  FILE *fp1;
  char stitle[200];
  int dummy;

  int drsID = 13118;
  if (idrs == 1) {
    drsID = 21333;
  }

  //**************************************
  // Load Voltage Calibration
  //**************************************

  std::cout << "\n=== Loading voltage calibration ===\n" << std::endl;
  double off_mean[2][9][1024];
  for (int i = 0; i < 2; i++) {
    sprintf(stitle, "./%d/Tables_gr%d_cell.txt", drsID, i);
    fp1 = fopen(stitle, "r");
    printf("Loading offset data from %s\n", stitle);
    int ich, it, ioff;
    for (int k = 0; k < 1024; k++) {
      for (int j = 0; j < 9; j++) {
        dummy = fscanf(fp1, "%i", &ich);
        dummy = fscanf(fp1, "%i", &it);
        dummy = fscanf(fp1, "%i", &ioff);
        off_mean[i][ich][it] = ioff;
        //              dummy = fscanf( fp1, "%lf ", &off_mean[i][j][k] );
      }
    }
    fclose(fp1);
  }

  //**************************************
  // Load Sample Calibration
  //**************************************

  std::cout << "\n=== Loading voltage calibration ===\n" << std::endl;
  double calib_sample[2][9][1024];
  for (int i = 0; i < 2; i++) {
    sprintf(stitle, "./%d/Tables_gr%d_nsample.txt", drsID, i);
    fp1 = fopen(stitle, "r");
    printf("Loading sample data from %s\n", stitle);
    int ich, it, ioff;
    for (int k = 0; k < 1024; k++) {
      for (int j = 0; j < 9; j++) {
        dummy = fscanf(fp1, "%i", &ich);
        dummy = fscanf(fp1, "%i", &it);
        dummy = fscanf(fp1, "%i", &ioff);
        calib_sample[i][ich][it] = ioff;
        //              dummy = fscanf( fp1, "%lf ", &calib_sample[i][j][k] );
      }
    }
    fclose(fp1);
  }

  //**************************************
  // Load Time Calibration
  //**************************************

  std::cout << "\n=== Loading time calibration ===\n" << std::endl;
  double fdummy;
  double tcal_dV[2][1024];
  for (int i = 0; i < 2; i++) {
    sprintf(stitle, "./%d/Tables_gr%d_time.txt", drsID, i);
    fp1 = fopen(stitle, "r");
    printf("Loading dV data from %s\n", stitle);
    int it;
    float y;
    for (int k = 0; k < 1024; k++) {
      dummy = fscanf(fp1, "%i", &it);
      dummy = fscanf(fp1, "%f", &y);
      tcal_dV[i][it] = y;
      //          dummy = fscanf( fp1, "%lf", &tcal_dV[i][k] );
    }
    fclose(fp1);
  }
  double dV_sum[2] = {0, 0};
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 1024; j++)
      dV_sum[i] += tcal_dV[i][j];
  }

  //double fullTimeScale = 200.0;
  double tcal[2][1024];
  for (int i = 0; i < 2; i++) {
    tcal[i][0] = 0.;
    for (int j = 1; j < 1024; j++) {
      //          tcal[i][j] = tcal_dV[i][j] / dV_sum[i] * fullTimeScale;
      tcal[i][j] = tcal_dV[i][j] - tcal_dV[i][j - 1];
    }
  }

  //*********************************************************
  // Check if has valid input file, otherwise exit with error
  //*********************************************************
  ifstream ifile(inputFileName);
  if (!ifile) {
    std::cerr << "[ERROR]: !USAGE! Input file does not exist. Please enter valid file name" << std::endl;
    exit(0);
  }

  //**************************************
  // Define output
  //**************************************

  TFile *file = new TFile(outputFileName.c_str(), "RECREATE", "CAEN V1742");
  TTree *tree = new TTree("pulse", "Digitized waveforms");

  int event;
  float channel[2][9][1024];
  float times[2][1024];

  float amplitude[18432];  // 2 * 9 * 1024
  float timevalue[2048];   // 2 * 1024
  tree->Branch("run", &irun, "run/I");
  tree->Branch("event", &event, "event/I");
  tree->Branch("amplitude", amplitude, "amplitude[18432]/F");
  tree->Branch("timevalue", timevalue, "timevalue[2048]/F");

  // temp variables for data input
  uint event_header;
  uint temp[3];
  ushort samples[9][1024];

  //*************************
  // Open Input File
  //*************************

  FILE *fpin = fopen(inputFileName.c_str(), "r");

  //*************************
  //Event Loop
  //*************************

  std::cout << "\n=== Processing input data ===\n" << std::endl;
  int nGoodEvents = 0;
  int maxEvents = 999999;

  for (int iEvent = 0; iEvent < maxEvents; iEvent++) {
    if (iEvent % 1000 == 0) {
      std::cout << "Event " << iEvent << "\n";
    }
    event = nGoodEvents;  // for output tree

    // first header word
    dummy = fread(&event_header, sizeof(uint), 1, fpin);

    // second header word
    dummy = fread(&event_header, sizeof(uint), 1, fpin);
    uint grM = event_header & 0b11;  // 2-bit channel group mask
    // std::cout << " Group mask = " << grM << std::endl;

    // third and fourth header words
    dummy = fread(&event_header, sizeof(uint), 1, fpin);
    dummy = fread(&event_header, sizeof(uint), 1, fpin);

    // check for end of file
    if (feof(fpin))
      break;

    //*************************
    // Parse group mask into channels
    //*************************

    bool _isGR_On[2];
    _isGR_On[0] = (grM & 0x01);
    _isGR_On[1] = (grM & 0x02);

    int activeGroupsN = 0;
    int realGroup[2] = {-1, -1};
    for (int l = 0; l < 2; l++) {
      if (_isGR_On[l]) {
        realGroup[activeGroupsN] = l;
        activeGroupsN++;
      }
    }

    //************************************
    // Loop over channel groups
    //************************************
    double tscale[4] = {1., 2., 5., 6.6667};
    for (int group = 0; group < activeGroupsN; group++) {
      // Read group header
      dummy = fread(&event_header, sizeof(uint), 1, fpin);
      // ushort tcn = (event_header >> 20) & 0xfff; // trigger counter bin
      ushort tcn = (event_header << 2) >> 22;  // trigger counter bin
      UInt_t FREQ = (event_header >> 16) & 0b11;
      uint TR = (event_header >> 12) & 0b1;
      // std::cout << " Group =  " << group << "   TR0 = " << TR << std::endl;

      // Check if all channels were active (if 8 channels active return 3072)
      int nsample = (event_header & 0xfff) / 3;
      // std::cout << " Group =  " << group << "   samples = " << nsample << std::endl;

      // Define time coordinate
      times[realGroup[group]][0] = 0.0;
      for (int i = 1; i < 1024; i++) {
        times[realGroup[group]][i] =
            float(tcal[realGroup[group]][(i - 1 + tcn) % 1024] * tscale[FREQ] + times[realGroup[group]][i - 1]);
      }

      //************************************
      // Read sample info for group
      //************************************

      for (int i = 0; i < nsample; i++) {
        dummy = fread(&temp, sizeof(uint), 3, fpin);
        samples[0][i] = temp[0] & 0xfff;
        samples[1][i] = (temp[0] >> 12) & 0xfff;
        samples[2][i] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
        samples[3][i] = (temp[1] >> 4) & 0xfff;
        samples[4][i] = (temp[1] >> 16) & 0xfff;
        samples[5][i] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
        samples[6][i] = (temp[2] >> 8) & 0xfff;
        samples[7][i] = temp[2] >> 20;
      }

      // Trigger channel
      if (TR) {
        for (int j = 0; j < nsample / 8; j++) {
          fread(&temp, sizeof(uint), 3, fpin);
          samples[8][j * 8 + 0] = temp[0] & 0xfff;
          samples[8][j * 8 + 1] = (temp[0] >> 12) & 0xfff;
          samples[8][j * 8 + 2] = (temp[0] >> 24) | ((temp[1] & 0xf) << 8);
          samples[8][j * 8 + 3] = (temp[1] >> 4) & 0xfff;
          samples[8][j * 8 + 4] = (temp[1] >> 16) & 0xfff;
          samples[8][j * 8 + 5] = (temp[1] >> 28) | ((temp[2] & 0xff) << 4);
          samples[8][j * 8 + 6] = (temp[2] >> 8) & 0xfff;
          samples[8][j * 8 + 7] = temp[2] >> 20;
        }
      } else {
        for (int j = 0; j < nsample / 8; j++) {
          samples[8][j * 8 + 0] = 0;
          samples[8][j * 8 + 1] = 0;
          samples[8][j * 8 + 2] = 0;
          samples[8][j * 8 + 3] = 0;
          samples[8][j * 8 + 4] = 0;
          samples[8][j * 8 + 5] = 0;
          samples[8][j * 8 + 6] = 0;
          samples[8][j * 8 + 7] = 0;
        }
      }

      //************************************
      // Loop over channels 0-8
      //************************************

      for (int i = 0; i < 9; i++) {
        // Fill pulses
        for (int j = 0; j < 1024; j++) {
          channel[group][i][j] = (double)(samples[i][j]) - (double)(off_mean[realGroup[group]][i][(j + tcn) % 1024]);
          channel[group][i][j] -= (double)(calib_sample[realGroup[group]][i][j]);
          channel[group][i][j] = 1000. * ((channel[group][i][j] - 0) / 4095. - 0.5) + 0;
        }
      }
      dummy = fread(&event_header, sizeof(uint), 1, fpin);
    }
    for (int igr = 0; igr < 2; igr++) {
      for (int ich = 0; ich < 9; ich++) {
        for (int it = 0; it < 1024; it++) {
          int indx = it + 1024 * ich + (1024 * 9) * igr;
          amplitude[indx] = channel[igr][ich][it];
        }
      }
    }
    for (int igr = 0; igr < 2; igr++) {
      for (int it = 0; it < 1024; it++) {
        int indx = it + 1024 * igr;
        timevalue[indx] = times[igr][it];
      }
    }
    tree->Fill();
    nGoodEvents++;
  }

  fclose(fpin);
  cout << "\nProcessed total of " << nGoodEvents << " events\n";

  file->Write();
  file->Close();

  return 0;
}
