/******************************************************************************
*
* CAEN SpA - Front End Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the
* software, documentation and results solely at his own risk.
******************************************************************************/

#ifndef READOUT_TEST_FUNCTIONS
#define READOUT_TEST_FUNCTIONS

#include "keyb.h"
#include "CAENDigitizerType.h"
#include "DAQSettings.h"

#ifdef WIN32

    #include <time.h>
    #include <sys/timeb.h>
    #include <conio.h>
    #include <process.h>
	#define getch _getch     /* redefine POSIX 'deprecated' */
	#define kbhit _kbhit     /* redefine POSIX 'deprecated' */

#else
    #include <unistd.h>
    #include <stdint.h>   /* C99 compliant compilers: uint64_t */
    #include <ctype.h>    /* toupper() */
    #include <sys/time.h>
#endif

#define HV_MAXCHANNELS      (2) // DT5790 has maximum 2 HV channels

#define HV_VSET_ADDR        (0x1020)
#define HV_ISET_ADDR        (0x1024)
#define HV_RAMPUP_ADDR      (0x1028)
#define HV_RAMPDOWN_ADDR    (0x102C)
#define HV_VMAX_ADDR        (0x1030)
#define HV_VMON_ADDR        (0x1040)
#define HV_IMON_ADDR        (0x1044)
#define HV_POWER_ADDR       (0x1034)
#define HV_STATUS_ADDR      (0x1038)
#define HV_CONTROL_ADDR     HV_POWER_ADDR

#define HV_REGBIT_PWDOWN        (1)
#define HV_PWDOWN_BITVALUE_RAMP (1)
#define HV_PWDOWN_BITVALUE_KILL (0)

typedef struct
{
    CAEN_DGTZ_ConnectionType LinkType;
    uint32_t VMEBaseAddress;
	uint32_t RecordLength;
	uint32_t ChannelMask;
    int EventAggr;
    CAEN_DGTZ_PulsePolarity_t PulsePolarity;
    CAEN_DGTZ_DPP_AcqMode_t AcqMode;
    CAEN_DGTZ_IOLevel_t IOlev;
} DigitizerParams_t;


/* ###########################################################################
*  Functions
*  ########################################################################### */

DAQSettings_t readDAQSettings(char infile[1000]); 

/*! \fn      static long get_time()
*   \brief   Get time in milliseconds
*   \return  time in msec */ 
long get_time();

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      int DataConsistencyCheck(uint32_t *buff32, int NumWords)
*   \brief   Do some data consistency check
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int DataConsistencyCheck(uint32_t *buff32, int NumWords);


/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      SaveHistograms(uint32_t EHisto[8][1<<MAXNBITS])
*   \brief   Save Histograms to output files
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */

int SaveHistogram(char *basename, int b, int ch, uint32_t *EHisto);

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      SaveWaveforms(int b, int ch, int trace, int size, int16_t *WaveData)
*   \brief   Save Waveforms to output file
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int SaveWaveform(int b, int ch, int trace, int size, int16_t *WaveData);

int SaveFunWaveform(int b, int ch, int trace, int size, int16_t *WaveData, char *inName);

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      SaveDigitalProbe(int b, int ch, int trace, int size, uint8_t *WaveData)
*   \brief   Save Digital Waveforms to output file
*   \return  0=success; -1=error */
/* --------------------------------------------------------------------------------------------------------- */
int SaveDigitalProbe(int b, int ch, int trace, int size, uint8_t *WaveData);

/* --------------------------------------------------------------------------------------------------------- */
/*! \fn      PrintInterface()
*   \brief   Print the interface to screen
*   \return  none
/* --------------------------------------------------------------------------------------------------------- */
void PrintInterface();

#endif
