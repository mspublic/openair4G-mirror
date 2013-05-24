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

/*! \file PHY/LTE_TRANSPORT/dlsch_scrambling.c
* \brief Routines for the scrambling procedure of the PDSCH physical channel from 36-211, V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "defs.h"
#include "extern.h"
#include "PHY/extern.h"
void dlsch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      int mbsfn_flag,
		      LTE_eNB_DLSCH_t *dlsch,
		      int G,
		      u8 q,
		      u8 Ns) {

  int i,j,k=0;
  //  u8 reset;
  u32 x1, x2, s=0;
  u8 *e=dlsch->e;

  //  reset = 1;
  // x1 is set in lte_gold_generic
  if (mbsfn_flag == 0) {
    x2 = (dlsch->rnti<<14) + (q<<13) + ((Ns>>1)<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.3.1
  }
  else {
    x2 = ((Ns>>1)<<9) + frame_parms->Nid_cell_mbsfn; //this is c_init in 36.211 Sec 6.3.1
  }
  //  printf("scrambling: rnti %x, q %d, Ns %d, Nid_cell %d, length %d\n",dlsch->rnti,q,Ns,frame_parms->Nid_cell, G);  
  s = lte_gold_generic(&x1, &x2, 1);
  for (i=0; i<(1+(G>>5)); i++) {
 
    for (j=0;j<32;j++,k++) {
      //            printf("scrambling %d : %d => ",k,e[k]);
      e[k] = (e[k]&1) ^ ((s>>j)&1);
      //                  printf("%d\n",e[k]);
    }
   s = lte_gold_generic(&x1, &x2, 0);
  }

}


void dlsch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			int mbsfn_flag,
			LTE_UE_DLSCH_t *dlsch,
			int G,
			s16* llr,
			u8 q,
			u8 Ns) {

  int i,j,k=0;
  //  u8 reset;
  u32 x1, x2, s=0;
  
  //  reset = 1;
  // x1 is set in first call to lte_gold_generic

  if (mbsfn_flag == 0)
    x2 = (dlsch->rnti<<14) + (q<<13) + ((Ns>>1)<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.3.1
  else
    x2 = ((Ns>>1)<<9) + frame_parms->Nid_cell_mbsfn; //this is c_init in 36.211 Sec 6.3.1
  //  printf("unscrambling: rnti %x, q %d, Ns %d, Nid_cell %d length %d\n",dlsch->rnti,q,Ns,frame_parms->Nid_cell,G);
  s = lte_gold_generic(&x1, &x2, 1);
  for (i=0; i<(1+(G>>5)); i++) {
    for (j=0;j<32;j++,k++) {
      //               printf("unscrambling %d : %d => ",k,llr[k]);
      llr[k] = ((2*((s>>j)&1))-1)*llr[k];
      //             printf("%d\n",llr[k]);
    }
    s = lte_gold_generic(&x1, &x2, 0);
  }
} 
