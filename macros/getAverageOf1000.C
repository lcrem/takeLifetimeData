{

int ig=0;

TGraph gavg;
TGraph *gt[5];

TCanvas c;

printf("Doing averages\n");

for (ch=0; ch<MaxNChannels; ch++){
  if (set.chEnabled[ch]==0) continue;
    
  favg[ch]->cd();

  // Doing 50 averages
  for (ig=0; ig<(MAX_WAVEFORMS/50); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg50[ch]->cd();
    gavg.Write(Form("gavg50_%d", ig));
      
  }

  // Doing 100 averages
  for (ig=0; ig<(MAX_WAVEFORMS/100); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg100[ch]->cd();
    gavg.Write(Form("gavg100_%d", ig));
      
    if (MAX_WAVEFORMS==100)  addToCanvas(ch, outdir, gavg, elog, c);
      
  }

  // Doing 200 averages
  for (ig=0; ig<(MAX_WAVEFORMS/200); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg200[ch]->cd();
    gavg.Write(Form("gavg200_%d", ig));
      
  }

  favg[ch]->cd();

  if (MAX_WAVEFORMS==1000){
    for (int ii=0; ii<5; ii++) gt[ii] = (TGraph*)favg[ch]->Get(Form("avg200/gavg200_%d", ii));
    for (ip=0; ip<set.length; ip++){
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.;
      for (int ii=0; ii<5; ii++) gavg.GetY()[ip] += gt[ii]->GetY()[ip] ;
      gavg.GetY()[ip] = gavg.GetY()[ip]/5.;
    }
    gavg.Write("justAvg");

    addToCanvas(ch, outdir, gavg, elog, c);
  }



    
  favg[ch]->Close(); 
  printf(".");
  delete favg[ch]; 
 } int ig=0;

TGraph gavg;
TGraph *gt[5];

TCanvas c;

printf("Doing averages\n");

for (ch=0; ch<MaxNChannels; ch++){
  if (set.chEnabled[ch]==0) continue;
    
  favg[ch]->cd();

  // Doing 50 averages
  for (ig=0; ig<(MAX_WAVEFORMS/50); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg50[ch]->cd();
    gavg.Write(Form("gavg50_%d", ig));
      
  }

  // Doing 100 averages
  for (ig=0; ig<(MAX_WAVEFORMS/100); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg100[ch]->cd();
    gavg.Write(Form("gavg100_%d", ig));
      
    if (MAX_WAVEFORMS==100)  addToCanvas(ch, outdir, gavg, elog, c);
      
  }

  // Doing 50 averages
  for (ig=0; ig<(MAX_WAVEFORMS/200); ig++){
    gt[0] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2));
    gt[1] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2+1));
    gavg.Set(set.length);
    for (ip=0; ip<set.length; ip++){ 
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]);
    }
    favg[ch]->cd();
    avg200[ch]->cd();
    gavg.Write(Form("gavg200_%d", ig));
      
  }

  favg[ch]->cd();

  if (MAX_WAVEFORMS==1000){
    for (int ii=0; ii<5; ii++) gt[ii] = (TGraph*)favg[ch]->Get(Form("avg200/gavg200_%d", ii));
    for (ip=0; ip<set.length; ip++){
      gavg.GetX()[ip] = gt[0]->GetX()[ip];
      gavg.GetY()[ip] = 0.;
      for (int ii=0; ii<5; ii++) gavg.GetY()[ip] += gt[ii]->GetY()[ip] ;
      gavg.GetY()[ip] = gavg.GetY()[ip]/5.;
    }
    gavg.Write("justAvg");

    addToCanvas(ch, outdir, gavg, elog, c);
  }



    
  favg[ch]->Close(); 
  printf(".");
  delete favg[ch]; 
 }
}
