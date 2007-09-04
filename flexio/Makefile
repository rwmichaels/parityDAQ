#  Build system for Flexio library
#
#  July 2005   Bob Michaels


export OSNAME       := $(shell uname)

CC  = $(PURE) cc

CODA = /adaqfs/coda/2.5
CODA_BIN = $(CODA)/$(OSNAME)/bin
CODA_LIB = $(CODA)/$(OSNAME)/lib
CODA_INC = $(CODA)/common/include

AC_FLAGS  = -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS 
CODAFLAGS = -I$(CODA)/$(OSNAME)/include -I.

VXDIR=/site/vxworks/5.4/ppc/target/h
CCVXFLAGS =  -I$(VXDIR) -DCPU_FAMILY=PPC -DCPU=PPC604

ifeq ($(OSNAME),Linux)
    AC_FLAGS += -DMUTEX_NOSHARE
    CFLAGS =  -O2 -fPIC $(AC_FLAGS) 
    LIBS =   -lieee -lm -lpthread -ldl -lresolv
    SWFLAGS = -DLINUX
endif

ifeq ($(OSNAME),SunOS)
   CFLAGS    = -mt -fast -xO5 -KPIC $(AC_FLAGS) 
   LIBS    = -lm -lposix4 \
      -lthread -lC -lm -lsocket -lnsl -lresolv -ldl 
   SWFLAGS = -DSUNVERS
endif

all :  flexio_lib.o

flexio_lib.o: flexio_lib.c 
	rm -f $@
	ccppc -c $(CCVXFLAGS) flexio_lib.c

clean : 
	rm -f *.o 






