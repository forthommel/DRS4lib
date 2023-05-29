// C++ includes
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>


// ROOT includes
#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TCanvas.h>


using namespace std;

class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};


std::vector<pair<uint,uint> >  listOfGoodEvents(std::string inputFileName)
{
    pair<uint,uint> counter;
    std::vector<pair<uint,uint> > result;
    uint   dummy;
    uint   event_header;
    uint   temp[3];
    ifstream ifile(inputFileName);
    FILE* fpin = fopen( inputFileName.c_str(), "r" );

    for( int iEvent = 0; iEvent < 1000000; iEvent++){ 
        bool isGoodEvent = true;
        
        dummy = fread( &event_header, sizeof(uint), 1, fpin);
        UInt_t INIT = event_header>>28;
        isGoodEvent = isGoodEvent && (INIT==0b1010);
        UInt_t TOTAL_EVENT_SIZE = (event_header << 4)>>4;
        isGoodEvent = isGoodEvent && (TOTAL_EVENT_SIZE==6920);
        
        dummy = fread( &event_header, sizeof(uint), 1, fpin);  
        UInt_t BF = (event_header>>26) & 0b1;
        isGoodEvent = isGoodEvent && (BF==0);
        UInt_t grM     = event_header & 0b11; // 2-bit channel group mask

        dummy = fread( &event_header, sizeof(uint), 1, fpin);  
        UInt_t EVENTCOUNTER = (event_header<<8)>>8;

        dummy = fread( &event_header, sizeof(uint), 1, fpin);  
        if (feof(fpin)) break;
      
        bool _isGR_On[2];
        _isGR_On[0] = (grM & 0x01);
        _isGR_On[1] = (grM & 0x02);      
        int activeGroupsN = 0;
        for ( int l = 0; l < 2; l++ ) {
            if ( _isGR_On[l] ) {
                activeGroupsN++;
            }
        }
        isGoodEvent = isGoodEvent && (activeGroupsN == 2);
        for ( int group = 0; group < activeGroupsN; group++ ) {
          dummy = fread( &event_header, sizeof(uint), 1, fpin); 
          UInt_t CONTROL1 = event_header>>30;
          UInt_t CONTROL2 = (event_header>>18) & 0b11;
          UInt_t CONTROL3 = (event_header>>13) & 0b111;
          isGoodEvent = isGoodEvent && ((CONTROL1+CONTROL2+CONTROL3) == 0);
          UInt_t TR = (event_header>>12) & 0b1;
          isGoodEvent = isGoodEvent && (TR==1);
          int nsample = (event_header & 0xfff) / 3;
          for ( int i = 0; i < nsample; i++ ) {
              dummy = fread( &temp, sizeof(uint), 3, fpin );  
          }
          if(TR){
              for(int j = 0; j < nsample/8; j++){
                  fread( &temp, sizeof(uint), 3, fpin);  
              }
          }
          dummy = fread( &event_header, sizeof(uint), 1, fpin);
        }
      
        if(!isGoodEvent) continue;
        counter.first  = iEvent;
        counter.second = EVENTCOUNTER;
        result.push_back(counter);
    }
    fclose(fpin);
    return result;
}





int main(int argc, char **argv) {
    
    std::string inputFileName0 = "output0.dat";
    std::string inputFileName1 = "output1.dat";
    std::string outputFileName = "list_tmp.txt";
    InputParser input(argc, argv);
    if(input.cmdOptionExists("-h")){
        std::cout << " Creates list of events in sync and saves it in list_tmp.txt " << std::endl;
        std::cout << " Format: <event number in DRS0> <event number in DRS1> <trigger number>   " << std::endl;
        std::cout << " Usage:  ./listGoodEvents  -h" << std::endl;
        std::cout << "                 prints this text " << std::endl;
        std::cout << "        ./listGoodEvents  " << std::endl;
        std::cout << "                 reads from output0.dat and output1.dat by default " << std::endl;
        std::cout << "        ./listGoodEvents -r XXX  " << std::endl;
        std::cout << "                 reads from RUNXXX_output0.dat and RUNXXX_output1.dat " << std::endl;
        std::cout << "        ./listGoodEvents -r XXX  -p PATH/ " << std::endl;
        std::cout << "                 reads from PATH/RUNXXX_output0.dat and PATH/RUNXXX_output1.dat " << std::endl;
        exit(0);
    }
    const std::string &srun = input.getCmdOption("-r");
    if (!srun.empty()){
        inputFileName0 = "RUN" + srun + "_output0.dat";
        inputFileName1 = "RUN" + srun + "_output1.dat";
    }  
    const std::string &pathname = input.getCmdOption("-p");
    if (!pathname.empty()){
        inputFileName0 = pathname + inputFileName0;
        inputFileName1 = pathname + inputFileName1;
    }  

    std::cout << inputFileName0 << std::endl;
    std::cout << inputFileName1 << std::endl;

    ifstream ifile0(inputFileName0);
    if (!ifile0)
    {
      std::cerr << "[ERROR]: !USAGE! Input file 0 does not exist. Please enter valid file name" << std::endl;
      exit(0);
    }
    ifstream ifile1(inputFileName1);
    if (!ifile1)
    {
      std::cerr << "[ERROR]: !USAGE! Input file 1 does not exist. Please enter valid file name" << std::endl;
      exit(0);
    }
 
    std::vector<pair<uint,uint> > vlist0 = listOfGoodEvents(inputFileName0);
    std::vector<pair<uint,uint> > vlist1 = listOfGoodEvents(inputFileName1);
  
    unsigned int i0 = 0;
    unsigned int i1 = 0;
    std::vector<uint> good_event_0;
    std::vector<uint> good_event_1;
    std::vector<uint> good_trigger;
    while(i0 < vlist0.size() && i1 < vlist1.size()){
        if(vlist0[i0].second != vlist1[i1].second){
            if(vlist0[i0].second < vlist1[i1].second){
                i0++;
            }else{
                i1++;
            }
        }else{
            good_event_0.push_back(vlist0[i0].first);
            good_event_1.push_back(vlist1[i1].first);
            good_trigger.push_back(vlist0[i0].second);
            i0++;
            i1++;
        }
    }
    
    std::ofstream fout(outputFileName.c_str());
    for(unsigned int i=0; i<good_trigger.size(); i++){
        fout << good_event_0[i] << "   " << good_event_1[i] << "   " << good_trigger[i] << "\n";
    }
    
    std::cout << " Number of good events in drs0 = " << vlist0.size() << std::endl;
    std::cout << " Number of good events in drs1 = " << vlist1.size() << std::endl;
    std::cout << " Number of good events in sync = " << good_trigger.size() << std::endl;
    return 0;
}



