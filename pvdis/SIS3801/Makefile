#  Build system for scaler readout of SIS3800 and SIS3801 scalers
#

export OSNAME       := $(shell uname)

VXDIR=/adaqfs/halla/a-onl/vxworks/headers/5.5/h
CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604


all : SIS3801.o


SIS3801.o: SIS3801.c SIS3801.h SIS.h
	rm -f $@
	ccppc -c $(CCVXFLAGS) SIS3801.c


clean : 
	rm -f *.o






