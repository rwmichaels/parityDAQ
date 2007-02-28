#  Build system for HAPPEX 18 bit ADC
#  R. Michaels, Feb 2007

export OSNAME       := $(shell uname)

VXDIR=$(VXWORKS_INC)
CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604 -mlongcall


all : hapAdc18Lib.o 


hapAdc18Lib.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -c $(CCVXFLAGS) hapAdc18Lib.c

clean : 
	rm -f *.o






