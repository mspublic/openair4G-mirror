/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 2012 Eurecom - THALES Communications & Security

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : 

*******************************************************************************/

/*! \file virtual_link.h
* \brief structures and data types related to virtual links
* \author Philippe Agostini and navid nikaein 
* \date 2012
* \version 0.1
* @ingroup _mac
*/


#ifndef __VIRTUAL_LINK_H__
#define __VIRTUAL_LINK_H__

/************
 * Includes *
 ************/

//#include "defs.h"
//#include "PHY/defs.h"

/***********
 * Defines * 
 ***********/

/* The maximum number of MRs per virtual link */
#define MAX_MR_PER_VLINK 4

/* The maximum number of virtual links per CH and MR */
//#define MAX_VLINK_PER_CH 4
//#define MAX_VLINK_PER_MR 4

/* Status of a virtual link */
#define VLINK_CONNECTED 1
#define VLINK_NOT_CONNECTED 0

/* Number max of CH in OpenAirInterface */
#define NB_MAX_CH 7
#define NB_MAX_MR 16

/*****************************
 * Structures and data types *
 *****************************/
// MAC structure 
/* virtual link and collaborative RNTI definition */
struct mr_array {
	int count; /* Nb of MR currently in the list */
	unsigned char array[MAX_MR_PER_VLINK]; /* List of UE_index */
};

typedef struct  {
  unsigned char vlinkID;
  unsigned short PCellIdsourceCH;
  unsigned short PCellIddestCH;
  struct mr_array MRarray;
  unsigned char status;
  
  /* For now M-TMSI replaced by UE_indexes */
  //unsigned int MRMTMSIlist[MAX_MR_PER_VLINK];
}virtual_link;

typedef struct  {
	int count; //the number of virtual link in the table
	virtual_link array[MAX_VLINK_PER_CH];
}virtual_links;

// CORNTIs array for UE/eNB MAC/PHY structures
typedef struct  {
  u8 count;
  // Collaborative data sequence number;
  u16 sn[MAX_VLINK_PER_CH];
  u16 array[MAX_VLINK_PER_CH];
}cornti_array;


/* Forwarding table definition */
#define MAX_FW_ENTRY 10 

typedef struct  { 
  u8  vlid;
  // u16 lcid; 
  u16 cornti1;
  u16 cornti2;
}forwardingTableEntry;

typedef struct  { 
  u8 count;
  forwardingTableEntry array[MAX_FW_ENTRY];
}forwarding_Table;



#endif
