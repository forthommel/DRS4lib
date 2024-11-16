#include "TCanvas.h"
#include "TFile.h"
#include "TH2D.h"
#include "TProfile2D.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

pair<double, double> FindAmplitudeAndTime(float *x, float *y, double fraction = 0.5) {
  pair<double, double> result;
  result.first = 0;
  result.second = -1e+6;

  double ymin = +1e+6;
  int imin = -1;
  double sx = 0;
  for (int i = 3; i != 1024; ++i) {
    if (y[i] < ymin) {
      ymin = y[i];
      imin = i;
    }
    if (i < 53) {
      sx += y[i];
    }
  }
  result.first = sx / 50. - ymin;
  double threshold = sx / 50. - fraction * result.first;
  for (int i = imin - 2; i > 5; i--) {
    if (y[i] >= threshold && y[i + 1] < threshold) {
      result.second = x[i] + (threshold - y[i]) / (y[i + 1] - y[i]) * (x[i + 1] - x[i]);
      break;
    }
  }
  return result;
}

void CheckSync() {
  TFile *drs_file = TFile::Open("output.root");
  TTreeReader reader("pulse", drs_file);
  TTreeReaderValue<int> run(reader, "run");
  TTreeReaderValue<int> event0(reader, "event0");
  TTreeReaderValue<int> event1(reader, "event1");
  TTreeReaderValue<int> trigger(reader, "trigger");
  TTreeReaderArray<float> timevalue(reader, "timevalue");
  TTreeReaderArray<float> amplitude(reader, "amplitude");

  // for a channel drs/group/ch:
  // channel index = (1024*9*2) * drs + (1024*9) * group + 1024 * ch
  // time index = (1024*2) * drs + 1024 * group

  // 1x1 cm scintillator is connected to both DRS modules
  // drs=0, group=1, ch=5
  // drs=1, group=1, ch=6
  int ich1 = (1024 * 9 * 2) * 0 + (1024 * 9) * 1 + 1024 * 5;
  int ich2 = (1024 * 9 * 2) * 1 + (1024 * 9) * 1 + 1024 * 6;
  int it1 = (1024 * 2) * 0 + 1024 * 1;
  int it2 = (1024 * 2) * 1 + 1024 * 1;

  // Wire Chambers channels:
  // Left:  drs=1, group=1, ch=2
  // Right: drs=1, group=1, ch=1
  // Up:    drs=1, group=1, ch=5
  // Down:  drs=1, group=1, ch=3
  int ichL = (1024 * 9 * 2) * 1 + (1024 * 9) * 1 + 1024 * 2;
  int ichR = (1024 * 9 * 2) * 1 + (1024 * 9) * 1 + 1024 * 1;
  int ichU = (1024 * 9 * 2) * 1 + (1024 * 9) * 1 + 1024 * 5;
  int ichD = (1024 * 9 * 2) * 1 + (1024 * 9) * 1 + 1024 * 3;
  int itL = (1024 * 2) * 1 + 1024 * 1;
  int itR = (1024 * 2) * 1 + 1024 * 1;
  int itU = (1024 * 2) * 1 + 1024 * 1;
  int itD = (1024 * 2) * 1 + 1024 * 1;

  TH2D *h1 = new TH2D("h1", ";S1 amplitude in DRS0 (mV) ;S1 amplitude in DRS1 (mV)", 200, 0, 1000, 200, 0, 1000);
  TProfile2D *h2 =
      new TProfile2D("h2", ";x track (mm) ;y track (mm); mean S1 amplitude (mV)", 200, -10, 30, 200, -20, 20);

  int event = 0;
  while (reader.Next()) {
    // 1x1 cm2 counter
    pair<double, double> reco1 = FindAmplitudeAndTime(&timevalue[it1], &amplitude[ich1]);
    pair<double, double> reco2 = FindAmplitudeAndTime(&timevalue[it2], &amplitude[ich2]);
    h1->Fill(reco1.first, reco2.first);

    // Beam position
    pair<double, double> recoL = FindAmplitudeAndTime(&timevalue[itL], &amplitude[ichL]);
    pair<double, double> recoR = FindAmplitudeAndTime(&timevalue[itR], &amplitude[ichR]);
    pair<double, double> recoU = FindAmplitudeAndTime(&timevalue[itU], &amplitude[ichU]);
    pair<double, double> recoD = FindAmplitudeAndTime(&timevalue[itD], &amplitude[ichD]);

    double x_trk = -1e+6;
    double y_trk = -1e+6;
    bool isOK = true;
    isOK = isOK && (recoL.first > 20.);
    isOK = isOK && (recoR.first > 20.);
    isOK = isOK && (recoU.first > 20.);
    isOK = isOK && (recoD.first > 20.);
    if (isOK) {
      isOK = isOK && (recoL.second > 0.);
      isOK = isOK && (recoR.second > 0.);
      isOK = isOK && (recoU.second > 0.);
      isOK = isOK && (recoD.second > 0.);
    }
    if (isOK) {
      x_trk = 7. / 36. * (recoR.second - recoL.second);
      y_trk = 7. / 36. * (recoD.second - recoU.second);
    }

    h2->Fill(x_trk, y_trk, reco1.first);

    event++;
    if (event % 1000 == 0) {
      cout << " Event " << event << endl;
    }
  }

  TCanvas *c1 = new TCanvas("canvas", "canvas", 1920, 850);
  c1->Divide(2, 1);
  c1->cd(1);
  h1->Draw("colz");
  c1->cd(2);
  h2->Draw("colz");
}
