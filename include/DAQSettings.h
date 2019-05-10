/*
 * DAQ Settings
 * CAEN DT5730 output file format, raw digitiser input
 * 
 */

#ifndef DAQ_SETTINGS
#define DAQ_SETTINGS

#include <unistd.h>
#include <stdint.h>   /* C99 compliant compilers: uint64_t */
#include <ctype.h>    /* toupper() */
#include <sys/time.h>


/* class DAQSettings  */
/* { */
/*  public: */
  
/*    DAQSettings();    */
/*    ~DAQSettings();    */
  

typedef struct {
   // Data taking parameters 
   int nmax;            // Total number of waveforms to be saved. This parameter can be overridden with the option -n 
   char outfolder[1000]; // Output folder. This parameter can be overridden with option -o 
 // DPP Parameters 
   int thr;             // Trigger Threshold 
   int nsbl;            // Number of samples for the baseline averaging:	 0 -> absolute Bl, 1 -> 16samp, 2 -> 64samp, 3 -> 256samp, 4 -> 1024samp  
   int lgate;           // Long Gate Width (N*2ns for x730  and N*4ns for x725) 
   int sgate;           // Short Gate Width (N*2ns for x730  and N*4ns for x725) 
   int pgate;           // Pre Gate Width (N*2ns for x730  and N*4ns for x725)  
   int selft;           // Self Trigger Mode 0-> Disabled,  1 -> Enabled  

   int tvaw;            // Trigger Validation Acquisition Window 
   int csens;           // Charge sensibility:  
                          // Options for Input Range 2Vpp: 0->5fC/LSB; 1->20fC/LSB; 2->80fC/LSB; 3->320fC/LSB; 4->1.28pC/LSB; 5->5.12pC/LSB  
 	                 // Options for Input Range 0.5Vpp: 0->1.25fC/LSB; 1->5fC/LSB; 2->20fC/LSB; 3->80fC/LSB; 4->320fC/LSB; 5->1.28pC/LSB  

   int purh;             // Purity Gap in LSB units (1LSB = 0.12 mV for 2Vpp Input Range, 1LSB = 0.03 mV for 0.5 Vpp Input Range ) 
   int blthr;            // Baseline Threshold 
   int trgho;            // Trigger HoldOff 
 
} DAQSettings_t;

#endif
