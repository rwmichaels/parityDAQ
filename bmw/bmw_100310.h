/* bmw.h - include file for beam modulation works */

/* 
DESCRIPTION
This module contains #defines and typedefs used by both bmw_client and
bmw_server.
*/

/*
  $Header$
 */

/***** includes *****/

/***** defines *****/

/* Size of names for bmwClient */
#define MAX_NAMELENGTH   10              /* length of longest object */
					 /* name */

#define GLITCH           (-2)            /* an ERROR we can try to */
					 /* recover from */
#define VETO             (-3)            /* server didn't like our request */
#define MAX_OBJS         8               /* maximum number of objects */
#define SIZE_OBJLIST     (MAX_OBJS * MAX_NAMELENGTH) /* size of an objlist */
#define SERVER_NUM       1044            /* server port address */

/* Defined requests */
#define C_HELLO          13579246        /* sent by client to say */
					 /* hello */
#define C_SET            1001            /* set value for an element */
#define C_READBACK       1002            /* read back value for an element */

/* Defined replies */
#define S_HELLO          97531864        /* sent by server in response */
					 /* to C_HELLO */
/* Server reply codes */

#define S_OK             0               /* "OK, I did that" */
#define S_NOT_MINE      (-1)             /* "That object isn't mine" */
#define S_LOW           (-2)             /* "Set value too low" */
#define S_HIGH          (-3)             /* "Set value too high" */
#define S_UNKN_REQ      (-4)             /* "Unrecognized request" */

/***** typedefs *****/

typedef char objlist[MAX_OBJS][MAX_NAMELENGTH];
                                         /* list of objects to be */
					 /* modulated (e.g. coils, E */
                                         /* vernier) */ 

struct reqreply       /* structure for a request or a reply */
{
  int   req;          /* the type of request or reply being made */
  int   object;       /* ID number of the object this request or reply */
		      /* applies to (e.g. coil, E vernier) */ 
                      /* (this is its index in an objlist) */
  float value;        /* the value being set or read back */
};

/***** globals *****/

/***** locals *****/

/***** forward declarations *****/

