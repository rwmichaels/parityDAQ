/* Header for JLab 18-bit ADC for HAPPEX */
/* Nov 2006,  Bob Michaels */


#define ADC18_MAXBOARDS 10
#define TIME_INT 11.0	 /* sample time (us) for internal timing sequence */	

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
		 };

volatile struct adc18_struct *adc18p[ADC18_MAXBOARDS];
volatile unsigned long *adc18_data[ADC18_MAXBOARDS];
volatile int adc18_ok[ADC18_MAXBOARDS];

static int t_mode;
