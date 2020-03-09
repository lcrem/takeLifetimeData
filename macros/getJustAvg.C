//Very rough macro to average wavefors from files that for some reason didn't produce a justAvg graph, by ABF. Based on Linda's awesome code.

#include <stdio.h>
#include <stdlib.h>

{

  TGraph *gt[5];
  TGraph gavg;

  f1 = new TFile("/data/PurityMonitor/GasTests/Run084/RawAverages_ch5.root","read");

  for (int ii=0; ii<5; ii++) gt[ii] = (TGraph*)f1->Get(Form("avg200/gavg200_%d", ii));
  
  gavg.Set((500000));
  for (int ip=0; ip<(500000); ip++){  
    gavg.GetX()[ip] = gt[0]->GetX()[ip];
    gavg.GetY()[ip] = 0.;
    for (int ii=0; ii<5; ii++) gavg.GetY()[ip] += gt[ii]->GetY()[ip] ;
    gavg.GetY()[ip] = gavg.GetY()[ip]/5.;
  }

  f2 = new TFile("/data/PurityMonitor/GasTests/Run084/justAvg_run084_ch5.root", "RECREATE");
  gavg.Write("justAvg");
  f2->Close();

 }
