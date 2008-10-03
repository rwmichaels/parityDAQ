/* scaler2.h          R.Michaels
   July 1998 port to Radstone PPC

   Executible statement definitions, to put after declarations. 
   Contains:
      VME offset addresses vmeoff[] 
      Types of scalers scalertype[] = Types of scalers --
           1151   =  LeCroy model 1151 
           7200   =  Struck model 7200
           560    =  CAEN model V560           
      The correspondence between VME addresses and scaler
      type is specified here.

   Sept 2003.  Removed the 2nd redundant helicity scaler.
               Added 2 units of SIS3801, remove a 1151.
               vxWorks 5.4: 0xe0000000 -> 0xfa000000
   Mod. Oct 2003
               Use sysBusToLocalAdrs.

*/

int ix, res;
unsigned long laddr;
static int addr_assign=0;

/* Address starts for Motorola 2400 */
vmeoff[0]=0xce1000;
vmeoff[1]=0xce1000;
vmeoff[2]=0xce2000;  
vmeoff[3]=0xce2000;  

/* VME header is a traditional header used by decoding, and
   is independent of the cpu offset. */
vmeheader[0]=0xbab00000;
vmeheader[1]=0xbab10000;
vmeheader[2]=0xbab20000;
vmeheader[3]=0xbab30000;

scalertype[0]=380101;  /* plus helicity, 1st scaler */
scalertype[1]=380102;  /* minus helicity, 1st scaler */
scalertype[2]=380103;  /* plus, 2nd */
scalertype[3]=380104;  /* minus, 2nd */






