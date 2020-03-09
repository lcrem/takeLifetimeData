########################################################################
#
##              --- CAEN SpA - Computing Division ---
#
##   CAENDigitizer Software Project
#
##   Created  :  October    2009      (Rel. 1.0)
#
##   Auth: A. Lucchesi
#
#########################################################################
ARCH	=	`uname -m`

OUTDIR  =    	./bin/$(ARCH)/Release/
OUTNAME =    	takeLifetimeData.bin
OUT     =    	$(OUTDIR)/$(OUTNAME)

#CC	=	gcc
CC	=	g++

COPTS	=	-fPIC -DLINUX -O2 -fpermissive

ROOTCFLAGS =$(shell root-config --cflags)
ROOTLIBS=$(shell root-config --libs)

#FLAGS	=	-soname -s
#FLAGS	=       -Wall,-soname -s
#FLAGS	=	-Wall,-soname -nostartfiles -s
#FLAGS	=	-Wall,-soname

DEPLIBS	=	-lCAENDigitizer -lRootFftwWrapper

LIBS	=	-L.. -L${CAENlibraries}/lib ${ROOTLIBS} -L$(FFTW_UTIL_INSTALL_DIR)/lib 

INCLUDEDIR =     -I./include -I${CAENlibraries}/include -I$(FFTW_UTIL_INC_DIR)

OBJS	=	src/takeLifetimeData.o src/keyb.o src/Functions.o src/DAQSettings.o

ROOTOBJS =       src/RootFunctions.o

INCLUDES =	./include/* 

#########################################################################

all	:	$(OUT)

clean	:
		/bin/rm -f $(OBJS) $(OUT)

$(OUT)	:	$(OBJS)
		/bin/rm -f $(OUT)
		if [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi
		$(CC) $(ROOTCFLAGS) $(FLAGS) -o $(OUT) $(OBJS) $(DEPLIBS) ${LIBS}

$(OBJS)	:	$(INCLUDES) Makefile


%.o	:	%.c
		$(CC) $(COPTS) $(ROOTCFLAGS) $(INCLUDEDIR) -c -o $@ $<

