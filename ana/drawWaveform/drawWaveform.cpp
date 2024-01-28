void drawWaveform(Int_t evt_num=0){
    Double_t offset = 0;  // The offset of 4 channels (set in picoscope software)
    TString fpath = "../../demo/7ch_example.root";

    TFile* ff = TFile::Open(fpath, "READ");
    TTree* tt = (TTree*) ff->Get("wfm");

	// if the macro has the function name (e.g. drawWaveform), the vector has to be registered in this way (new ...)
    std::vector<Double_t> *ChA1_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChB1_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChC1_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChD1_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChA2_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChB2_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChC2_T  = new std::vector<Double_t>;
    std::vector<Double_t> *ChD2_T  = new std::vector<Double_t>;

    std::vector<Double_t> *ChA1_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChB1_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChC1_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChD1_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChA2_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChB2_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChC2_V  = new std::vector<Double_t>;
    std::vector<Double_t> *ChD2_V  = new std::vector<Double_t>;

    tt->SetBranchAddress("ChA1_T", &ChA1_T);    // Comment unused channels
    tt->SetBranchAddress("ChB1_T", &ChB1_T);    // Comment unused channels
    tt->SetBranchAddress("ChC1_T", &ChC1_T);    // Comment unused channels
//    tt->SetBranchAddress("ChD1_T", &ChD1_T);    // Comment unused channels
    tt->SetBranchAddress("ChA2_T", &ChA2_T);    // Comment unused channels
    tt->SetBranchAddress("ChB2_T", &ChB2_T);    // Comment unused channels
    tt->SetBranchAddress("ChC2_T", &ChC2_T);    // Comment unused channels
    tt->SetBranchAddress("ChD2_T", &ChD2_T);    // Comment unused channels
    tt->SetBranchAddress("ChA1_V", &ChA1_V);    // Comment unused channels
    tt->SetBranchAddress("ChB1_V", &ChB1_V);    // Comment unused channels
    tt->SetBranchAddress("ChC1_V", &ChC1_V);    // Comment unused channels
//    tt->SetBranchAddress("ChD1_V", &ChD1_V);    // Comment unused channels
    tt->SetBranchAddress("ChA2_V", &ChA2_V);    // Comment unused channels
    tt->SetBranchAddress("ChB2_V", &ChB2_V);    // Comment unused channels
    tt->SetBranchAddress("ChC2_V", &ChC2_V);    // Comment unused channels
    tt->SetBranchAddress("ChD2_V", &ChD2_V);    // Comment unused channels

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

    for (int i=0; i<ChA1_T->size(); i++){
        g1->SetPoint(i, ChA1_T->at(i), ChA1_V->at(i) - offset + 300);
        g2->SetPoint(i, ChB1_T->at(i), ChB1_V->at(i) - offset + 200);
        g3->SetPoint(i, ChC1_T->at(i), ChC1_V->at(i) - offset + 100);
        g4->SetPoint(i, ChA2_T->at(i), ChA2_V->at(i) - offset);
        g5->SetPoint(i, ChB2_T->at(i), ChB2_V->at(i) - offset - 100);
        g6->SetPoint(i, ChC2_T->at(i), ChC2_V->at(i) - offset - 200);
        g7->SetPoint(i, ChD2_T->at(i), ChD2_V->at(i) - offset - 300);
    }

    mg->Add(g1); mg->Add(g2); mg->Add(g3); mg->Add(g4); mg->Add(g5); mg->Add(g6); mg->Add(g7);
    lgd->AddEntry(g1, "Channel A, Offset =  300", "l");
    lgd->AddEntry(g2, "Channel B, Offset =  200", "l");
    lgd->AddEntry(g3, "Channel C, Offset =  100", "l");
    lgd->AddEntry(g4, "Channel D, Offset =    0", "l");
    lgd->AddEntry(g5, "Channel D, Offset = -100", "l");
    lgd->AddEntry(g6, "Channel D, Offset = -200", "l");
    lgd->AddEntry(g7, "Channel D, Offset = -300", "l");

    mg->SetTitle(Form("4-Channel Waveforms, Event %d; Time [#mus]; Voltage [mV]", evt_num));
    mg->GetHistogram()->GetXaxis()->SetRangeUser(-100, 1900);
    mg->GetHistogram()->GetYaxis()->SetRangeUser(-550, 320);
    mg->Draw("A");

    lgd->Draw();
//	cc->Draw();
	cc->SaveAs(Form("plots/%05d.png", evt_num));
    delete cc;
}
