/************************************************************************
 *  HAPADC.c                                                            *
 *   subroutines which are used to interface with the                   *
 *   HAPPEX ADC for test purposes                                       *
 *                                                                      *  
 * Date  : R. Michaels/ A. Vacheret  October 2000                       *   
 * Modifications:                                                       *
 *                                                                      *
 ************************************************************************/

#include <vxWorks.h>
#include <types.h>
#include <vme.h>
#include <stdioLib.h>
#include <logLib.h>
#include <math.h>
#include <string.h>
#include <ioLib.h>
#include <stdlib.h>

#include "../module_map.h" 
#define DEFAULTADC 0x490000


/* Definition of csr bits */
/*
    bits 0-1 = diagnostic mux (0,1,2,3 -> which ADC to send data to outputs)
    bit 2    = enable interrupts
    bit 3    = enable RNG
    bit 4    = integrator select range
    bit 5-7  = not used

   7 6 5 4 3 2 1 0 
   0 0 0 x 0 0 y z    x = hi/lo = 1/0   (z,y) = 0,1,2,3 

   example:  hi gain, mux=2 (3rd) -> mycsr = 0x12;   */


int a24addr = 0;
unsigned long mycsr = 0x0;  
int did_init = 0;
volatile struct vme_happex_adc *adcboard;
extern int rand();

/******************************************************
 *
 *  Initialize the HAPPEX ADC by defining its address
 *  and setting the CSR
 *
 ******************************************************/
int initHAPADC(int initaddr)
{
  int res;
  unsigned long vmeaddr;

  if (initaddr == NULL) {
    a24addr = DEFAULTADC;
  } else {
    a24addr = initaddr;
  }

  /* Get the ADC Address.. A24 Addressing */
  res = sysBusToLocalAdrs(0x39,(char *)a24addr,(char **)&vmeaddr);
  if (res != 0) {
    printf("initHAPADC: ERROR in sysBusToLocalAdrs(0x29,0x%x,&vmeaddr) \n"
	   ,a24addr);
    return(ERROR);
  }

  adcboard = (struct vme_happex_adc *) vmeaddr;
  adcboard->csr = mycsr; 
  did_init = 1;
  printf("HAPPEX ADC at 0x%x  programmed with csr = 0x%x\n",
         vmeaddr,mycsr);
  return 0;
}

/******************************************************
 *
 *  Argument 'newcsr' is used to set the csr of the ADC
 *
 ******************************************************/
int newcsrHAPADC(int newcsr)
{
   mycsr = newcsr;
   initHAPADC(a24addr);
}

/******************************************************
 *
 *  Argument 'newcsr' is used to set the csr of the ADC
 *   Used to set variable without initializing/writing
 *  
 ******************************************************/
int setcsrHAPADC(int newcsr)
{
   mycsr = newcsr;
}


/******************************************************
 *
 *  Argument 'newaddr' is used to set the A24 address 
 *  the ADC board.  It over-writes a24addr
 *
 ******************************************************/
int newaddrHAPADC(int newaddr)
{
  a24addr = newaddr;
  initHAPADC(a24addr);
}


/******************************************************
 * 
 *  dump registers to screen
 *
 ******************************************************/
int dumpHAPADC()
{
  int i;
  if ( !did_init ) {
    printf("Cannot dump ADC.  It was not initialized\n");
    printf("Did you call initHAPADC ?\n");
    return 0;
  }
  printf("\n Dump of ADC registers:\n");
  for (i=0; i<4; i++) {  
    printf("channel %d   data = %d (dec)   = 0x%x (hex) \n",
      i,adcboard->adchan[i],adcboard->adchan[i]);
  }

  printf("csr = 0x%x \n",adcboard->csr);
  printf("rng = 0x%x \n",adcboard->rng);
  printf("done = 0x%x \n",adcboard->done);
}

int getran() {
  int rannum=0;
  while (rannum<2048 || rannum>63488) {
    rannum = rand() + rand();
  }
  return rannum;
}
