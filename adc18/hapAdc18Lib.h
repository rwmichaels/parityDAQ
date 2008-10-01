/* Header for JLab 18-bit ADC for HAPPEX */
/* Nov 2006,  1st version, Bob Michaels */

/* modifications */

/* Sept 2007,  RWM,   modified to have crate-dependent parameters 
                      like NADC, and ADC* 

*/


#define ADC18_MAXBOARDS 10

/* number of HAPPEX 18-bit ADCs */
#ifdef  TESTCRATE
#define NADC 1   
#endif
#ifdef  COUNTINGHOUSE
#define NADC 0    //changed 11/27/07 B. Hahn
#endif
#ifdef  INJECTOR
#define NADC 0    //changed 11/27/07 B. Hahn
#endif
#ifdef  LEFTSPECT
#define NADC 0    //changed 11/27/07 B. Hahn
#endif
#ifdef  RIGHTSPECT
#define NADC 1    
#endif

#define TIME_INT 11.0	 /* sample time (us) for internal timing sequence */	

//   Relative Addresses of ADCs

#ifdef   TESTCRATE

#define  ADC0 0x820000
#define  ADC1 0xff0000   // Undefined.
#define  ADC2 0xff0000
#define  ADC3 0xff0000
#define  ADC4 0xff0000
#define  ADC5 0xff0000
#define  ADC6 0xff0000
#define  ADC7 0xff0000
#define  ADC8 0xff0000
#define  ADC9 0xff0000

#define  ADCLAB0 7
#define  ADCLAB1 0xf   // Undefined
#define  ADCLAB2 0xf
#define  ADCLAB3 0xf
#define  ADCLAB4 0xf
#define  ADCLAB5 0xf
#define  ADCLAB6 0xf
#define  ADCLAB7 0xf
#define  ADCLAB8 0xf
#define  ADCLAB9 0xf

#endif

#ifdef   COUNTINGHOUSE
#define  ADC0 0x470000
#define  ADC1 0x850000
#define  ADC2 0x400000
#define  ADC3 0x4d0000
#define  ADC4 0x480000
#define  ADC5 0x4c0000
#define  ADC6 0x4b0000
#define  ADC7 0x510000
#define  ADC8 0x490000
#define  ADC9 0x490000
#define  ADCLAB0 2
#define  ADCLAB1 26
#define  ADCLAB2 23
#define  ADCLAB3 22
#define  ADCLAB4 15
#define  ADCLAB5 13
#define  ADCLAB6 7
#define  ADCLAB7 12
#define  ADCLAB8 14
#define  ADCLAB9 14
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
#define  ADCLAB0 8
#define  ADCLAB1 6
#define  ADCLAB2 16
#define  ADCLAB3 23
#define  ADCLAB4 24
#define  ADCLAB5 25
#define  ADCLAB6 26
#define  ADCLAB7 27
#define  ADCLAB8 21
#define  ADCLAB9 29
#endif

#ifdef   LEFTSPECT
#define  ADC0 0x420000
#define  ADC1 0x430000
#define  ADC2 0x820000
#define  ADC3 0x500000
#define  ADC4 0x440000
#define  ADC5 0x4c0000
#define  ADC6 0x4b0000
#define  ADC7 0x510000
#define  ADC8 0x490000
#define  ADC9 0x5F0000
#define  ADCLAB0 18
#define  ADCLAB1 19
#define  ADCLAB2 27
#define  ADCLAB3 9
#define  ADCLAB4 20
#define  ADCLAB5 45
#define  ADCLAB6 46
#define  ADCLAB7 47
#define  ADCLAB8 48
#define  ADCLAB9 49
#endif

#ifdef   RIGHTSPECT
#define  ADC0 0xee000
#define  ADC1 0xed0000
#define  ADC2 0xed4000
#define  ADC3 0xee000
#define  ADC4 0x4B0700
#define  ADC5 0x480000
#define  ADC6 0x4c0000
#define  ADC7 0x4b0000
#define  ADC8 0x510000
#define  ADC9 0x490000
#define  ADCLAB0 29
#define  ADCLAB1 17
#define  ADCLAB2 62
#define  ADCLAB3 63
#define  ADCLAB4 64
#define  ADCLAB5 65
#define  ADCLAB6 66
#define  ADCLAB7 67
#define  ADCLAB8 68
#define  ADCLAB9 69
#endif


struct adc18_struct {
		 long mod_id;		/* 00 */
		 long csr;		/* 04 */
		 long ctrl;		/* 08 */
		 long config;		/* 0C */
		 long evt_ct;		/* 10 */
		 long evt_word_ct;	/* 14 */
		 long dac_value;	/* 18 */
		 long reserved_1;	/* 1C */
		 long dac_memory;	/* 20 */
		 long spare[3];		/* 24,28,2C */
		 long delay_1;		/* 30 */
		 long delay_2;		/* 34 */
                 long reserved_2;	/* 38 */
		 long reserved_3;	/* 3C */
		 long pedestal[4];	/* 40,44,48,4C  (New, Jan 2008) */
		 };

volatile struct adc18_struct *adc18p[ADC18_MAXBOARDS];
volatile unsigned long *adc18_data[ADC18_MAXBOARDS];
volatile int adc18_ok[ADC18_MAXBOARDS];

// Integer Labels of ADCs 
LOCAL unsigned long ADCLABEL[10] = {ADCLAB0, ADCLAB1, ADCLAB2, ADCLAB3, ADCLAB4, ADCLAB5, ADCLAB6, ADCLAB7, ADCLAB8, ADCLAB9};

// Addresses of ADCs 
LOCAL unsigned long ADCADDR[10] = {ADC0, ADC1, ADC2, ADC3, ADC4, ADC5, ADC6, ADC7, ADC8, ADC9};

static int t_mode;
