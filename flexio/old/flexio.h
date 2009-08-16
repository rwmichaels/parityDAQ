/************************************************************************
 *  flexio.h                                                            *
 *   Define the VME map of flexio for each crate,                       *
 ************************************************************************/

/* number of HAPPEX ADCs */
#ifdef COUNTINGHOUSE
#define FLEXAD 0x0ed0
#endif
#ifdef  RIGHTSPECT
#define FLEXAD 0x0a40
#endif

int FLEXIO_ADDR = FLEXAD;



