void quickPlot(string filename="outwaves/Waveform_0_0_1.txt"){

  ifstream infile( filename.c_str() );

  double x[10000], y[10000];
  int ic=0;
  double mean=0;

  while (!infile.eof()){
    infile >> y[ic];
    x[ic]=ic*2;
    if (ic<30) mean+=y[ic];
    ic++;   
  }
 

  mean /= 30;

  // zero baseline and scale adc
  for (int i=0; i<ic;i++){
    y[i]-=mean;
    y[i]*=0.122;
  }


  TGraph *g = new TGraph(ic-1, x, y);

  g->Draw("Al");


}
