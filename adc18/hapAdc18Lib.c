/* Library for JLab 18bit ADC for HAPPEX             */
/* R. Michaels, Nov 2006                             */

#include "vxWorks.h"
#include "stdio.h"
#include "string.h"
#include "logLib.h"
#include "taskLib.h"
#include "intLib.h"
#include "iv.h"
#include "semLib.h"
#include "vxLib.h"

#include "hapAdc18Lib.h"

/* default ADC config bits */
#define DEFAULT_CONFIG_BITS 0xC00

/* function prototypes */

/* Define memory state (power up condition) */
void adc18_zeromem();
/* Initialize board pointers */
int adc18_init(int id, UINT32 adc_addr, UINT32 data_addr);
/* Reset the ADC */
int adc18_reset(int id);
/* Soft reset of the ADC */
int adc18_softreset(int id);
/* Set timing if internal source */
int adc18_timesrc(int id, int source, int ramp, int window);
/* Set gate mode */
int adc18_gatemode(int id, int which);
/* Set DAC value (once) */
int adc18_setdac(int id, long dac_value);
/* Load 256K DAC values */
int adc18_loaddac(int id, int choice);
/* Set integrator gain */
int adc18_intgain(int id, long gain);
/* Set V-to-C conversion */
int adc18_setconv(int id, long conv);
/* Set # samples */
int adc18_setsample(int id, int nsample);
/* To see if ADC is busy converting (1) or not (0) */
int adc18_Busy(int id);
/* Print the ADC setup */
int adc18_print_setup(int id);
/* Get one event (initiates DAQ sequence) */
int adc18_go(int id);
/* Get event word count */
long adc18_getcnt(int id);
/* Get CRL */
long adc18_getcsr(int id);
/* Get next in FIFO */
unsigned long adc18_getdata(int id);   
/* Get all data and print it */
int adc18_testdata(int id);
/* print & decode registers and data word */
void adc18_reg_decode(int id);
void adc18_reg_status(int id);
void adc18_decode_data(int id, unsigned long data);
/* check that the ID is within range, etc */
int adc18_chkid(int id);
/* default setup */
int adc18_defaultSetup(int id);
/* interface for rapid setup */
int adc18_setup(int id, int time, int intgain, int conv);


void adc18_zeromem() {
/* This has to be done when you reboot the VME board, to define zero memory */
  int id;
  t_mode = 0;
  for (id = 0; id < ADC18_MAXBOARDS; id++) {
    adc18p[id] = 0;
    adc18_data[id] = 0;
    adc18_ok[id] = 0;
  }
}

int adc18_init(int id, UINT32 adc_addr, UINT32 data_addr) {

/* Initialize board pointers : adc at adc_addr and data at data_addr */
/* This can be done when you reboot VME, but at least before you run */
 
  unsigned long laddr;
  int res, rdata;

  adc18_ok[id] = 0;
  if (adc18_chkid(id) == -1) return -1;
  printf("Initializing ADC18 board %d now ... \n",id);

  sysBusToLocalAdrs(0x39,adc_addr,&laddr);
  printf("ADC18 board %d  register local address = 0x%x\n",id,laddr);  
  adc18p[id] = (struct adc18_struct *)laddr;
  adc18p[id]->mod_id = id;
  res = vxMemProbe((char *) &(adc18p[id]->mod_id), VX_READ, 4, (char *)&rdata);
  if (res < 0) {
    printf("ADC18:ERR: No addressable board %d  at addr 0x%x\n",id,adc_addr);
    return -1;
  } else {
    printf("ADC18:Init: Board ID = %d \n",rdata);
  }
  sysBusToLocalAdrs(0x39,data_addr,&laddr);
  printf("ADC18 board %d   data local address = 0x%x\n",id,laddr);  
  adc18_data[id] = (volatile unsigned long *)laddr;

  adc18_ok[id] = 1;
  adc18_reset(id);

  return 0;

}


int adc18_reset(int id) {
/* Clear the ADC, like a power up condition.
   In CODA do this in download.  */
  if (adc18_chkid(id) < 1) return -1;
  adc18p[id]->csr = 0x100;  /* hard reset */
  adc18p[id]->config = 0;
  adc18p[id]->dac_value = 0;
  adc18p[id]->delay_1 = 0;
  adc18p[id]->delay_2 = 0;
  return 0;
}

int adc18_softreset(int id) {
  if (adc18_chkid(id) < 1) return -1;
  adc18p[id]->csr = 0x200;  /* soft reset */
  return 0;
}


int adc18_chkid(int id) {
/* id is the index of the board (id = 0,1,2... ) for N boards in a crate.
   This routine checks that 'id' is within range and whether
   the module was initialized and configured.
      returns: 
          -1  -->  wrong id index
           0  -->  not yet initialized
           1  -->  board was initialized (adc18_init)
           2  -->  board was configured (config nonzero)
 */
  if (id < 0 || id >= ADC18_MAXBOARDS) {
    printf("ADC18:ERR:  Board id %d is outside range !!\n",id);
    return -1;
  }
  if (adc18_ok[id] == 0) {
    printf("ADC18:chkid:  Board id %d unitiailzed. \n",id);
    return 0;
  }
  if (adc18p[id]->config != 0) return 2;  
  return 1;
}


int adc18_timesrc(int id, int source, int ramp, int window) {

// Set the timing source for ADC #id
//    source == 0 means external, 1 is internal
//    the other parameters only matter if source = internal.
//    then, ramp & window are the ramp delay and integration window.

  float set_window;

  if (adc18_chkid(id) < 1) return -1;

  if (source != 0 && source < 1) {
    printf("ADC18:ERR:  Time source may be 0 or 1 only \n");
    return -1;
  }

  if( source == 0 ) {
     t_mode = 0;			/* external timing sequence */
     printf("\nUse external timing module to generate timing sequence\n\n");
  } 
  else {
    t_mode = 1;    		/* internal timing sequence */
    printf("\nUse internal timer to generate timing sequence\n\n");
    adc18p[id]->delay_1 = 4;		/* set ramp to 10 us */
    if( (window >= 1) && (window <= 163840) ) {
      set_window = ( ( ((float)window) + TIME_INT ) / 2.5 ) + 0.5;
    } 
    else {
      printf("!!! ERROR in input value (use default value of 100 us)\n");    
      window = 100;    
      set_window = ( ( ((float)window) + TIME_INT ) / 2.5 ) + 0.5;
    }
    printf("  Integration window = %d usec \n",window);
    printf("\n     Integration count value = %d\n", (long)set_window);    
    adc18p[id]->delay_2 = (long)set_window;
  }
  
}

long adc18_getcsr(int id) {
   
  return adc18p[id]->csr;

}


long adc18_getcnt(int id) {
/* Get event word count */
  long event_word_count;

  if (adc18_chkid(id) < 1) return 0;

  event_word_count = 0xFF & (adc18p[id]->evt_word_ct);
  return event_word_count;
}


unsigned long adc18_getdata(int id) {
/* It's assumed the user has already called adc18_getcnt */
/* Then this routine gets data out of the FIFO  */
/* Usage:
/*    Let N = adc18_getcnt(id) 
          for (i = 0; i < N; i++) {
             mydata = adc18_getdata(id);  // pop off FIFO
          }
*/

  if (adc18_chkid(id) < 1) return -1;

  return *adc18_data[id];
}

void adc18_reg_decode(id)
{
      long value, ii;
      long divider, div_n;

      if (adc18_chkid(id) < 1) printf("ADC18:ERR: cannot decode registers\n");
	
      value = adc18p[id]->mod_id;
	
      printf("\nModule ID = %X   Version = %X\n", (value & 0x1F), ((value >> 8) & 0xFF) );

      value = adc18p[id]->config;
	
      printf("Integration gain factor (0-3) = %d\n", ((value >> 1) & 0x3) );

      printf("V-to-C conversion gain (0-15) = %d\n", ((value >> 4) & 0xF) );
		
      if( value & 0x1 )
           printf("Integration gate FORCED OPEN\n");
      else
           printf("SWITCHING integration gate\n");
	    
      if( value & 0x400 )
           printf("Raw data output\n");
      else
           printf("Raw data suppressed from output\n");
           	
      if( value & 0x800 )
           printf("DAC value output\n");
      else
           printf("DAC value suppressed from output\n");
	    	
      div_n = ( (0x3) & (value >> 8) );
      divider = 1;
      for(ii = 0; ii < div_n; ii++)
          divider = divider * 2;
	    
      printf("%d samples each for baseline & peak\n",divider);

      value = adc18p[id]->csr;
	
      if( value & 0x1 )
           printf("EVENT AVAILABLE in output buffer\n");
	    	
      if( value & 0x2 )
           printf("Output buffer EMPTY\n");
	    	
      if( value & 0x4 )
           printf("Output buffer FULL\n");
	    	
      if( value & 0x8 )
           printf("Power O.K.\n");
      else
           printf("Power NOT O.K.\n");
	    	
      if( value & 0x10 )
           printf("Channel 1 ADC BUSY asserted\n");
      if( value & 0x20 )
           printf("Channel 2 ADC BUSY asserted\n");
      if( value & 0x40 )
           printf("Channel 3 ADC BUSY asserted\n");
      if( value & 0x80 )
           printf("Channel 4 ADC BUSY asserted\n");
	    	
      value = adc18p[id]->ctrl;
       
      if( value & 0x1 )
           printf("GO asserted\n");
      else
           printf("GO NOT asserted\n");
	    
      if( value & 0x2 )
           printf("Force DAC = 0\n");
	
      if( value & 0x4 )
	{
	    printf("Self Trigger ENABLED\n");
	    value = adc18p[id]->delay_2;
	    printf("Integration Window = %.2f us\n",((float)(value & 0xFFFF)) * 2.5);
	}
	
      value = adc18p[id]->dac_value;
      printf("Current DAC value (hex) = %LX\n", (value & 0xFFFF));
       
      value = adc18p[id]->evt_ct;
      printf("Event Count = %d\n", (value & 0x3FFFFFF));
	
      value = adc18p[id]->evt_word_ct;
      printf("Event Word Count = %d\n", (value & 0xFF));
	
}

int adc18_Busy(int id) {
  /* To check if the ADC is busy.  
         Returns 1 if busy                   
         Retruns 0 if not busy and event ready */

  /* To be fast I'll assume 'id' is valid (has been checked) */

      long value;
      int event_avail_flag = 0x1;
      int busy_flag = 0xf0;

      value = adc18p[id]->csr;
	
      if ( value & event_avail_flag && !(value & busy_flag) ) return 0;
      return 1;
}


void adc18_reg_status(int id)
{
      if (adc18_chkid(id) < 1) printf("ADC18:ERR: cannot print reg status\n");
      printf("\nmod_id (hex) = %lx\n", (adc18p[id]->mod_id));
      printf("csr (hex) = %lx\n", 0xFFFF & (adc18p[id]->csr));
      printf("ctrl (hex) = %lx\n", 0xFFFF & (adc18p[id]->ctrl));
      printf("config (hex) = %lx\n", 0xFFFF & (adc18p[id]->config));
      printf("event_count = %ld\n", 0xFFFF & (adc18p[id]->evt_ct));
      printf("event_word_count = %ld\n", 0xFF & (adc18p[id]->evt_word_ct));
      printf("dac_value (hex) = %lx\n", 0xFFFF & (adc18p[id]->dac_value));
      printf("delay_1 = %ld\n", 0xFFFF & (adc18p[id]->delay_1));
      printf("delay_2 = %ld\n\n", 0xFFFF & (adc18p[id]->delay_2));
}

int adc18_gatemode(int id, int which) {
/* Set gate mode 
      which == 0   --> switch
      which == 1   --> open
*/
  long config;
  if (adc18_chkid(id) < 1) return -1;

  if (which != 0 && which < 1) {
      printf("ADC18:ERR:  Gate mode must be a zero or one \n!");
      return -1;
  }
   
  config = (0xfffffffe & adc18p[id]->config);
 
  config |= which;
  config |= DEFAULT_CONFIG_BITS;

  adc18p[id]->config = config;
  return 0;

}

int adc18_loaddac(int id, int dac_choice) {
/* Load a pattern of 256K dac values */

  int i;
  int dac_min = 20000;
  int dac_max = 60000;
  int dac_const;
  int ncnt = 256000;
  int sign = 1;
  int dac_value = dac_min;

  int UPDOWN=1;
  int JAGSAW=2;
  int CONSTVAL=3;

  if (dac_choice != UPDOWN && dac_choice != JAGSAW && dac_choice != CONSTVAL) {
    printf("ERROR:adc18_loaddac: Could not find your dac_choice.\n");
    dac_choice = CONSTVAL;
  }
  if (dac_choice==UPDOWN) printf("Using ramping up and down DAC\n");
  if (dac_choice==JAGSAW) printf("Using discont. sawtooth DAC\n");
  if (dac_choice==CONSTVAL) {
      dac_const = 0.5*(dac_max + dac_min);
      printf("Using constant DAC = %d \n",dac_const);
  }

  if (adc18_chkid(id) < 1) return -1;

  adc18p[id]->ctrl = 0;  /* set GO = 0 */

   for (i = 0; i <= ncnt; i++) {  
    
     if (dac_choice == UPDOWN) { //for steady up & down ramping 
        if (dac_value > dac_max) sign = -1;
        if (dac_value < dac_min) sign = 1;
        dac_value = dac_value + sign*10;
     }

     if (dac_choice == JAGSAW) { //for discontinuous sawtooth function
        dac_value = dac_value + sign*10;
        if (dac_value > dac_max) dac_value = dac_min;
     }

    //constant value
     if (dac_choice == CONSTVAL) dac_value = dac_const;
    
    if (i == 0) dac_value |= 0x10000;
    if (i == ncnt) dac_value |= 0x20000;
    adc18p[id]->dac_memory = dac_value;

   }

   return 0;

}



int adc18_setdac(int id, long dac_value) {
/* Set DAC value */

  if (adc18_chkid(id) < 1) return -1;

  adc18p[id]->ctrl = 0;  /* set GO = 0 */

   /* assert first & last flags */
  adc18p[id]->dac_memory = 0x30000 | dac_value;	

  return 0;
}

int adc18_intgain(int id, long gain) {
/* Set integrator gain factor   */
/* note: a gain of '3' is minimum, '0' is max (in terms of result per volt) */

  long config;
     
  if (adc18_chkid(id) < 1) return -1;

  printf("adc18_intgain  adc %d   gain = %d \n",id,gain);

  if (gain < 0 || gain > 3) {
      printf("ADC18:ERR:  int_gain is outside of range. \n",id,gain);
      return -1;
  }
   
  config = adc18p[id]->config;
 
  config |= gain << 1;
  config |= DEFAULT_CONFIG_BITS;

  adc18p[id]->config = config;
  return 0;
}

int adc18_setconv(int id, long conv) {
/* Set V-to-C conversion */
/* The gain is linear in 'conv' */

  long config;

  if (adc18_chkid(id) < 1) return -1;

  printf("adc18_setconv  adc %d   conv = %d \n",id,conv);

  if (conv < 0 || conv > 15) {
      printf("ADC18:ERR:  vc_gain is outside of range. \n");
      return -1;
  }
   
  config = adc18p[id]->config;
 
  config |= conv << 4;
  config |= DEFAULT_CONFIG_BITS;

  adc18p[id]->config = config;
  return 0;
}

int adc18_setsample(int id, int nsample) {
/* Set # samples on baseline and peak */

  long config;
  int n_mode;

  if (adc18_chkid(id) < 1) return -1;

  printf("adc18_setsample  adc %d   sample = %d \n",id,nsample);

  switch (nsample) {

    case 1:
      printf("Using sample by 1 \n");
      n_mode=0;
      break;
    case 2:
      printf("Using sample by 2 \n");
      n_mode=1;
      break;
    case 4: 
      printf("Using sample by 4 \n");
      n_mode=2;
      break;
    case 8:
      printf("Using sample by 8 \n");
      n_mode=2;
      break;
    default:
      printf("ADC18:ERR: nsample can only be 1, 2, 4, or 8 \n");
      return -1;

  }
   
  config = adc18p[id]->config;
 
  config |= n_mode << 8;
  config |= DEFAULT_CONFIG_BITS;

  adc18p[id]->config = config;
  return 0;
}


int adc18_go(int id) {
  // Initialize one DAQ sequence

  // ADC must be configured
   if (adc18_chkid(id) != 2) return -1;  


   adc18p[id]->ctrl = 0 | (t_mode << 2);
   adc18p[id]->ctrl = 1 | (t_mode << 2);	/* go */

   if (t_mode) adc18p[id]->csr = 0x8000;

   return 0;
}


int adc18_print_setup(int id) {
/*   Print the ADC setup   */

  int i;
  long nmax;
  unsigned long rdata;

  printf("ADC id %d   chkid = %d \n",id,adc18_chkid(id));

  if (adc18_chkid(id) < 1) {
       printf("ADC is not initalized !\n");
       return -1;
  }

  adc18_reg_status(id);
  
  adc18_reg_decode(id);

  nmax = adc18_getcnt(id);
  printf("Num of words in ADC %d \n",nmax);
 
  if (nmax > 5000) {
       printf("WARNING: num words too big !\n");
       nmax = 5000;
  }

  for (i = 0; i < nmax; i++) {
    rdata = adc18_getdata(id);    
    printf("Data [%d]  =  %d  =  0x%x \n",i,rdata,rdata);
  }

  return 0;

}

int adc18_setup(int id, int time, int intgain, int conv) {

  adc18_init(id,0xed000,0xed100);  /* VME address of board */

  if (time < 0 || time > 163840) {
    printf("ERROR:  time must be 0 to 163840 usec \n");
    return -1;
  }
  if (intgain < 0 || intgain > 3) {
    printf("ERROR:  integain must be 0 to 3 \n");
    return -1;
  }
  if (conv < 0 || conv > 15 ) {
    printf("ERROR:  V-to-C conversion must be 0 to 15 \n");
    return -1;
  }
   
  adc18_reset(id);    
  adc18_timesrc(id,1,0, time);  
  adc18_gatemode(id,1);            
  adc18_intgain(id,intgain);  
  adc18_setconv(id,conv);

  adc18_print_setup(id);
}  


int adc18_defaultSetup(int id) {

  adc18_init(id,0xed000,0xed100);  /* VME address of board */

  adc18_reset(id);    
  adc18_timesrc(id,1,0,30000);     /* Internal time source, window = 30 msec */
  adc18_gatemode(id,1);            
  adc18_intgain(id,3);  
  adc18_setconv(id,0);

  adc18_print_setup(id);

  return 0;
}

void adc18_decode_data(int id, unsigned long data)
{
    unsigned long module_id, event_number, ch_number, divider, div_n, data_type;
    unsigned long diff_value, sample_number, raw_value, dac_value;
    long sign, difference, ii;
    double diff_avg;
    int lprint = 1;
		
    if( data & 0x80000000 ) {		/* header */
        module_id = (0x1F) & (data >> 26);
        event_number = data & 0x3FFFFFF;
        if( lprint )
          printf("\n**** HEADER (%LX)  ID = %LX  event number = %d\n", 
          		  data, module_id, event_number);

      } else {		 /* data */

        ch_number = (0x3) & (data >> 29);
        div_n = ( (0x3) & (data >> 25) );
        divider = 1;
        for(ii = 0; ii < div_n; ii++)
              divider = divider * 2;
        data_type = (0x7) & (data >> 22 );
          
        if( data_type == 4 ) {	/* DAC value */
            dac_value = 0xFFFF & data;
	    if( lprint )	        
	     	printf("     DAC (%LX)  value = %d\n", data, dac_value);	                   
	}
	    	        
	if( data_type == 0 ) {
	     diff_value = (0x1FFFFF) & data;
	     if( data & 0x200000 ) {
	        sign = -1;
	        difference = sign * ((~diff_value & 0x1FFFFF) + 1);	    /* 2's complement */
	     } else {
	        sign = 1;
	        difference = diff_value;
	     }
	     diff_avg = ((float)difference)/((float)divider);
	     if( lprint ) printf("     DIFF(peak-base) (%LX)  CH = %d  DIV = %d  SIGN = %d  DIFF/DIV = %.3f\n", 
	    	  data, ch_number, divider, sign, diff_avg);
	     
              } else if( data_type == 1 ) {

	         sample_number = (0xF) & (data >> 18);
	         raw_value = 0x3FFFF & data;
	         if( lprint ) printf("     RAW(base) (%LX)  CH = %d  sample = %d  value = %d\n", 
	    		      data, ch_number, sample_number, raw_value);	                   
	    	} else if( data_type == 2 ) {
	          sample_number = (0xF) & (data >> 18);
	          raw_value = 0x3FFFF & data;
	          if( lprint ) printf("     RAW(peak) (%LX)  CH = %d  sample = %d  value = %d\n", 
	    		     	data, ch_number, sample_number, raw_value);	                   
	    	}
      }
}

int adc18_testdata(int id) {

  /* This is an example of how a trigger routine might work.
     Get one event, then print it out.  (A trigger routine
     would not print but only put data to a buffer.) */

  int i;
  long nmax;
  unsigned long rdata;

  if (adc18_chkid(id) < 1) {
       printf("ADC is not initalized !\n");
       return -1;
  }

  adc18_go(id);

  nmax = adc18_getcnt(id);
  printf("Num of words in ADC %d \n",nmax);
 
  if (nmax > 5000) {
       printf("WARNING: num words too big !\n");
       nmax = 5000;
  }

  for (i = 0; i < nmax; i++) {
    rdata = adc18_getdata(id);    
    printf("Data [%d]  =  %d  =  0x%x \n",i,rdata,rdata);
    adc18_decode_data(id, rdata);
  }

  return 0;

}
