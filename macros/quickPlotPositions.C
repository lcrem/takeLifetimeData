void quickPlotPositions(string folder="/home/lindac/DUNE/DAQtests/NIMinput"){


  int npos=5;
  TGraph *graph[8];
  double *x = new double [1000000];
  double *y = new double [1000000];

  TLegend *leg = new TLegend(0.6, 0.4, 0.89, 0.89);

  for (int ig=0; ig<npos; ig++){

    ifstream infile( Form("%s_Position%i_Ch7/Waveform_0_7_1.txt", folder.c_str(), ig) );
    
    int ic=0;
    double mean=0;
    
    while (!infile.eof()){
      infile >> y[ic];
      x[ic]=ic*2;
      if (ic<30) mean+=y[ic];
      ic++;   
    }
    
    infile.close();

    mean /= 30;

    // zero baseline and scale adc
    for (int i=0; i<ic;i++){
      y[i]-=mean;
      y[i]*=0.122;
    }


    graph[ig] = new TGraph(ic-1, x, y);
    graph[ig]->SetLineColor(ig+1);
    leg->AddEntry(graph[ig], Form("Position %i", ig), "l");

  }


  TCanvas *c = new TCanvas("c");
  graph[0]->SetTitle("Photodiode signal;Time [ns];Amplitude [mV]");
  graph[0]->SetMinimum(0.1);
  graph[0]->GetXaxis()->SetRangeUser(80e3, 150e3);
  graph[0]->Draw("Al");

  for (int ig=1; ig<npos; ig++) graph[ig]->Draw("l");
  leg->Draw();

  c->Print("plots/PhotodiodeSignal_positions.png");

  // c->SetLogy();

  // c->Print("plots/PhotodiodeSignal_positions_logy.png");


}
