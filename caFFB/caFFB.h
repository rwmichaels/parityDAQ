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

#define MY_SEVCHK(status)    \
{                            \
  if(status != ECA_NORMAL )  \
    {                        \
      SEVCHK(status,NULL);   \
      return(status);        \
    }                        \
}
