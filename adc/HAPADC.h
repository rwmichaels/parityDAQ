/************************************************************************
 *  HAPADC.h                                                            *
 *   Define the VME map of one HAPPEX ADC, for tests                    *
 *                                                                      *  
 * October, 2000.  R. Michaels / A. Vacheret                            *
 * Modifications:                                                       *
 *                                                                      *  
 ************************************************************************/

//
#define DAC_ON 8
#define DAC_OFF 0
#define LO_GAIN 0
#define HI_GAIN 16
//

/* number of HAPPEX ADCs */
#ifdef  COUNTINGHOUSE
#define NADC 9    // must be <10, or requires changes to static arrays.
#endif
#ifdef  INJECTOR
#define NADC 3    // must be <10, or requires changes to static arrays.
#endif
#ifdef  LEFTSPECT
#define NADC 4    // must be <10, or requires changes to static arrays.
#endif
#ifdef  RIGHTSPECT
#define NADC 0    // must be <10, or requires changes to static arrays.
#endif

#define MAXRETRY 10   // max number of attempts to write csr

#define DEF_TIMEOUT 2000  // max number of polls of done bit during readout

#define DEF_DACVAL 3000  // dac value for non-dac noise adcs

//   Relative Addresses of ADCs
#ifdef   COUNTINGHOUSE
#define  ADC0 0x470000
#define  ADC1 0x830000
#define  ADC2 0x400000
#define  ADC3 0x4d0000
#define  ADC4 0x480000
#define  ADC5 0x4c0000
#define  ADC6 0x4b0000
#define  ADC7 0x510000
#define  ADC8 0x490000
#define  ADC9 0x490000
// #define  ADC2 0x840000   Removed March 25, 2003
#define  ADCLAB0 40
#define  ADCLAB1 41
#define  ADCLAB2 42
#define  ADCLAB3 43
#define  ADCLAB4 44
#define  ADCLAB5 45
#define  ADCLAB6 46
#define  ADCLAB7 47
#define  ADCLAB8 48
#define  ADCLAB9 49
#endif
#ifdef   INJECTOR
#define  ADC0 0x4e0000
#define  ADC1 0x980000
#define  ADC2 0x4d0000
#define  ADC3 0x400000
#define  ADC4 0x4d0000
#define  ADC5 0x480000
#define  ADC6 0x4c0000
#define  ADC7 0x4b0000
#define  ADC8 0x510000
#define  ADC9 0x490000
#define  ADCLAB0 10
#define  ADCLAB1 11
#define  ADCLAB2 12
#define  ADCLAB3 13
#define  ADCLAB4 14
#define  ADCLAB5 15
#define  ADCLAB6 16
#define  ADCLAB7 17
#define  ADCLAB8 81
#define  ADCLAB9 19
#endif
#ifdef   LEFTSPECT
#define  ADC0 0x850000
#define  ADC1 0x820000
#define  ADC2 0x8F0000
#define  ADC3 0x840000
#define  ADC4 0x480000
#define  ADC5 0x4c0000
#define  ADC6 0x4b0000
#define  ADC7 0x510000
#define  ADC8 0x490000
#define  ADC9 0x5F0000
#define  ADCLAB0 20
#define  ADCLAB1 21
#define  ADCLAB2 22
#define  ADCLAB3 23
#define  ADCLAB4 24
#define  ADCLAB5 25
#define  ADCLAB6 26
#define  ADCLAB7 27
#define  ADCLAB8 28
#define  ADCLAB9 29
#endif
#ifdef   RIGHTSPECT
#define  ADC0 0x850000
#define  ADC1 0x820000
#define  ADC2 0x8F0000
#define  ADC3 0x810000
#define  ADC4 0x4B0700
#define  ADC5 0x480000
#define  ADC6 0x4c0000
#define  ADC7 0x4b0000
#define  ADC8 0x510000
#define  ADC9 0x490000
#define  ADCLAB0 30
#define  ADCLAB1 31
#define  ADCLAB2 32
#define  ADCLAB3 33
#define  ADCLAB4 34
#define  ADCLAB5 35
#define  ADCLAB6 36
#define  ADCLAB7 37
#define  ADCLAB8 38
#define  ADCLAB9 39
#endif

// gain setting for each adc
#define ADC0_DEF_GAINBIT LO_GAIN
#define ADC1_DEF_GAINBIT LO_GAIN
#define ADC2_DEF_GAINBIT LO_GAIN
#define ADC3_DEF_GAINBIT LO_GAIN
#define ADC4_DEF_GAINBIT LO_GAIN
#define ADC5_DEF_GAINBIT LO_GAIN
#define ADC6_DEF_GAINBIT LO_GAIN
#define ADC7_DEF_GAINBIT LO_GAIN
#define ADC8_DEF_GAINBIT LO_GAIN
#define ADC9_DEF_GAINBIT LO_GAIN
	    
// DAC noise settings for each adc
#define ADC0_DEF_DACBIT DAC_ON 
#define ADC1_DEF_DACBIT DAC_ON
#define ADC2_DEF_DACBIT DAC_ON
#define ADC3_DEF_DACBIT DAC_ON
#define ADC4_DEF_DACBIT DAC_ON
#define ADC5_DEF_DACBIT DAC_ON
#define ADC6_DEF_DACBIT DAC_ON
#define ADC7_DEF_DACBIT DAC_ON
#define ADC8_DEF_DACBIT DAC_ON
#define ADC9_DEF_DACBIT DAC_ON

// diagnostic mux setting for each adc
#define ADC0_DEF_MUXBIT 3  // channel number muxed to diagnostic IntOut
#define ADC1_DEF_MUXBIT 3
#define ADC2_DEF_MUXBIT 3
#define ADC3_DEF_MUXBIT 3
#define ADC4_DEF_MUXBIT 3
#define ADC5_DEF_MUXBIT 3
#define ADC6_DEF_MUXBIT 3
#define ADC7_DEF_MUXBIT 3
#define ADC8_DEF_MUXBIT 3
#define ADC9_DEF_MUXBIT 3
	    


/* HAPPEX custom 16-bit ADC (board made by Princeton/Harvard) */
struct vme_happex_adc {
  volatile unsigned short adcchan[4];   /* four channels of 16-bit data */
  volatile unsigned short dac;         /* DAC value */
  volatile unsigned short dummy1;
  volatile unsigned short csr;         /* Control, Status Register */
  volatile unsigned short dummy2;
  volatile unsigned short rng;         /* DAC random number (usually not used) */
  volatile unsigned short done;        /* Done = digitization complete */
};

// EXTERN
extern int CODA_RUN_IN_PROGRESS;


// GLOBALS

int ADC_NUMSLOTS = NADC;

// LOCALS

// forward declaration of local methods
LOCAL int writeCSRHAPADC(int, int);
LOCAL int ADC_TIMEOUT  = DEF_TIMEOUT;

//
// arrays of default values
//

// Integer Labels of ADCs (maximum of 10 for now)
LOCAL unsigned long ADCLABEL[10] = {ADCLAB0, ADCLAB1, ADCLAB2, ADCLAB3, ADCLAB4, ADCLAB5, ADCLAB6, ADCLAB7, ADCLAB8, ADCLAB9};

// Addresses of ADCs (maximum of 10 for now)
LOCAL unsigned long ADCADDR[10] = {ADC0, ADC1, ADC2, ADC3, ADC4, ADC5, ADC6, ADC7, ADC8, ADC9};

LOCAL unsigned long ADC_GAINBIT[10] = {ADC0_DEF_GAINBIT, ADC1_DEF_GAINBIT, ADC2_DEF_GAINBIT, ADC3_DEF_GAINBIT, ADC4_DEF_GAINBIT, ADC5_DEF_GAINBIT, ADC6_DEF_GAINBIT, ADC7_DEF_GAINBIT, ADC8_DEF_GAINBIT, ADC9_DEF_GAINBIT}; 

LOCAL unsigned long ADC_MUXBIT[10] = {ADC0_DEF_MUXBIT, ADC1_DEF_MUXBIT, ADC2_DEF_MUXBIT, ADC3_DEF_MUXBIT, ADC4_DEF_MUXBIT, ADC5_DEF_MUXBIT, ADC6_DEF_MUXBIT, ADC7_DEF_MUXBIT, ADC8_DEF_MUXBIT, ADC9_DEF_MUXBIT}; 

LOCAL unsigned long ADC_DACBIT[10] = {ADC0_DEF_DACBIT, ADC1_DEF_DACBIT, ADC2_DEF_DACBIT, ADC3_DEF_DACBIT, ADC4_DEF_DACBIT, ADC5_DEF_DACBIT, ADC6_DEF_DACBIT, ADC7_DEF_DACBIT, ADC8_DEF_DACBIT, ADC9_DEF_DACBIT};
LOCAL int did_init[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

//
// local arrays
//

volatile struct vme_happex_adc *adcbrds[NADC];

LOCAL dacvalue[NADC];


