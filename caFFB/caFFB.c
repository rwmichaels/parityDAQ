/************************************************************************
 * caFFB.c
 *    subroutines that utilize Channel Access to EPICS Variables
 *
 * Usage:
 *    cagetFFB    
 *        - returns 1(0) if FFB is enabled(disabled)
 *    caputFFB(int chAlias, int val)
 *        - chAlias = 0, 1 for Hall A, Hall C FFB
 *        - val = 0(1) to run (pause) FFB
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
#define DEBUG


double caget(int chAlias) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  

  switch (chAlias) 
    {      
    case 0:
      chName = "psub_pl_ipos";
      chDesc = "RHWP setpoint";
      break;      

    case 1:
      chName = "psub_pl_pos";
      chDesc = "RHWP readback";
      break;      

    case 2:
      chName = "psub_pl_move";
      chDesc = "RHWP motion";
      break;      

    case 3:
      chName = "IGLdac2:G2Ch3Pos";
      chDesc = "PC POS (Gun2)";
      break;

    case 4:
      chName = "IGLdac2:G2Ch4Neg";
      chDesc = "PC NEG (Gun2)";
      break;

    case 5:
      chName = "IGLdac3:ao_7";
      chDesc = "Hall-A IA Setpoint";
      break;

    case 6:
      chName = "IGLdac3:ao_5";
      chDesc = "Hall A PZT-X setpoint";
      break;

    case 7:
      chName = "IGLdac3:ao_6";
      chDesc = "Hall A PZT-Y setpoint";
      break;

    case 8:
      chName = "IGLdac3:ao_4";
      chDesc = "Hall-C IA Setpoint";
      break;

    default:
      chName = "IGLdac3:ao_7";      
      chDesc = "Hall-A IA Setpoint";
    }

/*   printf("caget(): %s (%s)\n",chDesc,chName); */

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
/*   printf("caget(): Current Value = %f\n",val); */
  return(val);

}

int caput(int chAlias, double val) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;

  switch (chAlias) 
    {      
    case 0:
      chName = "psub_pl_ipos";
      chDesc = "RHWP setpoint";
      break;      

    case 1:
      chName = "psub_pl_pos";
      chDesc = "RHWP readback";
      break;      

    case 2:
      chName = "psub_pl_move";
      chDesc = "RHWP motion";
      break;      

    case 3:
      chName = "IGLdac2:G2Ch3Pos";
      chDesc = "PC POS (Gun2)";
      break;

    case 4:
      chName = "IGLdac2:G2Ch4Neg";
      chDesc = "PC NEG (Gun2)";
      break;

    case 5:
      chName = "IGLdac3:ao_7";
      chDesc = "Hall-A IA Setpoint";
      break;

    case 6:
      chName = "IGLdac3:ao_5";
      chDesc = "Hall A PZT-X setpoint";
      break;

    case 7:
      chName = "IGLdac3:ao_6";
      chDesc = "Hall A PZT-Y setpoint";
      break;

    case 8:
      chName = "IGLdac3:ao_4";
      chDesc = "Hall-C IA Setpoint";
      break;

    default:
      chName = "IGLdac3:ao_7";      
      chDesc = "Hall-A IA Setpoint";
    }

/*   printf("caput(): %s (%s)\n",chDesc,chName); */

  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
  status = ca_put(DBR_DOUBLE, channelID, &val);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);

  
/*   printf("caput(): Set Value = %f\n",val); */
  return(1);

}



int cagetFFBverbose(int chAlias) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  

  switch (chAlias) 
    {      
    case 0:
      chName = "FB_A:pause";
      chDesc = "Hall A FFB Pause";
      break;      

    case 1:
      chName = "FB_C:pause";
      chDesc = "Hall C FFB Pause";
      break;      

    case 2:
      chName = "HallAMCOrb:PAUSE";
      chDesc = "Compton Orbit Lock Pause";
      break;      

    default:
      chName = "FB_A:pause";      
      chDesc = "Hall A FFB Pause";
    }

  printf("ECA_NORMAL is 0x%x\n",ECA_NORMAL);

  printf("caget(): %s (%s)\n",chDesc,chName);

  status = ca_task_initialize();
  printf("ca_talk_initialize() 0x%x\n", status);
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  printf("ca_search() 0x%x, 0x%x\n", status,channelID);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  printf("ca_pend_io() 0x%x\n", status);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  printf("ca_get() 0x%x\n", status);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  printf("ca_pend_io() 0x%x\n", status);
  MY_SEVCHK(status);
  
  printf("Value is %f\n", val);
  
  if(val>=1.0) return(1);
  if(val==0.0) return(0);

}



int cagetFFB(int chAlias) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  

  switch (chAlias) 
    {      
    case 0:
      chName = "FB_A:pause";
      chDesc = "Hall A FFB Pause";
      break;      

    case 1:
      chName = "FB_C:pause";
      chDesc = "Hall C FFB Pause";
      break;      

    case 2:
      chName = "HallAMCOrb:PAUSE";
      chDesc = "Compton Orbit Lock Pause";
      break;      

    default:
      chName = "FB_A:pause";      
      chDesc = "Hall A FFB Pause";
    }

/*   printf("caget(): %s (%s)\n",chDesc,chName); */

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
/*   printf("caget(): Current Value = %f\n",val); */
  
  if(val>=1.0) return(1);
  if(val==0.0) return(0);

}

int caputFFB(int chAlias, double val) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;

  switch (chAlias) 
    {      
    case 0:
      chName = "FB_A:pause";
      chDesc = "Hall A FFB Pause";
      break;      

    case 1:
      chName = "FB_C:pause";
      chDesc = "Hall C FFB Pause";
      break;      

    case 2:
      chName = "HallAMCOrb:PAUSE";
      chDesc = "Compton Orbit Lock Pause";
      break;      

    default:
      chName = "FB_A:pause";      
      chDesc = "Hall A FFB Pause";
    }


  status = ca_task_initialize();
  /*   printf("caput(): %s (%s)\n",chDesc,chName); */
  MY_SEVCHK(status);

  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);
  
  status = ca_put(DBR_DOUBLE, channelID, &val);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  MY_SEVCHK(status);

  
/*   printf("caput(): Set Value = %f\n",val); */
  return(1);
}



int caputFFBverbose(int chAlias, double val) {

  // Somewhat generic method

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;

  switch (chAlias) 
    {      
    case 0:
      chName = "FB_A:pause";
      chDesc = "Hall A FFB Pause";
      break;      

    case 1:
      chName = "FB_C:pause";
      chDesc = "Hall C FFB Pause";
      break;      

    case 2:
      chName = "HallAMCOrb:PAUSE";
      chDesc = "Compton Orbit Lock Pause";
      break;      

    default:
      chName = "FB_A:pause";      
      chDesc = "Hall A FFB Pause";
    }

  printf("caputFFB(): %s (%s)\n",chDesc,chName);

  status = ca_task_initialize();
  printf("ca_task_initialize() 0x%x\n", status);
  MY_SEVCHK(status);

  status = ca_search(chName, &channelID);
  printf("ca_search() 0x%x\n", status);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  printf("ca_pend_io() 0x%x\n", status);
  MY_SEVCHK(status);
  
  status = ca_put(DBR_DOUBLE, channelID, &val);
  printf("ca_put() 0x%x\n", status);
  MY_SEVCHK(status);

  status = ca_pend_io(0.0);
  printf("ca_pend_io() 0x%x\n", status);
  MY_SEVCHK(status);
  
  printf("caput(): Set Value = %f\n",val); 
  return(1);

}

