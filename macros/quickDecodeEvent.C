
#define MAX_UINT16_CHANNEL_SIZE     (64)

typedef struct 
{
    uint32_t            ChSize[MAX_UINT16_CHANNEL_SIZE]; // the number of samples stored in DataChannel array  
    uint16_t            *DataChannel[MAX_UINT16_CHANNEL_SIZE]; // the array of ChSize samples
} CAEN_DGTZ_UINT16_EVENT_t;



void quickDecodeEvent(){

  CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;

  int ch;

  int MaxNChannels=8;


  FILE *fp;
  char inname[180];
  cout << __LINE__ << endl;

  sprintf(inname, "/home/lindac/DUNE/DAQtests/TestBinaryOutput/Binary.bin");
  cout << __LINE__ << endl;
  fp = fopen (inname, "r");
  cout << __LINE__ << endl;

  fread(Evt, sizeof(*Evt), 1, fp);

  cout << __LINE__ << endl;

  for(ch=0; ch<MaxNChannels; ch++) {  
    cout << __LINE__ << endl;
    printf("Channel %i with size %i \n", ch, Evt->ChSize[ch]);
    
    if (Evt->ChSize[ch]>0){
      cout << __LINE__ << endl;
      
      for(int ev=0; ev<Evt->ChSize[ch]; ev++){
	cout << __LINE__ << endl;
	printf("%i %i %i %i \n", ch, ev, Evt->ChSize[ch], Evt->DataChannel[ch][ev]);
      }
    }
    
  }
}


