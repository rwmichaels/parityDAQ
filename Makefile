# version
VERS=1.0

# Uncomment if to profile (warning: this will slow down the code)
# export PROFILE = 1


ROOTLIBS   = $(shell root-config --libs --new)
ROOTGLIBS  = $(shell root-config --glibs --new)
#DCDIR      = $(HOME)/hana_decode_1.6
#INCLUDES   = -I$(ROOTSYS)/include -I$(DCDIR) 
INCLUDES   = -I$(ROOTSYS)/include
CXX        = g++
CXXFLAGS   = -O -Wall -fno-rtti -fno-exceptions -fPIC $(INCLUDES) 
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
#VXDIR = /adaqfs/halla/apar/vxworks/5.4/h
VXDIR = $(VXWORKS_INC)
#CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604
CCVXFLAGS =  -I$(VXDIR)

ifdef PROFILE
  CXXFLAGS += -pg
  LDFLAGS += -pg
endif

SRC = config/GreenMonster.C config/GreenMonsterDict.C config/GreenTB.C \
      config/GreenADC.C

HEAD = $(SRC:.C=.h) cfSock/GreenSock.h config/GMSock.h
DEPS = $(SRC:.C=.d)
OBJS = $(SRC:.C=.o) cfSock/cfSockCli.o

# Executible program
PROGS = config/config
SHLIB =
#SHLIB = config/libGreenMonster.so
ADC = adc/HAPADC.o adc/HAPADC_config.o
BMW = bmw/bmwClient.o bmw/bmw_config.o 
TB = timebrd/HAPTB_util.o timebrd/HAPTB_config.o
SOCK =  cfSock/cfSockSer.o cfSock/cfSockCli.o

adc: $(ADC)
timebrd: $(TB)
bmw: $(BMW)
socket: $(SOCK)
vxall: $(ADC) $(BMW) $(SOCK) $(TB)
all:  $(ADC) $(BMW) $(SOCK) $(TB) $(PROGS) $(SHLIB)
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
	rm -f bmw/core bmw/*.o bmw/*.d
	rm -f cfSock/core cfSock/*.o cfSock/*.d
	rm -f timebrd/core timebrd/*.o timebrd/*.d

realclean:  clean
	rm -f *.d


config/config: config/config.o $(OBJS) $(SRC) $(HEAD)
	rm -f $@
	$(CXX) $(CXXFLAGS) -o $@ config/config.o $(OBJS) $(ALL_LIBS) 


adc/HAPADC.o : adc/HAPADC.c adc/HAPADC.h
	rm -f $@
	ccppc -o $@ $(CCVXFLAGS) adc/HAPADC.c

adc/HAPADC_config.o : adc/HAPADC_config.c adc/HAPADC_cf_commands.h
	cd adc; rm -f $@
	ccppc -o $@ $(CCVXFLAGS) adc/HAPADC_config.c

bmw/bmwClient.o : bmw/bmwClient.c bmw/bmw.h
	rm -f $@
	ccppc -o $@ -c $(CCVXFLAGS) bmw/bmwClient.c

bmw/bmw_config.o : bmw/bmw_config.c bmw/bmw_config.h bmw/bmw_cf_commands.h
	rm -f $@
	ccppc -o $@ -c $(CCVXFLAGS) bmw/bmw_config.c
 
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










