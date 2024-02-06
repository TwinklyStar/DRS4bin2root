#include "ChannelReader.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TLegend.h"
#include "TH1F.h"

void drawWaveform(Int_t evt_num=0){
    Double_t offset = 0;  // The offset of 4 channels (set in picoscope software)
    TString fpath = "../../demo/7ch_example.root";

    TFile* ff = TFile::Open(fpath, "READ");
    TTree* tt = (TTree*) ff->Get("wfm");

    // Initialize ChannelReader Class AFTER reading the tree and BEFORE creating any instance
    ChannelReader::Initialize(tt);

    // Input parameter is the channel name that determines which channel the reader will read
    ChannelReader ChA1("ChA1"), ChB1("ChB1"), ChC1("ChC1"), ChD1("ChD1"),
                  ChA2("ChA2"), ChB2("ChB2"), ChC2("ChC2"), ChD2("ChD2");

    tt->GetEntry(evt_num);

    auto cc = new TCanvas("cc", "cc", 1600, 600);
    auto mg = new TMultiGraph;
    auto g1 = new TGraph; g1->SetLineColor(kBlue);      g1->SetLineWidth(3); g1->SetTitle("Channel A1");
    auto g2 = new TGraph; g2->SetLineColor(kRed);       g2->SetLineWidth(3); g2->SetTitle("Channel B1");
    auto g3 = new TGraph; g3->SetLineColor(kGreen);     g3->SetLineWidth(3); g3->SetTitle("Channel C1");
    auto g4 = new TGraph; g4->SetLineColor(kOrange);    g4->SetLineWidth(3); g4->SetTitle("Channel A2");
    auto g5 = new TGraph; g5->SetLineColor(kViolet);    g5->SetLineWidth(3); g5->SetTitle("Channel B2");
    auto g6 = new TGraph; g6->SetLineColor(kBlack);    g6->SetLineWidth(3); g6->SetTitle("Channel C2");
    auto g7 = new TGraph; g7->SetLineColor(kPink-9);    g7->SetLineWidth(3); g7->SetTitle("Channel D2");
    auto lgd = new TLegend(0.85, 0.70, 0.995, 0.99);

    for (int i=0; i<ChA1.T->size(); i++){
        g1->SetPoint(i, ChA1.T->at(i), ChA1.V->at(i) - offset + 300);
        g2->SetPoint(i, ChB1.T->at(i), ChB1.V->at(i) - offset + 200);
        g3->SetPoint(i, ChC1.T->at(i), ChC1.V->at(i) - offset + 100);
        g4->SetPoint(i, ChA2.T->at(i), ChA2.V->at(i) - offset);
        g5->SetPoint(i, ChB2.T->at(i), ChB2.V->at(i) - offset - 100);
        g6->SetPoint(i, ChC2.T->at(i), ChC2.V->at(i) - offset - 200);
        g7->SetPoint(i, ChD2.T->at(i), ChD2.V->at(i) - offset - 300);
    }

    mg->Add(g1); mg->Add(g2); mg->Add(g3); mg->Add(g4); mg->Add(g5); mg->Add(g6); mg->Add(g7);
    lgd->AddEntry(g1, "Channel A1, Offset =  300", "l");
    lgd->AddEntry(g2, "Channel B1, Offset =  200", "l");
    lgd->AddEntry(g3, "Channel C1, Offset =  100", "l");
    lgd->AddEntry(g4, "Channel A2, Offset =    0", "l");
    lgd->AddEntry(g5, "Channel B2, Offset = -100", "l");
    lgd->AddEntry(g6, "Channel C2, Offset = -200", "l");
    lgd->AddEntry(g7, "Channel D2, Offset = -300", "l");

    mg->SetTitle(Form("4-Channel Waveforms, Event %d; Time [#mus]; Voltage [mV]", evt_num));
    mg->GetHistogram()->GetXaxis()->SetRangeUser(-100, 1900);
    mg->GetHistogram()->GetYaxis()->SetRangeUser(-550, 320);
    mg->Draw("A");

    lgd->Draw();
//	cc->Draw();
	cc->SaveAs(Form("plots/%05d.png", evt_num));
    delete cc;
}
