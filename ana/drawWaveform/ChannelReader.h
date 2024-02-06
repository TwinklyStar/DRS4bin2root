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
        }
    }

    ~ChannelReader();


private:
    static TTree* tt;
    TString ch_name;
    bool is_open;
};

ChannelReader::~ChannelReader()=default;

std::vector<Double_t> *ChannelReader::pico_T = nullptr;
bool ChannelReader::is_DRS4 = false;
TTree *ChannelReader::tt = nullptr;

#endif //CSV2ROOT_CHANNELREADER_H
