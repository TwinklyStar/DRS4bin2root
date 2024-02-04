//
// ********************************************************************************
// *               DRS4 binary data analysis program                              *
// *                                                                              *
// *  This is a analysis program written initially to analyze the                 *
// *  data from DRS4 for PSI muon beam compression experiment.                    *
// *  It can be used for any experiments. List of experiments using it:           *
// *  1) Muon Beam Compression at PSI, Oct 2011                                   *
// *  2) Development of PSF Tracker at ETH, Nov 2011                              *
// *                                                                              *
// *  Documentation of DRS4 can be found at                                       *
// *  http://drs.web.psi.ch/docs/manual_rev31.pdf                                 *
// *                                                                              *
// *  Author : Kim Siang KHAW (ETH Zurich, 27.10.2011)                            *
// *  Contact : khaw@phys.ethz.ch                                                 *
// *                                                                              *
// *  History:                                                                    *
// *  1st Edition : Basic histograms for visualization implemented. (27.10.2011)  *
// *  2nd Edition : Tree and Branches are implemented. (02.11.2011)               *
// *              : Compare [0-100]bins and [101-200]bins for RMS and mean        *
// *                calculations.                                                 *
// *              : Flagging events with more than one peak. (13.11.2011)         *
// *  3rd Edition : Calculation of area below the pulse are implemented.          *
// *                (23.11.2011)                                                  *
// *  3.1 Edition : First 10 events from CH1 and CH2 are saved. S/N are           *
// *                calculated.                                                   *
// *                (12.12.2011)                                                  *
// *                                                                              *
// ********************************************************************************
//

// include std libraries
#include <iostream>
#include <fstream>
#include <regex>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iomanip> // 加入头文件以使用 setw 和 setfill

// include ROOT libraries
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TTree.h"
#include "TChain.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TFolder.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TMath.h"
#include "TFile.h"
#include "TSystem.h"
#include "TProfile.h"
#include "TSpectrum.h"

using namespace std;


int verbose = 0;

struct channel_stat{
    Double_t max_v;
    Double_t max_t;
    Double_t min_v;
    Double_t min_t;
    Double_t mean;
    Double_t RMS;
};

struct time_info{
    Short_t year;
    Short_t month;
    Short_t day;
    Short_t hour;
    Short_t min;
    Short_t sec;
    Short_t ms;
};

int main (int argc, char **argv){


    cout << "********************************************************************" << endl;
    cout << "*****              Welcome to DRS4 data analysis               *****" << endl;
    cout << "********************************************************************" << endl;
    cout << endl;

    ifstream file;		// read file directly

    if (argc == 2){
		file.open (argv[1], ios::in | ios::binary);
		cout << ">> Opening file " << argv[1] << " ......" << endl;
		cout << endl;

		if (!file.is_open()){	// terminate if the file can't be opened
			cerr << "!! File open error:" << argv[1] << endl;
			return 1;
		}
    }
    else {	// terminate if there is no input file or more than 1 input file
		cout << "!! No input file" << endl;
		return 1;
    }

    // automatically change XXXX.dat to XXXX.root
    int file_len = strlen (argv[1]);
    string filename = argv[1];
    filename.replace (file_len - 3, 3, "root");
	
	char FileHeader[5];
    char TimeHeader[5];
    char EventHeader[5];
    char BoardSerialNumber[5];
    char TriggerCellNumber[5];
    short Date[8];
    short number;
    float EventTime[1024];
    int SerialNumber;
    int ScalerNumber;
    char ChannelHeader[5];
    unsigned short ChannelDataRaw[1024];
    unsigned short ChannelData[1024];
    bool endoffile = false;
    int n (0);
    clock_t start = clock();
    time_t realtime;

    channel_stat chstat_manager[8];     // Maximum 8 channels, i.e. 2 boards in chain;
    std::vector<Double_t> chT_manager[8];     // Maximum 8 channels, i.e. 2 boards in chain;
    std::vector<Double_t> chV_manager[8];     // Maximum 8 channels, i.e. 2 boards in chain;
    time_info time_manager;
    UInt_t nevent = 0;
    Int_t  data_length = 1024;

    cout << ">> Start reading file" << argv[1] << " ......" << endl;
    cout << endl;

    TFile *treefile = new TFile ((char *) filename.c_str(), "recreate");
    cout << ">> Creating rootfile " << filename << " ......" << endl;
    cout << endl;
    TTree *tree = new TTree ("wfm", "A tree storing waveform data from DRS4");
    tree->Branch ("EvtNum", &nevent, "EvtNum/i");
    tree->Branch("Data_Length", &data_length, "Data_Length/I");
    tree->Branch("EvtTime", &time_manager, "year/S:month/S:day/S:hour/S:min/S:sec/S:ms/S");

    // Read file header
    file.read ((char *) &FileHeader, 4);
    FileHeader[4] = '\0';

    cout<<"File header: "<<FileHeader<<endl;

    // Read time header
    file.read ((char *) &TimeHeader, 4);
    TimeHeader[4] = '\0';

    cout<<"Time header: "<<TimeHeader<<endl;

    // Read board serial number
    file.read ((char *) &BoardSerialNumber, 2);
    BoardSerialNumber[2] = '\0';
    file.read ((char *) &number, 2);

    cout<<"Board serial number: "<<BoardSerialNumber << number << endl;


    bool end_of_timeinfo = false;
    int bd_itr=0;   // Board iterator
    std::vector<short> channel_seq; // Channel iterator

    // Header matching template
    std::regex bd_serial_template("B#..");
    std::regex ch_header_template("C00.");

    // Identify channel number and create branches
    std::cout << ">> Starting identifying channel and board number..." << std::endl;
    while (! end_of_timeinfo) {
        // Read channel header
        file.read((char*)&ChannelHeader, 4);
//        std::cout << ChannelHeader << std::endl;
        ChannelHeader[4] = '\0';
        if (std::regex_match(ChannelHeader, ch_header_template)) {  // Matching the channel header
            if (strcmp(ChannelHeader, "C001") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChA1_T", &chT_manager[0]);
                    tree->Branch("ChA1_V", &chV_manager[0]);
                    tree->Branch("ChA1_max_v", &chstat_manager[0].max_v, "ChA1_max_v/D");
                    tree->Branch("ChA1_max_t", &chstat_manager[0].max_t, "ChA1_max_t/D");
                    tree->Branch("ChA1_min_v", &chstat_manager[0].min_v, "ChA1_min_v/D");
                    tree->Branch("ChA1_min_t", &chstat_manager[0].min_t, "ChA1_min_t/D");
                    tree->Branch("ChA1_mean",  &chstat_manager[0].mean,  "ChA1_mean/D");
                    tree->Branch("ChA1_RMS",   &chstat_manager[0].RMS,   "ChA1_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[0].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[0].back();
                    }
                    channel_seq.push_back(0);
                } else if (bd_itr == 1) {
                    tree->Branch("ChA2_T", &chT_manager[4]);
                    tree->Branch("ChA2_V", &chV_manager[4]);
                    tree->Branch("ChA2_max_v", &chstat_manager[4].max_v, "ChA2_max_v/D");
                    tree->Branch("ChA2_max_t", &chstat_manager[4].max_t, "ChA2_max_t/D");
                    tree->Branch("ChA2_min_v", &chstat_manager[4].min_v, "ChA2_min_v/D");
                    tree->Branch("ChA2_min_t", &chstat_manager[4].min_t, "ChA2_min_t/D");
                    tree->Branch("ChA2_mean",  &chstat_manager[4].mean,  "ChA2_mean/D");
                    tree->Branch("ChA2_RMS",   &chstat_manager[4].RMS,   "ChA2_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[4].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[4].back();
                    }
                    channel_seq.push_back(4);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C002") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChB1_T", &chT_manager[1]);
                    tree->Branch("ChB1_V", &chV_manager[1]);
                    tree->Branch("ChB1_max_v", &chstat_manager[1].max_v, "ChB1_max_v/D");
                    tree->Branch("ChB1_max_t", &chstat_manager[1].max_t, "ChB1_max_t/D");
                    tree->Branch("ChB1_min_v", &chstat_manager[1].min_v, "ChB1_min_v/D");
                    tree->Branch("ChB1_min_t", &chstat_manager[1].min_t, "ChB1_min_t/D");
                    tree->Branch("ChB1_mean",  &chstat_manager[1].mean,  "ChB1_mean/D");
                    tree->Branch("ChB1_RMS",   &chstat_manager[1].RMS,   "ChB1_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[1].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[1].back();
                    }
                    channel_seq.push_back(1);
                } else if (bd_itr == 1) {
                    tree->Branch("ChB2_T", &chT_manager[5]);
                    tree->Branch("ChB2_V", &chV_manager[5]);
                    tree->Branch("ChB2_max_v", &chstat_manager[5].max_v, "ChB2_max_v/D");
                    tree->Branch("ChB2_max_t", &chstat_manager[5].max_t, "ChB2_max_t/D");
                    tree->Branch("ChB2_min_v", &chstat_manager[5].min_v, "ChB2_min_v/D");
                    tree->Branch("ChB2_min_t", &chstat_manager[5].min_t, "ChB2_min_t/D");
                    tree->Branch("ChB2_mean",  &chstat_manager[5].mean,  "ChB2_mean/D");
                    tree->Branch("ChB2_RMS",   &chstat_manager[5].RMS,   "ChB2_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[5].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[5].back();
                    }
                    channel_seq.push_back(5);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C003") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChC1_T", &chT_manager[2]);
                    tree->Branch("ChC1_V", &chV_manager[2]);
                    tree->Branch("ChC1_max_v", &chstat_manager[2].max_v, "ChC1_max_v/D");
                    tree->Branch("ChC1_max_t", &chstat_manager[2].max_t, "ChC1_max_t/D");
                    tree->Branch("ChC1_min_v", &chstat_manager[2].min_v, "ChC1_min_v/D");
                    tree->Branch("ChC1_min_t", &chstat_manager[2].min_t, "ChC1_min_t/D");
                    tree->Branch("ChC1_mean",  &chstat_manager[2].mean,  "ChC1_mean/D");
                    tree->Branch("ChC1_RMS",   &chstat_manager[2].RMS,   "ChC1_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[2].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[2].back();
                    }
                    channel_seq.push_back(2);
                } else if (bd_itr == 1) {
                    tree->Branch("ChC2_T", &chT_manager[6]);
                    tree->Branch("ChC2_V", &chV_manager[6]);
                    tree->Branch("ChC2_max_v", &chstat_manager[6].max_v, "ChC2_max_v/D");
                    tree->Branch("ChC2_max_t", &chstat_manager[6].max_t, "ChC2_max_t/D");
                    tree->Branch("ChC2_min_v", &chstat_manager[6].min_v, "ChC2_min_v/D");
                    tree->Branch("ChC2_min_t", &chstat_manager[6].min_t, "ChC2_min_t/D");
                    tree->Branch("ChC2_mean",  &chstat_manager[6].mean,  "ChC2_mean/D");
                    tree->Branch("ChC2_RMS",   &chstat_manager[6].RMS,   "ChC2_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[6].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[6].back();
                    }
                    channel_seq.push_back(6);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C004") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChD1_T", &chT_manager[3]);
                    tree->Branch("ChD1_V", &chV_manager[3]);
                    tree->Branch("ChD1_max_v", &chstat_manager[3].max_v, "ChD1_max_v/D");
                    tree->Branch("ChD1_max_t", &chstat_manager[3].max_t, "ChD1_max_t/D");
                    tree->Branch("ChD1_min_v", &chstat_manager[3].min_v, "ChD1_min_v/D");
                    tree->Branch("ChD1_min_t", &chstat_manager[3].min_t, "ChD1_min_t/D");
                    tree->Branch("ChD1_mean",  &chstat_manager[3].mean,  "ChD1_mean/D");
                    tree->Branch("ChD1_RMS",   &chstat_manager[3].RMS,   "ChD1_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[3].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[3].back();
                    }
                    channel_seq.push_back(3);
                } else if (bd_itr == 1) {
                    tree->Branch("ChD2_T", &chT_manager[7]);
                    tree->Branch("ChD2_V", &chV_manager[7]);
                    tree->Branch("ChD2_max_v", &chstat_manager[7].max_v, "ChD2_max_v/D");
                    tree->Branch("ChD2_max_t", &chstat_manager[7].max_t, "ChD2_max_t/D");
                    tree->Branch("ChD2_min_v", &chstat_manager[7].min_v, "ChD2_min_v/D");
                    tree->Branch("ChD2_min_t", &chstat_manager[7].min_t, "ChD2_min_t/D");
                    tree->Branch("ChD2_mean",  &chstat_manager[7].mean,  "ChD2_mean/D");
                    tree->Branch("ChD2_RMS",   &chstat_manager[7].RMS,   "ChD2_RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        chT_manager[7].push_back((Double_t)EventTime[i] + time_last);
                        time_last = chT_manager[7].back();
                    }
                    channel_seq.push_back(7);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
        }
        else if (std::regex_match(ChannelHeader, bd_serial_template)) bd_itr++;
        else if (strcmp(ChannelHeader, "EHDR") == 0){   // End of time info part
            end_of_timeinfo = true;
        }
        else {
            std::cerr << "ERROR(TimeInfo): Unexpected Channel Header!!!" << std::endl;
            std::cout << "Current Channel Header: " << ChannelHeader << std::endl;
            return -1;
        }
    }

	std::cout << "DRS4 board number: " << bd_itr+1 << std::endl << "Channel number: " << channel_seq.size() << std::endl << "Activated channels: ";
    for (auto ch_num : channel_seq) std::cout << ch_num+1 << " ";
    std::cout << std::endl;
    if (channel_seq.at(0) >= 5){
        std::cerr << "ERROR: No channel in the first board is active!!!" << std::endl;
        return -1;
    }
    std::cout << "First Event Header: " << ChannelHeader << std::endl;

    // ******** At present time bin has some problem with #3098 board. Copy the time info. from the 2nd board to 1st board **********
//    for (int i=0; i<4; i++){
//        for (int j=0; j<1024; j++){
//            ch_manager[i].Time[j] = ch_manager[i+4].Time[j];
//        }
//    }
    // *******************************

//    tree->Fill();
//    tree->Write();
//    treefile->Close();
//    return 0;

    while (!endoffile){ // event loop

		// Count Event, show the progress every 1000 events
		if (nevent % 100000 == 0){
			time (&realtime);
			cout << ">> Processing event No." << nevent << ", Time elapsed : " <<
			(double) (clock() - start) / CLOCKS_PER_SEC << " secs, Current time : " << ctime (&realtime) << endl;
			//start = clock();
		}

        if (nevent>0){  // not the first event header
            // Read event header
            file.read ((char *) &EventHeader, 4);
            EventHeader[4] = '\0';
            if (strcmp(EventHeader, "EHDR") != 0) {
                cerr << "ERROR: Wrong event header in event " << nevent << endl;
                return -1;
            }
            if (verbose >= 1 && nevent % 1000 == 1) cout << nevent << " th Event header: " << EventHeader << endl;
        }

		// Read event serial number
		file.read ((char *) &SerialNumber, 4);

		if (verbose >= 1) cout<<"Event serial number: "<<SerialNumber<<endl;

		// Read date (YY/MM/DD/HH/mm/ss/ms/rr)
		file.read ((char *) &Date, 16);

        time_manager.year = Date[0];
        time_manager.month = Date[1];
        time_manager.day = Date[2];
        time_manager.hour = Date[3];
        time_manager.min = Date[4];
        time_manager.sec = Date[5];
        time_manager.ms = Date[6];

//		int LastTime;
//		int CurrentTime;
//        int StartTime;
//
//		// calculate time since last event in milliseconds
//		LastTime = CurrentTime;
//		CurrentTime =
//		Date[3] * 3600000 + Date[4] * 60000 + Date[5] * 1000 + Date[6];
//
//        if (nevent == 0) StartTime = CurrentTime;
//
//		if (verbose >= 1)  cout<<"Current time: "<<CurrentTime<<endl;

        // Start channel looping in a single event
        short last_ch_num = -1;

        for (auto current_ch_num : channel_seq){
            if ((last_ch_num < 0 && current_ch_num >= 0) || (last_ch_num < 4 && current_ch_num >= 4)){  // First channel of a board
                // Read board serial number
                file.read ((char *) &BoardSerialNumber, 2);
                BoardSerialNumber[2] = '\0';
                file.read((char *) &number, 2);

                if (verbose >= 1) cout<<"Board serial number: "<<BoardSerialNumber << number << endl;

                // Read trigger cell number
                file.read ((char *) &TriggerCellNumber, 2);
                file.read((char *) &number, 2);
                TriggerCellNumber[2] = '\0';

                if (verbose >= 1) cout<<"Trigger cell number: "<<TriggerCellNumber << number <<endl;
            }
            last_ch_num = current_ch_num;

            file.read ((char *) &ChannelHeader, 4);
            ChannelHeader[4] = '\0';

            if (verbose >= 1) cout << "Channel Header : " << ChannelHeader << endl;

            // Read scaler number
            file.read ((char *) &ScalerNumber, 4);

            if (verbose >= 1) cout<<"Scaler number: "<<ScalerNumber<<endl;

            // get amplitude of each channel
            file.read ((char *) &ChannelDataRaw, 2048);

            // apply 5-sample average of waveform smoothing
            // this will decrease the resolution, but ok for very noisy daq
            ChannelData[0] = ChannelDataRaw[0];
            ChannelData[1] = ChannelDataRaw[1];
            ChannelData[1022] = ChannelDataRaw[1022];
            ChannelData[1023] = ChannelDataRaw[1023];

            for (int i = 2; i < 1022; i++){
                ChannelData[i] = (ChannelDataRaw[i - 2] + ChannelDataRaw[i - 1] + ChannelDataRaw[i] + ChannelDataRaw[i + 1] + ChannelDataRaw[i + 2]) / 5;
            }

            // Unit conversion: 0 -> -500, 65535 -> 500
            for (int i=0; i<1024; i++){
                chV_manager[current_ch_num].push_back(ChannelData[i] * (1000.0 / 65535.0) + (Date[7] - 500));
            }

//            double v_RMS[5];
//
//            for (int j = 0; j < 5; j++){
//                v_RMS[j] = TMath::RMS (&ch_manager[current_ch_num-1].Voltage[j * 200], &ch_manager[current_ch_num-1].Voltage[(j + 1) * 200]);	// calculate RMS for 5 sections
//            }
//            int index_v_RMS = TMath::LocMin (5, v_RMS);	// locate the section for minimum RMS
//            double vRMS = v_RMS[index_v_RMS];	// use RMS in that section
//            double vmean = TMath::Mean (&ch_manager[current_ch_num-1].Voltage[index_v_RMS * 200], &ch_manager[current_ch_num-1].Voltage[(index_v_RMS + 1) * 200]);	// use mean in that section
//
            // Find Max and Min of the Channel data (Voltage)
            auto itr_min = TMath::LocMin (chV_manager[current_ch_num].begin(), chV_manager[current_ch_num].end());	// return index of the min
            chstat_manager[current_ch_num].min_v = *itr_min;	// return value of the vmin
            int idx_min = std::distance(chV_manager[current_ch_num].begin(), itr_min);
            chstat_manager[current_ch_num].min_t = chT_manager[current_ch_num].at(idx_min);	// return value of the tmin

            auto itr_max = TMath::LocMax (chV_manager[current_ch_num].begin(), chV_manager[current_ch_num].end());	// return index of the max
            chstat_manager[current_ch_num].max_v = *itr_max;	// return value of the vmax
            int idx_max = std::distance(chV_manager[current_ch_num].begin(), itr_max);
            chstat_manager[current_ch_num].max_t = chT_manager[current_ch_num].at(idx_max);	// return value of the tmax

            // Calculate the mean and RMS of each channel
            chstat_manager[current_ch_num].mean = TMath::Mean(chV_manager[current_ch_num].begin(), chV_manager[current_ch_num].end());
            chstat_manager[current_ch_num].RMS  = TMath::RMS(chV_manager[current_ch_num].begin(), chV_manager[current_ch_num].end());

        }

		tree->Fill();		// fill the tree event by event
        ++nevent;
        for (auto current_ch_num : channel_seq){
            chV_manager[current_ch_num].clear();
            chV_manager[current_ch_num].shrink_to_fit();
        }
		if (file.eof()){
			cout << ">> Reach End of the file .... " << endl;
			cout << ">> Total event number: " << nevent << endl;
			endoffile = true;
			break;
		}
    } // end of event loop

    cout << "The tree was saved." << endl;
	cout << "Last Event Header: " << EventHeader << endl;
//    treefile->Write();
    tree->Write();
    treefile->Close();
    cout << "The treefile was saved." << endl;
    cout << "Time elapsed : " <<
         (double) (clock() - start) / CLOCKS_PER_SEC << " secs, Current time : " << ctime (&realtime) << endl;


    file.close();

    return 0;
}