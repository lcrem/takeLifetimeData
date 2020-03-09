#!/bin/bash

cd /home/lindac/DUNE/takeLifetimeData/

./bin/x86_64/Release/takeLifetimeData.bin -s 1 -n 100 -i exampleConfigBothPrMs_lowField.cfg -o /data/PurityMonitor/SoftTriggers/ -m "Taking software triggers." 
