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
 *  Modified for PREX dithering, 14 Mar, 2010
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
  
  printf("caget(): %s (%s)\n",chDesc,chName); 
  
/*   cagetFFB_gen(chName); */

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
  
  printf("caget(): Current Value = %f\n",val);
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

//
// New routines for all EPICS beam modulation
//

/**************caget ***********/

// get the Beam Modulation State:: On/Off (1/0)
// when the modulation is turned on, 
// the default state s CONFIG_STATE
double cagetFFB_modState() {
/*   char *chName = "BEAMMODSWITCH"; */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  chName = "BEAMMODSWITCH";
  chDesc = "BEAM MODULATION";

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  //  status = ca_get(DBR_INT,channelID, &val);
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_wavestate(): %s = %f\n",chName,val); 

  return(val);
}

// get the WAVESTATE
// OFF_STATE      0
// CONFIG_STATE   1
// TRIGGER_STATE  2
// TEST_STATE     3
double cagetFFB_waveState(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"WAVESTATE"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:WAVESTATE";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:WAVESTATE";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:WAVESTATE";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:WAVESTATE";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:WAVESTATE";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:WAVESTATE";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:WAVESTATE";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:WAVESTATE";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:WAVESTATE";
      chDesc = "MHF0I08H";
    }

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_wavestate(): %s = %f\n",chName,val); 

  return(val);
}

// get the frequency of the SINEWAVE
double cagetFFB_freq(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"FREQUENCY"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:FREQUENCY";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:FREQUENCY";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:FREQUENCY";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:FREQUENCY";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:FREQUENCY";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:FREQUENCY";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:FREQUENCY";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:FREQUENCY";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:FREQUENCY";
      chDesc = "MHF0I08H";
    }
 
  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  //  status = ca_get(DBR_INT,channelID, &val);
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_freq(): %s = %f\n",chName,val); 

  return(val);
}

// get the amplitude of the SINEWAVE
double cagetFFB_amp(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"AMPLITUDE"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:AMPLITUDE";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:AMPLITUDE";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:AMPLITUDE";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:AMPLITUDE";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:AMPLITUDE";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:AMPLITUDE";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:AMPLITUDE";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:AMPLITUDE";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:AMPLITUDE";
      chDesc = "MHF0I08H";
    }
 
  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  //  status = ca_get(DBR_INT,channelID, &val);
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_amp(): %s = %f\n",chName,val); 

  return(val);
}

// get the number of SINEWAVE periods
double cagetFFB_period(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"RCRINPUT"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:RCRINPUT";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:RCRINPUT";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:RCRINPUT";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:RCRINPUT";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:RCRINPUT";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:RCRINPUT";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:RCRINPUT";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:RCRINPUT";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:RCRINPUT";
      chDesc = "MHF0I08H";
    }
 
  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  //  status = ca_get(DBR_INT,channelID, &val);
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_period(): %s = %f\n",chName,val); 

  return(val);
}

// check whether or not the SINEWAVE has been loaded
double cagetFFB_load(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"SINEWAVE"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:SINEWAVE";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:SINEWAVE";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:SINEWAVE";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:SINEWAVE";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:SINEWAVE";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:SINEWAVE";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:SINEWAVE";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:SINEWAVE";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:SINEWAVE";
      chDesc = "MHF0I08H";
    }

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_sinewave(): %s = %f\n",chName,val); 

  return(val);
}

// get the "Enter Trigger State" state
// if the "Hardware Trigger" is in initiated state, 
// settig Trigger State to 1 initiates sine wave outputs
double cagetFFB_enterTrig(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"TRIGSTATEBTN"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:TRIGSTATEBTN";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:TRIGSTATEBTN";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:TRIGSTATEBTN";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:TRIGSTATEBTN";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:TRIGSTATEBTN";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:TRIGSTATEBTN";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:TRIGSTATEBTN";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:TRIGSTATEBTN";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:TRIGSTATEBTN";
      chDesc = "MHF0I08H";
    }

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_enterTrig(): %s = %f\n",chName,val); 

  return(val);
}

// get the "Leave Trigger State" state
// When this state is set to 0, the state will be CONFIG_STATE
double cagetFFB_leaveTrig(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"STOPBTN"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:STOPBTN";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:STOPBTN";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:STOPBTN";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:STOPBTN";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:STOPBTN";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:STOPBTN";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:STOPBTN";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:STOPBTN";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:STOPBTN";
      chDesc = "MHF0I08H";
    }

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_leaveTrig(): %s = %f\n",chName,val); 

  return(val);
}

// get the "Software Trigger" state
double cagetFFB_trig(int chAlias) {
/*   char *chName = (char*)caFFB_coils(chAlias,"TRIGBTN"); */
/*   return cagetFFB_gen(chName); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  double val;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:TRIGBTN";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:TRIGBTN";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:TRIGBTN";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:TRIGBTN";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:TRIGBTN";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:TRIGBTN";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:TRIGBTN";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:TRIGBTN";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:TRIGBTN";
      chDesc = "MHF0I08H";
    }

  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_trig(): %s = %f\n",chName,val); 

  return(val);
}

// set the Relay for the output from the function generator into DAQ
double cagetFFB_output(int chAlias) {
  char *chName1, *chName2;

  // not working properly ??
  chName1 = caFFB_coilsRelay(chAlias);
/*   cagetFFB_gen(chName1); */

/*   //  sleep(30);   */

/*   chName2 = caFFB_coilsRelay(chAlias+8); */
/*   cagetFFB_gen(chName2); */
}

double cagetFFB_gen(char *chName) {
 
  chid channelID=0;
  int  status;
  double val;
 
  status = ca_task_initialize();
  MY_SEVCHK(status);
  
  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);
  
  // ca_pend_io(TIMEOUT)
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  //  status = ca_get(DBR_INT,channelID, &val);
  status = ca_get(DBR_DOUBLE,channelID, &val);
  MY_SEVCHK(status);
  
  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  printf("cagetFFB_gen(): %s = %f\n",chName,val); 

  return(val);
}

/**********caput **************/

// get the Beam Modulation State:: On/Off (1/0)
int caputFFB_modState(double val) {

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;

  if(val>=0&&val<=1){
    chName = "BEAMMODSWITCH";
    chDesc = "BEAM MODULATION";
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);

    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_modState(): %s = %f\n",chName,val);
    
    return(1);
    
    /*     return caputFFB_gen(chName,val); */
  }else{
    printf("caputFFB_modState(): Allowed valuess 0/1.\n"); 
    printf("caputFFB_modState(): Entered value = %f \n",val); 

    return(0);
  }
}

// set the SINEWAVE frequency
// can only be done in CONFIG_STATE
int caputFFB_freq(int chAlias, double val) {

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;

  if(val>=10&&val<=250){
/*     chName = (char*)caputFFB_coils(chAlias,"FREQUENCY"); */
/*     return caputFFB_gen(chName,val); */
  
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:FREQUENCY";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:FREQUENCY";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:FREQUENCY";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:FREQUENCY";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:FREQUENCY";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:FREQUENCY";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:FREQUENCY";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:FREQUENCY";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:FREQUENCY";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_freq(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_freq(): Allowed freq range 10-250 Hz.\n"); 
    printf("caputFFB_freq(): Entered freq = %f Hz\n",val); 

    return(0);
  }
}

// set the SINEWAVE amplitude
// can only be done in CONFIG_STATE
int caputFFB_amp(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=0&&val<=300){ */
/*     chName = (char*)caputFFB_coils(chAlias,"AMPLITUDE"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=0&&val<=300){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:AMPLITUDE";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:AMPLITUDE";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:AMPLITUDE";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:AMPLITUDE";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:AMPLITUDE";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:AMPLITUDE";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:AMPLITUDE";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:AMPLITUDE";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:AMPLITUDE";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_amp(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_amp(): Allowed amp range 0-300 mA.\n"); 
    printf("caputFFB_amp(): Entered amp = %f mA\n",val); 

    return(0);
  }
}

// set the number of SINEWAVE to produce
// can only be done in CONFIG_STATE
int caputFFB_period(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=1&&val<=511){ */
/*     chName = (char*)caputFFB_coils(chAlias,"RCRINPUT"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=1&&val<=511){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:RCRINPUT";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:RCRINPUT";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:RCRINPUT";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:RCRINPUT";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:RCRINPUT";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:RCRINPUT";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:RCRINPUT";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:RCRINPUT";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:RCRINPUT";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_period(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_period(): Allowed period range 1-511.\n"); 
    printf("caputFFB_period(): Entered period = %f\n",val); 

    return(0);
  }
}

// load/unload the SINEWAVE
// can only be done in CONFIG_STATE
int caputFFB_load(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=0&&val<=1){ */
/*     chName = (char*)caputFFB_coils(chAlias,"SINEWAVE"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=0&&val<=1){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:SINEWAVE";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:SINEWAVE";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:SINEWAVE";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:SINEWAVE";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:SINEWAVE";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:SINEWAVE";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:SINEWAVE";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:SINEWAVE";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:SINEWAVE";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_load(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_load(): Allowed values 0/1.\n"); 
    printf("caputFFB_load(): Entered value = %f \n",val); 

    return(0);
  }
}

// set the "Enter Trigger State" state
// if the "Hardware Trigger" is in initiated state, 
// settig Trigger State to 1 initiates sine wave outputs
int caputFFB_enterTrig(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=0&&val<=1){ */
/*     chName = (char*)caputFFB_coils(chAlias,"TRIGSTATEBTN"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=0&&val<=1){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:TRIGSTATEBTN";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:TRIGSTATEBTN";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:TRIGSTATEBTN";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:TRIGSTATEBTN";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:TRIGSTATEBTN";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:TRIGSTATEBTN";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:TRIGSTATEBTN";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:TRIGSTATEBTN";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:TRIGSTATEBTN";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_enterTrig(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_enterTrig(): Allowed valuess 0/1.\n"); 
    printf("caputFFB_enterTrig(): Entered value = %f \n",val); 

    return(0);
  }
}

// set the "Leave Trigger State" state
// When this state is set to 0, the state will be CONFIG_STATE
int caputFFB_leaveTrig(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=0&&val<=1){ */
/*     chName = (char*)caputFFB_coils(chAlias,"STOPBTN"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=0&&val<=1){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:STOPBTN";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:STOPBTN";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:STOPBTN";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:STOPBTN";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:STOPBTN";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:STOPBTN";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:STOPBTN";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:STOPBTN";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:STOPBTN";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_leaveTrig(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_leaveTrig(): Allowed valuess 0/1.\n"); 
    printf("caputFFB_leaveTrig(): Entered value = %f \n",val); 

    return(0);
  }
}

// set the "Software Trigger" state
int caputFFB_trig(int chAlias, double val) {
/*   char *chName; */
/*   if(val>=0&&val<=1){ */
/*     chName = (char*)caputFFB_coils(chAlias,"TRIGBTN"); */
/*     return caputFFB_gen(chName,val); */

  char *chName;
  char *chDesc;
  chid channelID=0;
  int  status;
  
  if(val>=0&&val<=1){
    switch (chAlias) 
      {      
      case 0:
	chName = "BMOD1:CHAN0:TRIGBTN";
	chDesc = "MHF0I08H";
	break;      
      case 1:
	chName = "BMOD1:CHAN1:TRIGBTN";
	chDesc = "MHF0I08V";
	break;      
      case 2:
	chName = "BMOD1:CHAN2:TRIGBTN";
	chDesc = "MHF0I010H";
	break;      
      case 3:
	chName = "BMOD1:CHAN3:TRIGBTN";
	chDesc = "MHF0I010V";
	break;      
      case 4:
	chName = "BMOD2:CHAN0:TRIGBTN";
	chDesc = "MHF0I01H";
	break;      
      case 5:
	chName = "BMOD2:CHAN1:TRIGBTN";
	chDesc = "MHF0I02H";
	break;      
      case 6:
	chName = "BMOD2:CHAN2:TRIGBTN";
	chDesc = "MHF0I03V";
	break;      
      case 7:
	chName = "BMOD2:CHAN3:TRIGBTN";
	chDesc = "SL20 Energy Modulation";
	break;      
	
      default:
	chName = "BMOD1:CHAN0:TRIGBTN";
	chDesc = "MHF0I08H";
      }
    
    status = ca_task_initialize();
    MY_SEVCHK(status);
    
    status = ca_search(chName, &channelID);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    status = ca_put(DBR_DOUBLE, channelID, &val);
    MY_SEVCHK(status);
    
    status = ca_pend_io(10.0);
    MY_SEVCHK(status);
    
    printf("caputFFB_trig(): %s = %f\n",chName,val);
    
    return(1);

  }else{
    printf("caputFFB_trig(): Allowed valuess 0/1.\n"); 
    printf("caputFFB_trig(): Entered value = %f \n",val); 

    return(0);
  }
}

// set the Relay for the output from the function generator into DAQ
int caputFFB_output(int chAlias, double val) {
  char *chName1;
  char *chName2;
  if(val>=0&&val<=1){
    // not working properly ??
/*     chName1 = caFFB_coilsRelay(chAlias); */
/*     caputFFB_gen(chName1,val); */
/*     chName2 = caFFB_coilsRelay(chAlias+8); */
/*     caputFFB_gen(chName2,val); */
  }else{
    printf("caputFFB_output(): Allowed valuess 0/1.\n"); 
    printf("caputFFB_output(): Entered value = %f \n",val); 
  }
}

int caputFFB_gen(char *chName, double val){

  chid channelID=0;
  int  status;

  status = ca_task_initialize();
  MY_SEVCHK(status);

  status = ca_search(chName, &channelID);
  MY_SEVCHK(status);

  status = ca_pend_io(10.0);
  MY_SEVCHK(status);
  
  status = ca_put(DBR_DOUBLE, channelID, &val);
  MY_SEVCHK(status);

  status = ca_pend_io(10.0);
  MY_SEVCHK(status);

  printf("caputFFB_gen(): %s = %f\n",chName,val);

  return(1);
}

/***************coils ***************/

char *caFFB_coils(int chAlias, char *var) {

  char *chName;
  char *chDesc;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "BMOD1:CHAN0:";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "BMOD1:CHAN1:";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "BMOD1:CHAN2:";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "BMOD1:CHAN3:";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "BMOD2:CHAN0:";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BMOD2:CHAN1:";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BMOD2:CHAN2:";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BMOD2:CHAN3:";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "BMOD1:CHAN0:";
      chDesc = "MHF0I08H";
    }

  strcat(chName, var);

  //  printf("caFFB_coils(): %s (%s)\n", chDesc, chName); 
  return chName;
}

char *caFFB_coilsRelay(int chAlias) {

  char *chName;
  char *chName2;
  char *chDesc;
  
  switch (chAlias) 
    {      
    case 0:
      chName = "B0";
      chDesc = "MHF0I08H";
      break;      
    case 1:
      chName = "B2";
      chDesc = "MHF0I08V";
      break;      
    case 2:
      chName = "B4";
      chDesc = "MHF0I010H";
      break;      
    case 3:
      chName = "B6";
      chDesc = "MHF0I010V";
      break;      
    case 4:
      chName = "B8";
      chDesc = "MHF0I01H";
      break;      
    case 5:
      chName = "BA";
      chDesc = "MHF0I02H";
      break;      
    case 6:
      chName = "BC";
      chDesc = "MHF0I03V";
      break;      
    case 7:
      chName = "BE";
      chDesc = "SL20 Energy Modulation";
      break;      
    case 8:
      chName = "B1";
      chDesc = "MHF0I08H";
      break;      
    case 9:
      chName = "B2";
      chDesc = "MHF0I08V";
      break;      
    case 10:
      chName = "B5";
      chDesc = "MHF0I010H";
      break;      
    case 11:
      chName = "B7";
      chDesc = "MHF0I010V";
      break;      
    case 12:
      chName = "B9";
      chDesc = "MHF0I01H";
      break;      
    case 13:
      chName = "BB";
      chDesc = "MHF0I02H";
      break;      
    case 14:
      chName = "BD";
      chDesc = "MHF0I03V";
      break;      
    case 15:
      chName = "BF";
      chDesc = "SL20 Energy Modulation";
      break;      

    default:
      chName = "B1";
      chDesc = "MHF0I08H";
    }

  chName2 = "BMRELAYSET.";

  strcat(chName2, chName);

  printf("caFFB_coilsRelay(): %s (%s)\n", chDesc, chName2); 
  return chName2;
}
