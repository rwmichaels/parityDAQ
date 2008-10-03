/************************************************************************
 *  SIS3801.c                                                           *
 *   subroutines which are used to interface with the                   *
 *   32 channel scaler card STRUCK model SIS3801                        *
 *                                                                      *  
 * Date  : January 22 99                                                *   
 * Modifications:                                                       *
 * 01/28/99    R.M.  Some renaming of address map, a few corrections,   *
 *                  replace STR->SIS, throw out old code;   NOTE:       *
 *                  I put "version 0" in SIS3801_v0.c (vers 0)          *
 *                                                                      *
 * 01/29/99    R.M.  New addressing scheme.  Seems FIFO likes to        *
 *                  be read with A32.                                   *
 *                                                                      *
 * 02/04/99    R.T.  Add routines which  test the FIFO operation        *
 *                  in the "parity mode". "testInt" and "testExt"       *
 *                  test the FIFO with internal and external pulser     *
 *                  respectively.                                       *
 *                                                                      *
 * 03/15/99    R.T.  Remove the test routine and rename some routine    *
 *                   Init -> initSIS,  etc...                           *
 *                   And make the routines to be used with more than one*
 *                   scaler.                                            *
 *                   With the PPC Board, we do not need the double      *
 *                   addressing scheme (A24 and A32 for the FIFO read)  *
 *                                                                      *
 * 03/16/99    R.T.  Add the Producer SISProducer and the interrupt     *
 *                   service routine SISInterrupt.                      *
 *                   Add the initialization of the ISR initISR_SIS      *
 *                                                                      *
 * 06/07/99    R.M.  Make one routine for SISFIFO_Read reading scaler;  *
 *                   it is shared by SISInterrupt and SISProducer.      *
 *                   This routine also calls SISintime or SISouttime    *
 *                   to load the scaler data into variable arrays,      *
 *                   depending on if the scalers are setup with in-time *
 *                   or out-of-time helicity information.               *
 *                   SISHel_Chk_Load routine added for checking helicity.     *
 *                   For OUT_OF_TIME_MODE, the pseudorandom helicity    *
 *                   prediction is used to check the delayed helicity   *
 *                   Added SISendrun() to flush buffers for out-of-time *
 *                   mode at end of a CODA run.                         *
 *                                                                      *
 * 7/20/99      R.T. Made the out-of-time mode work. We think.  Several *
 *                   flags added to interact with CODA end of run       *
 *                                                                      *
 * 9/25/01      R.M. Add a method Read3801(header, ichan) to simplify   *
 *                   clients.  At the moment it works in-time.  Make    *
 *                   SISProducer skip readout if CODA is running, since *
 *                   CODA's TS will do the readout using SISInterrupt   *
 *                                                                      *
 * 7/25/02      R.M. Use G0 Helicity structure, which has quartets and  *
 *                   a different random helicity routine                *
 *                                                                      *
 * 8/20/02      R.M. Massive simplification.  Eliminate SIS*buffer      *
 *                   and SIS*Cumulative, etc.  Keep SIS*Plus/Minus      *
 *                   cumulatives.  Mostly no args passed to functions,  *
 *                   rely mostly on globals.  Eliminate uneeded flags.  *
 *                   Interface to 3801 changed to                       *
 *                   int Read3801(int iscaler, int ihelicity, int ichan)*
 *                                                                      *
 * 3/05/03      R.M. Eliminate SISintime() and SISouttime(), create     *
 *                   SISDataPkg() to fill ring buffer and to package    *
 *                   the data online by helicity                        *
 *                                                                      *
 *10/22/03      R.M. Start using sysBusToLocalAddrs to get the VME      *
 *                   offsets.  Eliminated 1 of the 2 redundant scalers. *
 *                   The 2nd scaler was SIS2 and was easily turned      *
 *                   off by setting NUMSCALERS = 1 in SIS3801.h         *
 *                                                                      *
 * 1/6/06       R.M. System of globals (scaDebug, scaNbError, etc) to   *
 *                   facilitate debugging while deployed.               *
 *                                                                      *
 * 1/6/06       R.M. Let long_start_flag do the job of pointing to      *
 *                   num_read, while start_flag is a flag as before     *
 *                   (problem: when recovery started for large num_read *
 *                    start_flag would pick up a sign since it was a    *
 *                    short int !!!).                                   *
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
#include <time.h>

// #define INTENABLE 1

/****************************************************************************/

#include "SIS.h"
#include "SIS3801.h"

/* Externals variables used by CODA */
extern int rocActivate;   /* Means ROC is activated (set by "go") */
/*  = 0 --> not active */
/*  = +1 --> active     */
/*  = -1 --> can load words but not increment SIS counts */

extern int scaDebug;  /* to debug scalers */

extern int scaRunReady;   /* Means scaler is ready to readout 
                             (synonymous with shreg_init_flag here) */

extern int scaNbError;   /* count of the number of errors, for crl */

static struct fifo_scaler *pSIS[NUMSCALERS];
/*extern struct fifo_scaler *sfs;*/

/* my choice for initial CSR */
unsigned long mycsr = 0xff00fc00;  

/* SIS1 and SIS2 are two redundant scalers.  SIS2 was removed Sept 2003.
        SIS1Last  == last reading of SIS1.  similar for SIS2
        SIS1plus  == helicity plus data for SIS1.
        SIS1minus == helicity minus data for SIS1
*/

struct SISbuffer SIS1Last;
struct SISbuffer SIS2Last;
struct SISbuffer SIS1Previous;
struct SISbuffer SIS2Previous;

struct SISbuffer SIS1plus;
struct SISbuffer SIS1minus;
struct SISbuffer SIS2plus;
struct SISbuffer SIS2minus;

/* Buffer data for error condition -- hopefully remains empty */

struct SISbuffer SIS1bad;
struct SISbuffer SIS2bad;

/* Ring buffer of minimal data to insert into datastream */
struct SISring SIS1ring[RING_DEPTH];

/*  Global variables */

int nb_error;
unsigned  long num_read[NUMSCALERS];
unsigned  long client_read;
unsigned  long num_check;
unsigned  long last_bad = 0;

int waitime;

short start_flag=-1; /* used to know when start a quadruplet
                      start= 0 => first window is odd  buffer number
	              start= 1 => first window is even buffer number 
	              start=-1 => first window not yet found
	              This flag is calculated in the routine SISFIFO_Read */ 

unsigned long long_start_flag=0; /* same as old "start_flag" but unsigned long */

short shreg_init_flag=0; /* used to know if the shreg is initialized 
                          shreg_init=0 => Not yet initialized
		          shreg_init=1 =>  Initialized
	           This flag is calculated in the routine SISHel_Chk_Load */ 
/* Note: The shreg_init_flag is, at present, synonmous with scaRunReady 
   (extern flag) which is used by CRL to see if scalers are ready to read */
   

short end_run_flag=0;   /*  end_run_flag=1 => SISProducer is done with the 
                         keeping the cumulative counts (sees coda_end_run and 
	                 no more gates) 
                         This flag is set to 1 in the routine SISouttime */

short coda_end_run=0;   /*  coda_end_run=1 => CODA Stops the run 
                         This flag is set to 1 by the routine SISendrun 
		         Called by coda at the end of the run */

short coda_start_run=0;   /*  coda_start_run=1 => CODA Starts the run 
                           This flag is set to 1 by the routine runStartClrSIS
			   Called by coda at the start of the run */

short first_pointer=1; /* logical flag to tell if to make the first 
                          assignment of read and write pointers
                          which happens at start of run */   

short read_pointer=0;  /* location to start reading ring buffer of
                          minimal data */
short write_pointer=0;  /* location of last write of ring buffer */

int hbits[SHREG_NBIT];   /* The SHREG_NBIT shift register.
                            SHREG_NBIT (and other capital letters)
                            are define statements in header file   */

int error_cond;           /* error condition of present reading   */

int present_reading;      /* present reading of helicity  */
int predicted_reading;    /* prediction of present reading 
                             (should = present_reading) */
int present_helicity;     /* present helicity 
                             using prediction if OUT_OF_TIME_MODE
                             or using measurement if in-time mode   */
int q1_predicted_reading; /* predicted helicity at start of quad */
int q1_present_helicity;  /* present helicity (using prediction) at 
                             start of quad */

int recovery_flag = 0;   /* flag to determine if we need to recover */
                         /* from an error (1) or not (0) */

int fault_tolerant = 1;   /* permit faults (for GeN in high-rad environ) */
int ignore_err = 1;       /* dont print so many error messages */

unsigned int iseed;            /* value of iseed for present_helicity */
unsigned int iseed_earlier;    /* iseed for predicted_reading */



 

/***********************************************************************
 *                                                                     *
 * SISInterrupt()   --  Interrupt service routine                      *
 *                                                                     *
 *  If the scaler FIFO becomes too full (generally > 50%, depending on *
 *  the setup), it will call this ISR which will readout the buffer.   *
 *  The same readout routines as in SISProducer are used. The idea is  *
 *  that if SISProducer is too slow, this routine will keep up with    *
 *  the FIFO.                                                          *
 *                                                                     *
 **********************************************************************/

void  SISInterrupt(int arg) {
   
  int isca;
  int debug;
  int not_empty;

  debug = arg;
   
  if(debug) logMsg("In SIS3801 Interrupt service routine\n",0,0,0,0,0,0);
 

/* Update the FIFO NOT_EMPTY flag which is 0 if all FIFO scalers are empty */      

  not_empty =0;
  for(isca=0;isca<NUMSCALERS;isca++){
      if ((SISVMEADDR[isca] != 0) &&
         ((pSIS[isca]->csr&FIFO_EMPTY) == 0))
	   not_empty++;
  }
   
  while(not_empty != 0) {

       not_empty = SISFIFO_Read();

  }

}
 
 
 
 /***************************************************************************************
 *  initISR_SIS()            : Init the interrupt service routine 
 *       input parameters    : none    
 *       return              : 0 
 */

int initISR_SIS()
{
  int isca;
#ifdef INTENABLE

  for (isca=0; isca<NUMSCALERS; isca++){
     if(SISRELADDR[isca] != 0){
	 scalIntInit(SISRELADDR[isca]);
	 scalIntConnect(SISInterrupt,DEBUG_INT);
	 scalIntEnable(INTERRUPT_FLAG);
	 printf("Interrupt Service routine initialized using scaler %d\n",isca);
	 break;
     }
  }
#endif
  return 0;
}

  
/***************************************************************************************
 *  initSIS()                : Init SIS3801 cards
 *       input parameters    : none    
 *       return              : 0 
 */

int initSIS()
{
  int isca, res;
  unsigned long laddr;

  for (isca=0; isca<NUMSCALERS; isca++){
    SISVMEADDR[isca] = 0;
    if(SISRELADDR[isca] != 0){
       res = sysBusToLocalAdrs(0x39, SISRELADDR[isca], &laddr);
       if (res != 0) {
          printf("initSIS3801: ERROR:  sysBusToLocalAdrs address %x\n",laddr);
        } else {
	  SISVMEADDR[isca] = laddr; 
          pSIS[isca] = (struct fifo_scaler *)SISVMEADDR[isca];
          pSIS[isca]->reset=1;
          pSIS[isca]->csr=mycsr;
          pSIS[isca]->clear=1;
          enableClockSIS(isca);
	  setControlInpMode(2,isca);
          enableExtNextSIS(isca);
          enableExtDisSIS(isca);
          printf("initSIS in SIS3801[%d] at addr %x \n", isca,SISVMEADDR[isca]);  
	}
     }
  }
  ring_map[0] = 2;   /* trigger 3 */
  ring_map[1] = 6;  /* upstream BCM (x3 gain) */
  ring_map[2] = 12;  /* L1A (accepted triggers)  */
  ring_map[3] = 4;   /* trigger 5 */

  return 0;
}


/***************************************************************************************
 *  clrAllCntSIS()           : clear all counters , and overflow bit
 *       input parameters    : none    
 *       return              : 0 
 */

int clrAllCntSIS()
{
  int isca;
  for (isca=0;isca<NUMSCALERS; isca++) {
	if(SISVMEADDR[isca] != 0) pSIS[isca]->clear=1;
  }
  return 0;
 }

/************************************************************************
/* The idea of using the following three routines is this:
   In the crl code you want to 
     n = NumRing()
     for (i = 0; i < n; i++) {
       for (j = 0; j < NRING_CHANNEL+2; j++) *rol->dabufp++ = ReadRing(i,j);
     }
     ResetRing(n);
*/
/************************************************************************


/************************************************************************
*                                                                       *
*   NumRing    R. Michaels   Apr 2001                                   *
*                                                                       *
*   Purpose:  See how much data is in ring buffer                       *
*                                                                       *
*   Input args:                                                         *
*        none                                                           *
*   Return value:                                                       *
*        amount of unread data, i.e. diff between read pointer          *
*        and write pointer, modulo buffer size                          *
*                                                                       *
************************************************************************/

int NumRing() {
  if (write_pointer >= read_pointer) {
      return write_pointer - read_pointer;  
  } else {
      return RING_DEPTH + write_pointer - read_pointer; 
  }
}

/************************************************************************
*                                                                       *
*   ReadRing    R. Michaels   Apr 2001                                  *
*                                                                       *
*   Purpose:  Reads data out of the ring buffer                         *
*                                                                       *
*   Input args:                                                         *
*        loc = location in ring buffer relative to read pointer         *
*        var = variable you want                                        *
*              0 = clock                                                *
*              1 = qrt & helicity bits                                  *
*              2,3,4... = data locations                                *
*   Return value:                                                       *
*        data                                                           *
*                                                                       *
************************************************************************/

long ReadRing(int loc, int var) {
  int location;
  location = (read_pointer + loc)%RING_DEPTH;
  if (var == 0) return SIS1ring[location].clock;
  if (var == 1) return SIS1ring[location].qrt_hel;
  return SIS1ring[location].data[var-2]&DATA_MASK;
};


/************************************************************************
*                                                                       *
*   ResetRing    R. Michaels   Apr 2002                                 *
*                                                                       *
*   Purpose:  Reset the read pointer after we have read out the ring    *
*                                                                       *
*   Input args:                                                         *
*        nread = number of places to bump read_pointer                  *
*        (normally this is how many readings you took)                  *
*                                                                       *
************************************************************************/

void ResetRing(int nread) {
  read_pointer = (read_pointer + nread)%RING_DEPTH;
};


/*********************************************************************
 *  vmeNext3801(isca)                : VME next clock (twice)
 *       input parameters     : isca -- number of the scaler 
 *  

  *          return           : 0
 */
int vmeNext3801(int isca)
{
  pSIS[isca]->next=1;
  return 0;
} 


/*********************************************************************
 *  dump registers to screen
 *  But only those registers which are readable.
 */
int dumpCSR(int isca)
{

printf("  -------\n");
  printf("ADDR(%#8x) : CSR register        : %#8x \n", &pSIS[isca]->csr,pSIS[isca]->csr)  ; 
  printf("ADDR(%#8x) : VME IRQ control reg : %#8x \n", &pSIS[isca]->irq,pSIS[isca]->irq ) ; 
 }
  
/*******************************************************************************
 *   setControlInpMode(inpmode,isca)  : set configuration for control input pins
 *              input parameters     : inpmode -- type of Mode 0-3
 *                                   : isca -- number of the scaler (see the 
 *                                     SIS3801.h to know which number correspond 
 *                                     to which scaler. 
 *              return               : 0
 *
 */
 int setControlInpMode(int inpmode, int isca)
 { 
  int temp;
  
  if ((inpmode>3)||(inpmode<0)) return ERROR;
  temp=inpmode<<2;
  pSIS[isca]->csr=0x00000C00; /* clear Mode bits to Mode 0*/
  pSIS[isca]->csr=temp;       /* set new Mode */
  return 0;
 }



  
  

/***************************************************************************************
 *  enableClockSIS(isca)            : enable next clock logic  
 *       input parameters     : isca -- number of the scaler (see the 
 *                              SIS3801.h to know which number correspond 
 *                              to which scaler. 
 *
 *       return              : 0 
 */
int enableClockSIS(int isca) 
{
  pSIS[isca]->enable=1;
  return 0;
 }


/***************************************************************************************
 *  disableClockSIS(isca)           : disable next clock logic
 *       input parameters     : isca -- number of the scaler (see the 
 *                              SIS3801.h to know which number correspond 
 *                              to which scaler. 
 *       return              : 0 
 *
 *     Notes           :  This is the state after Power on or KEY_RESET 
 */
int disableClockSIS(int isca) 
{
  pSIS[isca]->disable=1;
  return 0;
 }

/*******************************************************************************
 *   enableExtNextSIS(isca)  : Enable External Next Clock
 *       input parameters     : isca -- number of the scaler 
 *  
 *              return               : 0
 *
 */
 int enableExtNextSIS(int isca)
 { 
  pSIS[isca]->csr= 0x10000; 
  return 0;
 }


/*******************************************************************************
 *   disableExtNextSIS(isca)  : Disable External Next Clock 
 *       input parameters     : isca -- number of the scaler 
 *  
 *              return               : 0
 *
 */
 int disableExtNextSIS(int isca)
 { 
  pSIS[isca]->csr= 0x1000000; 
  return 0;
 }

 /*******************************************************************************
 *   enableExtDisSIS(isca)  : Enable External Disable
 *       input parameters     : isca -- number of the scaler 
 *  
 *              return               : 0
 *  
 */
 int enableExtDisSIS(int isca)
 { 
  pSIS[isca]->csr=0x40000; 
  return 0;
 }


/*******************************************************************************
 *   disableExtDisSIS(isca)  : Disable External Disable 
 *       input parameters     : isca -- number of the scaler 
 *  
 *              return               : 0
 *
 */
 int disableExtDisSIS(int isca)
 { 
  pSIS[isca]->csr=0x4000000; 
  return 0;
 }



/***********************************************************************
 * InitBuffer3801()   --  Initialize the buffer to zero                    
 *                                                                     
 *
 */

void InitBuffer3801(void){

  int i;
  bzero((char *)&SIS1Last,       sizeof(SIS1Last));
  bzero((char *)&SIS2Last,       sizeof(SIS2Last));
  bzero((char *)&SIS1Previous,   sizeof(SIS1Previous));
  bzero((char *)&SIS2Previous,   sizeof(SIS2Previous));
  bzero((char *)&SIS1plus,       sizeof(SIS1plus));
  bzero((char *)&SIS1minus,      sizeof(SIS1minus));
  bzero((char *)&SIS2plus,       sizeof(SIS2plus));
  bzero((char *)&SIS2minus,      sizeof(SIS2minus));
  bzero((char *)&SIS1bad,        sizeof(SIS1bad));
  bzero((char *)&SIS2bad,        sizeof(SIS2bad));
  bzero((char *)&SIS1ring,       sizeof(SIS1ring));

  bzero((char *)&num_read,       sizeof(num_read));

  num_check = 0;
  for (i = 0; i < SHREG_NBIT; i++) hbits[i] = 0;
  present_reading = 0;
  predicted_reading = 0;
  present_helicity= 0;
  q1_predicted_reading = 0;
  q1_present_helicity= 0;
  iseed = 0;
  iseed_earlier = 0;
  client_read =0;
  last_bad = 0;
  nb_error = 0;
  scaNbError = 0;

  ResetErrorCond();

}



/* To dump the last SIS buffer.  For debugging. */

void dumpLastBuffer() 
{
        int isca,i,loc;

        printf("\n -------- Dump of Last Reading \n");

        printf("SIS1 Last :::
           Upper bits 0x%x  qrt %d   hbit %d\n",loc,
             SIS1Last.upbit, 
	     SIS1Last.qrt, 
  	     SIS1Last.hbit);
        for (i=0;i<NUM_CHANNEL;i++) 
            printf("SIS1 Last. data [ %d ] = %d\n",
      	         i,(SIS1Last.data[i]&DATA_MASK));

        if (NUMSCALERS >= 2) {
	printf("SIS2 Last :::
           Upper bits 0x%x  qrt %d   hbit %d\n",loc,
       	      SIS2Last.upbit, 
	      SIS2Last.qrt, 
  	      SIS2Last.hbit);
        for (i=0;i<NUM_CHANNEL;i++) 
             printf("SIS2 Last. data [ %d ] = %d\n",
    	          i,(SIS2Last.data[i]&DATA_MASK));
	}
}




/***********************************************************************
 * runStartClrSIS()   --  Initialize the buffer, the counters, and send  
 *                        a VME Next Clock.    
 *     note : Routine used at the beginnig of the run.
 */

void runStartClrSIS(void){

   int isca;

   start_flag = -1; 
   long_start_flag = 0;
   end_run_flag = 0;   
   coda_end_run = 0;   
   scaRunReady = 0;
   shreg_init_flag = 0;
   first_pointer = 1;
   coda_start_run = 1;  
   recovery_flag = 1;  
   nb_error = 0;
   scaNbError = 0;

   for (isca=0;isca<NUMSCALERS;isca++) {
      if(SISVMEADDR[isca] != 0){ 
	   disableExtNextSIS(isca);
           pSIS[isca]->clear=1;
      }
   }

   InitBuffer3801();

   for (isca=0;isca<NUMSCALERS;isca++) {
      if(SISVMEADDR[isca] != 0){ 
	   vmeNext3801(isca);
           enableExtNextSIS(isca);
       }
   }


 /*   printf("Start a new run \n"); */
}


/***********************************************************************
 * WaitAjust()   --  Ajust the Delay time                     
 *                                                                     
 *
 */

void WaitAjust(void){

   if ((2*num_read[0]>=num_check)&&(waitime>0)) {
      waitime--;
   } else if (3*num_read[0]<num_check){
      waitime++;
   }

   if(waitime<TIME_DELAY_SIS-2) waitime = TIME_DELAY_SIS-2;
   if(waitime<1) waitime = TIME_DELAY_SIS;
   if(waitime>TIME_DELAY_SIS+2) waitime = TIME_DELAY_SIS+2;
   if (rocActivate > 0) waitime = 30;

}


/***********************************************************************
 *                                                                     *
 * SISProducer()   --  Read the FIFO and put the data in the shared    *
 *	                	  Memory.                              *
 *                                                                     *
 *     This routine is spawned when the vxWorks kernel boots up.  It   *
 *     runs continuously, updating the Cumulative counts and the ring  *
 *     buffer of scaler data.  Thus, even though the scaler is run as  *
 *     a 3801 (with FIFO), the data appears like a reading of a 3800   *
 *     (traditional scaler) but with a time lag due to waitime         *
 *                                                                     *
 ***********************************************************************/

void SISProducer (void) {
   
  int i,j,k,n;
  int isca;
  int not_empty=0;
  static int cnt=0;


/* Initialize the buffer to zero */

   runStartClrSIS();
 
   waitime=TIME_DELAY_SIS;


FOREVER {
 
 WaitAjust();
   
 taskDelay(waitime); 

 if ( rocActivate == 0 ) {
     
   num_check++;
   
   if (DEBUG1) {
	printf("\n We enter in the %dth pass\n",num_check);
   }
	
/* Flush the ring buffer.  It is only needed during a CODA run. */

   if (coda_start_run == 0) {
      n = NumRing();
      ResetRing(n); 
   }

/* Update the FIFO NOT_EMPTY flag which is 0 if all FIFO scalers are empty */      

    not_empty =0;
    for(isca=0;isca<NUMSCALERS;isca++){
      if ((SISVMEADDR[isca] != 0) && 
         ((pSIS[isca]->csr&FIFO_EMPTY) == 0))
	   not_empty++;
    }
   

   while(not_empty != 0) {
	

	if (DEBUG1) 
          printf("Num_read = %d, Num_check = %d\n",num_read[0],num_check);
      

/* Disable the interrupt during the read of the FIFO */ 	   

#ifdef INTENABLE
  	scalIntDisable(); 	   
#endif
  	   
        not_empty = SISFIFO_Read();

/* Enable the interrupt after the read of the FIFO */ 	   

#ifdef INTENABLE
	scalIntEnable(INTERRUPT_FLAG); 	   
#endif
  	   
 
        if (DEBUG1) {
            dumpLastBuffer();
	}


   } /* End While not empty */

 } /* End of !rocActivate */

} /* End FOREVER */


}


/************************************************************************
*                                                                       *
*   Read3801    R. Michaels   Sept 2001                                 *
*                                                                       *
*   Purpose:  Provides and interface to the data for clients which      *
*             is simpler than addressing external structures            *
*   Input args:                                                         *
*        iscaler = 0 or 1  for the 2 scalers involved                   *
*        ihelicity = 0 or 1 for the 2 helicities                        *
*               for ihelicity = -1, read the "bad" data                 *
*        ichan  = channel number                                        *
         if chan = 100 return the number of reads                       *
*   Return value:                                                       *
*        data, if the inputs are valid                                  *
*        zero, if the inputs are not valid                              *
*                                                                       *
************************************************************************/

int Read3801(int iscaler, int ihelicity, int ichan) {

  if (iscaler == 0) {
    if (ichan >= 0 && ichan < NUM_CHANNEL) {
      if (ihelicity == 0) return SIS1minus.data[ichan];
      if (ihelicity == 1) return SIS1plus.data[ichan];
      if (ihelicity == -1) return SIS1bad.data[ichan];
    }
    if (ichan == 100) {
      if (ihelicity == 0) return SIS1minus.numread;
      if (ihelicity == 1) return SIS1plus.numread;
      if (ihelicity == -1) return SIS1bad.numread;
    }
  } else if (iscaler == 1) {
    if (ichan >= 0 && ichan < NUM_CHANNEL) {
      if (ihelicity == 0) return SIS2minus.data[ichan];
      if (ihelicity == 1) return SIS2plus.data[ichan];
      if (ihelicity == -1) return SIS2bad.data[ichan];
    }
    if (ichan == 100) {
      if (ihelicity == 0) return SIS2minus.numread;
      if (ihelicity == 1) return SIS2plus.numread;
      if (ihelicity == -1) return SIS2bad.numread;
    }
  }
  return 0;
}


/************************************************************************
*                                                                       *
*   SISFIFO_Read    R. Michaels   June 1999                             *
*                                                                       *
*   Purpose:  To read the FIFO, determine the helicity, check the       *
*   helicity, and assign data to proper location in scaler banks.       *
*                                                                       *
*   Return value:  not_empty  = 1 means FIFO not empty                  *
*                               0 means FIFO is empty                   *
*                                                                       *
************************************************************************/

int SISFIFO_Read (void) {

    int i,j,k,isca,not_empty; 

/* Get the the data from the FIFO */		

    for (isca=0;isca<NUMSCALERS;isca++){
       if((SISVMEADDR[isca] != 0) && 
         ((pSIS[isca]->csr&FIFO_EMPTY) != FIFO_EMPTY)){

     	   if (++num_read[isca]>ROLLOVER) {
	       printf("WARNING: number of reads is rolling over\n");
               num_read[isca] = 1;
   	   }

	   switch (isca) {
		
	     case 0: 
	        for(i=0;i<NUM_CHANNEL;i++) SIS1Last.data[i] = 
                                 pSIS[isca]->fifo[0];
                SIS1Last.numread   = num_read[isca];
		SIS1Last.upbit = (SIS1Last.data[0]&UPBIT_MASK)>>24;
                SIS1Last.qrt = (SIS1Last.data[0]&QRT_MASK)>>31;
                SIS1Last.hbit = (SIS1Last.data[0]&HELICITY_MASK)>>30;
        	break;
	     
	    case 1: 
		for(i=0;i<NUM_CHANNEL;i++) SIS2Last.data[i] = 
                                 pSIS[isca]->fifo[0];
	        SIS2Last.numread   = num_read[isca];
                SIS2Last.upbit = (SIS2Last.data[0]&UPBIT_MASK)>>24;
                SIS2Last.qrt = (SIS2Last.data[0]&QRT_MASK)>>31;
                SIS2Last.hbit = (SIS2Last.data[0]&HELICITY_MASK)>>30;

         	break;
         
            default:
	  }
	}
    }
   

  /* Check and load the helicities */

    SISHel_Chk_Load();


  /* Keep the cumulative counts in memory, depending on in-time
     or out-of-time mode */

    SISDataPkg();


   if (DEBUG5) {
     printf("Helicity bit %d   chan 31 hel %d\n",SIS1Last.hbit,SIS1Last.data[31]&DATA_MASK);
   }


/* After a few events been read try to know when a quad starts
 * start_flag == -1 => Start point not yet determined  
 * start_flag ==  1 => reading at which the first quad is seen 
 * long_start_flag = actual pointer to 1st data */
	

   if(OUT_OF_TIME_MODE) {

     if((start_flag < 0)&&(num_read[0] > 10)) {

	if (SIS1Last.qrt == 1) {
            long_start_flag = num_read[0];
            start_flag = 1;
	}

     }
   }	   
	

/* Update the FIFO NOT_EMPTY flag which is 0 if all FIFO scalers are empty */      
    not_empty = 0;
    for(isca=0;isca<NUMSCALERS;isca++){
       if ((SISVMEADDR[isca] != 0) 
         & ((pSIS[isca]->csr&FIFO_EMPTY) == 0))
            not_empty++;
    }

    return not_empty;

}     




/*************************************************************
*                                                            *
*  SISDataPkg     R. Michaels     Mar 2003                   *
*                                                            *
*  To load the cumulative based on the present helicity.     *
*  The present helicity depends on whether we are in         *
*  in-time mode or delayed/G0 mode.                          *
*  Note that SIS1 and SIS3 no longer pertain to scalers      *
*  but to helicities.  One scaler (isca=0) is sufficient     *
*  to retrieve the data.  The other scaler (isca=2) is       *
*  a redundant copy of identical data.                       * 
*                                                            * 
*************************************************************/ 

int SISDataPkg()
{

   int i,ignore_shreg;
   static int nint=0;

   nint++;

   if((coda_end_run)&&((SIS1Last.data[7]&DATA_MASK)==0)&&(end_run_flag==0)){
      end_run_flag=1;
      printf("end_run_flag set to %d at read # %d\n",end_run_flag,num_read[0]);
   }
   
   if (coda_start_run == 0) {
     write_pointer = 0;
     read_pointer = 0;
     first_pointer = 1;
   } else {
     write_pointer = (num_read[0]-1)%RING_DEPTH;
     if (first_pointer) {
       read_pointer = write_pointer;
       first_pointer = 0;
     } else {
       if ((write_pointer == read_pointer) && rocActivate == 1) { 
         if (OUT_OF_TIME_MODE) {  /* we dont care if the helicity is in-time */
           if (!ignore_err) printf("WARN: ring buff wrap around. DAQ rate is too low.\n");
           error_cond = 1;   
	 }
       }
     }
   }
   if (DEBUG2) printf("coda_start_run %d  end_run_flag %d\n",
		      coda_start_run, end_run_flag);

   if (DEBUG3 || (DEBUG4==1 && shreg_init_flag==1)) {
     printf("SISDataPkg -- pointers %d %d %d %d\n",
       first_pointer,num_read[0],write_pointer,read_pointer); 
     printf("end_run_flag %d  rocActivate %d shreg_init_flag %d  error_cond %d \n",end_run_flag, rocActivate, shreg_init_flag, error_cond);
     printf("helicity %d\n",present_helicity);
   }


   for (i=0; i < NRING_CHANNEL; i++) {
        SIS1ring[write_pointer].data[i] = SIS1Last.data[ring_map[i]];
   }
   SIS1ring[write_pointer].clock = SIS1Last.data[7]&DATA_MASK;
   SIS1ring[write_pointer].qrt_hel = ((SIS1Last.qrt<<4) | SIS1Last.hbit);


   /*   printf("stuff %d %d %d %d \n",end_run_flag,rocActivate,error_cond,shreg_init_flag);*/
   /*   if (error_cond != 0) printf("PROBLEM with error_cond flag = %d\n",error_cond);  */

   ignore_shreg = 0;
   if (error_cond != 0 && fault_tolerant ) {
       ignore_shreg = 1;
       scaNbError++;
       error_cond = 0;
   }

   if (scaDebug & (nint%101)==0) printf("SISDataPkg state %d %d %d %d %d %d %d %d %d %d %d\n",rocActivate,scaRunReady,shreg_init_flag,ignore_shreg,error_cond,OUT_OF_TIME_MODE,present_helicity,end_run_flag,long_start_flag,num_read[0],SIS1Last.qrt);


   if ( (rocActivate == 0) || ((rocActivate > 0) && (scaRunReady > 0))) {

    if (end_run_flag==0 || rocActivate==0) {
   
     if (error_cond == 0) {

        if ( (shreg_init_flag==1) || (OUT_OF_TIME_MODE==0) || (ignore_shreg) ) {

	  if (present_helicity == 1) { 

	    for (i=0;i<NUM_CHANNEL;i++) 
               SIS1plus.data[i]  += (SIS1Previous.data[i]&DATA_MASK);

	    SIS1plus.numread  = SIS1Previous.numread;

            if (NUMSCALERS > 1) {
	    for (i=0;i<NUM_CHANNEL;i++) 
               SIS2plus.data[i]  += (SIS2Previous.data[i]&DATA_MASK);

	    SIS2plus.numread  = SIS2Previous.numread;
	    }

	 } else {

	    for (i=0;i<NUM_CHANNEL;i++) 
               SIS1minus.data[i]  += (SIS1Previous.data[i]&DATA_MASK);

	    SIS1minus.numread  = SIS1Previous.numread;

            if (NUMSCALERS > 1) {
  	      for (i=0;i<NUM_CHANNEL;i++) 
                 SIS2minus.data[i]  += (SIS2Previous.data[i]&DATA_MASK);

	      SIS2minus.numread  = SIS2Previous.numread;
	    }
	 }
	}

      } else {     /* Problematic Scaler Readout */

        if((!ignore_err) && (nint%50)==0) printf("WARNING: Lots of bad scaler data (helicity problems)\n");

	for (i=0;i<NUM_CHANNEL;i++) 
           SIS1bad.data[i]  += (SIS1Previous.data[i]&DATA_MASK);
        SIS1bad.numread  = SIS1Previous.numread;

        if (NUMSCALERS > 1) {
	  for (i=0;i<NUM_CHANNEL;i++) 
             SIS2bad.data[i]  += (SIS2Previous.data[i]&DATA_MASK);
          SIS2bad.numread  = SIS2Previous.numread;
	}
      }
    }
   }

   for(i=0;i<NUM_CHANNEL;i++) SIS1Previous.data[i] = SIS1Last.data[i];
   SIS1Previous.numread = SIS1Last.numread;
   SIS1Previous.upbit = SIS1Last.upbit; 
   SIS1Previous.qrt = SIS1Last.qrt;
   SIS1Previous.hbit = SIS1Last.hbit;

   if (NUMSCALERS > 1) {
     for(i=0;i<NUM_CHANNEL;i++) SIS2Previous.data[i] = SIS2Last.data[i];
     SIS2Previous.numread = SIS2Last.numread;
     SIS2Previous.upbit = SIS2Last.upbit; 
     SIS2Previous.qrt = SIS2Last.qrt;
     SIS2Previous.hbit = SIS2Last.hbit;
   }

   return 0;
}


/************************************************************
*                                                           *
*  ResetErrorCond    R. Michaels   June 1999                *
*                                                           *
*  To reset the error condition flag in the Cumulative      *
*  buffers.  It can only be reset here.  This way the client*
*  is forced to see and reset this flag, which identifies   *
*  periods of bad data                                      *
*                                                           *
************************************************************/ 

int ResetErrorCond(void){
    bzero((char *)&SIS1bad,        sizeof(SIS1bad));
    bzero((char *)&SIS2bad,        sizeof(SIS2bad));
    return 0;
}


/************************************************************
*                                                           *
*  SIScodaendrun     R. Tieulent   July 1999                *
*                                                           *
*  Routine call by CODA                                     *
*  Set flag which tells to the Producer that CODA had been  *
* stop the run.                                             *
*                                                           *
************************************************************/ 

int SIScodaendrun()
{

   coda_end_run=1;
   coda_start_run=0;
   printf("SIS3801:  nb_error = %d  scaNbError = %d \n",nb_error,scaNbError);
/* printf("coda_end_run set to %d at read # %d\n",coda_end_run,num_read[0]);*/

}     



/***********************************************************************
 *                                                                     *
 *  SISHel_Chk_Load    R. Michaels    June 1999                        *
 *       (modified July 2002 for G0 mode)                              *
 *                                                                     *
 *  To check the helicities of the two helicity scalers.               *
 *  The number of reads must be similar (within a tolerance).          *
 *  The helicities must be the same if the number of reads is the same.*
 *  For delayed helicity mode, the measured helicity must agree with   *
 *  the prediction, when this prediction is available                  *
 *  For the measurement of helicity, SIS1 is used.                     *
 *  For the in-time mode, the present helicity is taken from the       *
 *  measurement.                                                       *
 *  The result of this code is to set error_cond flag and to           *
 *  load present_helicity.  If delayed mode it also loads              *
 *  predicted_helicity.                                                *
 *                                                                     *
 **********************************************************************/

int SISHel_Chk_Load()
{

    int i;
    int ok;
   
    error_cond = 0;

    
/* Sept 2003, removed the redundant 2nd scaler (NUMSCALER == 1 now) */
    if (NUMSCALERS > 1) {

      ok = 1;
    
      if(num_read[0]<num_read[1]) {
          if((num_read[1]-num_read[0])>NREADTOL) ok = 0;
      }
      if(num_read[0]>num_read[1]) {
          if((num_read[0]-num_read[1])>NREADTOL) ok = 0;
      }
      if(!ok) {
         error_cond += 1;
         if (!ignore_err) printf("Helicity Check:  Inconsistent num reads in 2 scalers\n"); 
      }

      if((num_read[0]==num_read[1])&&(SIS1Last.hbit!=SIS2Last.hbit)) {    
          error_cond += 2;
	  /*        printf("chk %d %d %d %d\n",num_read[0],num_read[1],SIS1Last.hbit,SIS2Last.hbit); */
          if (!ignore_err) printf("Helicity Check : Not same helicity in both scaler\n");
      }
    }
 
    if(DEBUG3 || (DEBUG4==1 && shreg_init_flag==1)) {
        printf("SISHel_Chk_Load, num_read[0],[1] %d %d\n",
            num_read[0],num_read[1]);
        printf("helicity 1 and 2 %d %d \n",SIS1Last.hbit,SIS2Last.hbit);
    }

    present_reading = SIS1Last.hbit;  /* choose one of the two scalers. */

    /*    if ((num_read[0]%100) == 0) {
      printf("scaRunReady = %d \n",scaRunReady);
      } */

    if(OUT_OF_TIME_MODE) {

       if ( SIS1Last.qrt == 0 && start_flag < 0 && last_bad != 0 && num_read[0]-last_bad > 10) {
         if (!ignore_err & ((num_read[0]-last_bad)%100 == 0)) {
   	  printf("ERROR: Scaler online packaging stuck. QRT cable fell out ?\n");
          printf("Try restarting a run (probably works).  Or reboot VME.\n"); 

          printf("Flags %d %d %d %d %d %d %d %d %d\n",long_start_flag, end_run_flag, coda_end_run, scaRunReady, shreg_init_flag, first_pointer, coda_start_run, recovery_flag,num_read[0]);         
	 }
       }

       if ( SIS1Last.qrt == 1 && start_flag > 0) {
	   
          if (loadHelicity() == 1) { /* finished loading */       

         /* Shift reg is ready and scalers can be read out now.  */
         /* The first time of a run, this takes a few sec    */

	    shreg_init_flag = 1;
            q1_present_helicity = present_helicity;      
            q1_predicted_reading = predicted_reading;

	  }
       }


        if (shreg_init_flag != 0 && start_flag > 0) {  

         /* G0 quad structure  (0,1,1,0) or (1,0,0,1) */
          if( ((num_read[0]-long_start_flag)%4 == 1) || 
              ((num_read[0]-long_start_flag)%4 == 2) ) {
                 predicted_reading = 1 - q1_predicted_reading;
                 present_helicity = 1 - q1_present_helicity;
	  }

	  if( (num_read[0]-long_start_flag)%4 == 3 ) {
                 predicted_reading = q1_predicted_reading;
                 present_helicity = q1_present_helicity;
	  }

          if (present_reading != predicted_reading) {  /* error ! */
  	      nb_error++;
              if ( !ignore_err ) printf("ERROR in predicted helicity. You are losing data !  %d %d \n",present_reading,predicted_reading);
              last_bad = num_read[0];
	      if (DEBUG4) { 
                printf("Event %d   long_start_flag %d\n",num_read[0],long_start_flag);
                printf("%d %d %d %d %d \n",SIS1Last.qrt,present_helicity,q1_predicted_reading, predicted_reading,present_reading);
	      } 
              error_cond += 4;
	      recovery_flag = 1;  /* ask for recovery */
              start_flag = -1;
              long_start_flag = 0;
              shreg_init_flag = 0; 
	  } else {
	    if (DEBUG3 || (DEBUG4==1 && shreg_init_flag==1)) 
                 printf("--> Helicity prediction ok %d \n",
                    present_helicity);
	  }
       }

    } else {  

/*  For in-time mode the present_helicity is what we measure  */

       present_helicity = present_reading;  

    }


    if(DEBUG3 || (DEBUG4==1 && shreg_init_flag==1)) 
       printf("SISHel_Chk_Load results:
         error_cond %d   num_read[0] %d   shreg_init_flag %d
         present_reading %d  predicted_reading %d   start_flag %d \n",
	 error_cond, num_read[0], shreg_init_flag, 
	 present_reading, predicted_reading, start_flag);

    return 1;
}



/****************************************************************/
/* This is the random bit generator according to the G0         */
/* algorithm described in "G0 Helicity Digital Controls" by     */
/* E. Stangland, R. Flood, H. Dong, July 2002.                  */
/* Argument:                                                    */
/*        which = 0 or 1                                        */
/*     if 0 then iseed_earlier is modified                      */
/*     if 1 then iseed is modified                              */
/* Return value:                                                */
/*        helicity (0 or 1)                                     */
/****************************************************************/

int ranBit(int which) {

#define IB1 1            /* Bit 1     */
#define IB3 4            /* Bit 3     */
#define IB4 8            /* Bit 4     */
#define IB24 8388608     /* Bit 24    */
#define MASK 8388621     /* IB1+IB3+IB4+IB24 */

  if (which == 0) {
     if(iseed_earlier & IB24) {    
         iseed_earlier = ((iseed_earlier^MASK)<<1) | IB1;
         return 1;
     } else  { 
         iseed_earlier <<= 1;
         return 0;
     }
  } else {
     if(iseed & IB24) {    
         iseed = ((iseed^MASK)<<1) | IB1;
         return 1;
     } else  { 
         iseed <<= 1;
         return 0;
     }
  }

};


/* **************************************************************/
/* getSeed                                                      */
/* Obtain the seed value from a collection of SHREG_NBIT bits.  */        
/* This code is the inverse of ranBit.                          */
/* Input:                                                       */
/*   int hbits[SHREG_NBIT]  -- global array of bits of          */
/*                             shift register                   */
/* Return:                                                      */
/*   seed value                                                 */
/* **************************************************************/


unsigned int getSeed() {
  int i,seedbits[SHREG_NBIT];
  unsigned int ranseed = 0;
  if (SHREG_NBIT != 24) {
    printf("ERROR: SHREG_NBIT is not 24.  This is unexpected.\n");
    return 0;      /* Awkward, but you probably won't care.*/
  }
  for (i = 0; i < 20; i++) seedbits[23-i] = hbits[i];
  seedbits[3] = hbits[20]^seedbits[23];
  seedbits[2] = hbits[21]^seedbits[22]^seedbits[23];
  seedbits[1] = hbits[22]^seedbits[21]^seedbits[22];
  seedbits[0] = hbits[23]^seedbits[20]^seedbits[21]^seedbits[23];
  for (i=SHREG_NBIT-1; i >= 0; i--) ranseed = ranseed<<1|(seedbits[i]&1);
  ranseed = ranseed&0xFFFFFF;
  return ranseed;
}



/* **************************************************************/
/* loadHelicity()                                               */   
/* Loads the helicity to determine the seed.                    */
/* After loading (nb==SHREG_NBIT), predicted helicities are     */
/*                                 available.                   */
/* **************************************************************/

int loadHelicity() {
  int i,dummy;
  static int nb;
  if (recovery_flag) nb = 0;
  /*  printf("into loadHelicity  %d %d %d %d\n",recovery_flag,nb,iseed_earlier,iseed);
  printf("hbits %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n",hbits[0],hbits[1],hbits[2],hbits[3],hbits[4],hbits[5],hbits[6],hbits[7],hbits[8],hbits[9],hbits[10],hbits[11],hbits[12],hbits[13],hbits[14],hbits[15],hbits[16],hbits[17],hbits[18],hbits[19],hbits[20],hbits[21],hbits[22],hbits[23]);
  */
  recovery_flag = 0;
  if (nb < SHREG_NBIT) {
      hbits[nb] = present_reading;
      nb++;
      return 0;
  } else if (nb == SHREG_NBIT) {   /* Have finished loading */
      iseed_earlier = getSeed();
      iseed = iseed_earlier;
      for (i = 0; i < SHREG_NBIT+1; i++) {
           predicted_reading = ranBit(0);
           dummy = ranBit(1);
      }
      for (i = 0; i < NDELAY; i++)
          present_helicity = ranBit(1);
      nb++;
      return 1;
  } else {      
      predicted_reading = ranBit(0);
      present_helicity = ranBit(1);
      return 1;
  }

}










