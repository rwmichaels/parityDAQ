/************************************************************************
 * caFFB.h
 *    Header file for EPICS Channel Access 
 *
 * June, 2003 B. Moffit
 *
 ***********************************************************************/

// Function Prototypes
int cagetFFB(void);
int caputFFB(int);

#define MY_SEVCHK(status)    \
{                            \
  if(status != ECA_NORMAL )  \
    {                        \
      SEVCHK(status,NULL);   \
      return(status);        \
    }                        \
}
