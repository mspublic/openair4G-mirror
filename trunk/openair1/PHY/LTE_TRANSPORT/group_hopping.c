/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

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
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/group_hopping.c
* \brief Top-level routines for group/sequence hopping and nPRS sequence generationg for DRS and PUCCH from 36-211, V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"

//#define DEBUG_GROUPHOP 1

void generate_grouphop(LTE_DL_FRAME_PARMS *frame_parms) {

  uint8_t ns;
  uint8_t reset=1;
  uint32_t x1, x2, s=0;
  // This is from Section 5.5.1.3
  uint32_t fss_pusch = frame_parms->Nid_cell + frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH;

  x2 = frame_parms->Nid_cell/30;
#ifdef DEBUG_GROUPHOP
      msg("[PHY] GroupHop:");
#endif
  for (ns=0;ns<20;ns++) {
    if (frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled == 0)
      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.grouphop[ns] = fss_pusch%30;
    else {
      if ((ns&3) == 0) {
	s = lte_gold_generic(&x1,&x2,reset);
	reset = 0;
      }
      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.grouphop[ns] = (((uint8_t*)&s)[ns&3]+fss_pusch)%30;
    }
#ifdef DEBUG_GROUPHOP
      msg("%d.",frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.grouphop[ns]);
#endif
  }
#ifdef DEBUG_GROUPHOP
  msg("\n");
#endif
}

void generate_seqhop(LTE_DL_FRAME_PARMS *frame_parms) {

  uint8_t ns,reset=1;
  uint32_t x1, x2, s=0;
  // This is from Section 5.5.1.3
  uint32_t fss_pusch = frame_parms->Nid_cell + frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH;

  x2 = (32*(frame_parms->Nid_cell/30) + fss_pusch)%30;

  s = lte_gold_generic(&x1,&x2,reset);
#ifdef DEBUG_GROUPHOP
      msg("[PHY] SeqHop:");
#endif
  for (ns=0;ns<20;ns++) {
    if ((frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled == 0) && 
	(frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled == 1))
      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.seqhop[ns] = (s>>(ns&0x1f))&1;
    else
      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.seqhop[ns] = 0;
#ifdef DEBUG_GROUPHOP
      msg("%d.",frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.seqhop[ns]);
#endif
  }
#ifdef DEBUG_GROUPHOP
  msg("\n");
#endif
}

void generate_nPRS(LTE_DL_FRAME_PARMS *frame_parms) {

  uint16_t n=0;
  uint8_t reset=1;
  uint32_t x1, x2, s=0;
  // This is from Section 5.5.1.3
  uint8_t Nsymb_UL = (frame_parms->Ncp_UL == 0) ? 7 : 6;
  uint16_t next = 0;
  uint8_t ns=0;

  uint32_t fss_pusch = frame_parms->Nid_cell + frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH;

  x2 = (32*(uint32_t)(frame_parms->Nid_cell/30)) + fss_pusch;
#ifdef DEBUG_GROUPHOP
  msg("[PHY] nPRS:");
#endif
  for (n=0;n<(20*Nsymb_UL);n++) {  //loop over total number of bytes to generate
    if ((n&3) == 0) {
      s = lte_gold_generic(&x1,&x2,reset);
      reset = 0;
      //      printf("n %d : s (%d,%d,%d,%d)\n",n,((uint8_t*)&s)[0],((uint8_t*)&s)[1],((uint8_t*)&s)[2],((uint8_t*)&s)[3]);
    }
    if (n == next) {
      frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[ns] = ((uint8_t*)&s)[next&3];
#ifdef DEBUG_GROUPHOP
      msg("%d.",frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[ns]);
#endif
      ns++;
      next+=Nsymb_UL;
    }
  }
#ifdef DEBUG_GROUPHOP
  msg("\n");
#endif
}

void init_ul_hopping(LTE_DL_FRAME_PARMS *frame_parms) {

  generate_grouphop(frame_parms);
  generate_seqhop(frame_parms);
  generate_nPRS(frame_parms);
}
