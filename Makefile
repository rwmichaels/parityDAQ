# version
VERS=1.0

# Uncomment if to profile (warning: this will slow down the code)
# export PROFILE = 1

# Use vxWorks 5.5 for 6100 boards, else use 5.4 until further notice
export USE55=1

ROOTLIBS   = $(shell root-config --libs --new)
ROOTGLIBS  = $(shell root-config --glibs --new)
#DCDIR      = $(HOME)/hana_decode_1.6
#INCLUDES   = -I$(ROOTSYS)/include -I$(DCDIR) 
INCLUDES   = -I$(ROOTSYS)/include
EPICS_INC  = -I/adaqfs/halla/apar/bryan/epics/base/include
CXX        = g++
#CXXFLAGS   = -O -Wall -fno-rtti -fno-exceptions -fPIC $(INCLUDES) 
CXXFLAGS   = -O -Wall -fno-exceptions -fPIC $(INCLUDES) 
LD         = g++
LDFLAGS    = 
SOFLAGS    = -shared
# LIBS       = $(ROOTLIBS)
GLIBS      = $(ROOTGLIBS) -L/usr/X11R6/lib -lXpm -lX11
#LIBDC      = libdc.a
LIBET = $(CODA)/Linux/lib/libet.so
ONLIBS = $(LIBET) -lieee -lpthread -ldl -lresolv
ALL_LIBS   = $(GLIBS) $(ROOTLIBS)

CC = gcc
SWFLAGS = -DLINUX
LIBS = -lieee -ldl -lresolv
#VXDIR = /adaqfs/halla/apar/tornado
#VXDIR = $(VXWORKS_INC)
ifdef USE55
#  VXDIR = /adaqfs/halla/apar/vxworks/5.5/5.5
  VXDIR = /adaqfs/halla/a-onl/vxworks/headers/5.5/h
else
#  VXDIR = /adaqfs/halla/apar/vxworks/5.4/h
  VXDIR = /adaqfs/halla/a-onl/vxworks/headers/5.4/h
endif
#CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604
CCVXFLAGS =  -I$(VXDIR)
ifdef USE55
  CCVXFLAGS += -c -mlongcall
endif

ifdef PROFILE
  CXXFLAGS += -pg
  LDFLAGS += -pg
endif

SRC = config/GreenMonster.C config/GreenMonsterDict.C config/GreenTB.C \
      config/GreenADC.C config/GreenADC18.C 

HEAD = $(SRC:.C=.h) cfSock/GreenSock.h config/GMSock.h
DEPS = $(SRC:.C=.d)
OBJS = $(SRC:.C=.o) cfSock/cfSockCli.o

# Executible program
PROGS = config/config
SHLIB =
#SHLIB = config/libGreenMonster.so
# Of course, "ADC" means the 16-bit ADC (while ADC18 is obvious)
# I might change this later to ADC16, etc.
ADC = adc/HAPADC_ch.o adc/HAPADC_inj.o adc/HAPADC_lspec.o \
	adc/HAPADC_rspec.o  adc/HAPADC_test.o  adc/HAPADC_config.o
ADC18 = adc18/hapAdc18Test.o adc18/hapAdc18Count.o \
	adc18/hapAdc18Left.o adc18/hapAdc18Right.o \
	adc18/hapAdc18Inj.o 
BMW = bmw/bmwClient.o bmw/bmw_config.o 
TB = timebrd/HAPTB_util.o timebrd/HAPTB_config.o
AUXTB = auxtimebrd/AUXTB_util.o
SOCK =  cfSock/cfSockSer.o cfSock/cfSockCli.o
SCAN = scan/SCAN_util.o scan/SCAN_config.o
CAFFB = caFFB/caFFB.o
AUTO = auto/auto_rhwp.o auto/auto_filter.o auto/auto_led.o
#FLEXIO = flexio/flexio_lib_ch.o flexio/flexio_lib_rspec.o
FLEXIO = flexio/flexioLib.o
#CAFFB = 

adc: $(ADC)
adc18: $(ADC18)
timebrd: $(TB)
auxtimebrd: $(AUXTB)
bmw: $(BMW)
socket: $(SOCK)
scan: $(SCAN)
caFFB: $(CAFFB)
auto: $(AUTO)
flexio: $(FLEXIO)
vxall: $(ADC) $(ADC18) $(BMW) $(SOCK) $(TB)  $(SCAN) $(CAFFB) $(AUXTB) $(AUTO) $(FLEXIO)
all:  $(ADC) $(ADC18) $(BMW) $(SOCK) $(TB) $(SCAN) $(CAFFB) $(PROGS) $(SHLIB) $(AUXTB) $(AUTO) $(FLEXIO)
dict:
#dict: config/GreenMonsterDict.C

config/libGreenMonster.so: $(OBJS) $(HEAD)
	$(CXX) -shared -O -o config/libGreenMonster.so $(OBJS) $(ALL_LIBS)


# The main code is config
version: clean
	mkdir $(VERS) 
	cp *.C ./$(VERS) 
	cp *.h ./$(VERS) 
	cp Makefile ./$(VERS) 
	cp README ./$(VERS)

clean:
	rm -f *.o core *~ *.d *.out $(PROGS) *Dict* *.so
	rm -f core *.o
	rm -f config/core config/*.o config/*Dict* config/*.d config/*.so
	rm -f adc/core adc/*.o adc/*.d
	rm -f adc18/*.o adc18/*.d
	rm -f bmw/core bmw/*.o bmw/*.d
	rm -f cfSock/core cfSock/*.o cfSock/*.d
	rm -f timebrd/core timebrd/*.o timebrd/*.d
	rm -f auxtimebrd/core auxtimebrd/*.o auxtimebrd/*.d
	rm -f scan/core scan/*.o scan/*.d
	rm -f caFFB/core caFFB/*.o caFFB/*.d
#	rm -f flexio/flexio_lib_ch.o flexio/flexio_lib_rspec.o
	rm -f flexio/flexioLib.o
	rm -r auto/auto_rhwp.o auto/auto_filter.o auto/auto_led.o

realclean:  clean
	rm -f *.d


config/config: config/config.o $(OBJS) $(SRC) $(HEAD)
	rm -f $@
	$(CXX) $(CXXFLAGS) -o $@ config/config.o $(OBJS) $(ALL_LIBS) 

adc/HAPADC_ch.o : adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DCOUNTINGHOUSE adc/HAPADC.c

adc/HAPADC_inj.o : adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DINJECTOR adc/HAPADC.c

adc/HAPADC_lspec.o : adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DLEFTSPECT adc/HAPADC.c

adc/HAPADC_rspec.o : adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DRIGHTSPECT adc/HAPADC.c

adc/HAPADC_test.o: adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DTESTCRATE adc/HAPADC.c

adc/HAPADC_config.o : adc/HAPADC_config.c adc/HAPADC_cf_commands.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) adc/HAPADC_config.c

adc18/hapAdc18Test.o: adc18/hapAdc18Lib.c adc18/hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DTESTCRATE adc18/hapAdc18Lib.c

adc18/hapAdc18Count.o: adc18/hapAdc18Lib.c adc18/hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DCOUNTINGHOUSE adc18/hapAdc18Lib.c

adc18/hapAdc18Left.o: adc18/hapAdc18Lib.c adc18/hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DLEFTSPECT adc18/hapAdc18Lib.c

adc18/hapAdc18Right.o: adc18/hapAdc18Lib.c adc18/hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DRIGHTSPECT adc18/hapAdc18Lib.c

adc18/hapAdc18Inj.o: adc18/hapAdc18Lib.c adc18/hapAdc18Lib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) -DINJECTOR adc18/hapAdc18Lib.c

bmw/bmwClient.o : bmw/bmwClient.c bmw/bmw.h
	rm -f $@
	ccppc -o $@ -c $(CCVXFLAGS) bmw/bmwClient.c

bmw/bmw_config.o : bmw/bmw_config.c bmw/bmw_config.h bmw/bmw_cf_commands.h
	rm -f $@
	ccppc -o $@ -c $(CCVXFLAGS) bmw/bmw_config.c
 
flexio/flexioLib.o : flexio/flexioLib.c flexio/flexioLib.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) flexio/flexioLib.c

# flexio/flexio_lib_ch.o : flexio/flexio_lib.c flexio/flexio.h
# 	rm -f $@
# 	ccppc -o $@ $(CCVXFLAGS) -DCOUNTINGHOUSE flexio/flexio_lib.c

# flexio/flexio_lib_rspec.o : flexio/flexio_lib.c flexio/flexio.h
# 	rm -f $@
# 	ccppc -o $@ -c $(CCVXFLAGS) -DRIGHTSPECT flexio/flexio_lib.c

cfSock/cfSockCli.o : cfSock/cfSockCli.c cfSock/cfSock.h cfSock/cfSock_types.h
	rm -f $@
	$(CC) -o $@ -c $(CFLAGS) $(SWFLAGS) cfSock/cfSockCli.c 

cfSock/cfSockSer.o: cfSock/cfSockSer.c cfSock/cfSock.h  cfSock/cfSock_types.h \
        cfSock/cfSockCli.h cfSock/GreenSock.h
	rm -f $@
	ccppc -o $@ -c $(CCVXFLAGS) cfSock/cfSockSer.c

timebrd/HAPTB_util.o: timebrd/HAPTB_util.c timebrd/HAPTB.h    \
              timebrd/HAPTB_util.h
	cd timebrd; rm -f $@; \
	ccppc  -c $(CCVXFLAGS) HAPTB_util.c

timebrd/HAPTB_config.o: timebrd/HAPTB_config.c timebrd/HAPTB_cf_commands.h
	cd timebrd; rm -f $@; \
	ccppc  -c $(CCVXFLAGS) HAPTB_config.c

scan/SCAN_util.o: scan/SCAN_util.c  scan/SCAN_util.h
	cd scan; rm -f $@; \
	ccppc  -c $(CCVXFLAGS) SCAN_util.c

scan/SCAN_config.o: scan/SCAN_config.c scan/SCAN_cf_commands.h
	cd scan; rm -f $@; \
	ccppc  -c $(CCVXFLAGS) SCAN_config.c

caFFB/caFFB.o: caFFB/caFFB.c caFFB/caFFB.h
	ccppc  -c $(CCVXFLAGS) $(EPICS_INC) $< -o $@

auxtimebrd/AUXTB_util.o: auxtimebrd/AUXTB_util.c auxtimebrd/AUXTB.h   
	cd auxtimebrd; rm -f $@; \
	ccppc  -c $(CCVXFLAGS) AUXTB_util.c

auto/auto_rhwp.o: auto/auto_rhwp.c
	cd auto; rm -f $@; \
	ccppc -c $(CCVXFLAGS) auto_rhwp.c

auto/auto_led.o: auto/auto_led.c auto/auto_led.h
	cd auto; rm -f $@; \
	ccppc -c $(CCVXFLAGS) auto_led.c

auto/auto_filter.o: auto/auto_filter.c
	cd auto; rm -f $@; \
	ccppc -c $(CCVXFLAGS) auto_filter.c

#.SUFFIXES:
#.SUFFIXES: .c .cc .cpp .C .o .d

config/GreenMonsterDict.C: config/GreenMonster.h config/GreenMonsterLinkDef.h
	@echo "Generating dictionary $@..."
	cd config; \
	rootcint -f GreenMonsterDict.C -c GreenMonster.h GreenMonsterLinkDef.h

config/%.o: %.C
	$(CXX) $(CXXFLAGS) -c config/$<


#%.d:	%.C
#	@echo Creating dependencies for $<
#	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) -c $< \
#		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
#		[ -s $@ ] || rm -f $@'

#-include $(DEPS)










