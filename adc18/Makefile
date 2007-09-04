#  Build system for HAPPEX 18 bit ADC
#  R. Michaels, Feb 2007


export OSNAME       := $(shell uname)

VXDIR=$(VXWORKS_INC)
CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604 -mlongcall


# Normally you should "make adc18/adc18" in ../  (i.e. in directory above)

all : hapAdc18Lib.o 

adc18 : hapAdc18Lib.o hapAdc18Test.o hapAdc18Count.o hapAdc18Left.o hapAdc18Right.o hapAdc18Inj.o

hapAdc18Lib.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DTESTCRATE hapAdc18Lib.c

hapAdc18Test.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DTESTCRATE hapAdc18Lib.c

hapAdc18Count.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DCOUNTINGHOUSE hapAdc18Lib.c

hapAdc18Left.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DLEFTSPECT hapAdc18Lib.c

hapAdc18Right.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DRIGHTSPECT hapAdc18Lib.c

hapAdc18Inj.o: hapAdc18Lib.c hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DINJECTOR hapAdc18Lib.c

clean : 
	rm -f *.o






