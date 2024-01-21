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

// 整型数据存储结构 
struct Data_t_int {
	int channel_number;
	int* ch;
};

// 单精度浮点数存储结构 
struct Data_t_float {
	int channel_number;
	float* ch;
};

// 双精度浮点数存储解构
struct Data_t_double {
	int channel_number;
	double* ch;
};

int verbose = 0;
int channel_total_num = 2;
const int kMaxChannels = 16; // 你的最大通道数量，根据实际情况调整

//TSpectrum *s1 = new TSpectrum();
void loader(int rate)
{
    char proc[22];
    memset(proc, '\0', sizeof(proc));

    for (int i = 0; i < rate/5; i++)
    {
        proc[i] = '#';
    }

    printf("\r[%-20s] [%d%%]", proc, rate);
    fflush(stdout);
}

struct channel_info{
    Double_t Time[1024] = {0};
    Double_t Voltage[1024] = {0};
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

    channel_info ch_manager[8];     // Maximum 8 channels, i.e. 2 boards in chain;
    time_info time_manager;
    Long64_t nevent = 0;

    cout << ">> Start reading file" << argv[1] << " ......" << endl;
    cout << endl;

    TFile *treefile = new TFile ((char *) filename.c_str(), "recreate");
    cout << ">> Creating rootfile " << filename << " ......" << endl;
    cout << endl;
    TTree *tree = new TTree ("T", "An example of ROOT tree with a few branches");
    tree->Branch ("nevent", &nevent, "event_number/L");
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


    // Waveform of the first 5 events
    TH1F *hist_event1[8];
    TH1F *hist_event2[8];
    TH1F *hist_event3[8];
    TH1F *hist_event4[8];
    TH1F *hist_event5[8];

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
                    tree->Branch("ChA1", &ch_manager[0],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[0].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[0].Time[i];
                    }
                    channel_seq.push_back(0);
                } else if (bd_itr == 1) {
                    tree->Branch("ChA2", &ch_manager[4],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[4].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[4].Time[i];
                    }
                    channel_seq.push_back(4);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C002") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChB1", &ch_manager[1],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[1].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[1].Time[i];
                    }
                    channel_seq.push_back(1);
                } else if (bd_itr == 1) {
                    tree->Branch("ChB2", &ch_manager[5],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[5].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[5].Time[i];
                    }
                    channel_seq.push_back(5);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C003") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChC1", &ch_manager[2],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[2].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[2].Time[i];
                    }
                    channel_seq.push_back(2);
                } else if (bd_itr == 1) {
                    tree->Branch("ChC2", &ch_manager[6],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[6].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[6].Time[i];
                    }
                    channel_seq.push_back(6);
                } else {
                    std::cerr << "ERROR(TimeInfo): More than two DRS4 boards!!!" << std::endl;
                    return -1;
                }
            }
            if (strcmp(ChannelHeader, "C004") == 0) {
                if (bd_itr == 0) {
                    tree->Branch("ChD1", &ch_manager[3],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[3].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[3].Time[i];
                    }
                    channel_seq.push_back(3);
                } else if (bd_itr == 1) {
                    tree->Branch("ChD2", &ch_manager[7],
                                 "Time[1024]/D:Voltage[1024]/D:max_v/D:max_t/D:min_v/D:min_t/D:mean/D:RMS/D");

                    // Read event times
                    file.read((char *) &EventTime, 4096);
                    Double_t time_last = 0;
                    for (int i = 0; i < 1024; i++) {
                        ch_manager[7].Time[i] = (Double_t)EventTime[i] + time_last;
                        time_last = ch_manager[7].Time[i];
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

		int LastTime;
		int CurrentTime;
        int StartTime;

		// calculate time since last event in milliseconds
		LastTime = CurrentTime;
		CurrentTime =
		Date[3] * 3600000 + Date[4] * 60000 + Date[5] * 1000 + Date[6];

        if (nevent == 0) StartTime = CurrentTime;

		if (verbose >= 1)  cout<<"Current time: "<<CurrentTime<<endl;

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
                ch_manager[current_ch_num].Voltage[i] = ChannelData[i] * (1000.0 / 65535.0) + (Date[7] - 500);
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
            int index_min = TMath::LocMin (1024, ch_manager[current_ch_num].Voltage);	// return index of the min
            ch_manager[current_ch_num].min_v = ch_manager[current_ch_num].Voltage[index_min];	// return value of the vmin
            ch_manager[current_ch_num].min_t = ch_manager[current_ch_num].Time[index_min];	// return value of the tmin
            int index_max = TMath::LocMax (1024, ch_manager[current_ch_num].Voltage);	// return index of the max
            ch_manager[current_ch_num].max_v = ch_manager[current_ch_num].Voltage[index_max];	// return value of the vmin
            ch_manager[current_ch_num].max_t = ch_manager[current_ch_num].Time[index_max];	// return value of the tmin

            // Calculate the mean and RMS of each channel
            ch_manager[current_ch_num].mean = TMath::Mean(1024, &ch_manager[current_ch_num].Voltage[0]);
            ch_manager[current_ch_num].RMS = TMath::RMS(1024, &ch_manager[current_ch_num].Voltage[0]);

        }

		tree->Fill();		// fill the tree event by event
        ++nevent;
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