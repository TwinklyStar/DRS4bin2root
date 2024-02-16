# DRS4bin2root
This is a `C++` program that automatically convert binary dat file from DRS4 to CERN root file. This program is developed mainly based on previous conversion program by Prof. Kim Siang Khaw.  
Supported system: Linux & MacOS\
Author: Meng Lyu (Shanghai Jiao Tong University)\
Date: 2024/01/28

## Description
This program supports automatic channel number detection and up to 2 DRS4 boards in daisy-chain mode. Processing data from more
than 2 boards is not supported.  
The output file format is the same as the output from [picoCSV2root package](https://github.com/TwinklyStar/picoCSV2root/tree/main)
so that data from two digitizers can share the same analysis program (as provided in demo).

## Installation
Make sure you can use ROOT before installation

```
git clone https://github.com/TwinklyStar/DRS4bin2root.git
cd DRS4bin2root
make
```
If there is no error, an executable file `DRS4bin2root` should appear.
## Demo
### Convert dat to ROOT
```
./DRS4bin2root demo/7ch_example.dat
```

## Analysis
### Visualize first 10 waveforms (analysis example)
Go to `ana` directory
```
cd ana/drawWaveform
./run_drawWaveform.sh
```
Check `plots` folder, the first 10 waveforms of demo should be saved  
Please change the offset and channel number to fit with your data

### Simple analysis example
Go to `ana/waveAna` folder and find the `simpAna.cpp`. You can develop you own analysis code based on this example.  
To run demo:
```
root -l -q simpAna.C
```
To use it to your own data:
```
root -l -q 'simpAna.C("/path/to/your/data")'
```

### Peak search
Go to `ana/find-peaks` folder and find the `example.cpp`. You can develop you own analysis code based on this example.  
To run demo:
```
root -l -q example.C > test.log
```
In `test.log`, it will output the time of peaks in the first 10 waveforms.
To find positive peaks, change the last parameter in `PeakFinder::findPeaks` from -1 to 1.  

## ChannelReader Class
The demo analysis programs utilize the `ChannelReader` class to read data from pico and DRS4 and read in the channel data
as class members, which can largely simplify the analysis program. You can also include it for your own analysis program.  
When creating an instance of this class, you should give a channel name as input parameter:
```
ChannelReader myReader("ChA1");     // this reader will read data from ChA1
```
The constructor will automatically execute `SetBranchAddress` according to its channel name.  
For a closed channel or invalid channel name, you can still create a reader for it,
but `myReader.GetStatus()` will return false.

### Class Member Variables:
```
 std::vector<Double_t> *T;      // Pointer to the time vector
 std::vector<Double_t> *V;      // Pointer to the voltage vector
 Double_t max_v;
 Double_t max_t;
 Double_t min_v;
 Double_t min_t;
 Double_t mean;
 Double_t RMS;
 ```
 
### Class Member Functions:

**Get Channel Information**
```
 bool GetStatus()               // Returns true if channel is open
 TString GetName()              // Returns channel name, e.g., ChA1 
 int GetDataLength()            // Returns the data length
``` 
**Channel baseline** (where the 0V locates)  
For DRS4, the baseline is unstable and will fluctuate event by event (+/- 5 mV)  
For pico, this function can help find out the offset set in picoScope software
```
 // Calculate the baseline of this channel using the first test_num events (20 by default)
 Double_t CalculateBaseline(int test_num=20)
 
 // Get the calculated basedline
 Double_t GetBaseline()
 
 // Get the baseline of current entry (tt->GetEntry(i) should be excuted first)
 Double_t GetCurrentBaseline()
 ```
**Channel Polarity** (-1 for negative peak, 1 for positive peak)
```
 // Set the polarity of the pulse/peak 
 void SetPolarity(int p)
 
 // Get the polarity
 int GetPolarity()
```

**Search for index by time** (return -1 if out of boundary)
```
int GetIndexByTime(Double_t target_time)
```

**Calculate peak area**  
For a valid result, the baseline should be subtracted, and polarity should be correctly set first.  
It means that `CalculateBaseline()` and `SetPolarity()` must be executed before using this function.

For DRS4, the baseline is unstable and will fluctuate event by event (+/- 5 mV), therefore 
every event's baseline should be calculated separately by setting the third parameter to `true`  

To get the integral of the whole waveform: `myReader.CalculateArea(0, myReader.GetDataLength())`  

You can use `GetIndexByTime()` to get the integral within a certain time region. Just be careful with getting out of
time boundary (which returns -1)
`myReader.CalculateArea(myReader.GetIndexbyTime(t1), myReader.GetIndexbyTime(t2)-myReader.GetIndexbyTime(t1))`

```
// Calculate area in a given range
Double_t CalculateArea(int start_pt, int len, bool recal_baseline = false)
```
**Visualization**  
The final voltage will be: `polari * V + offset`
If you want to know what the waveform looks like in the `CalculateArea()` function,
```
 // Plot the waveform of current entry (tt->GetEntry(i) should be excuted first)
 void PlotWave(TGraph *gg, Double_t offset=0, int polari=1)
```

### File structure
The output root file structure in tree `wfm` is as follows:
<div align=center><img src="figure/roottree.png" height="400"></div>

Compared with picoCSV2root output, the data contain one additional branch `EvtTime`, and the rest are the same:

```
EvtNum: UInt_t  // Original event number of each waveform (not the event number in TTree)
                // Because during CSV conversion, some events will be discarded due to overflow
                
Data_Length: Int_t  // Number of data point (fixed 1024 for DRS4, flexibale for picoScope)

EvtTime:  // Capture time of each event (pico ver. does't have this branch)
    year: Short_t
    month: Short_t
    day: Short_t
    hour: Short_t
    min: Short_t
    sec: Short_t
    ms: Short_t

ChA(/B/C/D)1(/2)_T: *std::vector<Double_t>  // Time (ns for DRS4, ns-ms for picoScope)
ChA(/B/C/D)1(/2)_V: *std::vector<Double_t>  // Voltage (mV for DRS4, mV-V for picoScope)
// Due to flexible unit and offset in picoScope, again, please save a .pssetting file for each DAQ setting

ChA(/B/C/D)1(/2)_max_v: Double_t  // Maximum voltage
ChA(/B/C/D)1(/2)_max_t: Double_t  // Time where the maximum voltage locates
ChA(/B/C/D)1(/2)_min_v: Double_t  // You know what it means
ChA(/B/C/D)1(/2)_min_t: Double_t  // You know what it means
ChA(/B/C/D)1(/2)_mean:  Double_t  // mean of the voltage vector
ChA(/B/C/D)1(/2)_RMS:   Double_t  // RMS of the voltage vector
```

