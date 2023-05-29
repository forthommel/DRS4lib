// C++ includes
#include <fstream>
#include <string>
#include <iostream>

// ROOT includes
#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TCanvas.h>


using namespace std;


// Declaration for input DRS0
Int_t           run0;
Int_t           event0;
Float_t         amplitude0[18432];
Float_t         timevalue0[2048];
// List of branches
TBranch        *b_run0;
TBranch        *b_event0;   //!
TBranch        *b_amplitude0;   //!
TBranch        *b_timevalue0;   //!
// tree
TTree *tree0;


// Declaration for input DRS1
Int_t           run1;
Int_t           event1;
Float_t         amplitude1[18432];
Float_t         timevalue1[2048];
// List of branches
TBranch        *b_run1;
TBranch        *b_event1;   //!
TBranch        *b_amplitude1;   //!
TBranch        *b_timevalue1;   //!
// tree
TTree *tree1;




void LoadDRS0()
{
  char fn0[120];
  sprintf(fn0,"output0_tmp.root");
  TFile *f0 = new TFile(fn0);
  f0->GetObject("pulse",tree0);
  tree0->SetBranchAddress("run", &run0, &b_run0);
  tree0->SetBranchAddress("event", &event0, &b_event0);
  tree0->SetBranchAddress("amplitude", amplitude0, &b_amplitude0);
  tree0->SetBranchAddress("timevalue", timevalue0, &b_timevalue0);
}

void LoadDRS1()
{
  char fn1[120];
  sprintf(fn1,"output1_tmp.root");
  TFile *f1 = new TFile(fn1);
  f1->GetObject("pulse",tree1);
  tree1->SetBranchAddress("run", &run1, &b_run1);
  tree1->SetBranchAddress("event", &event1, &b_event1);
  tree1->SetBranchAddress("amplitude", amplitude1, &b_amplitude1);
  tree1->SetBranchAddress("timevalue", timevalue1, &b_timevalue1);
}



int main(int argc, char **argv) {

  LoadDRS0();
  Long64_t nDRS0Events = tree0->GetEntries();
  std::cout << " N DRS0 events = " << nDRS0Events << std::endl;
  
  LoadDRS1();
  Long64_t nDRS1Events = tree1->GetEntries();
  std::cout << " N DRS1 events = " << nDRS1Events << std::endl;
  

  // read list of events to merge
  std::vector<int> good_event0;  
  std::vector<int> good_event1;  
  std::vector<int> good_trigger;
  ifstream in1;
  in1.open("list_tmp.txt");
  while (1) {
    int i1, i2, i3;
    in1 >> i1 >> i2 >> i3;
    if (!in1.good()) break;
    good_event0.push_back(i1);
    good_event1.push_back(i2);
    good_trigger.push_back(i3);
  }
  in1.close();
  std::cout << " N good events = " << good_trigger.size() << std::endl;
  
  
  TFile* fileOut = new TFile( "output.root", "RECREATE");
  TTree* treeOut = new TTree("pulse", "Digitized waveforms");

  float timevalue[4096]; // calibrated time
  float amplitude[36864]; // calibrated amplitudes (in V)
  int   trigger; // trigger number
 
  treeOut->Branch("timevalue", timevalue, "timevalue[4096]/F");
  treeOut->Branch("amplitude", amplitude, "amplitude[36864]/F");
  treeOut->Branch("event0", &event0, "event0/I");
  treeOut->Branch("event1", &event1, "event1/I");
  treeOut->Branch("trigger", &trigger, "trigger/I");
  treeOut->Branch("run", &run1, "run/I");

  
  std::cout << " Merging ..." << std::endl;
  
  for(unsigned int iev=1; iev<good_trigger.size(); iev++){

    if(iev % 1000 == 0){
      std::cout << " Event " << iev << std::endl;
    }
    
    tree0->GetEntry(good_event0[iev]);
    tree1->GetEntry(good_event1[iev]);
    trigger = good_trigger[iev];

    for(int is=0; is<2048; is++){
      timevalue[is] = timevalue0[is];
      timevalue[2048 + is] = timevalue1[is];
    }
    for(int is=0; is<18432; is++){
      amplitude[is] = amplitude0[is];
      amplitude[18432 + is] = amplitude1[is];
    }
    treeOut->Fill();
  }
  fileOut->cd();
  treeOut->Write();
  fileOut->Close();
  return 0;
}
