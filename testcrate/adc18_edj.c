/*  "adc_test12.c"  ---- Test Hall A 18-bit ADC ---- */

/*  E.J.  8/14/2006  -  add option to write raw data to file */ 
/*        10/9/2006  -  prompt to specify # samples */ 

#include <stdioLib.h>
#include <usrLib.h>
#include <stdlib.h>
#include <taskLib.h>
#include <math.h>
#include <stddef.h>

#define ADC_REG  0x0ED000	/* base address of 18-bit ADC register space */
#define ADC_DATA 0x0ED100	/* base address of 18-bit ADC data space */	

#define TIME_MOD 0xB0B0		/* base address of timer module */

#define DATA_FILENAME "/adaqfs/halla/adev/hapc6/test_data_file.dat"	

#define PRINT_EVENT   1		/* print data/stats from each event */	
#define PRINT_RAW     1		/* print raw samples from each event (if PRINT_EVENT = 1) */	
#define PRINT_DAC     1		/* print DAC value for each event (if PRINT_EVENT = 1) */	
#define PRINT_STUDIES 1		/* print additional studies */	

#define NTRIG   1000		/* number of triggers to generate in a set */	

#define TIME_INT 11.0		/* sample time (us) for internal timing sequence */	
#define TIME_EXT 9.0		/* sample time (us) for external timing sequence */
	

struct time_struc {
  short ext_in;		/* 00 */
  short blank_1;		/* 02 */
  short blank_2;		/* 04 */
  short dac_12;		/* 06 */
  short dac_16;		/* 08 */
  short ramp;		/* 0C */
  short integrate;	/* 0E */
  short oversample;	/* 10 */
};
		 

struct adc_struc {
  long mod_id;		/* 00 */
  long csr;		/* 04 */
  long ctrl;		/* 08 */
  long cfg_adc;		/* 0C */
  long evt_ct;		/* 10 */
  long evt_word_ct;	/* 14 */
  long dac_value;	/* 18 */
  long reserved_1;	/* 1C */
  long dac_memory;	/* 20 */
  long spare[3];		/* 24,28,2C */
  long delay_1;		/* 30 */
  long delay_2;		/* 34 */
};
		 
void decode_data(unsigned long data);
void reg_decode(void);
void reg_status(void);
int readout(void);
int trig_gen(void);
void init_raw(void);
void event_stats_raw(void);
void event_stats(unsigned long ch, unsigned long base_peak, unsigned long x);
void init_event_stats(void);
void print_event_stats(void);
void init_stats(void);
void stats(long n_stat, unsigned long ch, float y);
void print_stats(long n_stat);
void hist_init(long mode);
void bin(long n_hist, float y);
void hist_print(long n_hist);
void print_studies(void);
void write_data_raw(FILE *fd, long n_trig, long n_samples);
long write_setup(FILE *fd);

unsigned long print_event;

unsigned long raw_data[4][2][8];	/* [i][j][k] = [ch_n][base/peak][sample_n] */
unsigned long raw_n[4][2];		/* [i][j] = [ch_n][base/peak] */
unsigned long raw_sum[4][2];		/* [i][j] = [ch_n][base/peak] */
		
volatile unsigned long xnum[4][2];	/* for event stats */
volatile double xavg[4][2];
volatile double x2avg[4][2];
volatile double xvar[4][2];
volatile unsigned long xmin[4][2];
volatile unsigned long xmax[4][2];

volatile unsigned long ynum[20][4];	/* for run stats */
volatile double yavg[20][4];
volatile double y2avg[20][4];
volatile double yvar[20][4];
volatile float ymin[20][4];
volatile float ymax[20][4];

volatile long hist[8][200];		/* for histograms */
volatile long h_sum[8];
volatile long h_under[8];
volatile long h_over[8];
volatile float h_avg0[8] = { 25800.,22300.,22300.,22600.,29500.,17000.,13000.,17800. };  
/* volatile float h_range[8] = { 200.,200.,200.,200.,400.,400.,400.,400. }; */  
volatile float h_range[8] = { 200.,200.,200.,200.,200.,200.,200.,200. };  
volatile float bin_size[8];  
volatile float h_min[8];  
volatile float h_max[8];

volatile float peak_avg[4][1000];	/* store data for systematics study */
  
volatile struct adc_struc *adc;
volatile unsigned long *adc_data;
		 
		 
void adc_test12()
{
  volatile struct time_struc *time_mod;
  long event_word_count;
  unsigned long laddr;
  unsigned long read_value, dac_error_count, ii, data_value;
  long key_value, dac_value, reg_value;
  long n_mode, t_mode, g_mode, int_gain, vc_gain;
  long readout_task, trig_task;
  long n_trig;
  long n_samples = 0;
  long file_write;
  int id_1, id_2;
  STATUS status;
  FILE *fd_1;
  float window;
  char data_filename[200] = "/adaqfs/halla/adev/hapc6/adc18test";
  char extension[10] = ".dat";
  char data_name[50];
  char next[18];
  char *gets();
	
  init_stats();			/* initialize run statistics */
  hist_init(0);			/* initialize histograms */
	
  print_event = PRINT_EVENT;

  /*  Motorola PPC board VME addresses for 18-bit ADC (A24/D32 transfers)  */
  sysBusToLocalAdrs(0x39,ADC_REG,&laddr);
  printf("ADC register local address = %x\n",laddr);  
  adc = (struct adc_struc *)laddr;
        
  sysBusToLocalAdrs(0x39,ADC_DATA,&laddr);
  printf("ADC data local address = %x\n",laddr);  
  adc_data = (volatile unsigned long *)laddr;

  /*  Motorola PPC board VME addresses for time mod (A16/D16 transfers)  */
  sysBusToLocalAdrs(0x29,TIME_MOD,&laddr);
  printf("timer module local address = %x\n",laddr);  
  time_mod = (struct time_struc *)laddr;
        
  adc->csr = 0x100;		/*  initialize adc board (csr bit 8)  */
	
  printf("\nRegister values after board initialization\n");  
  taskDelay(1);			
  reg_status();
	
  printf("\nEnter timing sequence source: 0 = external,  1 = internal - ");
  scanf("%ld",&key_value);
  if( key_value == 0 )
    {
      t_mode = 0;			/* external timing sequence */
      time_mod->ramp = 4;		/* set ramp to 10 us */
      printf("\nUse external timing module to generate timing sequence\n\n");
      printf("Enter integration window (1 - 163840 us) - ");
      scanf("%ld",&key_value);
      if( (key_value >= 1) && (key_value <= 163840) )
	{
	  window = ( ( ((float)key_value) + TIME_EXT ) / 2.5 ) + 0.5;
	  time_mod->integrate = (short)window;
	}    
      else
	{
	  printf("!!! ERROR in input value (use default value of 100 us)\n");    
	  key_value = 100;    
	  window = ( ( ((float)key_value) + TIME_EXT ) / 2.5 ) + 0.5;
	  time_mod->integrate = (short)window;
	}    
      printf("\n     Integration count value = %d\n", (short)window);    
    }    
  else
    {
      t_mode = 1;    		/* internal timing sequence */
      adc->delay_1 = 4;		/* set ramp to 10 us */
      printf("\nUse internally generated timing sequence\n\n");
      printf("Enter integration window (1 - 163840 us) - ");
      scanf("%ld",&key_value);
      if( (key_value >= 1) && (key_value <= 163840) )
	{
	  window = ( ( ((float)key_value) + TIME_INT ) / 2.5 ) + 0.5;
	  adc->delay_2 = (long)window;
	}    
      else
	{
	  printf("!!! ERROR in input value (use default value of 100 us)\n");    
	  key_value = 100;    
	  window = ( ( ((float)key_value) + TIME_INT ) / 2.5 ) + 0.5;
	  adc->delay_2 = (long)window;
	}    
      printf("\n     Integration count value = %d\n", (long)window);    
    }
       	    
  printf("\nEnter integration gate mode: 0 = switching,  1 = always open - ");
  scanf("%ld",&key_value);
  if( key_value == 1 )
    g_mode = 1;
  else
    g_mode = 0;    
       	    
  printf("\nEnter DAC value (hex: 0-FFFF) - ");
  scanf("%lx",&key_value);
  if( (key_value >= 0) && (key_value <= 0xFFFF) )
    dac_value = 0xFFFF & key_value;
  else
    {
      printf("!!! ERROR in input value (use default value of 0x8000)\n");    
      dac_value = 0x8000;
    } 
  adc->dac_memory = 0x30000 | dac_value;		/* assert first & last flags */

  printf("\nEnter integration gain factor (0-3) - ");
  scanf("%ld",&key_value);
  if( (key_value >= 0) && (key_value <= 3) )
    int_gain = key_value;
  else
    {
      printf("!!! ERROR in input value (use default value of 3)\n");    
      int_gain = 3;
    }
	 
  printf("\nEnter V-to-C conversion gain (voltage source only) (0-15) - ");
  scanf("%ld",&key_value);
  if( (key_value >= 0) && (key_value <= 15) )
    vc_gain = key_value;
  else
    {
      printf("!!! ERROR in input value (use default value of 15)\n");    
      vc_gain = 15;
    }
	
  printf("\nEnter # samples per measurement (1,2,4,8) - ");
  scanf("%ld",&key_value);
  if( key_value == 1 )
    n_mode = 0;
  else if( key_value == 2 )
    n_mode = 1;
  else if( key_value == 4 )
    n_mode = 2;
  else if( key_value == 8 )
    n_mode = 3;
  else    
    {
      printf("!!! ERROR in input value (use default value of 1)\n");    
      n_mode = 0;
    }
	 
  file_write = 0;
  printf("\nWrite raw data to file? ( 1 = yes, 0 = no ) - ");
  scanf("%ld",&key_value);
  if( key_value == 1 )
    {
      printf("Enter raw data file name (.dat appended):\n");
      scanf("%s",data_name);
      strcat(data_filename, data_name);
      strcat(data_filename, extension);	
      printf("\nData output file name = %s\n\n",data_filename);		
      file_write = 1;
      fd_1 = fopen(data_filename,"w");
    }    
	    
	 
  adc->mod_id = 0xA;		/* set module ID = 'A' */	  
  adc->cfg_adc = (0xC00) | (n_mode << 8) | (vc_gain << 4) | (int_gain <<1) | g_mode ;		
  adc->ctrl = 0 | (t_mode << 2);
  adc->ctrl = 1 | (t_mode << 2);	/* go */
	
  readout_task = 0;        
  trig_task = 0;        
		       
  if( t_mode == 0 )		/* spawn readout process if external timing mode */
    {
      readout_task = 1;
      id_1 = taskSpawn("rd",100,0,20000,readout,0,0,0,0,0,0,0,0,0,0);
    }    
	    
  printf("\nConfigured register values\n");
  taskDelay(1);			
  reg_status();

  while (1)
    {
      printf("\nEnter: 'r <CR>' for register status,     'R <CR>' for register decode\n");
      printf(  "       't <CR>' trigger single internal timing sequence\n");
      printf(  "       'a <CR>' trigger 100 timing sequences to adjust histogram limits (7 sec)\n");
      printf(  "       'm <CR>' trigger 1000 timing sequences (66 sec)\n");
      printf(  "       'T <CR>' trigger continuous 30 Hz internal timing sequence\n");
      printf(  "       'H <CR>' halt 30 Hz internal timing sequence\n");
      printf(  "       'p <CR>' to print running stats\n");
      printf(  "       'c <CR>' to clear stats\n");
      printf(  "       'i <CR>' to initialize board\n");
      printf(  "       'c <CR>' to clear stats\n");
      gets(next);
      if( *next == 'q' )
	{
	  if( readout_task )	
	    status = taskDelete(id_1); 		/* delete readout task if active */
	  if( trig_task )	
	    status = taskDelete(id_2); 		/* delete trigger generate task if active */
	  print_stats(2);    
	  print_stats(3);    
	  print_stats(4);    
	  print_stats(5);    
	  print_stats(0);    
	  print_stats(1);    
	  print_stats(6);    
	  print_stats(10);    
	  print_stats(11);    
	  for(ii = 0; ii < 8; ii++)
	    hist_print(ii);
	  if( file_write )
	    fclose(fd_1);  
	  break;
	}
      else if( *next == 'p' )
	{
	  print_stats(2);    
	  print_stats(3);    
	  print_stats(4);    
	  print_stats(5);    
	  print_stats(0);    
	  print_stats(1);    
	  print_stats(6);    
	  print_stats(10);    
	  print_stats(11);    
	  for(ii = 0; ii < 8; ii++)
	    hist_print(ii);    
	}
      else if( *next == 'c' )
	{
	  init_stats();    
	  init_event_stats();    
	  hist_init(1);			/* clear histograms (except expected averages) */
	}
      else if( *next == 't' )		/* single trigger */
	{
	  adc->csr = 0x8000;		/* trigger timing sequence */
	  taskDelay(3);			/* wait for sequence to finish */
	  printf("csr = %lx\n", 0xFFFF & (adc->csr));
	  printf("event_count = %ld\n", 0xFFFF & (adc->evt_ct));
	  event_word_count = 0xFF & (adc->evt_word_ct);
	  stats(11, 0, (float)event_word_count);	    	                
	  printf("event_word_count = %ld\n", event_word_count);
	  if( (event_word_count < 0) || (event_word_count > 100) ) 
	    event_word_count = 100;
	  init_raw();			/* initialize raw data array */
	  for(ii = 0; ii < event_word_count; ii++)
	    {
	      data_value = *adc_data;
	      decode_data(data_value);
	    }    
	  event_stats_raw();		/* compute & print stats for raw data */
	}	    
      else if( *next == 'm' )
	{
	  if( file_write )
	    {
	      printf("\n***** Raw data will be written to file *****\n\n");
	      n_samples = write_setup(fd_1);	/* write adc setup parameters to file */
	    }    
	  n_trig = 1;
	  while( n_trig <= NTRIG)		/* NTRIG triggers @ ~15 Hz */
	    {
	      if( (n_trig == 1) || (n_trig == NTRIG) )	/* print stats for first & last events */
		print_event = 1;
	      else
		print_event = PRINT_EVENT;		/* enforce normal print status */
	      adc->csr = 0x8000;		/* trigger timing sequence */
	      taskDelay(3);		/* wait 50 ms for sequence to finish */
	      event_word_count = 0xFF & (adc->evt_word_ct);
	      stats(11, 0, (float)event_word_count);	    	                
	      if( (event_word_count < 0) || (event_word_count > 100) ) 
		event_word_count = 100;
	      init_raw();			/* initialize raw data array */
	      for(ii = 0; ii < event_word_count; ii++)
		{
		  data_value = *adc_data;
		  decode_data(data_value);
		}
	      event_stats_raw();		/* compute & print stats for raw data */
	      peak_avg[0][n_trig - 1] = xavg[0][1];	/* store peak average values */
	      peak_avg[1][n_trig - 1] = xavg[1][1];	
	      peak_avg[2][n_trig - 1] = xavg[2][1];	
	      peak_avg[3][n_trig - 1] = xavg[3][1];
	      if( file_write )
		write_data_raw(fd_1, n_trig, n_samples);     /* write raw data to file */
	      n_trig++;
	      taskDelay(1);		/* wait 16.6 ms between sequences */
	    }
	  print_event = PRINT_EVENT;	/* enforce normal print status when done */
	  print_stats(2);    
	  print_stats(3);    
	  print_stats(4);    
	  print_stats(5);    
	  print_stats(0);    
	  print_stats(1);    
	  print_stats(6);    
	  print_stats(10);    
	  print_stats(11);    
	  for(ii = 0; ii < 8; ii++)
	    hist_print(ii);
	  if( PRINT_STUDIES )    
	    print_studies();        
	}	    
      else if( *next == 'a' )
	{
	  n_trig = 1;
	  while( n_trig <= 100)		/* NTRIG triggers @ ~15 Hz */
	    {
	      if( (n_trig == 1) )		/* print stats for first event */
		print_event = 1;
	      else
		print_event = PRINT_EVENT;		/* enforce normal print status */
	      adc->csr = 0x8000;		/* trigger timing sequence */
	      taskDelay(3);		/* wait 50 ms for sequence to finish */
	      event_word_count = 0xFF & (adc->evt_word_ct);
	      stats(11, 0, (float)event_word_count);	    	                
	      if( (event_word_count < 0) || (event_word_count > 100) ) 
		event_word_count = 100;
	      init_raw();			/* initialize raw data array */
	      for(ii = 0; ii < event_word_count; ii++)
		{
		  data_value = *adc_data;
		  decode_data(data_value);
		}
	      event_stats_raw();		/* compute & print stats for raw data */
	      n_trig++;
	      taskDelay(1);		/* wait 16.6 ms between sequences */
	    }
	  print_event = PRINT_EVENT;	/* enforce normal print status when done */
	  print_stats(2);    
	  print_stats(3);    
	  print_stats(4);    
	  print_stats(5);    
	  print_stats(0);    
	  print_stats(1);    
	  print_stats(6);    
	  print_stats(10);    
	  print_stats(11);    
	  hist_init(1);			/* clear histograms (except expected averages) */
	  init_stats();    
	  init_event_stats();    
		        
	}	    
      else if( *next == 'T' )
	{
	  id_1 = taskSpawn("rd",100,0,20000,readout,0,0,0,0,0,0,0,0,0,0);    
	  readout_task = 1;			/* trigger generate task active */    	        
	  id_2 = taskSpawn("trig",100,0,20000,trig_gen,0,0,0,0,0,0,0,0,0,0);
	  trig_task = 1;			/* trigger generate task active */    	        
	}
      else if( *next == 'H' )
	{
	  status = taskDelete(id_2);	/* delete trigger generate task */
	  trig_task = 0;			/* trigger generate task not active */    	        
	}
      else if( *next == 'r' )
	{
	  printf("\nCurrent register values\n");
	  reg_status();
	}		
      else if( *next == 'R' )
	{
	  reg_decode();
	}		
      else if( *next == 'i' )
	{
	  printf("\nInitialize board\n");
	  adc->csr = 0x100;		/*  initialize adc board (csr bit 8)  */
	}		
	       
    }
	        	        
}

void decode_data(unsigned long data)
{
  unsigned long module_id, event_number, ch_number, divider, div_n, data_type;
  unsigned long diff_value, sample_number, raw_value, dac_value;
  long sign, difference, ii;
  double diff_avg;
		
  if( data & 0x80000000 )		/* header */
    {
      module_id = (0x1F) & (data >> 26);
      event_number = data & 0x3FFFFFF;
      if( print_event )
	printf("\n**** HEADER (%LX)  ID = %LX  event number = %d\n", 
	       data, module_id, event_number);
    }
  else				/* data */
    {
      ch_number = (0x3) & (data >> 29);
      div_n = ( (0x3) & (data >> 25) );
      divider = 1;
      for(ii = 0; ii < div_n; ii++)
	divider = divider * 2;
      data_type = (0x7) & (data >> 22 );
	    
      if( data_type == 4 )	/* DAC value */
	{
	  dac_value = 0xFFFF & data;
	  if( print_event && PRINT_DAC )	        
	    printf("     DAC (%LX)  value = %d\n", data, dac_value);	                   
	  stats(10, 0, (float)dac_value);	    	                
	}
	    
      /* -------------------------------------------------------------------------------- */
      {
	    	        
	if( data_type == 0 )
	  {
	    diff_value = (0x1FFFFF) & data;
	    if( data & 0x200000 )
	      {
		sign = -1;
		difference = sign * ((~diff_value & 0x1FFFFF) + 1);	    /* 2's complement */
	      }	
	    else
	      {
		sign = 1;
		difference = diff_value;
	      }
	    diff_avg = ((float)difference)/((float)divider);
	    if( print_event )    
	      printf("     DIFF(peak-base) (%LX)  CH = %d  DIV = %d  SIGN = %d  DIFF/DIV = %.3f\n", 
		     data, ch_number, divider, sign, diff_avg);
	    stats(0, ch_number, diff_avg);	    	                
	  }
	else if( data_type == 1 )
	  {
	    sample_number = (0xF) & (data >> 18);
	    raw_value = 0x3FFFF & data;
	    if( print_event && PRINT_RAW )
	      printf("     RAW(base) (%LX)  CH = %d  sample = %d  value = %d\n", 
		     data, ch_number, sample_number, raw_value);	                   
	    raw_data[ch_number][0][sample_number - 1] = raw_value;	/* copy to raw array */
	    raw_n[ch_number][0] = sample_number;			                   
	  }
	else if( data_type == 2 )
	  {
	    sample_number = (0xF) & (data >> 18);
	    raw_value = 0x3FFFF & data;
	    if( print_event && PRINT_RAW )
	      printf("     RAW(peak) (%LX)  CH = %d  sample = %d  value = %d\n", 
		     data, ch_number, sample_number, raw_value);	                   
	    raw_data[ch_number][1][sample_number - 1] = raw_value;	/* copy to raw array */
	    raw_n[ch_number][1] = sample_number;			                   
	  }
      }
      /* -------------------------------------------------------------------------------- */
	    	        
    }    	

}

int readout(void)
{
  unsigned long data_value;
  long event_available, event_word_count, ii;

  while( 1 )
    {
      taskDelay(1);
      event_available = (0x1) & (adc->csr);	/* check if event is available */
      if( event_available )
	{
	  event_word_count = (0xFF) & (adc->evt_word_ct);
          printf("readout: event word cnt %d\n",event_word_count);
	  stats(11, 0, (float)event_word_count);	    	                
	  init_raw();		/* initialize raw data array */
	  for(ii = 0; ii < event_word_count; ii++)
	    {
	      data_value = *adc_data;
	      decode_data(data_value);		/* decode, copy, & print data */
	    }    
	  event_stats_raw();			/* compute & print stats for raw data */
	}	    
    }

}

int trig_gen(void)
{
  unsigned long data_value;
  long event_available, event_word_count, ii;

  while( 1 )
    {
      taskDelay(2);
      adc->csr = 0x8000;			/* trigger timing sequence */
    }

}

void init_raw(void)
{
  long ii,jj,kk;
	
  for(ii = 0; ii < 4; ii++)
    {
      for(jj = 0; jj < 2; jj++)
	{
	  raw_n[ii][jj] = 0;
	  for(kk = 0; kk < 8; kk++)
	    raw_data[ii][jj][kk] = 0;
	}	    
    }
}

void event_stats_raw(void)
{
  unsigned long ii,jj,kk;
  unsigned long x;
	
  init_event_stats();		/* initialize event stats */
	
  for(ii = 0; ii < 4; ii++)	/* go through entire raw data array for the event */
    {
      for(jj = 0; jj < 2; jj++)
	{
	  if( raw_n[ii][jj] != 0 )
	    {
	      for(kk = 0; kk < raw_n[ii][jj]; kk++)
		{
		  x = raw_data[ii][jj][kk];
		  event_stats(ii,jj,x);
		}
	    }        
	}	    
    }

  print_event_stats();			/* print stats */

}

void event_stats(unsigned long ch, unsigned long base_peak, unsigned long x)
{
  double ratio;
  double xx, xavg0, x2avg0, num;
	
  xnum[ch][base_peak] = xnum[ch][base_peak] + 1;
  xx = (double)x;
  num = (double)xnum[ch][base_peak];
  ratio = (num - 1.0)/num; 
  xavg0 = xavg[ch][base_peak];
  x2avg0 = x2avg[ch][base_peak];
  xavg[ch][base_peak] = ratio * xavg0  + xx/num;
  x2avg[ch][base_peak] = ratio * x2avg0 + (xx * xx)/num;
  xvar[ch][base_peak] = x2avg[ch][base_peak] - xavg[ch][base_peak] * xavg[ch][base_peak];  
	
  if( x < xmin[ch][base_peak] )
    xmin[ch][base_peak] = x;
	    
  if( x > xmax[ch][base_peak] )
    xmax[ch][base_peak] = x;
	    
  raw_sum[ch][base_peak] = raw_sum[ch][base_peak] + x;   
	
}	

void init_event_stats(void)
{
  long ii,jj,kk;
	
  for(ii = 0; ii < 4; ii++)
    {
      for(jj = 0; jj < 2; jj++)
	{
	  raw_sum[ii][jj] = 0;
	  xnum[ii][jj] = 0;
	  xavg[ii][jj] = 0.0;
	  x2avg[ii][jj] = 0.0;
	  xvar[ii][jj] = 0.0;
	  xmin[ii][jj] =  999999;
	  xmax[ii][jj] = 0;
	}	    
    }
}
	    	
void print_event_stats(void)
{
  long diff_sum, ii;
  float diff_avg[4];
  float f_value;

  if( print_event )
    printf("\nStatistics from raw samples\n");
		
  for(ii = 0; ii < 4; ii++)
    {
      if( raw_n[ii][0] != 0 )
	{
	  stats(2, ii, (float)(xavg[ii][0]));	/* base average distribution */
	  bin(ii, (float)(xavg[ii][0]));
	  f_value = (float)(sqrt(xvar[ii][0]));
	  stats(3, ii, f_value);			/* base sigma distribution */
	    	
	  stats(4, ii, (float)(xavg[ii][1]));	/* peak average distribution */
	  bin(ii + 4, (float)(xavg[ii][1]));
	  f_value = (float)(sqrt(xvar[ii][1]));
	  stats(5, ii, f_value);			/* peak sigma distribution */
	    	
	  diff_sum = (long)(raw_sum[ii][1]) - (long)(raw_sum[ii][0]);
	  diff_avg[ii] = ((float)(diff_sum))/((float)(raw_n[ii][0]));
	        
	  stats(1, ii, diff_avg[ii]);		/* average difference distribution */

	  stats(6, ii, (diff_avg[0] - diff_avg[ii]));	/* average difference from channel 0 */
	        
	  if( print_event )
	    {
	      printf("Ch %d  SUM(BASE) = %d  SUM(PEAK) = %d  DIFF_SUM(PEAK-BASE) = %d  DIFF_AVG = %.3f\n",
		     ii, raw_sum[ii][0], raw_sum[ii][1], diff_sum, diff_avg[ii] );
	        	
	      printf("   BASE:  AVG = %.3f  SIG = %.3f  SIG_AVG = %.3f  MIN = %d  MAX = %d  MAX-MIN = %d\n",
		     xavg[ii][0], sqrt(xvar[ii][0]), sqrt(xvar[ii][0])/sqrt(((float)(raw_n[ii][0]))), 
		     xmin[ii][0], xmax[ii][0], xmax[ii][0] - xmin[ii][0] );
	        	
	      printf("   PEAK:  AVG = %.3f  SIG = %.3f  SIG_AVG = %.3f  MIN = %d  MAX = %d  MAX-MIN = %d\n",
		     xavg[ii][1], sqrt(xvar[ii][1]), sqrt(xvar[ii][1])/sqrt(((float)(raw_n[ii][1]))),
		     xmin[ii][1], xmax[ii][1], xmax[ii][1] - xmin[ii][1] );
	        	    	    
	      printf("\n");
	    }
	        	
	}
    }
	
  if( print_event )
    printf("\n");
    	
}

void stats(long n_stat, unsigned long ch, float y)
{
  double ratio;
  double yy, yavg0, y2avg0, num;
	
  ynum[n_stat][ch] = ynum[n_stat][ch] + 1;
  yy = (double)y;
  num = (double)ynum[n_stat][ch];
  ratio = (num - 1.0)/num; 
  yavg0 = yavg[n_stat][ch];
  y2avg0 = y2avg[n_stat][ch];
  yavg[n_stat][ch] = ratio * yavg0  + yy/num;
  y2avg[n_stat][ch] = ratio * y2avg0 + (yy * yy)/num;
  yvar[n_stat][ch] = y2avg[n_stat][ch] - yavg[n_stat][ch] * yavg[n_stat][ch];  
	
  if( y < ymin[n_stat][ch] )
    ymin[n_stat][ch] = y;
  if( y > ymax[n_stat][ch] )
    ymax[n_stat][ch] = y;
	    
}

void init_stats(void)
{
  long ii,jj;
	
  for(ii = 0;ii < 20;ii++)	/* initialize running statistics */
    {
      for(jj = 0;jj < 4;jj++)	
	{
	  ynum[ii][jj] = 0;
	  yavg[ii][jj] = 0.0;
	  y2avg[ii][jj] = 0.0;
	  yvar[ii][jj] = 0.0;
	  ymax[ii][jj] = -999999.0;
	  ymin[ii][jj] =  999999.0;
	}	
    }
	
}
	
void print_stats(long n_stat)
{
  unsigned long ii, jj, ch_n;
  float events;
	   
  switch(n_stat)
    {
    case 0:
      printf("----- Statistics: (FPGA) DIFFERENCE (PEAK AVERAGE - BASE AVERAGE) ------\n");
      break;
    case 1:
      printf("----- Statistics: (COMPUTE) DIFFERENCE (PEAK AVERAGE - BASE AVERAGE) ------\n");
      break;
    case 2:
      printf("----- Statistics: BASE AVERAGE -----\n");
      break;
    case 3:
      printf("----- Statistics: BASE STANDARD DEVIATION -----\n");
      break;
    case 4:
      printf("----- Statistics: PEAK AVERAGE -----\n");
      break;
    case 5:
      printf("----- Statistics: PEAK STANDARD DEVIATION -----\n");
      break;
    case 6:
      printf("----- Statistics: (COMPUTE) DIFFERENCE from Channel 0 -----\n");
      break;
    case 10:
      printf("----- Statistics: DAC VALUE -----\n");
      break;
    case 11:
      printf("----- Statistics: NUMBER OF DATA WORDS -----\n");
      break;
    }       
 
  if( n_stat < 10 )		/* statistics per channel */
    {
      for(jj = 0;jj < 4;jj++)
	{
	  ch_n = jj;
	  printf("Ch %d  AVG = %.3f  SIG = %.3f  min = %.3f  max = %.3f  max-min = %.3f  (num = %d)\n",
		 ch_n, yavg[n_stat][ch_n], sqrt(yvar[n_stat][ch_n]), ymin[n_stat][ch_n], 
		 ymax[n_stat][ch_n], (ymax[n_stat][ch_n] - ymin[n_stat][ch_n]), ynum[n_stat][ch_n]); 
	}
      printf("\n");
    }
  else				/* common statistics */
    {
      printf("AVG = %.3f  SIG = %.5f  min = %.3f  max = %.3f  max-min = %.3f  (num = %d)\n",
	     yavg[n_stat][0], sqrt(yvar[n_stat][0]), ymin[n_stat][0], 
	     ymax[n_stat][0], (ymax[n_stat][0] - ymin[n_stat][0]), ynum[n_stat][0]); 
      printf("\n");
	
    }    
	
}

void reg_decode(void)
{
  long value, ii;
  long divider, div_n;
	
  value = adc->mod_id;
	
  printf("\nModule ID = %X   Version = %X\n", (value & 0x1F), ((value >> 8) & 0xFF) );

  value = adc->cfg_adc;
	
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

  value = adc->csr;
	
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
	    	
  value = adc->ctrl;
	
  if( value & 0x1 )
    printf("GO asserted\n");
  else
    printf("GO NOT asserted\n");
	    
  if( value & 0x2 )
    printf("Force DAC = 0\n");
	
  if( value & 0x4 )
    {
      printf("Self Trigger ENABLED\n");
      value = adc->delay_2;
      printf("Integration Window = %.2f us\n",((float)(value & 0xFFFF)) * 2.5);
    }
	
  value = adc->dac_value;
  printf("Current DAC value (hex) = %LX\n", (value & 0xFFFF));
	
  value = adc->evt_ct;
  printf("Event Count = %d\n", (value & 0x3FFFFFF));
	
  value = adc->evt_word_ct;
  printf("Event Word Count = %d\n", (value & 0xFF));
	
}

void reg_status(void)
{
  printf("\nmod_id (hex) = %lx\n", (adc->mod_id));
  printf("csr (hex) = %lx\n", 0xFFFF & (adc->csr));
  printf("ctrl (hex) = %lx\n", 0xFFFF & (adc->ctrl));
  printf("cfg_adc (hex) = %lx\n", 0xFFFF & (adc->cfg_adc));
  printf("event_count = %ld\n", 0xFFFF & (adc->evt_ct));
  printf("event_word_count = %ld\n", 0xFF & (adc->evt_word_ct));
  printf("dac_value (hex) = %lx\n", 0xFFFF & (adc->dac_value));
  printf("delay_1 = %ld\n", 0xFFFF & (adc->delay_1));
  printf("delay_2 = %ld\n\n", 0xFFFF & (adc->delay_2));
}

void hist_init(long mode)
{
  long ii, jj;
	
  for(ii=0; ii<8; ii++)
    {
      if( mode == 0 )		/* for initial setup */
	{
	  h_min[ii] = h_avg0[ii] - (h_range[ii]/2.0);
	  h_max[ii] = h_avg0[ii] + (h_range[ii]/2.0);
	  bin_size[ii] = h_range[ii]/200.0;
	}
      h_under[ii] = 0;
      h_over[ii] = 0;
      h_sum[ii] = 0;
      for(jj=0; jj<200; jj++)
	hist[ii][jj] = 0;
    }
	
  if( mode == 1)			/* for adjustment of histogram centers */
    {
      for(ii=0; ii<4; ii++)	/* go through channels */
	{
	  h_avg0[ii] = (float)( (long)(yavg[2][ii]) );	/* base average (nearest whole number) */ 
	  h_min[ii] = h_avg0[ii] - (h_range[ii]/2.0);
	  h_max[ii] = h_avg0[ii] + (h_range[ii]/2.0);
	        
	  h_avg0[ii+4] = (float)( (long)(yavg[4][ii]) );	/* peak average (nearest whole number) */ 
	  h_min[ii+4] = h_avg0[ii+4] - (h_range[ii+4]/2.0);
	  h_max[ii+4] = h_avg0[ii+4] + (h_range[ii+4]/2.0);
	}
    }    
	        
}

void bin(long n_hist, float y)
{
  long nbin;
	
  if( y < h_min[n_hist] )
    h_under[n_hist]++;
  else if( y >= h_max[n_hist] )
    h_over[n_hist]++;
  else
    {
      nbin = (long)( (y - h_min[n_hist])/bin_size[n_hist] );
      hist[n_hist][nbin]++;
      h_sum[n_hist]++;
    }
	
}    

void hist_print(long n_hist)
{				/* print histogram contents and do statistics */
  long ii,jj;
  long bin_1;
  double x, xx, avg, sig, sum_0, sum_1, sum_2, factor;

  sum_0 = 0.0;
  sum_1 = 0.0;
  sum_2 = 0.0;
	
  for(ii=0; ii<200; ii++)			/* do statistics for histogram (200 bins) */		
    {
      factor = (double)(ii) * (double)(bin_size[n_hist]);
      x = factor * (double)(hist[n_hist][ii]);
      xx = factor * factor * (double)(hist[n_hist][ii]);
      sum_0 = sum_0 + (double)(hist[n_hist][ii]);
      sum_1 = sum_1 + x;
      sum_2 = sum_2 + xx;
    }
	
  if( sum_0 == 0. )
    {
      avg = 999999.0;
      sig = 999999.0;
    }
  else
    {
      factor = 0.0;
      if( bin_size[n_hist] > 1.0 )		/* use bin center if bin size is > 1 count */
	factor = 0.5;    
      avg = sum_1/sum_0;
      sig = sqrt( (sum_2/sum_0) - (avg * avg) );
      avg = avg + ((double)h_min[n_hist]) + (factor * (double)bin_size[n_hist]);  /* actual value */
    }        
	
  switch(n_hist)
    {
    case 0:
      printf("\n------------------------ Histogram 0  Channel 0 Base -------------------------------------------\n");     
      break;
    case 1:
      printf("\n------------------------ Histogram 1  Channel 1 Base -------------------------------------------\n");     
      break;
    case 2:
      printf("\n------------------------ Histogram 2  Channel 2 Base -------------------------------------------\n");     
      break;
    case 3:
      printf("\n------------------------ Histogram 3  Channel 3 Base -------------------------------------------\n");     
      break;
    case 4:
      printf("\n------------------------ Histogram 4  Channel 0 Peak -------------------------------------------\n");     
      break;
    case 5:
      printf("\n------------------------ Histogram 5  Channel 1 Peak -------------------------------------------\n");     
      break;
    case 6:
      printf("\n------------------------ Histogram 6  Channel 2 Peak -------------------------------------------\n");     
      break;
    case 7:
      printf("\n------------------------ Histogram 7  Channel 3 Peak -------------------------------------------\n");     
      break;
    }       
  printf("In range = %d     Under = %d     Over = %d     AVG = %.3f     SIG = %.4f\n",
	 h_sum[n_hist],h_under[n_hist],h_over[n_hist],avg,sig);     
  for(ii=0; ii<20; ii++)		
    {
      bin_1 = ii * 10;		/* split print of 11 parameters - vxworks limit */
      printf("%.1f: %8d %8d %8d %8d %8d ", ( (((float)bin_1) * bin_size[n_hist]) + h_min[n_hist] ), 
	     hist[n_hist][bin_1], hist[n_hist][bin_1+1],hist[n_hist][bin_1+2],hist[n_hist][bin_1+3],hist[n_hist][bin_1+4]);
      printf("%8d %8d %8d %8d %8d\n", 
	     hist[n_hist][bin_1+5],hist[n_hist][bin_1+6],hist[n_hist][bin_1+7],hist[n_hist][bin_1+8],hist[n_hist][bin_1+9]);
    }        
  printf("-------------------------------------------------------------------------------------------------\n");     

}

void print_studies(void)
{
  long ii,jj,kk;
  float avg;

  for(jj=0; jj<4; jj++)		
    {
      printf("---------------------- Channel %d  (Peak - AVG Peak) (1000 events) -----------------------------\n",jj);
      for(ii=0; ii<100; ii++)		
	{
	  avg = (float)yavg[4][jj];
	  kk = ii * 10;		    /* split print of 10 parameters - vxworks limit */
	  printf("%.1f %.1f %.1f %.1f %.1f ",
		 (peak_avg[jj][kk] - avg), (peak_avg[jj][kk+1] - avg), (peak_avg[jj][kk+2] - avg),	        
		 (peak_avg[jj][kk+3] - avg), (peak_avg[jj][kk+4] - avg) );
	  printf("%.1f %.1f %.1f %.1f %.1f\n ",
		 (peak_avg[jj][kk+5] - avg), (peak_avg[jj][kk+6] - avg), (peak_avg[jj][kk+7] - avg), 
		 (peak_avg[jj][kk+8] - avg), (peak_avg[jj][kk+9] - avg) );
	}        
      printf("-------------------------------------------------------------------------------------------------\n");
    }     

}

void write_data_raw(FILE *fd_1, long n_trig, long n_samples)
{
  long ii,jj,kk;
	
  for(ii = 0; ii < 4; ii++)
    {
      fprintf(fd_1,"%d %d ", n_trig, (ii + 1));
      for(jj = 0; jj < 2; jj++)
	{
	  for(kk = 0; kk < n_samples; kk++)
	    {
	      fprintf(fd_1,"%d ",raw_data[ii][jj][kk]);
	    }    
	}
      fprintf(fd_1,"\n");	    
    }
}

long write_setup(FILE *fd_1)
{
  long value, ii;
  long divider, div_n;
  long int_gain, v_to_c, dac_value, n_samples, int_switch, window;
	
  value = adc->cfg_adc;
  int_gain = (value >> 1) & 0x3;
  v_to_c = (value >> 4) & 0xF;
	 
  if( value & 0x1 )
    int_switch = 0;
  else
    int_switch = 1;
	    
  div_n = ( (0x3) & (value >> 8) );
  divider = 1;
  for(ii = 0; ii < div_n; ii++)
    divider = divider * 2;
  n_samples = divider;    

  value = adc->delay_2;
  window = (long)( ((float)(value & 0xFFFF)) * 2.5 );
	
  value = adc->dac_value;
  dac_value = value & 0xFFFF;
	
  fprintf(fd_1,"%d %d %d %d %d %d %d\n", 
	  0, int_gain, v_to_c, dac_value, n_samples, int_switch, window);
		
  return n_samples;	 
	 	
}


	    
	
