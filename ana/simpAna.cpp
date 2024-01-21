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

void simpAna(TString infname="../demo/7ch_example.root"){
    channel_info ChA1, ChB1, ChC1, ChA2, ChB2, ChC2, ChD2;  // Depend on the channel configuration in your file
    time_info EvtTime;

    TFile* ff = TFile::Open(infname, "READ");
    TTree* tt = (TTree*) ff->Get("T");

    tt->SetBranchAddress("EvtTime", &EvtTime);
    tt->SetBranchAddress("ChA1", &ChA1);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChB1", &ChB1);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChC1", &ChC1);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChA2", &ChA2);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChB2", &ChB2);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChC2", &ChC2);    // Depend on the channel configuration in your file
    tt->SetBranchAddress("ChD2", &ChD2);    // Depend on the channel configuration in your file

    Long64_t nentries = tt->GetEntries();

    // Event loop
    for (int ientry=0; ientry<nentries; ientry++){
        tt->GetEntry(ientry);

        // Fill your analysis code here
        if (ientry % 1000 == 0){
            std::cout << "\nEvent " << ientry << ":\nEvent Date: " << EvtTime.year << "/" << EvtTime.month << "/" << EvtTime.day << std::endl;
            std::cout << "Minimum Voltage in ChA1: " << ChA1.min_v << std::endl;
        }
    }

}