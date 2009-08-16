/*  flexio_lib.c    R. Michaels   July 2005

    Library of code to setup, read and write to a FLEXIO.

    This assumes we have one input card [top] and one 
    output card [bottom] -- this is true for the HAPPEX crate.  
    Default setup:
    Input card:  external strobe and external latch mode, no interrupts
           This is how CRL does it.  See also card*allow below.
    Output card: Direct VME output (i.e. not pulsed nor strobed)

*/

#include "flexio.h" 


/* Memory map of FLEXIO board */
   struct vme_flexio_struct {
       unsigned short csr1;
       unsigned short data1;
       unsigned short csr2;
       unsigned short data2;
   } ;

   struct vme_flexio_struct *flex_io;

///* VME offset */
//   static int FLEXIO_ADDR = 0x0ed0;

/* Lock on cards.  There are 2 cards in a board.  If this is "0" its 
   assumed that another process (e.g. CRL) is only allowed to write 
   to the card.  This has to be compiled in. */
   
   int card1_allow = 0;
   int card2_allow = 1;

/* global variable =1 if successfully initialized */
   static int initialized=0;

/* static output data (useful if only one bit changed) */
   static int outputdata = 0;

/* To debug(1) or not (0) */
   static int debug=0;

/* Function prototypes.   */

   void FIO_Init();
   int FIO_Check(int icard);
   void FIO_Write(int data);
   void FIO_WriteChan(int chan, int data);
   int FIO_Read();
   int FIO_ReadChan(int chan);

/***************************************************************************/

void FIO_Init() { 
/* Initialize.  This is REQUIRED (and normally done at VME reboot) */
  int isca,res;
  unsigned long laddr;

  /* FlexIO Pointer */
  res = sysBusToLocalAdrs(0x29,FLEXIO_ADDR,&laddr);
  if (res != 0) {
    printf("Error in sysBusToLocalAdrs res=%d \n",res);
    printf("ERROR:  FlexIO POINTER NOT INITIALIZED");
    return;
  } else {
    printf("FlexIO address = 0x%x\n",laddr);
    flex_io = (struct vme_flexio_struct *)laddr;
  }

  if ( card1_allow ) {
    flex_io->csr1 = 0x8000; /* Reset Card */
    /*     flex_io->csr1 = 0;  */   /* straight VME reads (no strobe) */
    flex_io->csr1 = 0x3;   /* Enable External Strobe and Latch Mode */
  }
  if ( card2_allow ) {
    flex_io->csr2 = 0x8000; /* Reset Card */
    flex_io->csr2 = 0;   /* direct VME output */
  }
  initialized = 1;

}


int FIO_Check(int card) {
  /* Check if this card is setup properly and is usable */
  if (card < 1 || card > 2) return 0;
  if (card == 1 && !card1_allow) {
    printf("Not allowed to write to card %d \n",card);
    return 0;
  }
  if (card == 2 && !card2_allow) {
    printf("Not allowed to write to card %d \n",card);
    return 0;
  }
  if ( !initialized ) {
    printf("ERROR: Uninitialized card \n");
    return 0;
  }
  return 1;
}


void FIO_Write(int data) {
  if ( !FIO_Check(2) ) return;
  outputdata = data;
  flex_io->data2 = data;
}


void FIO_WriteChan(int chan, int data) {
/* Write to the channel (corresponding bit) if we are allowed to. 
   Whatever else was on the output (other channels) is preserved.
   Arguments:
       chan =  0,1,2,3,4,   = 1st,2nd,3rd.... channels
       data = 0 or 1 will write a 0 or 1 to that channel
          (if data > 1, we mask off all but the 1st bit)   

*/
  int i,ldata,pow,iout;
  ldata = (data & 0x1);
  if ( !FIO_Check(2) ) return;
  if (debug) printf("original output data = 0x%x\n",outputdata);
  iout = 0;
  pow = 1;
  for (i = 0; i < 16; i++) {
    if (i > 0) pow = 2*pow;
    if (i == chan) {
      if (ldata == 1) iout |= (1 << chan);
      if (debug) printf("output bit %d  0x%x\n",i,iout);
    } else {
      iout += (pow & outputdata);
      if (debug) 
        printf ("i %d   pow %d   stuff %d   iout 0x%x \n",
           i,pow,(pow&outputdata),iout);
    }
  }
  if (debug) printf("outputdata 0x%x  chan 0x%x  ldata 0x%x  iout 0x%x \n",
       outputdata,chan,ldata,iout);
  flex_io->data2 = iout;
  outputdata = iout;
}


int FIO_Read() {
  if (debug) printf("Read:  0x%x \n",flex_io->data1);
  return flex_io->data1;
}


int FIO_ReadChan(int chan) {
/* returns 0 or 1 if channel has a 1  (chan = 0,1,2...) */
   int rdata, bit;
   rdata = flex_io->data1;
   bit = (rdata & (1 << chan) ) >> chan;
   if (debug) printf("ReadChan %d =  0x%x \n",chan,bit);
   return bit;
}
