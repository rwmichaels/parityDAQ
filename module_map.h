/* Define the memory map of VME modules used by HAPPEX DAQ */
/* R. Michaels, A. Vacheret  Oct 2000 */

//  global, from timeboard utilities
extern int CODA_RUN_IN_PROGRESS;

// HAPPEX ADC utilities
extern int ADC_NUMSLOTS;   // number of HAPPEX ADCs
extern int showHAPADC();   // dump registers of adcs
extern unsigned long* readoutHAPADC(unsigned long*, int);  // readout adcs
extern int setDACHAPADC(int, int);   // set noise dac value on adc
extern int usesDACHAPADC(int); // check that this adc uses dac noise

// Timing Board utilities
extern int InitHAPTB();
extern int dumpRegHAPTB();
extern int getOverSampleCurrentHAPTB();
extern int getOverSampleHAPTB();
extern int getRampDelayHAPTB();
extern int getIntTimeHAPTB();
extern int getDACHAPTB();
extern int setDACHAPTB(int, int);
extern int setTimeHAPTB(int, int);
extern int setOverSampleHAPTB(int);

// global variables for bmw
extern int  bmw_object;
extern int  bmw_clean;
extern int  bmw_clean_pulse;
extern int  bmw_value;
extern int  bmw_cycle_number;


/* SISGmbH 3800 Scaler */
struct SIS3800
{ /*  register name              offset         description  (w-write, r-read, r/w-read/write)*/
  volatile unsigned long csr             ; /* 00      - Status register(read)/Conrol register(write)  */
  volatile unsigned long IntVerR         ; /* 04      - VME IRQ control register and module identification (r/w) */
  unsigned long empty0          ; /* 08 not used */
  volatile unsigned long disCntR         ; /* 0C      - Copy disable register (w) */
  unsigned long empty1[4]       ; /* 10-1C not used */
  volatile unsigned long clear           ; /* 20      - Clear all counters and overflow bits (w) */
  volatile unsigned long vmeclk          ; /* 24      - VME next clock (w) */
  volatile unsigned long enclk           ; /* 28      - Global count enable (w) */
  volatile unsigned long disclk          ; /* 2C      - Global count disable (w) */
  volatile unsigned long bclear          ; /* 30      - Broadcast: Clear FIFO, logic and counters (w) */
  volatile unsigned long bvmeclk         ; /* 34      - Broadcast: VME next clock (w) */
  volatile unsigned long benclk          ; /* 38      - Broadcast: Global count enable (w) */
  volatile unsigned long bdisclk         ; /* 3C      - Broadcast: Global count Disable  (w) */
  volatile unsigned long clear1_8        ; /* 40      - clear counter group and overflow channel 1-8 (w) */
  volatile unsigned long clear9_16       ; /* 44      - clear counter group and overflow channel 9-16 (w) */
  volatile unsigned long clear17_24      ; /* 48      - clear counter group and overflow channel 17-24 (w) */
  volatile unsigned long clear25_32      ; /* 4C      - clear counter group and overflow channel 25-32 (w) */
  volatile unsigned long gEncnt1         ; /* 50      - Enable reference pulser channel 1 (w)  */
  volatile unsigned long gDiscnt1        ; /* 54      - Disable reference pulser channel 1 (w) */
  unsigned long empty2[2]       ; /* 58-5C not used */  
  volatile unsigned long reset           ; /* 60      - Reset register (w) */
  unsigned long empty3          ; /* 64 not used */
  volatile unsigned long testclkR        ; /* 68      - test pulse (generate a single pulse) (w) */
  unsigned long empty4[37]      ; /* 6C-FC not used */
  volatile unsigned long clrCount[32]    ; /* 100-17C - clear counter N and its overflow bit (w) */
  volatile unsigned long clrOverCount[32]; /* 180-1FC - clear overflow bit of counter N (w) */
  volatile unsigned long readShadow[32]  ; /* 200-27C - Read Shadow register (does not initiate clock shadow) (r) */
  volatile unsigned long readCounter[32] ; /* 280-2FC - Read counter (initiates clock shadow also) (r) */
  volatile unsigned long readClrCount[32]; /* 300-37C - Read and clear all counters (r) */
  volatile unsigned long readOver1_8     ; /* 380     - Overflow register channel 1-8 (r) */
  unsigned long empty5[7]       ; /* 384-39C not used */
  volatile unsigned long readOver9_16    ; /* 3A0     - Overflow register channel 9-16 (r) */
  unsigned long empty6[7]       ; /* 384-39C not used */
  volatile unsigned long readOver17_24   ; /* 3C0     - Overflow register channel 17-24 (r) */
  unsigned long empty7[7]       ; /* 384-39C not used */
  volatile unsigned long readOver25_32   ; /* 3E0     - Overflow register channel 25-32 (r) */
};

/* SISGmbH 3801 Scaler */
struct fifo_scaler 
{ /*  register name              offset         description  (w-write, r-read, r/w-read/write)*/
  volatile unsigned long csr             ; /* 00      - Status register(read)/Conrol register(write)  */
  volatile unsigned long irq             ; /* 04      - VME IRQ control register and module identification (r/w) */
  unsigned long empty0          ; /* 08 not used */
  volatile unsigned long cdr             ; /* 0C      - Copy disable register (w) */
  volatile unsigned long wfifo           ; /* 10      - Write to FIFO (in FIFO test mode)*/
  unsigned long empty1          ; /* 14 not used */
  unsigned long empty2          ; /* 18 not used */
  unsigned long empty3          ; /* 1C not used */
  volatile unsigned long clear           ; /* 20      - Clear FIFO, logic and counters (w) */
  volatile unsigned long next            ; /* 24      - VME next clock (w) */
  volatile unsigned long enable          ; /* 28      - Enable next clock logic (w) */
  volatile unsigned long disable         ; /* 2C      - Disable next clock logic (w) */
  volatile unsigned long Bclear          ; /* 30      - Broadcast: Clear FIFO, logic and counters (w) */
  volatile unsigned long Bnext           ; /* 34      - Broadcast: VME next clock (w) */
  volatile unsigned long Benable         ; /* 38      - Broadcast: Enable next clock logic (w) */
  volatile unsigned long Bdisable        ; /* 3C      - Broadcast: Disable next clock logic (w) */
  unsigned long empty4[4]       ; /* 40-4C not used */
  volatile unsigned long gEncnt1         ; /* 50      - Enable reference pulser channel 1 (w)  */
  volatile unsigned long gDiscnt1        ; /* 54      - Disable reference pulser channel 1 (w) */
  unsigned long empty5[2]       ; /* 58-5C not used */  
  volatile unsigned long reset           ; /* 60      - Reset register (w) */
  unsigned long empty6          ; /* 64 not used */
  volatile unsigned long test            ; /* 68      - test pulse (generate a single pulse) (w) */
  unsigned long empty7[37]      ; /* 6C-FC not used */
  volatile unsigned long fifo[64]        ; /* 100-1FC - read FIFO registers (w) */
};

struct lecroy_1151_scaler {
  volatile unsigned short reset;
  volatile unsigned short blank1[7];
  volatile unsigned short bim[8];
  volatile unsigned short blank2[16];
  volatile unsigned long preset[16];
  volatile unsigned long scaler[16];
  volatile unsigned short blank3[29];
  volatile unsigned short id[3];
};

struct caen_v560_scaler {
  /*  register name          offset   description  (w-write, r-read, r/w-read/write)*/
  unsigned short empty1[2]; /* 00-02 - not used address */
  volatile unsigned short IntVectR ; /* 04    - interrupt Vector register (r/w) */
  volatile unsigned short IntLevR  ; /* 06    - interrupt Level&VETO register (r/w) */
  volatile unsigned short EnIntR   ; /* 08    - Enable VME interrupt register (r/w) */
  volatile unsigned short DisIntR  ; /* 0A    - Disable VME interrupt register (r/w) */
  volatile unsigned short ClrIntR  ; /* 0C    - Clear VME interrupt register (r/w) */
  volatile unsigned short RequestR ; /* 0E    - Request register (r/w) */
  volatile unsigned long  CntR[16] ; /* 10-4C - Counter 0 - 15 (r) */
  volatile unsigned short ClrR     ; /* 50    - Scale Clear register (r/w) */
  volatile unsigned short setVETOR ; /* 52    - VME VETO set register (r/w) */
  volatile unsigned short clrVETOR ; /* 54    - VME VETO reset register (r/w) */
  volatile unsigned short incCntR  ; /* 56    - Scale Increase register (r/w) */
  volatile unsigned short StatusR  ; /* 58    - Scale Status register (r/w) */
  unsigned short empty2[80]; /* 5A-F8 - not used address */
  volatile unsigned short FixCodeR ; /* FA    - Fixed Code  register (r) */
  volatile unsigned short ModTypeR ; /* FC    - Manufacturer&Module Type  register (r) */
  volatile unsigned short VersionR ; /* FE    - Version&Series  register (r) */
};
