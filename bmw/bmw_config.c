/*****************************************
*  bmw_config.c                          *
*  routines used to interface the        * 
*  bmw Client with the configuration GUI *
*                                        *
*  K. Paschke           May, 2002        *
******************************************/

#include <vxWorks.h>
#include <types.h>
#include <vme.h>
#include <stdioLib.h>
#include <logLib.h>
#include <math.h>
#include <string.h>
#include <ioLib.h>

#include "bmw_config.h"
#include "../cfSock/cfSock.h"

extern STATUS bmwClient();

void task_BMW(long* command, long *par1, long *par2)
{
  int junk;
  char                workName[16];  /* name of work task */
  switch (*command)
    {
    case BMW_KILL:
      // set kill flag for bmw
      bmw_die_die_die = TRUE;
      return;
    case BMW_GET_STATUS:
      // return flag for status of bmw
      *par1=bmw_status;
      *par2=bmw_die_die_die;
      return;
    case BMW_CHECK_ALIVE:
      // return flag for status of bmw
      *par1=bmw_status;
      *par2=bmw_alive;
      bmw_alive = 0;
      return;
    case BMW_START:
      printf("starting bmwClient\n");
      bmw_die_die_die = FALSE;
      taskSpawn("bmwClient", SERVER_WORK_PRIORITY, 0, SERVER_STACK_SIZE,
		bmwClient,0,0,0,0, 0, 0, 0, 0, 0, 0);      
      return;
    }

}










