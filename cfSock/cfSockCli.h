/* cfSockCli.h - header used by cfSock client, replacing the
                 missing vxWorks.h  */

#if	!defined(FALSE) || (FALSE!=0)
#define FALSE		0
#endif

#if	!defined(TRUE) || (TRUE!=1)
#define TRUE		1
#endif

/* low-level I/O input, output, error fd's */
#define	STD_IN	0
#define	STD_OUT	1
#define	STD_ERR	2

#include "GreenSock.h"
#include "cfSock.h"

/* Name or IP addresses of the various servers */
char * ServerName_CountingHouse="129.57.164.13"; 
char * ServerName_LeftSpect="0.0.0.0"; 
char * ServerName_RightSpect="0.0.0.0";
char * ServerName_Injector="0.0.0.0";   

