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

kinit -kt /home/lindac/.keytab lcremone@CERN.CH

echo "I have finished doing my things, now I will send stuff over to cern so that you can check that pretty webpage"

rsync -av /home/lindac/DUNE/LifetimeMeasurementTool/plots/Fi* /home/lindac/DUNE/LifetimeMeasurementTool/plots/Elog lcremone@lxplus.cern.ch:/eos/home-l/lcremone/www/ProtoDUNE/PurityMonitors/SummaryPlots/   2>&1 > /home/lindac/DUNE/rsyncSummaryPlots.log &
rsync -av  --include='*/' --include="*astRun" --include="*.png" --include="*.txt" --include='*.php'  --exclude='*' /data/PurityMonitor/* lcremone@lxplus.cern.ch:/eos/home-l/lcremone/www/ProtoDUNE/PurityMonitors/  2>&1 > /home/lindac/DUNE/rsyncLastRunPlots.log &

echo "Webpage is ready! While you check the pretty plots I will transfer the binaries and rootfiles to CERN"

rsync -av /data/PurityMonitor/* lcremone@lxplus.cern.ch:/eos/experiment/neutplatform/protodune/np02tier0/puritymon/  2>&1 > /home/lindac/DUNE/rsyncEverything.log &

echo "The process is running in the background, so you are good to go! Ciao ciao!"

cd $thisDir
