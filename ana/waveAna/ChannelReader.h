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

    static void Initialize(TTree* tin);

    TString GetName() {return ch_name;}

    bool GetStatus() {return is_open;}

    explicit ChannelReader(TString ch_name);

    ~ChannelReader();


private:
    static TTree* tt;
    TString ch_name;
    bool is_open;
};


#endif //CSV2ROOT_CHANNELREADER_H
