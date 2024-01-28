#!/bin/bash

if [ ! -d "plots" ];then
	mkdir plots
fi

for i in {0..9}
do
	root -l -q 'drawWaveform.cpp('$i')'
done
