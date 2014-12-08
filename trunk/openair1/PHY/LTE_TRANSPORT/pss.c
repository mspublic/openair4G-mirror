/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/pss.c
* \brief Top-level routines for generating primary synchronization signal (PSS) V8.6 2009-03
* \author F. Kaltenberger, O. Tonelli, R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it,knopp@eurecom.fr
* \note
* \warning
*/
/* file: pss.c
   purpose: generate the primary synchronization signals of LTE
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

//#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

int generate_pss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short symbol,
		 unsigned short slot_offset) {

  unsigned int Nsymb;
  unsigned short k,m,aa,a;
  uint8_t Nid2;
  short *primary_sync;


  Nid2 = frame_parms->Nid_cell % 3;

  switch (Nid2) {
  case 0:
    primary_sync = primary_synch0;
    break;
  case 1:
    primary_sync = primary_synch1;
    break;
  case 2:
    primary_sync = primary_synch2;
    break;
  default:
    msg("[PSS] eNb_id has to be 0,1,2\n");
    return(-1);
  }

  a = (frame_parms->mode1_flag == 0) ? amp : (amp*ONE_OVER_SQRT2_Q15)>>15;
  //printf("[PSS] amp=%d, a=%d\n",amp,a);

  Nsymb = (frame_parms->Ncp==NORMAL)?14:12;

  for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
  //  aa = 0;

  // The PSS occupies the inner 6 RBs, which start at
    k = frame_parms->ofdm_symbol_size-3*12+5;

    //printf("[PSS] k = %d\n",k);
    for (m=5;m<67;m++) {
      ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			       symbol*frame_parms->ofdm_symbol_size + k)] = 
	(a * primary_sync[2*m]) >> 15; 
      ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			       symbol*frame_parms->ofdm_symbol_size + k) + 1] = 
	(a * primary_sync[2*m+1]) >> 15;

      k+=1;

      if (k >= frame_parms->ofdm_symbol_size) {
	k++; //skip DC
	k-=frame_parms->ofdm_symbol_size;
      }

    }
  }
  return(0);
}
  
int generate_pss_emul(PHY_VARS_eNB *phy_vars_eNb,uint8_t sect_id) {
  
  msg("[PHY] EMUL eNB generate_pss_emul eNB %d, sect_id %d\n",phy_vars_eNb->Mod_id,sect_id);
  eNB_transport_info[phy_vars_eNb->Mod_id][phy_vars_eNb->CC_id].cntl.pss=sect_id;
  return(0);
}
