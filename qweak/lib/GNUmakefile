## Setup correct target type
#

TARGET=powerpc-wrs-vxworks
CC=$(TARGET)-gcc

all : vqwk.o vqwk_longcall.o


vqwk.o: vqwk.c vqwk.h
	$(CC) -c -o $@ vqwk.c

vqwk_longcall.o: vqwk.c vqwk.h
	$(CC) -c -mlongcall -o $@ vqwk.c

clean:
	rm -f *~ *.o
