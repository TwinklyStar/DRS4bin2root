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

void simpAna(TString infname="../demo/7ch_example.root"){
    channel_stat ChA1_stat, ChB1_stat, ChC1_stat, ChD1_stat, ChA2_stat, ChB2_stat, ChC2_stat, ChD2_stat;  // Depend on the channel configuration in your file
    time_info EvtTime;
    Int_t data_length;
    UInt_t evt_num;

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

    TFile* ff = TFile::Open(infname, "READ");
    TTree* tt = (TTree*) ff->Get("wfm");

    tt->SetBranchAddress("EvtNum", &evt_num);  // For pico output, ChA1_T - ChD1_T are identical
    tt->SetBranchAddress("Data_Length", &data_length);  // For pico output, ChA1_T - ChD1_T are identical
    tt->SetBranchAddress("EvtTime", &EvtTime);
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
    tt->SetBranchAddress("ChA1_stat", &ChA1_stat);    // Comment unused channels
    tt->SetBranchAddress("ChB1_stat", &ChB1_stat);    // Comment unused channels
    tt->SetBranchAddress("ChC1_stat", &ChC1_stat);    // Comment unused channels
//    tt->SetBranchAddress("ChD1_stat", &ChD1_stat);    // Comment unused channels
    tt->SetBranchAddress("ChA2_stat", &ChA2_stat);    // Comment unused channels
    tt->SetBranchAddress("ChB2_stat", &ChB2_stat);    // Comment unused channels
    tt->SetBranchAddress("ChC2_stat", &ChC2_stat);    // Comment unused channels
    tt->SetBranchAddress("ChD2_stat", &ChD2_stat);    // Comment unused channels

    Long64_t nentries = tt->GetEntries();

    // Event loop
    for (int ientry=0; ientry<nentries; ientry++){
        tt->GetEntry(ientry);

        // Fill your analysis code here
        if (ientry % 1000 == 0){
            std::cout << "\nEvent " << ientry << ":\nEvent Date: " << EvtTime.year << "/" << EvtTime.month << "/" << EvtTime.day << std::endl;
            std::cout << "Minimum Voltage in ChA1: " << ChA1_stat.min_v << "mV\t at " << ChA1_stat.min_t << "ns"
                      << "\nMaximum Voltage in ChA1: " << ChA1_stat.max_v << "mV\t at " << ChA1_stat.max_t << "ns"
                      << std::endl;     // The unit may change according to the settings in PicoScope software
            std::cout << "The 100th point in ChA1 is: " << ChA1_T->at(99) << "ns, " << ChA1_V->at(99) << "mV" << std::endl;
        }
    }

}