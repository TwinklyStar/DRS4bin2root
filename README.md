# DRS4bin2root
This is a `C++` program that automatically convert binary dat file from DRS4 to CERN root file. This program is developed mainly based on previous conversion program by Prof. Kim Siang Khaw.  
Supported system: Linux & MacOS\
Author: Meng Lyu (Shanghai Jiao Tong University)\
Date: 2024/01/21

## Description
This program supports automatic channel number detection and up to 2 DRS4 boards in daisy-chain mode. Processing data from more
than 2 boards is not supported.

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
The output tree will contain:  
1. nevent: event number
2. EvtTime: a C++ struct storing the time when this event was acuqired
3. ChA1: a C++ struct storing the information of Channel A in the first DRS4 board

You can use `TBrowser` to check the structure and channel number of your file.

### Analysis example
```
cd ana
root simpAna.cpp
```

