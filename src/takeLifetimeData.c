/******************************************************************************
 *
 * CAEN SpA - Front End Division
 * Via Vetraia, 11 - 55049 - Viareggio ITALY
 * +390594388398 - www.caen.it
 *
 *****************************************************************************
 * \note TERMS OF USE:
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the
 * software, documentation and results solely at his own risk.
 ******************************************************************************/

#include <CAENDigitizer.h>

#include <stdio.h>
#include <stdlib.h>

//#define MANUAL_BUFFER_SETTING   0
// The following define must be set to the actual number of connected boards
#define MAXNB   1
// NB: the following define MUST specify the ACTUAL max allowed number of board's channels
// it is needed for consistency inside the CAENDigitizer's functions used to allocate the memory
#define MaxNChannels 8

// The following define MUST specify the number of bits used for the energy calculation
#define MAXNBITS 16

/* include some useful functions from file Functions.h
   you can find this file in the src directory */
#include "Functions.h"

#include "signal.h"


#include "TFile.h" 
#include "TGraph.h" 
#include "TAxis.h" 
#include "TCanvas.h"
#include "TPaveText.h"
#include "TMath.h"
#include "TSystem.h"
#include "FFTtools.h"


/* ###########################################################################
 *  Functions
 *  ########################################################################### */

bool ABORT_EARLY=false;

double digDeltaT = 2.;      // 2ns delta t 
double digADCconv = 0.122;  // ADC conversion

void addToCanvas(int ch, char *outdir, TGraph gavg, char *elog, TCanvas &c){

  TGraph *g1 = new TGraph(gavg.GetN(), gavg.GetX(), gavg.GetY());
  g1->SetName(Form("g_%d", ch));

  TPaveText *pav = new TPaveText(0.25, 0.85, 0.99, 0.95, "ndc");
  pav->SetFillColor(kWhite);
  pav->AddText(elog);

  float min = 0;

  switch(ch){
  case 0:
    g1->SetTitle("Photodiode;Time [ns];Amplitude [mV]");
    g1->Draw("Al");
    pav->Draw("same");
    c.Print(Form("%s/Photodiode.png", outdir));
    break;
  case 1:
    g1->SetTitle("PrM1 Middle, Raw Averages;Time [ns];Amplitude [mV]");
    min = TMath::MinElement(g1->GetN(), g1->GetY());
    g1->GetYaxis()->SetRangeUser(min, -min);
    g1->Draw("Al");
    break;
  case 2:
    g1->Draw("l");
    g1->SetLineColor(kRed);
    pav->Draw("same");
    c.Print(Form("%s/PurityMonitor1_rawAvg.png", outdir));
    break;
  case 4:
    g1->SetTitle("PrM2 Bottom, Raw Averages;Time [ns];Amplitude [mV]");
    min = TMath::MinElement(g1->GetN(), g1->GetY());
    g1->GetYaxis()->SetRangeUser(min, -min);
    g1->Draw("Al");
    break;
  case 5:
    g1->Draw("l");
    g1->SetLineColor(kRed);
    pav->Draw("same");
    c.Print(Form("%s/PurityMonitor2_rawAvg.png", outdir));
    break;
    
  }
  c.Draw();
  c.Update();
  gSystem->ProcessEvents();

}


void interrupt_signal_handler(int sig){

  signal(sig, SIG_IGN);
  ABORT_EARLY=true;
  return;

}



/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      int ProgramDigitizer(int handle, DigitizerParams_t Params, DAQSettings_t set)
 *   \brief   Program the registers of the digitizer with the relevant parameters
 *   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int ProgramDigitizer(int handle, DigitizerParams_t Params, DAQSettings_t set)
{
  /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
  int i, ret = 0;

  /* Reset the digitizer */
  ret |= CAEN_DGTZ_Reset(handle);

  if (ret) {
    printf("ERROR: can't reset the digitizer.\n");
    return -1;
  }

  /* Set the DPP acquisition mode
     This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
     CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly        Only charge (DPP-PSD/DPP-CI v2) is returned
     CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
     CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime    Both charge and time are returned
     CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned */
  //ret |= CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
  // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);

  // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
  ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOlev);

  /* Set the digitizer's behaviour when an external trigger arrives:

     CAEN_DGTZ_TRGMODE_DISABLED: do nothing
     CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
     CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
     CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger

     see CAENDigitizer user manual, chapter "Trigger configuration" for details */
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

  // Set the enabled channels
  ret |= CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask);

  // Disable self trigger on all channels
  ret |= CAEN_DGTZ_SetChannelSelfTrigger(handle,CAEN_DGTZ_TRGMODE_DISABLED,0xFF); 

    
  /* Set the mode used to syncronize the acquisition between different boards.
     In this example the sync is disabled */
  ret |= CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);
 
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle,100);  /* Set the max number of events to transfer in a sigle readout */

  // Set the DPP specific parameters for the channels in the given channelMask
  //  ret |= CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams);
    
  for(i=0; i<MaxNChannels; i++) {
    if (Params.ChannelMask & (1<<i)) {

      printf ("Channel %i is enabled \n", i);
      // Set the number of samples for each waveform 
      //for x725 and x730 Recordlength is common to paired channels (you can set different RL for different channel pairs)
      ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength, i);

      // Set a DC offset to the input signal to adapt it to digitizer's dynamic range
      //      ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x8000);
      ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x7FFF);
           
      
      ret |= CAEN_DGTZ_SetChannelTriggerThreshold(handle,i,set.thr);                  /* Set selfTrigger threshold */
	   
      // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
      ret |= CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity);
    }
  }

  // Set post trigger size 
  ret |= CAEN_DGTZ_SetPostTriggerSize(handle, set.postt);


  if (ret) {
    printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
    return ret;
  } else {
    return 0;
  }
}

/* ########################################################################### */
/* MAIN                                                                        */
/* ########################################################################### */
int main(int argc, char *argv[])
{

  int MAX_WAVEFORMS=10;
  char outdir[1000];
  char outname[180]; 
  char configFile[1000];
  char elog[10000];

  // Default configuration file
  sprintf(configFile, "exampleConfigBothPrMs_highField.cfg");
  sprintf(outdir, "tmpout");
  if((argc==1)){ 
    printf("Usage : %s -i (configFile) -o (outdir) -n (MAX_WAVEFORMS) -m (ELOGMESSAGE)\n", argv[0]); 
    printf("You need to set at least the number of waveforms\n");
    printf("If you want software triggers use the flag -s 1\n");
    printf("If you want to perform temperature calibration use flag -c 1 \n"); 
    printf("If you want to save rootfiles directly use the flag -r 1 \n\n");
    return -1;
  }
  
  char clswitch; // command line switch
  int tmpmax=0;
  char tmpout[200];
  tmpout[0] = 0;

  bool softTrigger = false;
  bool doCalibration = false;
  bool saveRoot = false;

  if (argc>1) {
    while ((clswitch = getopt(argc, argv, "i:o:n:m:s:c:r:")) != EOF) {
      switch(clswitch) {
      case 'n':
        tmpmax=atoi(optarg);
	printf("Number of waveforms to take: %i\n", tmpmax);
        break;
      case 'i':
        sprintf(configFile, "%s", optarg);
	printf("Config file: %s\n", configFile);
        break;
      case 'o':
        sprintf(tmpout, "%s", optarg);
	printf("Output dir: %s\n", tmpout);
        break;
      case 'm':
	sprintf(elog, "%s", optarg);
	printf("Elog message: %s\n", elog);
	break;
      case 's':
	printf("Using software triggers!!!!\n");
	softTrigger=true;
	break;
      case 'c':
	printf("Performing calibration\n");
	doCalibration=true;
	break;
      case 'r':
	printf("Saving rootfiles\n");
	saveRoot=true;
	break;
      } // end switch
    } // end while
  } // end if arg>1
 

 // Function that reads the settings 
  DAQSettings_t set = readDAQSettings(configFile);
  printDAQSettings(set);
  MAX_WAVEFORMS=set.nmax;
  sprintf(outdir, "%s", set.outfolder);

  printf ("OUT DIR %s \n", outdir);

  if (tmpmax!=0) MAX_WAVEFORMS=tmpmax;
  if (tmpout[0] != 0)    sprintf(outdir, "%s", tmpout);

  if (doCalibration){
    sprintf(outdir, "%s/Calibration/", outdir);
  }

  // Get last run number
  int run=0;
  FILE *frun;
  if (( frun = fopen(Form("%s/LastRun", outdir), "r")) ){
    fscanf(frun,"%d", &run);
    printf("The last run number was %d\n", run);
    fclose(frun);
  }
  run=run+1;

  if (doCalibration) run=run-1;

  FILE *frunnew;
  frunnew = fopen(Form("%s/LastRun", outdir), "w");
  fprintf(frunnew, "%d", run);
  fclose(frunnew);

  sprintf(outdir, "%s/Run%03d", outdir, run);
    
  // Make sure output directory exhists
  char makedircmd[1000]; 
  sprintf(makedircmd, "mkdir -p %s", outdir); 
  system(makedircmd); 

  /* The following variable is the type returned from most of CAENDigitizer
     library functions and is used to check if there was an error in function
     execution. For example:
     ret = CAEN_DGTZ_some_function(some_args);
     if(ret) printf("Some error"); */
  CAEN_DGTZ_ErrorCode ret;

  /* Buffers to store the data. The memory must be allocated using the appropriate
     CAENDigitizer API functions (see below), so they must not be initialized here */
  char *buffer = NULL;                                    // readout buffer
  /* CAEN_DGTZ_DPP_PSD_Event_t       *Events[MaxNChannels];  // events buffer */
  /* CAEN_DGTZ_DPP_PSD_Waveforms_t   *Waveform=NULL;         // waveforms buffer */
  
  /* The following variables will store the digitizer configuration parameters */
  /* CAEN_DGTZ_DPP_PSD_Params_t DPPParams[MAXNB]; */
  DigitizerParams_t Params[MAXNB];
  
  /* Arrays for data analysis */
  uint64_t PrevTime[MAXNB][MaxNChannels];
  uint64_t ExtendedTT[MAXNB][MaxNChannels];
  uint32_t *EHistoShort[MAXNB][MaxNChannels];   // Energy Histograms for short gate charge integration
  uint32_t *EHistoLong[MAXNB][MaxNChannels];    // Energy Histograms for long gate charge integration
  float *EHistoRatio[MAXNB][MaxNChannels];      // Energy Histograms for ratio Long/Short
  int ECnt[MAXNB][MaxNChannels];                // Number-of-Entries Counter for Energy Histograms short and long gate
  int TrgCnt[MAXNB][MaxNChannels];

  /* The following variable will be used to get an handler for the digitizer. The
     handler will be used for most of CAENDigitizer functions to identify the board */
  int handle[MAXNB];

  /* Other variables */
  int i, b, ev;
  int ch=0;
  int Quit=0;
  int AcqRun = 0;
  uint32_t AllocatedSize, BufferSize;
  uint32_t numEvents;

  int Nb=0;
  int DoSaveWave[MAXNB][MaxNChannels];
  int MajorNumber;
  int BitMask = 0;
  uint64_t CurrentTime, PrevRateTime, ElapsedTime;
  uint32_t NumEvents[MaxNChannels];
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CAEN_DGTZ_EventInfo_t eventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;

  float *MeanDataChannel = new float [7500000]; 

  char * evtptr = NULL;
  int count[MAXNB];

  uint32_t temp;
  int countBf0=0;

  memset(DoSaveWave, 0, MAXNB*MaxNChannels*sizeof(int));
  for (i=0; i<MAXNBITS; i++)
    BitMask |= 1<<i; /* Create a bit mask based on number of bits of the board */

  /* *************************************************************************************** */
  /* Set Parameters                                                                          */
  /* *************************************************************************************** */
  memset(&Params, 0, MAXNB*sizeof(DigitizerParams_t));
  //  memset(&DPPParams, 0, MAXNB*sizeof(CAEN_DGTZ_DPP_PSD_Params_t));
  for(b=0; b<MAXNB; b++) {
    for(ch=0; ch<MaxNChannels; ch++) {
      EHistoShort[b][ch] = NULL; // Set all histograms pointers to NULL (we will allocate them later)
      EHistoLong[b][ch] = NULL;
      EHistoRatio[b][ch] = NULL;
    }

    /****************************		\
     * Communication Parameters   *
        \****************************/
    // Direct USB connection
    Params[b].LinkType = CAEN_DGTZ_USB;  // Link Type
    Params[b].VMEBaseAddress = 0;  // For direct USB connection, VMEBaseAddress must be 0

    // Direct optical connection
    //Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink;  // Link Type
    //Params[b].VMEBaseAddress = 0;  // For direct CONET connection, VMEBaseAddress must be 0

    // Optical connection to A2818 (or A3818) and access to the board with VME bus
    //Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink;  // Link Type (CAEN_DGTZ_PCIE_OpticalLink for A3818)
    //Params[b].VMEBaseAddress = 0x32100000;  // VME Base Address (only for VME bus access; must be 0 for direct connection (CONET or USB)
        
    // USB connection to V1718 bridge and access to the board with VME bus
    //Params[b].LinkType = CAEN_DGTZ_USB;  // Link Type (CAEN_DGTZ_PCIE_OpticalLink for A3818)
    //Params[b].VMEBaseAddress = 0x32110000;  // VME Base Address (only for VME bus access; must be 0 for direct connection (CONET or USB)

    uint32_t mask = 0;
    for (ch=0; ch<MaxNChannels; ch++){
      if (set.chEnabled[ch]==1){
	mask |= 1<<ch ;
      }
    }
    
        
    Params[b].IOlev = CAEN_DGTZ_IOLevel_NIM; //CAEN_DGTZ_IOLevel_TTL;
    /****************************\
     *  Acquisition parameters    *
        \****************************/
    //Params[b].AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;          // or CAEN_DGTZ_DPP_ACQ_MODE_List 
    Params[b].RecordLength = set.length;                       // Num of samples of the waveforms (only for Oscilloscope mode) 
    Params[b].ChannelMask = mask; //0xFF;                              // Channel enable mask  
    Params[b].EventAggr = 1;                                   // Number of events in one aggregate (0=automatic) 17554
    Params[b].PulsePolarity = CAEN_DGTZ_PulsePolarityPositive; // Pulse Polarity (this parameter can be individual)

  } 


  signal(SIGINT, interrupt_signal_handler);


  /* *************************************************************************************** */
  /* Open the digitizer and read board information                                           */
  /* *************************************************************************************** */
  /* The following function is used to open the digitizer with the given connection parameters
     and get the handler to it */
  for(b=0; b<MAXNB; b++) {
    /* IMPORTANT: The following function identifies the different boards with a system which may change
       for different connection methods (USB, Conet, ecc). Refer to CAENDigitizer user manual for more info.
       Some examples below */
        
    /* The following is for b boards connected via b USB direct links
       in this case you must set Params[b].LinkType = CAEN_DGTZ_USB and Params[b].VMEBaseAddress = 0 */
    ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, b, 0, Params[b].VMEBaseAddress, &handle[b]);


    /* The following is for b boards connected via 1 opticalLink in dasy chain
       in this case you must set Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink and Params[b].VMEBaseAddress = 0 */
    //ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, 0, b, Params[b].VMEBaseAddress, &handle[b]);

    /* The following is for b boards connected to A2818 (or A3818) via opticalLink (or USB with A1718)
       in this case the boards are accessed throught VME bus, and you must specify the VME address of each board:
       Params[b].LinkType = CAEN_DGTZ_PCI_OpticalLink (CAEN_DGTZ_PCIE_OpticalLink for A3818 or CAEN_DGTZ_USB for A1718)
       Params[0].VMEBaseAddress = <0xXXXXXXXX> (address of first board) 
       Params[1].VMEBaseAddress = <0xYYYYYYYY> (address of second board) 
       etc */
    //ret = CAEN_DGTZ_OpenDigitizer(Params[b].LinkType, 0, 0, Params[b].VMEBaseAddress, &handle[b]);
    if (ret) {
      printf("Can't open digitizer\n");
      goto QuitProgram;    
    }

    /* Once we have the handler to the digitizer, we use it to call the other functions */
    ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
    if (ret) {
      printf("Can't read board info\n");
      goto QuitProgram;
    }
    printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, b);
    printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
    printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

    // Check firmware revision (only DPP firmware can be used with this Demo) */
    sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
    if (MajorNumber >= 128) {
      printf("This digitizer has a DPP firmware and is incompatible with me!\n");
      goto QuitProgram;
    }

  }


  /* *************************************************************************************** */
  /* Program the digitizer (see function ProgramDigitizer)                                   */
  /* *************************************************************************************** */
  for(b=0; b<MAXNB; b++) {
    ret = ProgramDigitizer(handle[b], Params[b], set);
    if (ret) {
      printf("Failed to program the digitizer\n");
      goto QuitProgram;
    }
  }


  /* WARNING: The mallocs MUST be done after the digitizer programming,
     because the following functions needs to know the digitizer configuration
     to allocate the right memory amount */
  /* Allocate memory for the readout buffer */
  ret = CAEN_DGTZ_MallocReadoutBuffer(handle[0], &buffer, &AllocatedSize);
  /* Allocate memory for the events */
  //  ret |= CAEN_DGTZ_MallocDPPEvents(handle[0], Events, &AllocatedSize); 
  /* Allocate memory for the waveforms */
  // ret |= CAEN_DGTZ_MallocDPPWaveforms(handle[0], &Waveform, &AllocatedSize);

  //  printf("Allocated size %i and size of it %i\n", AllocatedSize, sizeof(AllocatedSize));

 
  if (ret) {
    printf("Can't allocate memory buffers\n");
    goto QuitProgram;    
  }
        
  /* *************************************************************************************** */
  /* Readout Loop                                                                            */
  /* *************************************************************************************** */
  // Clear Histograms and counters

  for(b=0; b<MAXNB; b++) {
    for(ch=0; ch<MaxNChannels; ch++) {
      // Allocate Memory for Histos and set them to 0
      TrgCnt[b][ch] = 0;
      ECnt[b][ch] = 0;
      PrevTime[b][ch] = 0;
      ExtendedTT[b][ch] = 0;
    }
  }



  PrevRateTime = get_time();
  AcqRun = 0;

  for(b=0; b<MAXNB; b++) { 

    CAEN_DGTZ_ClearData(handle[b]); 

    // Start Acquisition 
    // NB: the acquisition for each board starts when the following line is executed 
    // so in general the acquisition does NOT starts syncronously for different boards 
    CAEN_DGTZ_SWStartAcquisition(handle[b]); 
    printf("Acquisition Started for Board %d\n", b); 

    /* CAEN_DGTZ_DRS4Frequency_t frequency; */
    /* ret = CAEN_DGTZ_GetDRS4SamplingFrequency(handle[b], &frequency); */
    /* if (ret) { */
    /*   printf("Can't read sampling frequency\n"); */
    /*   goto QuitProgram; */
    /* } */
    
    /* printf("Sampling frequency is set to %5.3f\n", frequency); */





  } 

  {

  AcqRun = 1;

  sleep(1);

  // If MAX_WAVEFORMS==-1 perform calibration
  // Calibration should be every time the digitiser is turned on
  // We should wait a few seconds for the temperatures to stabilise
  if (doCalibration){
    
    printf("Waiting 10 seconds for temperature to stabilise\n");
    sleep(10);
    for (b = 0; b < MAXNB; b++) { 
      printf("\n"); 
      for (ch = 0; ch < MaxNChannels; ch++) { 
	// Read ADC temperature 
	CAEN_DGTZ_ReadTemperature(handle[b], ch, &temp); 
	printf("Ch %d  ADC temperature: %d %cC\n", ch, temp, 248); 
      } 
    } 
    
    for (b = 0; b < MAXNB; b++) 
      CAEN_DGTZ_Calibrate(handle[b]); 
    printf("\nADC calibration done\n");     goto QuitProgram;    
  }

  int digMeanNum = 100;

 
  FILE *felog;
  sprintf(outname, "%s/Elog.txt", outdir);
  felog = fopen(outname, "w");
  fprintf(felog, "%s\n", elog);
  fclose(felog);

  FILE *ftime;
  sprintf(outname, "%s/Timestamp.txt", outdir);
  ftime = fopen(outname, "w");
  std::time_t timestamp = std::time(0); 
  fprintf(ftime, "%d", timestamp);
  fclose(ftime);


  FILE *fp; 
  sprintf(outname, "%s/Binary.bin", outdir); 
  fp = fopen (outname, "wb"); 
   
  char rootout[200];
  TFile *fout;
  if (saveRoot){
    sprintf(rootout, "%s/RawWaveforms.root", outdir);
    fout = new TFile(rootout, "recreate");
    fout->SetCompressionLevel(0);
  }
  
  TFile *favg[MaxNChannels];
  /* TDirectory *avg25[MaxNChannels]; */
  /* TDirectory *avg50[MaxNChannels]; */
  TDirectory *avg100[MaxNChannels];
  /* TDirectory *avg200[MaxNChannels]; */
  for (ch=0; ch<MaxNChannels; ch++){
    if (set.chEnabled[ch]==0) continue;
    sprintf(rootout, "%s/RawAverages_ch%d.root", outdir, ch);
    favg[ch]   = new TFile(rootout, "recreate");
    /* avg25[ch]  = favg[ch]->mkdir("avg25"); */
    /* avg50[ch]  = favg[ch]->mkdir("avg50"); */
    avg100[ch] = favg[ch]->mkdir("avg100");
    /* avg200[ch] = favg[ch]->mkdir("avg200"); */
  }
  


  TGraph g;

  char goutname[100];
  int ip = 0;
  double mean = 0.;
  /* Read data from the boards */ 

  int numToAvg=100;
  int tmpch=0;

  for(b=0; b<MAXNB; b++) { 
    count[b]=0;

    memset(MeanDataChannel, 0, sizeof(MeanDataChannel)*sizeof(float));

    while(count[b] < MAX_WAVEFORMS){


      if (ABORT_EARLY){
	printf("Gracefully aborting early\n");
	goto QuitProgram;
      }

      if (softTrigger){

	//	sleep(1);
	  
	printf("Sending 100 software triggers now?\n");  
	// just test  
	for (int it=0; it<100; it++) CAEN_DGTZ_SendSWtrigger(handle[b]); // Send a software trigger to each board   

      }

	
      /* Read data from the board */ 
      ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize); 
      if (ret) { 
	printf("Readout Error\n"); 
	goto QuitProgram;     
      } 
      if (BufferSize == 0){
 	countBf0 = countBf0 + 1;
	printf("x");
	/* if (countBf0>10000) goto QuitProgram; */
	continue; 
      }
      Nb += BufferSize; 
      //      printf ("Got something \n");

      /* The buffer red from the digitizer is used in the other functions to get the event data
	 The following function returns the number of events in the buffer */
      ret |= CAEN_DGTZ_GetNumEvents(handle[b],buffer,BufferSize,&numEvents);
      
      //      printf(".");
      //      printf(" %d out of %d and num events is %d \n", count[b], MAX_WAVEFORMS, numEvents);
      

      for (i=0;i<numEvents;i++) {
	printf(".");
	
	/* Get the Infos and pointer to the event */
	ret = CAEN_DGTZ_GetEventInfo(handle[b],buffer,BufferSize,i,&eventInfo,&evtptr);
	
	/* Decode the event to get the data */
	ret = CAEN_DGTZ_DecodeEvent(handle[b],evtptr,&Evt);
	
	//*************************************
	// Event Elaboration
	//*************************************

	//	fwrite(Evt.Chsize, sizeof(Evt.ChSize), 1, fp);

	tmpch=0;
	for(ch=0; ch<MaxNChannels; ++ch) {  
	  //	  printf("Channel %i with size %i \n", ch, Evt->ChSize[ch]);
	   
	  if (Evt->ChSize[ch]>0){

	    TrgCnt[b][ch]++;  

	    if (TrgCnt[b][ch]<10) {
	      fwrite(&ch, sizeof(int), 1, fp);
	      fwrite(&Evt->ChSize[ch], sizeof(uint32_t), 1, fp);	      
	      //    printf("Just in case %i %i \n", ch, Evt->ChSize[ch]);
	      fwrite(Evt->DataChannel[ch], Evt->ChSize[ch]*sizeof(uint16_t), 1, fp);
	    }

	    //	    SaveFunWaveform(b, ch, count[b], Evt->ChSize[ch], Evt->DataChannel[ch], outdir); 
	    
	    for (ip=0; ip<Evt->ChSize[ch]; ip++){
	      MeanDataChannel[tmpch*set.length+ip] += Evt->DataChannel[ch][ip]*digADCconv/(numToAvg*1.0);
	    }

	    if (saveRoot) {
	      mean=0.;
	      g.Set(Evt->ChSize[ch]);
	      for (ip=0; ip<Evt->ChSize[ch]; ip++){
		g.GetX()[ip] = ip*digDeltaT;
		g.GetY()[ip] = Evt->DataChannel[ch][ip]*digADCconv;
		if (ip<digMeanNum) mean += Evt->DataChannel[ch][ip]*digADCconv;
		//y[i] = Evt->DataChannel[ch][i];
		//      cout << x[i] << " " << y[i] << endl;                                                                                            
	      }

	      mean = mean*1.0/digMeanNum;
	      for (ip=0; ip<Evt->ChSize[ch]; ip++){
		g.GetY()[ip] = g.GetY()[ip] - mean;
	      }

	      fout->cd();
	      sprintf(goutname, "g_ch%d_%d", ch, TrgCnt[b][ch]); 
	      g.Write(goutname); 
	    }

	    //	     for(ev=0; ev<Evt->ChSize[ch]; ev++){
	    //	       printf("%i %i %i %i \n", ch, ev, Evt->ChSize[ch], Evt->DataChannel[ch][ev]);
	    // }
	    //	    delete g;
	    tmpch++;
	  }

	}

	ret = CAEN_DGTZ_FreeEvent(handle[b],&Evt);

      
	if (TrgCnt[b][0]%numToAvg==0){
	 
	  tmpch=0;
	  for (ch=0; ch<MaxNChannels; ch++){
	    
	    if (set.chEnabled[ch]==0) continue;
	    
	    favg[ch]->cd();
	    avg100[ch]->cd();
	    g.Set(set.length);

	    mean=0.;

	    for (ip=0; ip<set.length; ip++){
	      g.GetX()[ip] = ip*digDeltaT;
	      g.GetY()[ip] = MeanDataChannel[tmpch*set.length+ip];
	      if (ip<digMeanNum) mean += g.GetY()[ip];
	    }

	    mean = mean*1.0/digMeanNum;

	    for (ip=0; ip<set.length; ip++){
	      g.GetY()[ip]-=mean;
	    }
	    
	    sprintf(goutname, "gavg100_%d", TrgCnt[b][ch]/numToAvg-1);  
	    g.Write(goutname);
	    
	    for (ip=0; ip<set.length; ip++){
              MeanDataChannel[tmpch*set.length+ip]=0.;
	    }
	   
	    tmpch++;
	  }
	 
	  //	  memset(MeanDataChannel, 0., sizeof(MeanDataChannel)*sizeof(float)); 
	  printf("Done avg %d / %d \n", TrgCnt[b][0], MAX_WAVEFORMS);
	}

      }
	count[b] +=numEvents;

    }


  }// loop on boards 
 
  
 

   /* Calculate throughput and trigger rate (every second) */ 
  CurrentTime = get_time(); 
  ElapsedTime = CurrentTime - PrevRateTime; /* milliseconds */ 
  if (ElapsedTime > 1000) { 
    /* system(CLEARSCR);  */
    /* PrintInterface();  */
    printf("Readout Rate=%.2f MB\n", (float)Nb/((float)ElapsedTime*1048.576f)); 
    for(b=0; b<MAXNB; b++) { 
      printf("\nBoard %d:\n",b); 
      for(i=0; i<MaxNChannels; i++) { 
	if (TrgCnt[b][i]>0) 
	  printf("\tCh %d:\tTrgRate=%.2f Hz\t%\n", b*8+i, (float)TrgCnt[b][i]*1000./(float)ElapsedTime); 
	else 
	  printf("\tCh %d:\tNo Data\n", i); 
	TrgCnt[b][i]=0; 
      } 
    } 
    Nb = 0; 
    PrevRateTime = CurrentTime; 
    printf("\n\n"); 
  }
  
  


  if (saveRoot){
    fout->Close();
    delete fout;
  }

  fclose(fp);
  

  int ig=0;

  TGraph gavg;
  TGraph *gt[10];

  TCanvas c;

  printf("Doing averages\n");

  for (ch=0; ch<MaxNChannels; ch++){
    if (set.chEnabled[ch]==0) continue;
    
    favg[ch]->cd();

    /* // Doing 50 averages */
    /* for (ig=0; ig<(MAX_WAVEFORMS/50); ig++){ */
    /*   gt[0] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2)); */
    /*   gt[1] = (TGraph*)favg[ch]->Get(Form("avg25/gavg25_%d", ig*2+1)); */
    /*   gavg.Set(set.length); */
    /*   for (ip=0; ip<set.length; ip++){  */
    /* 	gavg.GetX()[ip] = gt[0]->GetX()[ip]; */
    /* 	gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]); */
    /*   } */
    /*   favg[ch]->cd(); */
    /*   avg50[ch]->cd(); */
    /*   gavg.Write(Form("gavg50_%d", ig)); */
      
    /* } */

    /* // Doing 100 averages */
    /* for (ig=0; ig<(MAX_WAVEFORMS/100); ig++){ */
    /*   gt[0] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2)); */
    /*   gt[1] = (TGraph*)favg[ch]->Get(Form("avg50/gavg50_%d", ig*2+1)); */
    /*   gavg.Set(set.length); */
    /*   for (ip=0; ip<set.length; ip++){  */
    /* 	gavg.GetX()[ip] = gt[0]->GetX()[ip]; */
    /* 	gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]); */
    /*   } */
    /*   favg[ch]->cd(); */
    /*   avg100[ch]->cd(); */
    /*   gavg.Write(Form("gavg100_%d", ig)); */
      
    /*   if (MAX_WAVEFORMS==100)  addToCanvas(ch, outdir, gavg, elog, c); */
      
    /* } */

    /* // Doing 200 averages */
    /* for (ig=0; ig<(MAX_WAVEFORMS/200); ig++){ */
    /*   gt[0] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2)); */
    /*   gt[1] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ig*2+1)); */
    /*   gavg.Set(set.length); */
    /*   for (ip=0; ip<set.length; ip++){  */
    /* 	gavg.GetX()[ip] = gt[0]->GetX()[ip]; */
    /* 	gavg.GetY()[ip] = 0.5*(gt[0]->GetY()[ip]+gt[1]->GetY()[ip]); */
    /*   } */
    /*   favg[ch]->cd(); */
    /*   avg200[ch]->cd(); */
    /*   gavg.Write(Form("gavg200_%d", ig)); */
      
    /* } */

    favg[ch]->cd();

    if (MAX_WAVEFORMS==1000){
      for (int ii=0; ii<10; ii++) gt[ii] = (TGraph*)favg[ch]->Get(Form("avg100/gavg100_%d", ii));
      gavg.Set(set.length);
      for (ip=0; ip<set.length; ip++){
        gavg.GetX()[ip] = gt[0]->GetX()[ip];
        gavg.GetY()[ip] = 0.;
	for (int ii=0; ii<10; ii++) gavg.GetY()[ip] += gt[ii]->GetY()[ip]/10. ;
      }

      gavg.Write("justAvg");
      addToCanvas(ch, outdir, gavg, elog, c);
    }

    
    favg[ch]->Close(); 
    printf(".");
    delete favg[ch]; 
  }
  
  printf("Saved everything in %s\n", outdir);


  }
  


 QuitProgram:

  printf("Quitting digitiser stuff\n");
  /* stop the acquisition, close the device and free the buffers   */
  for(b=0; b<MAXNB; b++) {
    CAEN_DGTZ_SWStopAcquisition(handle[b]);
    CAEN_DGTZ_CloseDigitizer(handle[b]);
  }
  CAEN_DGTZ_FreeReadoutBuffer(&buffer);

  /* if (!doCalibration){ */
  /*   // Now let's compress the file  */
  /*   char zipfilecmd[1000];   */
  /*   sprintf(zipfilecmd, "gzip -f %s", outname);   */
  /*   printf("zipping binary file\n");  */
  /*   system(zipfilecmd);   */
  /*   printf("All done!\n");  */
  /* } */

  /* CAEN_DGTZ_FreeDPPEvents(handle[0], Events); */
  /* CAEN_DGTZ_FreeDPPWaveforms(handle[0], Waveform); */
    
  /* printf("\nPress a key to quit\n"); */
  /* getch(); */
  return ret;
}
    

