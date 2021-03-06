/* Client code to read VME scalers */
/* Server runs on the VME cpu */

/* Version for PVDIS.  Never looking back. 

*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include "time.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "vxWscaler.h"

/***********************************************************
*
*   Purpose:  Read the VME scalers.
*
*   vxWorks client running on a UNIX host
*
*
*/

main (int argc, char *argv[])
  {
#ifdef print_to_file
       int print_file;
       FILE *outfile;
#endif
       FILE *ringout;
       int   sFd;                            /*  socket file descriptor */
       int i,k,n,ilen,numchan;
       int iread, num_read;
       int getring = 0;
       long stat;
       int   mlen;                           /*  msg length  */
       int isca,sca,shead,nRead,nRead1;
       static int clearall=0;           /* flag to clear scalers (danger!) */
       int checkend;
       static int showrate=0;
       static int lprint=1;
       static int fprint=0;
       struct request myRequest;             /*  request to send to server */
       struct request bobreply;
       struct sockaddr_in serverSockAddr;    /*  socket addr of server */
       char  reply;
       char  replyBuf[REPLY_MSG_SIZE];
       static char  defMsg[] = "Bob wants some data";
       static int debug=0;
       int sleeptime;  
       int bytesread;
       int dainsert;
       int nskip,nchan,ntot;
       char cb;
       long tag;
       long ev_len;
       struct EVBUF {
           long header[4];
           long bobbuf[32*MAXSCALER];  /* should assume STR7200 (32 chan) */
	 } buf;
       long scalerd1[32*MAXSCALER],scalerd2[32*MAXSCALER];
       float srate[32*MAXSCALER];

#include "scaler1.h"
#include "scaler2.h"

	 /* Process arguments */

    dainsert=0;
    clearall=0;
    checkend=0;
    num_read=1;
    sleeptime=10; /* default sleep time = 10 */

    while (--argc > 0) {
        ++argv;
        if ((*argv)[0]==NULL) break;
        if ((*argv)[0]!= '-') goto usage;
        cb = (*argv)[1];
        switch (cb) {
           case 'c':                  /* clear all scalers */
              clearall=1;
              break;
   	   case 'e':                  /* check for end-run (for last_scaler)*/
              checkend=1;
              break;
   	   case 'r':                  /* show the rates */
  	      num_read=2;
              showrate=1;
              break;
           case 'x':                  /* Hexidecimal printout */
              lprint=2;
              break;
           case 'd':                  /* Decimal printout */
              lprint=3;
              break;
           case 'i':                  /* Data inserted into datastream */
              lprint=0;
              dainsert=1;
              break;
#ifdef print_to_file
  	   case 'p':                  /* Print to data file only selected scalers */
	      fprint=1;	  
	      break;
#endif
	   case 'h':                  /* help */
  	      goto usage;
           default:
              printf("ERROR: Illegal option %c \n\n",cb);
usage:
              printf("USAGE: scread [-c -r -e -d -x -h -i ");
#ifdef print_to_file
	      printf("-p ");
#endif
	      printf("]\n");
              printf("-c is to clear scalers (if I let you)\n");
              printf("-r is to show the rates (Hz)\n");
              printf("During experiment, the server probably be set up to\n");
              printf("disallow scread to clear -> instead by CODA RunStart\n");
              printf("-e is to check for end of run flag\n");
              printf("if 1 is printed, its ended, if not its a failure\n");
              printf("With no option, scread prints oldstyle to screen\n");
              printf("These printout options (normally for CODA 2.0):\n");
              printf("-x is to get hexidecimal printout \n");
              printf("-d is to get integer printout \n");
#ifdef print_to_file
	      printf("-p print to data file with option -d or -x\n");
#endif
              printf("-h is to get help (this printout)\n");
              printf("-i is to get data insertion via (et)or(da)Insert\n");
              exit(1);
	 }
    }


/* Check consistency between NUMBLOCKS and NUMSCALER */

   numchan=0;
   for (isca=0; isca<NUMSCALER; isca++) {
     /* 3801 scalers have 2 helicities and 32 channels */
     if (scalertype[isca]>=380101 && scalertype[isca]<=380104) {
         numchan += 32;
     } else if( (scalertype[isca]==7200) || (scalertype[isca]==3800)) {
        numchan += 32;
     } else {
        numchan += 16;
     }
    }

    if(numchan!=16*NUMBLOCKS) {
       printf("\n\n Error -- cannot run -- \n");
       printf(" The following is not satisfied: \n");
       printf("NUMSCALER = number of scaler units \n");
       printf("NUMBLOCKS = number of 16-channel blocks =\n"); 
       printf("num_1151_scalers + num_v560_scalers \n");
       printf("      + 2*num_str7200_scalers \n");
       printf("      + 2*num_sis3800_scalers \n");
       printf("      + 2*num_sis3801_scalers of each helicity \n");
       exit(1);
     }

/*    bzero (&bobreply, sizeof(bobreply));*/

      myRequest.clearflag = htonl(clearall);   

      myRequest.checkend = htonl(checkend);


       for (k=0; k<=sizeof(defMsg); k++) {
           myRequest.message[k] = defMsg[k];
       }


       for (iread=0; iread<num_read; iread++) {

/* create socket */

    if ((sFd = socket (PF_INET, SOCK_STREAM, 0)) == ERROR )
        {
        perror ("socket");
        return (ERROR);
        }


/* bind not required -- port number will be dynamic */

/* build server socket address */

/*    bzero (&serverSockAddr, sizeof(serverSockAddr) );*/
    serverSockAddr.sin_family = PF_INET;
    serverSockAddr.sin_port = htons (SERVER_PORT_NUM);
    serverSockAddr.sin_addr.s_addr = inet_addr (SERVER_INET_ADDR);

/* connect to server */
    
/*    printf("About to connect \n"); */

#ifdef HPVERS
    if(connect (sFd, &serverSockAddr, sizeof(serverSockAddr)) == ERROR)
#else
    if(connect (sFd, (struct sockaddr *) &serverSockAddr, sizeof(serverSockAddr)) == ERROR)
#endif
       {
       printf("failed to connect \n");
       perror ("connect");
       close (sFd);
       return (ERROR);
       }
/*    printf("Finished trying to connect \n"); */

/* build request, prompting user for message */

/*    printf("Message to send :  \n");
*    mlen = read (0, myRequest.message, sizeof(myRequest.message));
*    myRequest.message[mlen-1] = '\0';
*/
    myRequest.reply = 1;

#ifdef TEST_RING
    myRequest.getring = htonl(getring);
#else
    if (getring) printf("Warning, cannot test ring buffer (this vers)\n");
#endif

/* send request to server */

    if(write(sFd, (char *)&myRequest, sizeof(myRequest)) == ERROR)
      {
      perror ("write");
      close (sFd);
      return (ERROR);
      }

    nRead  =read (sFd, (char *)&bobreply, sizeof(bobreply));
    if(nRead < 0) {
       printf("Error from reading from scaler server\n");
       exit(0);
    }
    if (debug) printf("read nRead %d   %d\n",nRead,sizeof(bobreply));

    while (nRead < sizeof(bobreply)) {
       nRead1 = read (sFd, ((char *) &bobreply)+nRead,
                    sizeof(bobreply)-nRead);
       if (debug) printf("reading some more \n");
       if(nRead1 < 0) {
          printf("Error from reading from scaler server\n");
          exit(0);
       }
       nRead += nRead1;
    }

    if(clearall) {
          printf("Scalers cleared \n");
          exit(1);
    }
    if(checkend) {
          printf("%d",ntohl(bobreply.checkend));
          exit(1);
    }

/*    printf ("MESSAGE FROM SERVER :  \n %s \n",bobreply.message); */

/* byte swapping */
       for (k=0 ; k < 16*MAXBLOCKS; k++) bobreply.ibuf[k] = ntohl(bobreply.ibuf[k]);

       for (k=0; k<16*MAXBLOCKS; k++) {
          if (iread==0) {
             scalerd1[k] = bobreply.ibuf[k]&0xffffff;
          } else {
             scalerd2[k] = bobreply.ibuf[k]&0xffffff;
          }
       }



#ifdef TEST_RING
       for (k=0 ; k < NUM_IN_RING*MAXRING; k++) 
           bobreply.ring[k] = ntohl(bobreply.ring[k]);
#endif

       if(lprint==1) {
         printf ("\n\n\n  ========    Scalers  ===============\n\n");
         sca=0;
         for (k=0; k<NUMBLOCKS; k++) {     //test
            sca=2*k;

                for (i=0;i<2;i++) {
                printf ("%3d :  %8d %8d %8d %8d %8d %8d %8d %8d \n",
                8*(sca+i)+1,bobreply.ibuf[(sca+i)*8]&0xffffff,
                bobreply.ibuf[(sca+i)*8+1]&0xffffff,
                bobreply.ibuf[(sca+i)*8+2]&0xffffff, 
                bobreply.ibuf[(sca+i)*8+3]&0xffffff,
                bobreply.ibuf[(sca+i)*8+4]&0xffffff,
                bobreply.ibuf[(sca+i)*8+5]&0xffffff,
                bobreply.ibuf[(sca+i)*8+6]&0xffffff,
                bobreply.ibuf[(sca+i)*8+7]&0xffffff);
		} 
          
	  }
       }
       
      if(lprint==2) {
         k=0;
         for (isca=0; isca<NUMSCALER; isca++) {
            nchan=16;
            if(scalertype[isca]==7200) nchan=32;  
            if(scalertype[isca]==3800) nchan=32;  
            if(scalertype[isca]==3801) nchan=32;  
            if(scalertype[isca]>=380000) nchan=32;  
            shead = vmeheader[isca] + nchan;
            printf("%x\n",shead);
            for (i=0;i<nchan;i++) {
                printf("%x\n",bobreply.ibuf[k++]);
	    }
	 }
       }

       if(lprint==3) {
         k=0;
	 for (isca=0; isca<NUMSCALER; isca++) {
            nchan=16;
            if(scalertype[isca]==7200) nchan=32;  
            if(scalertype[isca]==3800) nchan=32;  
            if(scalertype[isca]==3801) nchan=32;  
            if(scalertype[isca]>=380000) nchan=32;  
            shead = vmeheader[isca] + nchan;
            printf("%x\n",shead);
            for (i=0;i<nchan;i++) {
                printf("%d\n",bobreply.ibuf[k++]);
	    }
         }
       }

       if (showrate) {
         if (iread==0) {
            printf("Sleeping for %d sec \n",sleeptime);
            sleep(sleeptime);
	 } else {
	   printf("\n\n ================ Rates (Hz) ===================\n");
	 }
       }
         

       } /* loop over num reads (if >0) */

#ifdef TEST_RING
       if (getring) {
         if((ringout=fopen("ring.out","a"))==NULL) {
           printf("could not open file ring.out\n");
         } else {
           n = bobreply.ring[0];
	   if (n > MAXRING-1) {  
  	      printf("Error :  replied ring buffer exceeds NUM_IN_RING*(MAXRING-1)\n");
              n = MAXRING-1;
	   }
           fprintf(ringout,"%d\n",n);
	   if (debug) printf("-->  n =   %d\n",n); 
           for (i = 0; i < n*NUM_IN_RING; i++) { 
                fprintf(ringout,"%d\n",bobreply.ring[i+1]);
	        if (debug) printf("%d  %d\n",i,bobreply.ring[i+1]); 
            }
            fclose(ringout);
         }   
       }
#endif

#ifdef print_to_file
       if(fprint==1) {
	 if((outfile=fopen(OUTPUT_FILE,"w+"))==NULL) {
	   if(debug) printf("could not open file %s\n",OUTPUT_FILE);
	   goto Skip_fprint;
	 }
	 k=0;
	 for (isca=0; isca<NUMSCALER; isca++) {
	    nchan=16;
            if(scalertype[isca]==7200) nchan=32;  
            if(scalertype[isca]==3800) nchan=32;  
            if(scalertype[isca]==3801) nchan=32;  
            if(scalertype[isca]>=380000) nchan=32;  
            shead = vmeheader[isca] + nchan;
	 }
	 fprintf(outfile,"Trigger 1 :%d\n",bobreply.ibuf[T1]);
	 fprintf(outfile,"Trigger 2 :%d\n",bobreply.ibuf[T2]);
	 fprintf(outfile,"Trigger 3 :%d\n",bobreply.ibuf[T3]);
	 fprintf(outfile,"Trigger 4 :%d\n",bobreply.ibuf[T4]);
	 fprintf(outfile,"Trigger 5 :%d\n",bobreply.ibuf[T5]);
	 fprintf(outfile,"BCM (x3)  :%d\n",bobreply.ibuf[BCMX3]);
	 fprintf(outfile,"Time      :%d\n",bobreply.ibuf[TIME]);
	 fclose(outfile);
       }
#endif
Skip_fprint:

      ntot=0;
      k=0;
      for (isca=0; isca<NUMSCALER; isca++) {
           nchan=16;
           if(scalertype[isca]==7200) nchan=32;
           if(scalertype[isca]==3800) nchan=32;  
           if(scalertype[isca]==3801) nchan=32;  
           if(scalertype[isca]>=380000) nchan=32;  
           shead = vmeheader[isca] + nchan;
           buf.bobbuf[ntot++]=shead;
           for (i=0; i<nchan; i++) {
              buf.bobbuf[ntot++] = bobreply.ibuf[k++];
           }
      }

      tag = 140;
      ev_len = 4 + ntot;
      buf.header[0] = ev_len-1;         /* event length */
      buf.header[1] = (tag<<16) + 0x10cc;
      buf.header[2] = ev_len-3;
      buf.header[3] = 1<<8;    /* data type of integer.  no tag for now */


/* Insert data into Data stream ? */

      if (dainsert) {
        if(debug) printf("\nAttempt to insert into data stream\n");

/* Using ET system (onla) */
        stat = etInsertEvent(buf.header,"/tmp/et_sys_onla");

        if(stat==0) {
         if(debug) printf("scaler_insert: etInsertEvent ok \n");
        } else {
         printf("scaler_insert error due to etInsertEvent status=%d\n",stat);
        }

      }
      close (sFd);

      if (showrate) {

#ifdef THING1
	printf("showing rate  \n");
 
        for (i=0; i<16; i++) {
          printf("d1: %d :  %d %d %d %d %d %d %d %d \n",i*8,
	    scalerd1[i*8+0],scalerd1[i*8+1],scalerd1[i*8+2],scalerd1[i*8+3],
            scalerd1[i*8+4],scalerd1[i*8+5],scalerd1[i*8+6],scalerd1[i*8+7]
		 );
	}
        for (i=0; i<16; i++) {
          printf("d2: %d :  %d %d %d %d %d %d %d %d \n",i*8,
	    scalerd2[i*8+0],scalerd2[i*8+1],scalerd2[i*8+2],scalerd2[i*8+3],
            scalerd2[i*8+4],scalerd2[i*8+5],scalerd2[i*8+6],scalerd2[i*8+7]
	  );
	}
#endif
        for (i=0; i<128; i++) {
          srate[i]=(scalerd2[i]-scalerd1[i])/sleeptime;
	}
        for (i=0; i<16; i++) {
          printf("%d:  %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f %5.0f \n",i*8+1,
		 srate[i*8+0],srate[i*8+1],srate[i*8+2],srate[i*8+3],
		 srate[i*8+4],srate[i*8+5],srate[i*8+6],srate[i*8+7]
	  );
	}
 
      }
      

  }
