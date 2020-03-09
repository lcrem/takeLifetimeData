#!/bin/bash

cd /home/lindac/DUNE/takeLifetimeData/

./bin/x86_64/Release/takeLifetimeData.bin -i exampleConfigBothPrMs_lowField.cfg -o /data/PurityMonitor/Filling/ -n 1000 -m "PrM 1 PrM 2 field 0.0.0Vcm, liquid, 1000 waveforms, 1 ms waveforms, both PrMs submerged."
