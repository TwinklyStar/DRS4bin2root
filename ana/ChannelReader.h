//
// Created by Meng Lyu on 2024/2/3.
//

#ifndef CSV2ROOT_CHANNELREADER_H
#define CSV2ROOT_CHANNELREADER_H

#include <iostream>
#include <vector>
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"
#include "TGraph.h"

class ChannelReader{
public:
    static bool is_DRS4;
    static std::vector<Double_t> *pico_T;
    std::vector<Double_t> *T;
    std::vector<Double_t> *V;
    Double_t max_v;
    Double_t max_t;
    Double_t min_v;
    Double_t min_t;
    Double_t mean;
    Double_t RMS;

    static void Initialize(TTree* tin){
        tt = tin;
        if (tt->FindBranch("Time") != nullptr){ // pico
            pico_T = new std::vector<Double_t>;
            tt->SetBranchAddress("Time", &pico_T);
            is_DRS4 = false;
        }
        else {
            is_DRS4 = true;
            pico_T = nullptr;
        }
    }

    TString GetName() {return ch_name;}

    bool GetStatus() {return is_open;}

    int GetDataLength() {return data_length;}

    int GetIndexByTime(Double_t target_time) {
        if (target_time < T->at(0) || target_time > T->back()) return -1;   // Out of boundary

        int idx = 0;
        while (T->at(idx) < target_time) idx++;
        return idx-1;
    }

    Double_t CalculateBaseline(int test_num = 20){
        baseline = 0;
        if (test_num > tt->GetEntries()){
            std::cout << "WARNING: ChannelReader::CalculateBaseline: Event number used to calculate baseline exceeds "
                      << "total number " << tt->GetEntries() << ". Set number to the total number..." << std::endl;
            test_num = tt->GetEntries();
        }

        for (int i=0; i<test_num; i++){
            tt->GetEntry(i);
            baseline += GetMostFreqVoltage() / test_num;
        }
        return baseline;
    }
    Double_t GetBaseline() {return baseline;}
    Double_t GetCurrentBaseline() {return GetMostFreqVoltage();}

    Double_t CalculateArea(int start_pt, int len, bool recal_baseline = false){  // If the third parameter is true, baseline will be recalculated in every event
        if (polarity == 0) {
            std::cerr << "ChannelReader::CalculateArea: Please set polarity use SetPolarity() first!" << std::endl;
            return 0;
        }
        if (baseline == 0 && !recal_baseline) {
            std::cout << "WARNING: ChannelReader::CalculateArea: Baseline wasn't calculated before calculating area. Set to 0" << std::endl;
        }
        if (T->size() == 0) {
            std::cerr << "ERROR: Please read an entry using tree->GetEntry() first before calculating area!" << std::endl;
            return 0;
        }
        if (start_pt < 0 || len <= 1 || start_pt+len > data_length){
            std::cerr << "ERROR: Invalid integral region: start_pt=" << start_pt << " len=" << len << "!" << std::endl;
            return 0;
        }

        if (recal_baseline){
            baseline = GetMostFreqVoltage();
        }

        // First order hold sampling reconstruction
        Double_t area = 0;
        for (int i=start_pt+1; i < len; i++){
            Double_t bin_width = T->at(i) - T->at(i-1);
            area += polarity * ((V->at(i-1) + V->at(i))/2 - baseline) * bin_width;
        }
        return area;
    }

    void SetPolarity(int p) {
        if (p != 1 && p != -1){
            std::cerr << "ChannelReader::SetPolarity: Polarity can only be 1 (positive) or -1 (negative)!" << std::endl;
            return;
        }
        else
            polarity = p;
    }
    int GetPolarity() {return polarity;}

    void PlotWave(TGraph *gg, Double_t offset = 0, int polari = 1){
        gg->Set(0);     // Clear TGraph
        for (int i=0; i<data_length; i++){
            gg->AddPoint(T->at(i), polari * (V->at(i))+offset);
        }
    }

    explicit ChannelReader(TString ch_name): ch_name(ch_name) {
        TString ch_T    = ch_name+"_T";
        TString ch_V    = ch_name+"_V";
        TString ch_maxv = ch_name+"_max_v";
        TString ch_maxt = ch_name+"_max_t";
        TString ch_minv = ch_name+"_min_v";
        TString ch_mint = ch_name+"_min_t";
        TString ch_mean = ch_name+"_mean";
        TString ch_RMS  = ch_name+"_RMS";
        if (tt->FindBranch(ch_V) == nullptr){    // This channel is closed or invalid
            is_open = false;
            T = nullptr;
            V = nullptr;
            max_v = 0;
            min_v = 0;
            max_t = 0;
            min_t = 0;
            mean  = 0;
            RMS   = 0;
            baseline = 0;
            data_length = 0;
            polarity = 0;
        } else{
            if (is_DRS4){
                T = new std::vector<Double_t>;
                tt->SetBranchAddress(ch_T, &T);
            }
            else {
                T = pico_T;
            }
            V = new std::vector<Double_t>;
            tt->SetBranchAddress(ch_V,    &V);
            tt->SetBranchAddress(ch_maxv, &max_v);
            tt->SetBranchAddress(ch_maxt, &max_t);
            tt->SetBranchAddress(ch_minv, &min_v);
            tt->SetBranchAddress(ch_mint, &min_t);
            tt->SetBranchAddress(ch_mean, &mean);
            tt->SetBranchAddress(ch_RMS,  &RMS);

            tt->GetEntry(0);
            data_length = T->size();
        }
    }

    ~ChannelReader();


private:
    static TTree* tt;
    TString ch_name;
    bool is_open = false;
    Double_t baseline = 0;
    int data_length = 0;
    int polarity = 0;

    Double_t GetMostFreqVoltage(){
        TH1D hist_v("hist_v", "hist_v", 200, mean-2*RMS, mean+2*RMS);
        if (data_length == 0) data_length = T->size();
        for (int j=0; j<data_length; j++){
            hist_v.Fill(V->at(j));
        }

        return hist_v.GetXaxis()->GetBinCenter(hist_v.GetMaximumBin());
    }
};

ChannelReader::~ChannelReader()=default;

std::vector<Double_t> *ChannelReader::pico_T = nullptr;
bool ChannelReader::is_DRS4 = false;
TTree *ChannelReader::tt = nullptr;

#endif //CSV2ROOT_CHANNELREADER_H
