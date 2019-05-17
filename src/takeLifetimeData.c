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

/* ###########################################################################
 *  Functions
 *  ########################################################################### */

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
 
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle,1);  /* Set the max number of events to transfer in a sigle readout */

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
  char configFile[1000];
  char elog[10000];

  // Default configuration file
  sprintf(configFile, "exampleConfig.cfg");
  sprintf(outdir, "tmpout");
  if((argc==1)){ 
    printf("Usage : %s -i (configFile) -o (outdir) -n (MAX_WAVEFORMS) -m (ELOGMESSAGE)\n", argv[0]); 
    printf("All inputs are optional so I'm just going to use my default ones\n");
    printf("if MAX_WAVEFORMS==-1 do calibration\n\n"); 
  }
  
  char clswitch; // command line switch
  int tmpmax=0;
  char tmpout[200];
  tmpout[0] = 0;

  if (argc>1) {
    while ((clswitch = getopt(argc, argv, "i:o:n:m:")) != EOF) {
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
      } // end switch
    } // end while
  } // end if arg>1

  // Function that reads the settings 
  DAQSettings_t set = readDAQSettings(configFile);
  printDAQSettings(set);
  MAX_WAVEFORMS=set.nmax;
  sprintf(outdir, "%s", set.outfolder);

  if (tmpmax!=0) MAX_WAVEFORMS=tmpmax;
  if (tmpout[0] != 0)    sprintf(outdir, "%s", tmpout);
  
  // Make sure output directory exhists
  char makedircmd[1000]; 
  sprintf(makedircmd, "mkdir -p %s", outdir); 
  system(makedircmd); 


  printf("Nwavef %i and outdir %s\n", MAX_WAVEFORMS, outdir);

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
  int i, b, ch, ev;
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
  CAEN_DGTZ_BoardInfo_t           BoardInfo;
  CAEN_DGTZ_EventInfo_t eventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;
  char * evtptr = NULL;
  int count[MAXNB];

  uint32_t temp;

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

    uint32_t mask;
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

  printf("Allocated size %i and size of it %i\n", AllocatedSize, sizeof(AllocatedSize));

 
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
  printf("%s %i \n", __FUNCTION__, __LINE__);
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
  AcqRun = 1;
  printf("%s %i \n", __FUNCTION__, __LINE__);

  sleep(1);



  // If MAX_WAVEFORMS==-1 perform calibration
  // Calibration should be every time the digitiser is turned on
  // We should wait a few seconds for the temperatures to stabilise
  if (MAX_WAVEFORMS==-1){
    
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
    printf("\nADC calibration done\n"); 
    goto QuitProgram;    
  }

  

  int countBf0=0;



    /* Read data from the boards */ 
    for(b=0; b<MAXNB; b++) { 
      count[b]=0;

      while(count[b] < MAX_WAVEFORMS){

	//printf("Sending software trigger now?\n");  
	// just test  
	//	CAEN_DGTZ_SendSWtrigger(handle[b]); // Send a software trigger to each board   


      /* Read data from the board */ 
      ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize); 
      if (ret) { 
	printf("Readout Error\n"); 
	goto QuitProgram;     
      } 
      if (BufferSize == 0){
 	countBf0 = countBf0 + 1;
	printf("Got nothing\n");
	/* if (countBf0>10000) goto QuitProgram; */
	continue; 
      }
      Nb += BufferSize; 
      printf ("Got something \n");

      /* The buffer red from the digitizer is used in the other functions to get the event data
	 The following function returns the number of events in the buffer */
      ret |= CAEN_DGTZ_GetNumEvents(handle[b],buffer,BufferSize,&numEvents);
      
      printf(".");
      count[b] +=numEvents;

      FILE *fp;
      char outname[180];
      sprintf(outname, "%s/Binary.bin", outdir);
      fp = fopen (outname, "w");

      for (i=0;i<numEvents;i++) {
	/* Get the Infos and pointer to the event */
	ret = CAEN_DGTZ_GetEventInfo(handle[b],buffer,BufferSize,i,&eventInfo,&evtptr);
	
	/* Decode the event to get the data */
	ret = CAEN_DGTZ_DecodeEvent(handle[b],evtptr,&Evt);
	
	//*************************************
	// Event Elaboration
	//*************************************

	fwrite(evtptr, sizeof(evtptr), 1, fp);

	 for(ch=0; ch<MaxNChannels; ch++) {  
	   printf("Channel %i with size %i \n", ch, Evt->ChSize[ch]);
	   
	   if (Evt->ChSize[ch]>0){

	     SaveFunWaveform(b, ch, count[b], Evt->ChSize[ch], Evt->DataChannel[ch], outdir); 
	     
	     //	     for(ev=0; ev<Evt->ChSize[ch]; ev++){
	     //	       printf("%i %i %i %i \n", ch, ev, Evt->ChSize[ch], Evt->DataChannel[ch][ev]);
	     // }
	   }

	 }

	ret = CAEN_DGTZ_FreeEvent(handle[b],&Evt);
      }

      fclose(fp);



      /******************* OLD STUFF ***********/

      /* //ret = DataConsistencyCheck((uint32_t *)buffer, BufferSize/4);  */
      /* ret |= CAEN_DGTZ_GetDPPEvents(handle[b], buffer, BufferSize, Events, NumEvents);  */
      /* if (ret) {  */
      /* 	printf("Data Error: %d\n", ret);  */
      /* 	goto QuitProgram;  */
      /* }  */
      
      /* /\* Analyze data *\/  */
      /* for(ch=0; ch<MaxNChannels; ch++) {  */
	
      /* 	printf("Channel is %i with events %i \n", ch, NumEvents[ch]); */

      /* 	if (!(Params[b].ChannelMask & (1<<ch)))   */
      /* 	  continue;   */
	
      /* 	/\* Update Histograms *\/  */
      /* 	for(ev=0; ev<NumEvents[ch]; ev++) {  */
	  
      /* 	  TrgCnt[b][ch]++;  */
      /* 	  /\* Time Tag *\/  */
      /* 	  if (Events[ch][ev].TimeTag < PrevTime[b][ch])   */
      /* 	    ExtendedTT[b][ch]++;  */
      /* 	  PrevTime[b][ch] = Events[ch][ev].TimeTag;  */
      /* 	  /\* Energy *\/  */
      /* 	  if ( (Events[ch][ev].ChargeLong > 0) && (Events[ch][ev].ChargeShort > 0)) {  */
      /* 	    // Fill the histograms  */
      /* 	    EHistoShort[b][ch][(Events[ch][ev].ChargeShort) & BitMask]++;  */
      /* 	    EHistoLong[b][ch][(Events[ch][ev].ChargeLong) & BitMask]++;  */
      /* 	    ECnt[b][ch]++;  */
      /* 	  }  */
       
      /* 	  /\* Get Waveforms (only from 1st event in the buffer) *\/  */
      /* 	  /\* if ((Params[b].AcqMode != CAEN_DGTZ_DPP_ACQ_MODE_List) && DoSaveWave[b][ch] && (ev == 0)) {  *\/ */

      /* 	  int16_t *WaveLine;  */
      /* 	  uint8_t *DigitalWaveLine;  */
      /* 	  CAEN_DGTZ_DecodeDPPWaveforms(handle[b], &Events[ch][ev], Waveform);  */
       
      /* 	  // Use waveform data here...  */
      /* 	  int size = (int)(Waveform->Ns); // Number of samples  */
      /* 	  printf ("Size of waveform is %i \n", size); */
      /* 	  WaveLine = Waveform->Trace1; // First trace (for DPP-PSD it is ALWAYS the Input Signal)  */
      /* 	  SaveFunWaveform(b, ch, count+ev, size, WaveLine, outdir);  */
       
      /* 	  printf("Waveforms saved numb %i for channel %i '\n", count+ev, ch);  */
      /* 	  /\* } // loop to save waves          *\/ */
	  
      /* 	} // loop on events  */
      /* } // loop on channels  */
      /* count = count + NumEvents[0]; */



    } // loop on boards 
 
  }
 
 
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
	  printf("\tCh %d:\tTrgRate=%.2f KHz\t%\n", b*8+i, (float)TrgCnt[b][i]/(float)ElapsedTime); 
	else 
	  printf("\tCh %d:\tNo Data\n", i); 
	TrgCnt[b][i]=0; 
      } 
    } 
    Nb = 0; 
    PrevRateTime = CurrentTime; 
    printf("\n\n"); 
  } 
  
 
 QuitProgram:
/* stop the acquisition, close the device and free the buffers   */
    for(b=0; b<MAXNB; b++) {
      CAEN_DGTZ_SWStopAcquisition(handle[b]);
      CAEN_DGTZ_CloseDigitizer(handle[b]);
    }
    CAEN_DGTZ_FreeReadoutBuffer(&buffer);
    /* CAEN_DGTZ_FreeDPPEvents(handle[0], Events); */
    /* CAEN_DGTZ_FreeDPPWaveforms(handle[0], Waveform); */
    
    /* printf("\nPress a key to quit\n"); */
    /* getch(); */
    return ret;
}
    
