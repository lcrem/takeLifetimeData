double deltat = 2.;      // 2ns delta t
double ADCconv = 0.122;  // ADC conversion
int meanNum = 30;        // Use the first 30 samples to calculate the baseline


void quickDecodeEvent(string folder="/home/lindac/DUNE/DAQtests/TEMP/"){

  int ch;

  int MaxNChannels=8;

  uint32_t size;

  FILE *fp;

  char inname[180];

  // Unzip binary file
  char unzipfilecmd[1000];
  sprintf(unzipfilecmd, "gzip -d %s/Binary.bin.gz", folder.c_str());
  system(unzipfilecmd);
  sprintf(inname, "%s/Binary.bin", folder.c_str());
  fp = fopen (inname, "rb");

  ch = -1;

  int count = 0;

  TFile *fout = new TFile(Form("%s/RootfileFromBinary.root", folder.c_str()), "recreate");

  int num[8] = { 0, 0, 0, 0, 0 ,0 ,0 ,0};
  double *x = new double [2000000];
  double *y = new double [2000000];

  uint16_t DataChannel[2000000];
     
  while ( !feof(fp) ) {
     
    fread(&ch, sizeof(ch), 1, fp);
    fread(&size, sizeof(size), 1, fp);
     
    // cout << __LINE__ << endl;
    double mean=0;

    printf("Channel %i, size %i, number %i  \n", ch, size, num[ch]);
    fread(&DataChannel, sizeof(uint16_t)*size, 1, fp);
    cout << DataChannel[0] << " " << DataChannel[100] << " " << DataChannel[2000] << endl;
    // cout << __LINE__ << endl;

    for (int i=0; i<size; i++){
      x[i] = i*deltat;
      y[i] = DataChannel[i]*1.0;
      if (i<meanNum) mean += y[i];

      //      cout << x[i] << " " << y[i] << endl;
    }

    mean = mean*1.0/meanNum;
    for (int i=0; i<size; i++){
      y[i]-=mean;
      y[i]*=ADCconv;
    }
    //     for (int i=0; i<size; i++) printf("%i ", DataChannel[i] );
    //    printf("\n");
    
    num[ch]++;

    TGraph *g = new TGraph(size, x, y);     
    g->Write(Form("g_ch%i_%i", ch, num[ch]));

    delete g;


  }

  fout->Close();

  cout << "Wrote everything in " << folder << endl;
∑


}


