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

#include "DRS4lib/DataFormat.h"
#include "DRS4lib/InputParser.h"
#include "DRS4lib/Reader.h"

using namespace std;

vector<pair<uint, uint> > listOfGoodEvents(string inputFileName) {
  pair<uint, uint> counter;
  vector<pair<uint, uint> > result;
  drs4::Reader reader;
  reader.addModule(inputFileName, 0);
  drs4::GlobalEvent global_event;

  for (int iEvent = 0; iEvent < 1000000; iEvent++) {
    bool isGoodEvent = true;
    if (!reader.next(global_event))
      break;

    const auto &event = global_event.moduleEvents().begin()->second;  // single module readout
    isGoodEvent &= (event.header().init() == 0b1010);
    isGoodEvent &= (event.header().size() == 6920);  // total event size
    isGoodEvent &= (event.header().bf() == 0);

    size_t activeGroupsN = 0;
    for (const auto &group_enabled : event.header().groupMask()) {
      if (group_enabled)
        activeGroupsN++;
    }
    isGoodEvent &= (activeGroupsN == 2);
    for (size_t group = 0; group < activeGroupsN; ++group) {
      const auto &group_event = event.groups().at(group);
      isGoodEvent &= (group_event.controlBits() == 0);
      isGoodEvent &= (group_event.triggerCounter() == 1);
    }

    if (!isGoodEvent)
      continue;
    counter.first = iEvent;
    counter.second = event.header().eventNumber();
    result.push_back(counter);
  }
  return result;
}

int main(int argc, char **argv) {
  string inputFileName0 = "output0.dat";
  string inputFileName1 = "output1.dat";
  string outputFileName = "list_tmp.txt";
  InputParser input(argc, argv);
  if (input.cmdOptionExists("-h")) {
    cout << " Creates list of events in sync and saves it in list_tmp.txt " << endl;
    cout << " Format: <event number in DRS0> <event number in DRS1> <trigger number>   " << endl;
    cout << " Usage:  ./listGoodEvents  -h" << endl;
    cout << "                 prints this text " << endl;
    cout << "        ./listGoodEvents  " << endl;
    cout << "                 reads from output0.dat and output1.dat by default " << endl;
    cout << "        ./listGoodEvents -r XXX  " << endl;
    cout << "                 reads from RUNXXX_output0.dat and RUNXXX_output1.dat " << endl;
    cout << "        ./listGoodEvents -r XXX  -p PATH/ " << endl;
    cout << "                 reads from PATH/RUNXXX_output0.dat and PATH/RUNXXX_output1.dat " << endl;
    exit(0);
  }
  const string &srun = input.getCmdOption("-r");
  if (!srun.empty()) {
    inputFileName0 = "RUN" + srun + "_output0.dat";
    inputFileName1 = "RUN" + srun + "_output1.dat";
  }
  const string &pathname = input.getCmdOption("-p");
  if (!pathname.empty()) {
    inputFileName0 = pathname + inputFileName0;
    inputFileName1 = pathname + inputFileName1;
  }

  cout << inputFileName0 << endl;
  cout << inputFileName1 << endl;

  ifstream ifile0(inputFileName0);
  if (!ifile0) {
    cerr << "[ERROR]: !USAGE! Input file 0 does not exist. Please enter valid file name" << endl;
    exit(0);
  }
  ifstream ifile1(inputFileName1);
  if (!ifile1) {
    cerr << "[ERROR]: !USAGE! Input file 1 does not exist. Please enter valid file name" << endl;
    exit(0);
  }

  vector<pair<uint, uint> > vlist0 = listOfGoodEvents(inputFileName0);
  vector<pair<uint, uint> > vlist1 = listOfGoodEvents(inputFileName1);

  unsigned int i0 = 0;
  unsigned int i1 = 0;
  vector<uint> good_event_0;
  vector<uint> good_event_1;
  vector<uint> good_trigger;
  while (i0 < vlist0.size() && i1 < vlist1.size()) {
    if (vlist0[i0].second != vlist1[i1].second) {
      if (vlist0[i0].second < vlist1[i1].second) {
        i0++;
      } else {
        i1++;
      }
    } else {
      good_event_0.push_back(vlist0[i0].first);
      good_event_1.push_back(vlist1[i1].first);
      good_trigger.push_back(vlist0[i0].second);
      i0++;
      i1++;
    }
  }

  ofstream fout(outputFileName.c_str());
  for (unsigned int i = 0; i < good_trigger.size(); i++) {
    fout << good_event_0[i] << "   " << good_event_1[i] << "   " << good_trigger[i] << "\n";
  }

  cout << " Number of good events in drs0 = " << vlist0.size() << endl;
  cout << " Number of good events in drs1 = " << vlist1.size() << endl;
  cout << " Number of good events in sync = " << good_trigger.size() << endl;
  return 0;
}
