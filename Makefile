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

CC	=	gcc

COPTS	=	-fPIC -DLINUX -O2

#FLAGS	=	-soname -s
#FLAGS	=       -Wall,-soname -s
#FLAGS	=	-Wall,-soname -nostartfiles -s
#FLAGS	=	-Wall,-soname

DEPLIBS	=	-lCAENDigitizer

LIBS	=	-L.. -L${CAENlibraries}/lib

INCLUDEDIR =	-I./include -I${CAENlibraries}/include

OBJS	=	src/takeLifetimeData.o src/keyb.o src/Functions.o src/DAQSettings.o

INCLUDES =	./include/*

#########################################################################

all	:	$(OUT)

clean	:
		/bin/rm -f $(OBJS) $(OUT)

$(OUT)	:	$(OBJS)
		/bin/rm -f $(OUT)
		if [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi
		$(CC) $(FLAGS) -o $(OUT) $(OBJS) $(DEPLIBS) ${LIBS}

$(OBJS)	:	$(INCLUDES) Makefile

%.o	:	%.c
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<

