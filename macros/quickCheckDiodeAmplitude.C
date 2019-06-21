void quickCheckDiodeAmplitude(){

  string filename="/home/lindac/DUNE/DAQtests/Photodiode/SuperTest.root";

  TFile *fin = new TFile(filename.c_str(), "read");

  double min = 200.;

  int count=0;
  for (int i=0; i<100; i++){
    TGraph *g = (TGraph*)fin->Get(Form("g_ch0_%i",i+1));
    double max = TMath::MaxElement(g->GetN(), g->GetY());
    cout << "The max is " << max << endl;
    if (max>min) count++;
  }

  cout << count << endl;

}
