/************************************************************************
 * caFFB.c
 *    subroutines that utilize Channel Access to EPICS Variables
 *
 * Usage:
 *    cagetFFB    
 *        - returns 1(0) if FFB is enabled(disabled)
 *    caputFFB(int val)
 *        - val = 1(0) to enable(disable) FFB
 *          returns 0 if sucessful
 *
 *     Both routines will return -1 if unsuccessful
 *        
 * June, 2003 - B. Moffit
 *   
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cadef.h>
#include "caFFB.h"

int cagetFFB()
{
	chid channelID;
	int  status;
	double val;

	status = ca_task_initialize();
	MY_SEVCHK(status);

	status = ca_search("HA:Q_ONOFF", &channelID);
	MY_SEVCHK(status);

	status = ca_pend_io(0.0);
	MY_SEVCHK(status);

	status = ca_get(DBR_DOUBLE,channelID, &val);
	MY_SEVCHK(status);

	status = ca_pend_io(0.0);
	MY_SEVCHK(status);

	//	printf("Value is %f\n", val);

	if(val>=1.0) return(1);
	if(val==0.0) return(0);
}

int caputFFB(int input)
{
  int status;
  chid channelID;
  double val;

  if(input==1) {
    val = 1.0;
  } else if(input==0) {
    val = 0.0;
  } else {
    return(-1);
  }

  status = ca_task_initialize();
  MY_SEVCHK(status);

  status = ca_search("HA:Q_ONOFF", &channelID);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
  status = ca_put(DBR_DOUBLE, channelID, &val);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);

  return(0);

}
