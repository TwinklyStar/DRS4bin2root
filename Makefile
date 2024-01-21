######################################################################
# Project: TMVA - Toolkit for Multivariate Data Analysis             #
# Code   : Example usage for TMVA analyses                           #
###################################################################### 

CXX = g++
CCFLAGS = $(shell root-config --cflags) -ggdb

LIBS = $(shell root-config --libs) -lMLP -lMinuit -lTreePlayer -lTMVA -lTMVAGui -lXMLIO  -lMLP -lm

all : exp

exp : read_v5.3.cpp
	$(CXX) read_v5.3.cpp $(CCFLAGS) $(LIBS) -o DRS4bin2root

clean: 
	rm *.out
