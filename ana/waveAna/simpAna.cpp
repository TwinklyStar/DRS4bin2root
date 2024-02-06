#include "../ChannelReader.h"
using namespace std;

struct time_info{
    Short_t year;
    Short_t month;
    Short_t day;
    Short_t hour;
    Short_t min;
    Short_t sec;
    Short_t ms;
};

void simpAna(TString infname="../../demo/7ch_example.root"){
    cout << "********************************************************************" << endl;
    cout << "*****            Welcome to waveform data analysis             *****" << endl;
    cout << "********************************************************************" << endl;
    cout << endl;

    TFile* ff = TFile::Open(infname, "READ");
    TTree* tt = (TTree*) ff->Get("wfm");

    // Initialize ChannelReader Class AFTER reading the tree and BEFORE creating any instance
    ChannelReader::Initialize(tt);

    // Input parameter is the channel name that determines which channel the reader will read
    ChannelReader ChA1("ChA1"), ChB1("ChB1"), ChC1("ChC1"), ChD1("ChD1"),
                  ChA2("ChA2"), ChB2("ChB2"), ChC2("ChC2"), ChD2("ChD2");

    time_info EvtTime;
    Int_t data_length;
    UInt_t evt_num;

    tt->SetBranchAddress("EvtNum", &evt_num);
    tt->SetBranchAddress("Data_Length", &data_length);
    tt->SetBranchAddress("EvtTime", &EvtTime);

    Long64_t nentries = tt->GetEntries();

    // Event loop
    for (int ientry=0; ientry<nentries; ientry++){
        tt->GetEntry(ientry);

        // ************************ Fill your analysis code here ************************

        if (ientry % 1000 == 0){
            std::cout << "\nEvent " << ientry << ":\nEvent Date: " << EvtTime.year << "/" << EvtTime.month << "/" << EvtTime.day << std::endl;
            if (ChA1.GetStatus()){  // ChA1 is open
                TString ch_name = ChA1.GetName();
                std::cout << TString("Minimum Voltage in ") + ch_name + ": " << ChA1.min_v << "mV\t at " << ChA1.min_t << "us"
                          << TString("\nMaximum Voltage in ") + ch_name + ": " << ChA1.max_v << "mV\t at " << ChA1.max_t << "us"
                          << std::endl;     // The unit may change according to the settings in PicoScope software
                std::cout << TString("The 100th point in ") + ch_name + TString(" is: ") << ChA1.T->at(99) << "us, " << ChA1.V->at(99) << "mV" << std::endl;
            }
            if (ChB1.GetStatus()){  // ChB1 is open
                TString ch_name = ChB1.GetName();
                std::cout << TString("Minimum Voltage in ") + ch_name + ": " << ChB1.min_v << "mV\t at " << ChB1.min_t << "us"
                          << TString("\nMaximum Voltage in ") + ch_name + ": " << ChB1.max_v << "mV\t at " << ChB1.max_t << "us"
                          << std::endl;     // The unit may change according to the settings in PicoScope software
                std::cout << TString("The 100th point in ") + ch_name + TString(" is: ") << ChB1.T->at(99) << "us, " << ChB1.V->at(99) << "mV" << std::endl;
            }
        }

        // ******************************************************************************
    }

}