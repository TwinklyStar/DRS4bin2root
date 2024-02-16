// This example is developed by Meng Lyu based on PeakFinder class
// GitHub link: https://github.com/claydergc/find-peaks

#include <iostream>
#include <algorithm>
#include "PeakFinder.cpp"
#include "TChain.h"
#include "TH1I.h"
#include "../ChannelReader.h"

void loader(int rate);

void example()
{
    Double_t offset = 0;  // The offset of 4 channels (set in picoscope software)
    Double_t threshold = -10;   // The threshold, below which it won't find peak

	TString infpath = "../../demo/7ch_example.root";
	TString outfpath = "7ch_example_peaks.root";

	TChain* tt = new TChain("wfm");
	tt->Add(infpath);

    // Initialize ChannelReader Class AFTER reading the tree and BEFORE creating any instance
    ChannelReader::Initialize(tt);

    // Input parameter is the channel name that determines which channel the reader will read
    ChannelReader ChA1("ChA1"), ChB1("ChB1"), ChC1("ChC1"), ChD1("ChD1"),
                  ChA2("ChA2"), ChB2("ChB2"), ChC2("ChC2"), ChD2("ChD2");

//    TFile* fo = TFile::Open(outfpath, "RECREATE");

	int nentries = tt->GetEntries();
	std::cout << "Processing " << nentries << " events..." << std::endl;

	std::vector<int> peakn_A1, peakn_B1, peakn_C1, peakn_D1,
                     peakn_A2, peakn_B2, peakn_C2, peakn_D2;

	for (int ientry=0; ientry<nentries; ientry++){

        if (nentries >= 100 && ientry % (nentries/100) == 0) loader(ientry/(nentries/100));

		tt->GetEntry(ientry);
		std::vector<float> ChA1_vec_float, ChB1_vec_float, ChC1_vec_float, ChD1_vec_float,  // PeakFinder only supports float
                           ChA2_vec_float, ChB2_vec_float, ChC2_vec_float, ChD2_vec_float;  // PeakFinder only supports float
		for (int i=0; i<ChA1.V->size(); i++){
			ChA1_vec_float.push_back(ChA1.V->at(i) - offset);
			ChB1_vec_float.push_back(ChB1.V->at(i) - offset);
			ChC1_vec_float.push_back(ChC1.V->at(i) - offset);
//			ChD1_vec_float.push_back(ChD1.V->at(i) - offset);
            ChA2_vec_float.push_back(ChA2.V->at(i) - offset);
            ChB2_vec_float.push_back(ChB2.V->at(i) - offset);
            ChC2_vec_float.push_back(ChC2.V->at(i) - offset);
            ChD2_vec_float.push_back(ChD2.V->at(i) - offset);
		}

		if(ChA1.min_v - offset <= threshold) PeakFinder::findPeaks(ChA1_vec_float, peakn_A1, false, -1);	// -1 is find minimum, 1 is find maximum
        if(ChB1.min_v - offset <= threshold) PeakFinder::findPeaks(ChB1_vec_float, peakn_B1, false, -1);
		if(ChC1.min_v - offset <= threshold) PeakFinder::findPeaks(ChC1_vec_float, peakn_C1, false, -1);
//		if(ChD1.min_v - offset <= threshold) PeakFinder::findPeaks(ChD1_vec_float, peakn_D, false, -1);
        if(ChA2.min_v - offset <= threshold) PeakFinder::findPeaks(ChA2_vec_float, peakn_A2, false, -1);	// -1 is find minimum, 1 is find maximum
        if(ChB2.min_v - offset <= threshold) PeakFinder::findPeaks(ChB2_vec_float, peakn_B2, false, -1);
        if(ChC2.min_v - offset <= threshold) PeakFinder::findPeaks(ChC2_vec_float, peakn_C2, false, -1);
        if(ChD2.min_v - offset <= threshold) PeakFinder::findPeaks(ChD2_vec_float, peakn_D2, false, -1);


        if (ientry < 10) {
            std::cout << std::endl << "Event: " << ientry << std::endl;
            std::cout << "ChA1: " << peakn_A1.size();
            for (auto i: peakn_A1) {
                std::cout << "\t" << ChA1.T->at(i);
            }
            std::cout << std::endl;
            std::cout << "ChB1: " << peakn_B1.size();
            for (auto i: peakn_B1) {
                std::cout << "\t" << ChB1.T->at(i);
            }
            std::cout << std::endl;
            std::cout << "ChC1: " << peakn_C1.size();
            for (auto i: peakn_C1) {
                std::cout << "\t" << ChC1.T->at(i);
            }
            std::cout << std::endl;
//		std::cout << "ChD1: " << peakn_D1.size();
//		for (auto i : peakn_D1){
//			std::cout << "\t" << ChD1.T->at(i);
//		}
//		std::cout << std::endl;
            std::cout << "ChA2: " << peakn_A2.size();
            for (auto i: peakn_A2) {
                std::cout << "\t" << ChA2.T->at(i);
            }
            std::cout << std::endl;
            std::cout << "ChB2: " << peakn_B2.size();
            for (auto i: peakn_B2) {
                std::cout << "\t" << ChB2.T->at(i);
            }
            std::cout << std::endl;
            std::cout << "ChC2: " << peakn_C2.size();
            for (auto i: peakn_C2) {
                std::cout << "\t" << ChC2.T->at(i);
            }
            std::cout << std::endl;
            std::cout << "ChD2: " << peakn_D2.size();
            for (auto i: peakn_D2) {
                std::cout << "\t" << ChD2.T->at(i);
            }
            std::cout << std::endl;
        }

		peakn_A1.clear(); peakn_A1.shrink_to_fit();
		peakn_B1.clear(); peakn_B1.shrink_to_fit();
		peakn_C1.clear(); peakn_C1.shrink_to_fit();
		peakn_D1.clear(); peakn_D1.shrink_to_fit();
        peakn_A2.clear(); peakn_A2.shrink_to_fit();
        peakn_B2.clear(); peakn_B2.shrink_to_fit();
        peakn_C2.clear(); peakn_C2.shrink_to_fit();
        peakn_D2.clear(); peakn_D2.shrink_to_fit();
	}
//	fo->Write();
//	fo->Close();
//	std::cout << "\nSave file as: " << outfpath << std::endl;

}

void loader(int rate)
{
    char proc[22];
    memset(proc, '\0', sizeof(proc));

    for (int i = 0; i < rate/5; i++)
    {
        proc[i] = '#';
    }

    printf("\r[%-20s] [%d%%]", proc, rate);        //C语言格式控制时默认右对齐，所以要在前面加-变成左对齐
    fflush(stdout);                                 //刷新屏幕打印
}
