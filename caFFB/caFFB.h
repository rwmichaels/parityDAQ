/************************************************************************
 * caFFB.h
 *    Header file for EPICS Channel Access 
 *
 * June, 2003 B. Moffit
 *
 ***********************************************************************/

// Function Prototypes
int cagetFFB(int);
int caputFFB(int,double);
int cagetFFBverbose(int);
int caputFFBverbose(int,double);
double caget(int);
int caput(int,double);

double cagetFFB_modState();
double cagetFFB_waveState(int);
double cagetFFB_freq(int);
double cagetFFB_amp(int);
double cagetFFB_period(int);
double cagetFFB_load(int);
double cagetFFB_enterTrig(int);
double cagetFFB_leaveTrig(int);
double cagetFFB_trig(int);
double cagetFFB_output(int);
double cagetFFB_gen(char *ch);

int caputFFB_modState(double);
int caputFFB_freq(int,double);
int caputFFB_amp(int,double);
int caputFFB_period(int,double);
int caputFFB_load(int,double);
int caputFFB_enterTrig(int,double);
int caputFFB_leaveTrig(int,double);
int caputFFB_trig(int,double);
int caputFFB_output(int,double);
int caputFFB_gen(char *ch,double);

char *caFFB_coils(int,char *ch);
char *caFFB_coilsRelay(int);

#define MY_SEVCHK(status)    \
{                            \
  if(status != ECA_NORMAL )  \
    {                        \
      SEVCHK(status,NULL);   \
      return(status);        \
    }                        \
}
