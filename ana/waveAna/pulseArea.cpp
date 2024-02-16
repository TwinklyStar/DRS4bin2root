//
// Created by Meng Lyu on 2024/2/16.
//
#include "../ChannelReader.h"
#include "TChain.h"
#include "TH1D.h"

void pulseArea(){
    TString infpath = "../../demo/7ch_example.root";
    TString outfpath = "demo_area.root";

    TChain* tt = new TChain("wfm");
    tt->Add(infpath);

    // Initialize ChannelReader Class AFTER reading the tree and BEFORE creating any instance
    ChannelReader::Initialize(tt);

    // Input parameter is the channel name that determines which channel the reader will read
    ChannelReader ChA1("ChA1"), ChB1("ChB1"), ChC1("ChC1"), ChD1("ChD1"),
            ChA2("ChA2"), ChB2("ChB2"), ChC2("ChC2"), ChD2("ChD2");

    TFile* fo = TFile::Open(outfpath, "RECREATE");
    TH1D* harea_A1 = new TH1D("harea_A1", "ChA1 Area", 200, 0, 10000);
    harea_A1->GetXaxis()->SetTitle("Area [mV #times ns]");

    // Essential steps before calculating area
    ChA1.CalculateBaseline();
    std::cout << "Baseline of " << ChA1.GetName() << " is: " << ChA1.GetBaseline() << " mV" << std::endl;
    ChA1.SetPolarity(-1);   // Negative pulse

    int nentries = tt->GetEntries();
    std::cout << "Processing " << nentries << " events..." << std::endl;
    for (int ientry=0; ientry < nentries; ientry++){
        tt->GetEntry(ientry);
        // Calculate area between 50 - 200 ns (The first peak)
        int start_point = ChA1.GetIndexByTime(50);
        int integral_length = ChA1.GetIndexByTime(250) - start_point;

        // For DRS4, the baseline is unstable. Each event's baseline should be calculated separately
        // Therefore, the third parameter of CalculateAres should be set to true
        harea_A1->Fill(ChA1.CalculateArea(start_point, integral_length, true));
    }

    fo->Write();
    fo->Close();
}
