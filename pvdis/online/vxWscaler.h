/* vxWscaler.h  -- used by server and client */

/* The following defines the IP address of the server */
/* pvdis1.jlab.org on R-HRS */
#define SERVER_INET_ADDR "129.57.192.58"   
/* The following defines the port number of the server */
#define SERVER_PORT_NUM (5026)

#define MSG_SIZE (50)
#include "scaler1.h"

/* structure for requests from client */
struct request
{ 
int reply;
long ibuf[16*MAXBLOCKS]; 
char message[MSG_SIZE];
int clearflag;
int checkend;
};


#define SERVER_WORK_PRIORITY  (100)
#define SERVER_STACK_SIZE (10000)
#define MAX_QUEUED_CONNECTIONS (5)

#define SOCK_ADDR_SIZE  sizeof(struct sockaddr_in))
#define REPLY_MSG_SIZE (1000)

#define ERROR (-1)

