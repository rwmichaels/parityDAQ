// Necessary modifications:
// 1) use array of modulation parameters, to allow different modulation
//  amplitudes on different magnets.
// 2) have bmwClient_test put its data words in the data stream like the loop does
// 3  bmwTest should make sure to set only one coil at a time (by unsetting when
//    coil number changes
// 4) greenmonster should allow changing the bmw words online.

/* launch with:

ld < 'bmwc2.o'

sp bmwClient

 */

/*
  $Header$
 */

/***** includes *****/

#include <vxWorks.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <selectLib.h>
#include <stdio.h>
#include <netinet/tcp.h>

#include "bmw.h"


/***** defines *****/
#define NUM_SERVERS 1
#define CHECK_OBJ_LIST 0 /* Do not check object list */

/***** locals *****/
char server_inet_addr[16][NUM_SERVERS];
LOCAL struct sockaddr_in  serverAddr[NUM_SERVERS]; /* server's address */
LOCAL int		  bmwSock[NUM_SERVERS];    /* socket file descriptor */
LOCAL float               val_read[MAX_OBJS];      /* object value from readback */
LOCAL float               val_set[MAX_OBJS];       /* object value to be set */
LOCAL float               val_zero[MAX_OBJS];      /* quiescent value of object */
LOCAL float               val_center[MAX_OBJS];    /* central value of object */
LOCAL float               val_mod[MAX_OBJS];       /* amount to change value by */
LOCAL objlist             objl_reply[NUM_SERVERS]; /* the replies */
LOCAL int                 stats[3];    /* number of errors, vetoes, and OKs */
LOCAL objlist             bmwObjlist;              /* the object list */


/***** globals *****/

int bmw_c_verbose = 0;                /* 1 to get lots of diagnostics */
int bmw_c_terse = 0;                  /* 1 to get a few diagnostics */

BOOL bmw_test_server = FALSE;          /* TRUE to test with dummy server */

/* These globals are used to communicate between the client and the ROC
 * task. */

int   bmw_object = -1;             /* Object being moved or most
				    * recently moved */
BOOL  bmw_clean = TRUE;           /* TRUE if this object is
				    * not being moved right now
				    * (set & cleared here) */
BOOL  bmw_clean_pulse = TRUE;     /* TRUE if this object was
				    * not being moved during
				    * this pulse (set here,
				    * cleared in ROC) */
int   bmw_value;                   /* Value object is set to */
char  bmw_objname[MAX_NAMELENGTH]; /* Name of this object */

int   bmw_cycle_count = 0;            // A running count of dithering cycles
int   bmw_cycle_number;            // Holds cycle number, or zero if cycle is over

/* This global flag is used to communicate a death-wish from users to
   the otherwise infinite bmw loop */
BOOL  bmw_die_die_die = FALSE; 

/* Global flag used to keep the total number of Clients to 1, and to
   provide status for external programs */
int  bmw_status = 0; 
int  bmw_alive = 0; 

// globals used only to communicate with test2
int bmw_test_object=1;
int bmw_test_value =0;
/* Modulation parameters */

/* 
 * Modulation pattern is: One cycle is <num_steps> up, 2*<num_steps> down, 
 * <num_steps> back.  Do <num_cycles> consecutive cycles, wait <num_pause>
 * steps, then go on to the next object.
 */

/*  int bmw_steps_per_qcycle =    3; /* Number of steps per 1/4 cycle */ 
/*  int bmw_cycles_per_pulse =    1; /* Number of cycles per pulse */ 
/*  int bmw_steps_per_pause  =  192; /* Number of steps to rest after last object */ 
/*  int bmw_ticks_per_step   =   12; /* Clock ticks per step */ 
/*  int bmw_coil_amplitude   =  400; /* Half amplitude for coil modulation (milliamps) */ 
/*  int bmw_coil_offset      =    0; /* DC offset for coil modulation (milliamps) */ 
/*  int bmw_e_amplitude      =  500; /* Half amplitude for energy modulation */
/*  				  * (keV per pass) */ 
/*  int bmw_e_offset         =    0; /* DC offset for energy modulation */
/*  				  * (keV per pass) */ 
/*  int bmw_starting_object  =    0; /* Which object to start with */ 
/*  int bmw_ending_object =       7; /* Which object to end with */ 


int bmw_steps_per_qcycle =    3; /* Number of steps per 1/4 cycle */ 
int bmw_cycles_per_pulse =    1; /* Number of cycles per pulse */ 
int bmw_steps_per_pause  =  192; /* Number of steps to rest after last object */ 
int bmw_ticks_per_step   =   12; /* Clock ticks per step */ 
int bmw_coil_amplitude   =  300; /* Half amplitude for coil modulation (milliamps)*/
int bmw_coil_offset      =    0; /* DC offset for coil modulation (milliamps) */ 
int bmw_e_amplitude      =    0; /* Half amplitude for energy modulation */
//                                   (keV per pass) 
int bmw_e_offset         =    0; /* DC offset for energy modulation */
//                                 * (keV per pass)
int bmw_starting_object  =    0; /* Which object to start with */ 
int bmw_ending_object =       7; /* Which object to end with */ 

/***** forward declarations *****/
STATUS prepareBMW ( int server, int connected[], int objToServer[] );
STATUS setupSocketBMW ( int server );
STATUS connectToServerBMW ( int server );
STATUS sayHelloBMW ( int server, struct reqreply *p_request );
STATUS getHelloReplyBMW ( int server, struct reqreply *p_request );
STATUS modulateObjectBMW ( int object, int server );
STATUS closeSocketBMW ( int server );
STATUS putReqBMW ( int req_type, int req_object, float req_value, int server,
		struct reqreply *p_request );
STATUS handleReplyBMW ( int server, struct reqreply *p_request );
STATUS putObjlistBMW ( objlist *p_objs, int server );
STATUS handleObjlistReplyBMW ( int server, objlist *p_objs, objlist *p_objreply );
STATUS waitReplyBMW ( int server );

STATUS bmwClient (int i) 
{
  if (i==1) return bmwClient_script();
  if (i==2) return bmwClient_test();
  if (i==3) return bmwClient_test2();
  return (ERROR);
}


STATUS bmwClient_test ()
{
  int i;
  int server;    /* server index */
  int object;    /* object index */
  int connected[NUM_SERVERS];  /* is server connected? */
  int objToServer[MAX_OBJS];   /* server index for each object */
  int modStat;   /* status returned by modulateObjectBMW */
  struct reqreply       request;    /* the request structure */
  char instring[100];
  double setpnt;
  float max;

  if (bmw_status != 0) return (ERROR);

  bmw_status = 2;
  if ( bmw_test_server )
    strcpy ( server_inet_addr[0], 
	     "129.57.164.13" );  /* halladaq6 -- Dummy server */
  else
    strcpy ( server_inet_addr[0], 
	     "129.57.236.67" );  /* iocsb4 -- A line coils, south linac
				  * energy vernier */

  for ( i = 0; i < 3; ++i)
    stats[i] = 0;
  fprintf(stderr,"bmwClient:preparing to PREPARE\n");
  for ( server=0; server<NUM_SERVERS; ++server )
    prepareBMW ( server, connected, objToServer );

  for ( object = 0; object<MAX_OBJS; ++object )
    fprintf ( stderr, "bmwClient::: Object %3d server %3d \n", object, objToServer[object] );

  while (!bmw_die_die_die)
    {
      // prompt for object or kill flag
      fprintf(stdout,"Input object name, with -1 to end.\n");
      if (read(fileno(stdin),instring,100)>1) object = atol(instring);
      if (object == -1) { 
	bmw_status = 0;
	closeSocketBMW(server);
	return (OK);
      } else if (object<bmw_starting_object || object >bmw_ending_object) {
	fprintf(stdout,"Invalid object number:%d (up to %d allowed).\n", object,
		bmw_ending_object);
	bmw_status=0;
	closeSocketBMW(server);
	return (ERROR);
      }
      // prompt for value
      max = 0.5; setpnt = 0;
      fprintf(stdout,"Input set point in mA (or keV).\n");
      if (read(fileno(stdin),instring,100)>1) 
	{ 
	  setpnt = atol(instring);
	  fprintf(stdout,
		  "  instring is: %s\n",instring);
	  setpnt = setpnt/1000.;
	  fprintf(stdout,
		  "  requesting setpoint of %f with max magnitude of %f \n",
		  setpnt,max);
	}
      if (setpnt>max || setpnt<-max) 
	{
	  fprintf(stdout,"Evil! Setpoint out of range!\n");
	  bmw_status=0;
	  closeSocketBMW(server);
	  return (ERROR);
	}

      // check for server, connect it if it isn't already
      server = objToServer[object];
      if ( server >= 0 )
	{
	  /* If the server gave an error last time, reconnect to it */
	  if ( !connected[server] )
	    {
	      if ( bmw_c_verbose || bmw_c_terse )
		fprintf ( stderr, 
		    "bmwClient::: Reconnecting to server %d (IP address %s)\n", 
		    server, server_inet_addr[server] );
		  closeSocketBMW ( server );
		  prepareBMW (server, connected, objToServer );
	    }
	  
	  // set value for object

	  if ( ( putReqBMW ( C_SET, object, setpnt, server, &request ) == ERROR ) ||
	       ( handleReplyBMW ( server, &request ) == ERROR ) )
	    {
	      // if error received, reconnect and set it
	      bmw_object = -1;
	      connected[server] = 0;
	      if ( bmw_c_verbose || bmw_c_terse ) {
		fprintf ( stderr, 
		   "bmwClient::: Reconnecting to server %d (IP address %s)\n", 
		    server, server_inet_addr[server] );
		closeSocketBMW ( server );
		prepareBMW (server, connected, objToServer );
	      }
	      if ( ( putReqBMW ( C_SET, object, 
				 setpnt, server, &request ) == ERROR ) ||
		   ( handleReplyBMW ( server, &request ) == ERROR ) )
		{
		  bmw_object = -1;
		  connected[server] = 0;
		  fprintf ( stderr, 
			    "bmwClient::: Error setting object %d \n", 
			    object );
		}
	    }
	  bmw_object = -1;
	}
      else
	{
	  /* If no server wants this object, reconnect all servers */
	  fprintf ( stderr, 
		    "bmwClient::: No server for object %d\n", object );
	  for ( server=0; server<NUM_SERVERS; ++server )
	    {
	      closeSocketBMW ( server );
	      prepareBMW (server, connected, objToServer );
	    }
	}
    }
  bmw_status =0;
  closeSocketBMW(server);
  return (OK);
}


STATUS bmwClient_script ()
{
  int i;
  int server;    /* server index */
  int object;    /* object index */
  int connected[NUM_SERVERS];  /* is server connected? */
  int objToServer[MAX_OBJS];   /* server index for each object */
  int modStat;   /* status returned by modulateObjectBMW */
  struct reqreply       request;    /* the request structure */
  //  int icnt;  // used for limiting number of cycles

  if (bmw_status != 0) return (ERROR);
  bmw_status =1;
  bmw_alive =1;

  if ( bmw_test_server )
    strcpy ( server_inet_addr[0], 
	     "129.57.164.13" );  /* halladaq6 -- Dummy server */
  else
    strcpy ( server_inet_addr[0], 
	     "129.57.236.67" );  /* iocsb4 -- A line coils, south linac
				  * energy vernier */

  bmw_alive =1;
  for ( i = 0; i < 3; ++i)
    stats[i] = 0;

  bmw_alive =1;
  if ( bmw_c_verbose )
    fprintf(stderr,"bmwClient:preparing to PREPARE\n");
  for ( server=0; server<NUM_SERVERS; ++server )
    prepareBMW ( server, connected, objToServer );

  bmw_alive =1;
  for ( object = 0; object<MAX_OBJS; ++object )
    if ( bmw_c_verbose || bmw_c_terse )
      fprintf ( stderr, "bmwClient::: Object %3d server %3d \n", object, objToServer[object] );

  //  icnt = 0;
  //  while (icnt<2 && !bmw_die_die_die)
  while (!bmw_die_die_die)
    {
      bmw_cycle_number = ++bmw_cycle_count;
      //      icnt = icnt +1;
      bmw_alive =1;
      // Loop over objects and modulate each
      for ( object = bmw_starting_object; 
	    object<=bmw_ending_object && !bmw_die_die_die; ++object )
	{
	  bmw_alive =1;
	  // Find out which server to use; if < 0, no server wants it...
	  server = objToServer[object];
	  if ( server >= 0 )
	    {
	      // If the server gave an error last time, reconnect to it
	      if ( !connected[server] )
		{
		  if ( bmw_c_verbose || bmw_c_terse )
		    fprintf ( stderr, 
			      "bmwClient::: Reconnecting to server %d (IP address %s)\n", 
			      server, server_inet_addr[server] );
		  closeSocketBMW ( server );
		  prepareBMW (server, connected, objToServer );
		}

	      // If server is now connected, try modulating
	      if ( connected[server] )
		{
		  // Modulate; if error occurs, tally it and mark server as
		  //   disconnected 
		  modStat = modulateObjectBMW ( object, server );
		  bmw_object = -1;
		  if ( modStat == ERROR )
		    {
		      ++stats[0];
		      fprintf ( stderr, 
				"bmwClient::: Errors %d / vetoes %d / OKs %d\n", 
				stats[0], stats[1], stats[2] );
		      connected[server] = 0;
		    }
		}
	    }
	  else
	    {
	      bmw_alive =1;
	      // If no server wants this object, reconnect all servers
	      fprintf ( stderr, 
			"bmwClient::: No server for object %d\n", object );
	      for ( server=0; server<NUM_SERVERS; ++server )
		{
		  closeSocketBMW ( server );
		  prepareBMW (server, connected, objToServer );
		}
	    }
	}
      // After last object (whether successful or not), take a break
      bmw_cycle_number =0;
      if ( bmw_c_verbose || bmw_c_terse )
	fprintf ( stderr, "bmwClient::: starting rest period\n");
      fprintf ( stderr, "bmwClient::: Errors %d / vetoes %d / OKs %d\n", 
		stats[0], stats[1], stats[2] );
//         Here is a kludge.  The server timeout is too short, so until
//          that's fixed, we bother the server every 10 steps for a
//          readback which we then ignore.  That should be frequent
//          enough to keep from timing out.  Once the server is modified
//          for a longer timeout we can replace this loop with:
//       
//         taskDelay ( bmw_ticks_per_step * bmw_steps_per_pause );
//       
//         This kludge implicitly assumes there is in fact only one
//         server for everything, so we only have to prod the server for
//         object 0.  This should work under present circumstances.  */
      for ( i = 0; i < bmw_steps_per_pause && !bmw_die_die_die; ++i)
	{
	  bmw_alive =1;
 	  taskDelay ( bmw_ticks_per_step );
	  if ( i % 10 == 0 )
	    {
	      if ( ( putReqBMW ( C_READBACK, 0, 0.0, objToServer[0], &request ) 
		     == ERROR ) ||
		   ( handleReplyBMW ( objToServer[0], &request ) == ERROR ) )
		connected[objToServer[0]] = 0;
	    }
	}
    }
  bmw_status =0;
  closeSocketBMW(server);
  return (OK);
}



STATUS bmwClient_test2 ()
{
  int i;
  int server;    /* server index */
  int object;    /* object index */
  int connected[NUM_SERVERS];  /* is server connected? */
  int objToServer[MAX_OBJS];   /* server index for each object */
  int modStat;   /* status returned by modulateObjectBMW */
  struct reqreply       request;    /* the request structure */
  float max;
  double setpnt;
  if (bmw_status != 0) return (ERROR);
  bmw_status =1;
  bmw_alive =1;

  if ( bmw_test_server )
    strcpy ( server_inet_addr[0], 
	     "129.57.164.13" );  /* halladaq6 -- Dummy server */
  else
    strcpy ( server_inet_addr[0], 
	     "129.57.236.67" );  /* iocsb4 -- A line coils, south linac
				  * energy vernier */

  bmw_alive =1;
  for ( i = 0; i < 3; ++i)
    stats[i] = 0;

  bmw_alive =1;
  if ( bmw_c_verbose )
    fprintf(stderr,"bmwClient:preparing to PREPARE\n");
  for ( server=0; server<NUM_SERVERS; ++server )
    prepareBMW ( server, connected, objToServer );

  bmw_alive =1;
  for ( object = 0; object<MAX_OBJS; ++object )
    if ( bmw_c_verbose || bmw_c_terse )
      fprintf ( stderr, "bmwClient::: Object %3d server %3d \n", object, objToServer[object] );

  while (!bmw_die_die_die)
    {
      bmw_alive =1;

      object = bmw_test_object;
      if (object<bmw_starting_object || object >bmw_ending_object) {
	fprintf(stdout,"Invalid object number:%d (up to %d allowed).\n", object,
		bmw_ending_object);
	bmw_status=0;
	closeSocketBMW(server);
	return (ERROR);
      }

      // prompt for value
      max = 0.5; setpnt = 0;
      setpnt = bmw_test_value;
      setpnt = setpnt/1000.;
      fprintf(stdout,
	      "  requesting setpoint of %f for %d (max magnitude = %f) \n",
	      setpnt,object,max);
      if (setpnt>max || setpnt<-max) 
	{
	  fprintf(stdout,"Evil! Setpoint out of range!\n");
	  bmw_status=0;
	  closeSocketBMW(server);
	  return (ERROR);
	}

      // check for server, connect it if it isn't already
      server = objToServer[object];
      if ( server >= 0 )
	{
	  /* If the server gave an error last time, reconnect to it */
	  if ( !connected[server] )
	    {
	      if ( bmw_c_verbose || bmw_c_terse )
		fprintf ( stderr, 
		    "bmwClient::: Reconnecting to server %d (IP address %s)\n", 
		    server, server_inet_addr[server] );
		  closeSocketBMW ( server );
		  prepareBMW (server, connected, objToServer );
	    }
	  
	  // set value for object

	  if ( ( putReqBMW ( C_SET, object, setpnt, server, &request ) == ERROR ) ||
	       ( handleReplyBMW ( server, &request ) == ERROR ) )
	    {
	      // if error received, reconnect and set it
	      bmw_object = -1;
	      connected[server] = 0;
	      if ( bmw_c_verbose || bmw_c_terse ) {
		fprintf ( stderr, 
		   "bmwClient::: Reconnecting to server %d (IP address %s)\n", 
		    server, server_inet_addr[server] );
		closeSocketBMW ( server );
		prepareBMW (server, connected, objToServer );
	      }
	      if ( ( putReqBMW ( C_SET, object, 
				 setpnt, server, &request ) == ERROR ) ||
		   ( handleReplyBMW ( server, &request ) == ERROR ) )
		{
		  bmw_object = -1;
		  connected[server] = 0;
		  fprintf ( stderr, 
			    "bmwClient::: Error setting object %d \n", 
			    object );
		}
	    }
	  bmw_object = -1;
	}
      else
	{
	  /* If no server wants this object, reconnect all servers */
	  fprintf ( stderr, 
		    "bmwClient::: No server for object %d\n", object );
	  for ( server=0; server<NUM_SERVERS; ++server )
	    {
	      closeSocketBMW ( server );
	      prepareBMW (server, connected, objToServer );
	    }
	}
      bmw_alive =1;
      for (i = 0; i<10 && !bmw_die_die_die; i++) {
	taskDelay ( bmw_ticks_per_step );
      }
    }
  bmw_test_value = 0;
  bmw_test_object =1;
  bmw_status =0;
  closeSocketBMW(server);
  return (OK);
}

/******************************************************************************

prepareBMW - set up connection to a server

This routine opens a connection to a server and says hello to it,
finding out which objects it can modulate.  Set connected[server] to 0
or 1 as appropriate; scan all servers and fill objToServer.

Returns - ERROR if any of the internal calls return ERROR, otherwise OK.

*/

STATUS prepareBMW ( int server, int connected[], int objToServer[] )
{
  STATUS status;
  int jserver;
  int object;
  struct reqreply request;

  status = ERROR;
  if ( bmw_c_verbose )
    {
      fprintf ( stderr, 
		"bmwClient::: Attempting to connect to server %d (IP address %s)\n", 
		server, server_inet_addr[server] );
    }
  if ( setupSocketBMW ( server ) == OK )
    {
      if ( connectToServerBMW ( server ) == OK )
	{
	  if ( setupRequests ( server ) == OK )
	    {
	      if ( sayHelloBMW ( server, &request ) == OK )
		{
		  if ( getHelloReplyBMW ( server, &request ) == OK )
		      status = OK;
		}
	    }
	}
      if ( status == ERROR )
	closeSocketBMW ( server );
    }
  connected[server] = ( status == OK );

  if ( bmw_c_verbose )
    {
      fprintf ( stderr, 
		"bmwClient::: Completed connection attempt to server %d (IP address %s)\n", 
		server, server_inet_addr[server] );
    }


  for ( object = 0; object<MAX_OBJS; ++object )
    {
      objToServer[object] = -1;
      for ( jserver=0; jserver<NUM_SERVERS; ++jserver )
	if ( strcmp ( objl_reply[jserver][object], "OK" ) == 0 )
	  objToServer[object] = jserver;
    }
  return (status);
}

/******************************************************************************

setupSocketBMW - get a socket ready for a connection.

This routine opens a socket and gets the server's IP address in preparation 
for making a TCP connection.

Returns - ERROR if any of the internal calls return ERROR, otherwise OK.

*/

STATUS setupSocketBMW ( int server )
{
  char  optval;     /* value for setsockopt */

  /* Zero out the sock_addr structure. */

  bzero ( (char *) &serverAddr[server], sizeof (serverAddr[server]) );

  /* Open the socket. */
  bmwSock[server] = socket (AF_INET, SOCK_STREAM, 0);
  if ( bmwSock[server] == ERROR )
    {
      perror ("bmwClient::: socket");
      return (ERROR);
    }

  optval = 1;
  setsockopt ( bmwSock[server], IPPROTO_TCP, TCP_NODELAY, 
	       (char *) &optval,
	       4 ); /* black magic from Chowdhary's code */

  setsockopt(bmwSock[server], SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  serverAddr[server].sin_family = AF_INET;
  serverAddr[server].sin_port   = htons ( SERVER_NUM );

  /* get server's Internet address */
  if ( ( serverAddr[server].sin_addr.s_addr = 
	 inet_addr ( server_inet_addr[server] ) ) == ERROR && 
       ( serverAddr[server].sin_addr.s_addr = 
	 hostGetByName ( server_inet_addr[server] ) ) == ERROR) 
    {
      perror ("bmwClient::: server address");
      close (bmwSock[server]);
      return (ERROR);
    }
  if ( bmw_c_verbose ) fprintf ( stderr, "bmwClient::: Finishing setupSocketBMW\n");

  return (OK);
}

/******************************************************************************

connectToServerBMW - set up connection

This routine sets up a connection request to the server.

Returns - ERROR if any of the internal calls return ERROR, otherwise OK.

*/

STATUS connectToServerBMW ( int server )
{
  int status;     /* result of attempt to request */

  /* request connection to the server */

  status = connect (bmwSock[server], (struct sockaddr *) &serverAddr[server],
		    sizeof (serverAddr[server]));
  if ( status == ERROR )
    {
      perror ("bmwClient::: connect");
      return (ERROR);
    }
  else
    if ( bmw_c_verbose || bmw_c_terse )
      fprintf ( stderr, "bmwClient::: Connection requested on server %d (IP address %s)\n",
		server, server_inet_addr[server] );

  return (OK);
}

STATUS setupRequests ()
{
  int i;          /* object index */

  /* Init object list */
  
  for ( i = 0; i < MAX_OBJS; i++ )
    strcpy ( bmwObjlist[i], "" );
  strcpy ( bmwObjlist[0], "MAT1C01H" );
  strcpy ( bmwObjlist[1], "MAT1C03H" );
  strcpy ( bmwObjlist[2], "MAT1C02V" );
  strcpy ( bmwObjlist[3], "MAT1C04H" );
  strcpy ( bmwObjlist[4], "MAT1C05H" );
  strcpy ( bmwObjlist[5], "MAT1C06V" );
  strcpy ( bmwObjlist[6], "MAT1C07V" );
  strcpy ( bmwObjlist[7], "E" ); 
  fprintf ( stderr, "bmwClient::: Object list initialized\n");
  for ( i = 0; i < MAX_OBJS; i++ )
    fprintf ( stderr, "bmwClient::: %2d <%10s>\n", i, bmwObjlist[i] );

  return (OK);
}

STATUS sayHelloBMW ( int server, struct reqreply *p_request )
{
  return ( putReqBMW ( C_HELLO, 0, 0.0, server, p_request ) );
}

STATUS getHelloReplyBMW ( int server, struct reqreply *p_request )
{
  return ( handleReplyBMW ( server, p_request ) );
}

/******************************************************************************

modulateObjectBMW - Handle a full modulation sequence for a single
object.

Object i is read for its present value, then gets modulated according
to the predefined prescription.  At any time if an error occurs,
modulation is aborted.

Returns: OK if sequence is completed without error,
         result of last putReqBMW if it is not OK.
*/

int modulateObjectBMW ( int object, int server )
{
  int                   count;      /* where we are in cycle */
  int                   cycle;      /* what cycle we're on */
  struct reqreply       request;    /* the request structure */

  if ( bmw_c_verbose || bmw_c_terse )
    fprintf ( stderr, 
	      "bmwClient::: starting to modulate object %d on server %d (IP address %s)\n",
	      object, server, server_inet_addr[server] );
  strcpy ( bmw_objname, bmwObjlist[object] );

#if 0
  if ( ( putReq ( C_READBACK, object, 0.0, server, &request ) == ERROR ) ||
       ( handleReplyBMW ( server, &request ) == ERROR ) )
    return (ERROR);

  val_zero[object] = val_read[object];
#endif
  /* 4/98 -- always modulate from zero */
  val_zero[object] = 0.0;

  if ( object < 7 )
    {
      val_center[object] = val_zero[object] + bmw_coil_offset / 1000.0;
      val_mod[object]  = ( bmw_coil_amplitude / 1000.0) / bmw_steps_per_qcycle;
      if ( bmw_c_verbose || bmw_c_terse )
	fprintf ( stderr, 
		  "bmwClient::: amp %d offset %d steps %d val_mod %f\n",
		  bmw_coil_amplitude, bmw_coil_offset, bmw_steps_per_qcycle,
		  val_mod[object]);
    }
  else
    {
      val_center[object] = val_zero[object] + bmw_e_offset / 1000.0;
      val_mod[object]  = ( bmw_e_amplitude / 1000.0) / bmw_steps_per_qcycle;
      if ( bmw_c_verbose || bmw_c_terse )
	fprintf ( stderr, 
		  "bmwClient::: amp %d offset %d steps %d val_mod %f\n",
		  bmw_e_amplitude, bmw_e_offset, bmw_steps_per_qcycle,
		  val_mod[object]);
    }


  for ( cycle = 0; cycle < bmw_cycles_per_pulse && !bmw_die_die_die; cycle++ )
    {
      bmw_alive =1;
      if ( bmw_c_verbose || bmw_c_terse )
	fprintf ( stderr, "bmwClient::: cycle %d = = = = = = = = = = = = = = =\n", cycle);
      val_set[object] = val_center[object];
      for ( count = 0; count < bmw_steps_per_qcycle && !bmw_die_die_die; count++ )
	{
	  bmw_alive =1;
	  val_set[object] += val_mod[object];
	  if ( ( putReqBMW ( C_SET, object, val_set[object], server, &request ) == ERROR ) ||
	       ( handleReplyBMW ( server, &request ) == ERROR ) )
	    return (ERROR);
	  taskDelay (bmw_ticks_per_step);
	}
      
      val_set[object] += val_mod[object];
      for ( ; count < bmw_steps_per_qcycle * 3 + 1 && !bmw_die_die_die; count++ )
	{
	  bmw_alive =1;
	  val_set[object] -= val_mod[object];
	  if ( ( putReqBMW ( C_SET, object, val_set[object], server, &request ) == ERROR ) ||
	       ( handleReplyBMW ( server, &request ) == ERROR ) )
	    return (ERROR);
	  taskDelay (bmw_ticks_per_step);
	}
      
      val_set[object] -= val_mod[object];
      for ( ; count < bmw_steps_per_qcycle * 4 + 2 && !bmw_die_die_die; count++ )
	{
	  bmw_alive =1;
	  val_set[object] += val_mod[object];
	  if ( ( putReqBMW ( C_SET, object, val_set[object], server, &request ) == ERROR ) ||
	       ( handleReplyBMW ( server, &request ) == ERROR ) )
	    return (ERROR);
	  taskDelay (bmw_ticks_per_step);
	}

      val_set[object] = val_zero[object];
      bmw_alive =1;
      if ( ( putReqBMW ( C_SET, object, val_set[object], server, &request ) == ERROR ) ||
	   ( handleReplyBMW ( server, &request ) == ERROR ) )
	return (ERROR);
    }

  if ( bmw_c_verbose || bmw_c_terse )
    fprintf ( stderr, "bmwClient::: finished modulating object %d\n", object);

  return (OK);
}

STATUS closeSocketBMW ( int server )
{
  close (bmwSock[server]);
  taskDelay ( bmw_ticks_per_step );
  return (OK);
}

/******************************************************************************

putReqBMW - Make request 

This routine sends a request to the server.

Returns - ERROR if any of the internal calls return ERROR, otherwise OK.

*/

STATUS putReqBMW ( int req_type, int req_object, float req_value, int server,
		struct reqreply *p_request )
{
  STATUS                status;     /* result of attempt to request */

  /* beginning to write to the server side     */

  p_request->req = req_type;
  p_request->object = req_object;
  p_request->value = req_value;

/* If we are setting an object, mark it unclean until server says it's */
/* ready */

  if ( p_request->req == C_SET )
    {
      bmw_clean = FALSE;
      bmw_clean_pulse = FALSE;
    }

  if ( bmw_c_verbose )
    fprintf ( stderr, "bmwClient::: Writing request %d %d %f to server %d (IP address %s)\n",
	      p_request->req, p_request->object, p_request->value,
	      server, server_inet_addr[server] );

  status = write (bmwSock[server], (char *) p_request, sizeof(*p_request) );

  if ( status != sizeof (*p_request) ) 
    {
      perror ("bmwClient::: write");
      fprintf ( stderr, "status = %d sizeof(request) = %d\n", status, sizeof (*p_request) );
      return (ERROR);
    }

  return (OK);
}

/******************************************************************************

handleReplyBMW - process the response to a request

Get the response from the server to the request passed in argument p_request; 
check it for validity.

Returns - ERROR if error occurs
          VETO if server rejects request, or sends wrong readback
          OK if no problem
*/
STATUS handleReplyBMW ( int server, struct reqreply *p_request )
{
  int                   status;     /* result of attempt to request */
  struct reqreply       reply;      /* reply from server */
  int                   bytes;      /* number of bytes read from */
				    /* socket */
  int                   object;     /* the object of our desire */
  int                   i;

  object = p_request->object;

  /*
  if ( bmw_c_verbose )
     fprintf ( stderr, "bmwClient::: Awaiting reply...\n");
  */

  status = waitReplyBMW ( server );
  if (status == ERROR) 
    {
      perror ("bmwClient::: waitReplyBMW");
      return (ERROR);
    }

  /* select says ready to read data from the read socket    */ 

  /*
  if ( bmw_c_verbose )
    fprintf ( stderr, "bmwClient::: Getting reply...\n");
  */

  bytes = read ( bmwSock[server], (char *) &reply, sizeof(reply) );
  if ( bytes == 0 )
    {
      perror ("bmwClient::: read");
      return (ERROR);
    }
  if ( bmw_c_verbose ) 
    fprintf ( stderr, "bmwClient::: reply = %d %d %f from server %d (IP address %s) \n", 
	      reply.req, reply.object, reply.value, server,
	      server_inet_addr[server] );

  if ( reply.req == S_UNKN_REQ )
    {
      fprintf ( stderr, "bmwClient::: server %d (IP address %s) fails to recognize request\n",
		server, server_inet_addr[server] );
      return ( ERROR );
    }
  if ( reply.object != p_request->object )
    {
      fprintf ( stderr, "bmwClient::: server %d (IP address %s) responded with wrong object\n",
		server, server_inet_addr[server] );
      return ( ERROR );
    }

  status = OK;
  switch ( p_request->req ) 
    {
    case C_HELLO:
      if ( reply.req != S_HELLO )
	{
	  perror ("bmwClient::: bad reply");
	  return (ERROR);
	}

#if CHECK_OBJ_LIST
      /* Send object list */
      printf ( "Sending object list to be checked\n" );
      if ( putObjlistBMW ( &bmwObjlist, server ) == OK )
	status = handleObjlistReplyBMW ( server, &bmwObjlist, &(objl_reply[server]) );
#else
      /* Assume object list is OK */
      printf ( "Assuming object list is OK\n" );
      for ( i = 0; i < MAX_OBJS; i++ )
	strcpy (objl_reply[server][i], "OK" );
#endif

      break;
    case C_SET:
    case C_READBACK:
      switch ( reply.req )
	{      
	case S_OK:
	  val_read[object] = reply.value;
	  if ( bmw_c_verbose )
	    {
	      fprintf ( stderr, 
			"bmwClient::: Request OK; requested, readback values = %g %g\n", 
			val_set[object], val_read[object]);
	    }
	  if ( p_request->req == C_SET )
	    {
	      // readout with 2-digit precision
	      bmw_value = (int) (100000.0 * val_read[object]); 
	      bmw_object = object;
	      bmw_clean = TRUE;
	    }
	  status = OK;
	  break;
	case S_LOW: /* should happen only for C_SET */
	  val_read[object] = reply.value;
	  fprintf ( stderr, "bmwClient::: value too low\n");
	  fprintf ( stderr, "bmwClient::: readback value = %g\n", val_read[object]);
	  status = VETO;
	  break;
	case S_HIGH: /* should happen only for C_SET */
	  val_read[object] = reply.value;
	  fprintf ( stderr, "bmwClient::: value too high\n");
	  fprintf ( stderr, "bmwClient::: readback value = %g\n", val_read[object]);
	  status = VETO;
	  break;
	case S_NOT_MINE:
	  fprintf ( stderr, "bmwClient::: object %d not recognized by server %d (IP address %s)\n",
		  object, server, server_inet_addr[server] );
	  status = ERROR;
	  break;
	default:
	  fprintf ( stderr, "bmwClient::: unrecognized error from server %d (IP address %s)\n",
		  server, server_inet_addr[server] );
	  status = ERROR;
	  break;
	}
      break;
    default:
    }
  if ( status == VETO )
    ++stats[1];
  if ( status == OK )
    ++stats[2];
  return (status);
}
  
/******************************************************************************

putObjlistBMW - Send object list.

This routine sends an object list to the server.

Returns - ERROR if we fail badly
          OK if no problem
*/

STATUS putObjlistBMW ( objlist *p_objs, int server )
{
  int                   status;     /* result of attempt to request */

  /* beginning to write to the server side     */ 	      

  if ( bmw_c_verbose )
    fprintf ( stderr, "bmwClient::: Putting object list\n");
   

  status = write ( bmwSock[server], (char *) p_objs, SIZE_OBJLIST );
  if ( status != SIZE_OBJLIST )
    {
      perror ("bmwClient::: write");
      return (ERROR);
    }

  return (OK);
}

/******************************************************************************

handleObjlistReplyBMW - process the response to an object list

Get the response from the server to the object list in argument data.
Entries marked "OK" will be processed by this server.

Returns - ERROR if response is flawed,
          OK if no problem */

STATUS handleObjlistReplyBMW ( int server, objlist *p_objs, objlist *p_objreply )
{
  int                   status;     /* result of attempt to request */
  int                   bytes;      /* number of bytes read from */
				    /* socket */
  int                   i;          /* index into list */

  if ( bmw_c_verbose )
    fprintf ( stderr, "bmwClient::: Awaiting objlist reply...\n");

  status = waitReplyBMW ( server );
  if (status == ERROR)
    {
      perror ("bmwClient::: waitReplyBMW");
      return (ERROR);
    }
  
  /* select says ready to read data from the read socket    */ 

  if ( bmw_c_verbose )
    fprintf ( stderr, "bmwClient::: Getting objlist reply...\n");

  bytes = read ( bmwSock[server], (char *) p_objreply, SIZE_OBJLIST );
  if ( bytes == 0 )
    {
      perror ("bmwClient::: read");
      return (ERROR);
    }

  fprintf(stderr,"bmwClient::object list reply size %d\n",SIZE_OBJLIST);
  if (bmw_c_verbose || bmw_c_terse)
    {
      fprintf ( stderr, "bmwClient::: Checking object list reply\n");
      for ( i = 0; i < MAX_OBJS; i++ )
	fprintf ( stderr, "bmwClient::: %2d <%10s> <%10s>\n", i, (*p_objs)[i],
		(*p_objreply)[i]); 
    }

  return (OK);
}

/******************************************************************************

waitReplyBMW - wait for a reply from server

*/

STATUS waitReplyBMW ( int server )
{
  int                   nfound;     /* result of select */
  struct fd_set	        readFds;    /* bit mask for select */
  int	                width;      /* number of fds on which to select */
  struct timeval        pTimeOut;   /* time out on socket */

  /* set up select command for time out */

  FD_ZERO (&readFds);
  FD_SET ( bmwSock[server], &readFds );
  width = bmwSock[server];
  width++;
  pTimeOut.tv_sec = 15.0;
  pTimeOut.tv_usec = 0.0;

  nfound = select ( width, &readFds, NULL, NULL, &pTimeOut );
  if ( nfound == ERROR) 
    {
      perror ("bmwClient::: select (failed on client end)");
      return (ERROR);
    }
  if ( nfound == 0 )
    {
      perror ("bmwClient::: select (no response from server)");
      return (ERROR);
    }
  return (OK);
}
