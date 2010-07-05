/* file: defs.h
   purpose: data structures and function prototypes for ACCS procedures
   author: florian.kaltenberger@eurecom.fr
   date: 05.07.2010 
*/

#ifndef __ACCS_DEFS__H__
#define __ACCS_DEFS__H__
#include "PHY/defs.h"

/*! This is the Component Carrier Allocation Table (CCAT). It is filled by the sensing performed during the OTAC phase. */
typedef struct {
  unsigned char eNb_id;
  unsigned char PCC_id;
  unsigned short PCC_RSRP;
  unsigned char SCC_id;
  unsigned short SCC_RSRP;
} CCAT_entry;


#define MAX_CCAT_SIZE 4
typedef CCAT_entry CCAT_table[MAX_CCAT_SIZE];


/*! This function sets the two RF chains to the desired frequency band
@param ccID1 (0..3) ID of the first CC
@param ccID2 (0..3) ID of the second CC
@returns 0 if successfull
*/
int accs_set_cc(unsigned char ccID1, unsigned char ccID2) ; 

/*! This function gets the frequency band of the two RF chains
@param ccID1 (0..3) ID of the first CC
@param ccID2 (0..3) ID of the second CC
@returns 0 if successfull
*/
int accs_get_cc(unsigned char *ccID1, unsigned char *ccID2);

/*! This function gets the RSRP of a given ccID. Basically this is a table lookup in the CCAT table. 
@param ccat CCAT
@param ccID (0..3) ID of the CC
@returns 0 if successfull
*/
unsigned short accs_get_RSRP(CCAT_table ccat, unsigned char ccID);

/*! This function selects the PCC based on the CCAT
@param ccat CCAT
@returns selected PCC
*/
unsigned char accs_select_pcc(CCAT_table ccat);

#endif
