#!/bin/bash                                                                                                                                                                       

thisDir=`pwd` 

cd /home/lindac/DUNE/LifetimeMeasurementTool/

source env.sh

./keepUpWithDecoding.sh

./keepUpWithFilteredAverages.sh 

./lastRunPurity.sh

#root -b -q macros/makePlotsVsTime.C'(1,30)'
#root -b -q macros/makePlotsVsTime.C'(2,30)'
#root -b -q macros/makePlotsVsTime.C'(1,50)'
#root -b -q macros/makePlotsVsTime.C'(2,50)'
#root -b -q macros/makePlotsVsTime.C'(1,60)'
#root -b -q macros/makePlotsVsTime.C'(2,70)'

df -h /data > /home/lindac/DUNE/LifetimeMeasurementTool/plots/Elog/spaceLeft.txt
python /home/lindac/DUNE/takeLifetimeData/printElog.py /data/PurityMonitor/Filling/ > /home/lindac/DUNE/LifetimeMeasurementTool/plots/Elog/Elog.txt

#root -b -q macros/makePrettyLifetimePlots.C

./makeElogEntry

echo "Let me do some cleaning up now ... "

./cleanUpRawAverages.sh
./cleanUpSomeBinaries.sh

cd $thisDir
