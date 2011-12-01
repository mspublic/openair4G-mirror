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

/*! \file PHY/LTE_TRANSPORT/pilots.c
* \brief Top-level routines for generating DL cell-specific reference signals V8.6 2009-03
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,florian.kaltenberger.fr
* \note
* \warning
*/
//#include "defs.h"
#include "PHY/defs.h"

void generate_pilots(PHY_VARS_eNB *phy_vars_eNB,
		     mod_sym_t **txdataF,
		     s16 amp,
		     u16 Ntti) {

  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNB->lte_frame_parms;

  u32 tti,tti_offset,slot_offset,Nsymb,samples_per_symbol;
  u8 second_pilot;



  Nsymb = (frame_parms->Ncp==0)?14:12;
  second_pilot = (frame_parms->Ncp==0)?4:3;

  //  printf("Doing TX pilots Nsymb %d, second_pilot %d\n",Nsymb,second_pilot);

  for (tti=0;tti<Ntti;tti++) {
	  
	


#ifdef IFFT_FPGA
    tti_offset = tti*frame_parms->N_RB_DL*12*Nsymb;
    samples_per_symbol = frame_parms->N_RB_DL*12;
#else    
    tti_offset = tti*frame_parms->ofdm_symbol_size*Nsymb;
    samples_per_symbol = frame_parms->ofdm_symbol_size;
#endif
    slot_offset = (tti*2)%20;
    
    //    printf("tti %d : offset %d (slot %d)\n",tti,tti_offset,slot_offset);
    //Generate Pilots
    
    //antenna 0 symbol 0 slot 0
    lte_dl_cell_spec(phy_vars_eNB,&txdataF[0][tti_offset],
		     amp,
		     slot_offset,
		     0,
		     0);

    
    //    printf("tti %d : second_pilot offset %d \n",tti,tti_offset+(second_pilot*samples_per_symbol));
    //antenna 0 symbol 3/4 slot 0
    lte_dl_cell_spec(phy_vars_eNB,&txdataF[0][tti_offset+(second_pilot*samples_per_symbol)],
		     amp,
		     slot_offset,
		     1,
		     0);

    //    printf("tti %d : third_pilot offset %d \n",tti,tti_offset+((Nsymb>>1)*samples_per_symbol));    
    //antenna 0 symbol 0 slot 1
    lte_dl_cell_spec(phy_vars_eNB,&txdataF[0][tti_offset+((Nsymb>>1)*samples_per_symbol)],
		     amp,
		     1+slot_offset,
		     0,
		     0);
        
    //    printf("tti %d : third_pilot offset %d \n",tti,tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol));    
    //antenna 0 symbol 3/4 slot 1
    lte_dl_cell_spec(phy_vars_eNB,&txdataF[0][tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol)],
		     amp,
		     1+slot_offset,
		     1,
		     0);
    

    if (frame_parms->nb_antennas_tx > 1) {
      if (frame_parms->mode1_flag) {
      // antenna 1 symbol 0 slot 0
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset],
		       amp,
		       slot_offset,
		       0,
		       0);
      
      // antenna 1 symbol 3 slot 0
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(second_pilot*samples_per_symbol)],
		       amp,
		       slot_offset,
		       1,
		       0);
      
      //antenna 1 symbol 0 slot 1
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(Nsymb>>1)*samples_per_symbol],
		       amp,
		       1+slot_offset,
		       0,
		       0);
      
      // antenna 1 symbol 3 slot 1
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol)],
		       amp,
		       1+slot_offset,
		       1,
		       0);

      }
      else {

      // antenna 1 symbol 0 slot 0
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset],
		       amp,
		       slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 0
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(second_pilot*samples_per_symbol)],
		       amp,
		       slot_offset,
		       1,
		       1);
      
      //antenna 1 symbol 0 slot 1
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(Nsymb>>1)*samples_per_symbol],
		       amp,
		       1+slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 1
      lte_dl_cell_spec(phy_vars_eNB,&txdataF[1][tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol)],
		       amp,
		       1+slot_offset,
		       1,
		       1);
      }
    }
  }
}
	    
int generate_pilots_slot(PHY_VARS_eNB *phy_vars_eNB,
			 mod_sym_t **txdataF,
			 s16 amp,
			 u16 slot) {

  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNB->lte_frame_parms;  
  u32 slot_offset,Nsymb,samples_per_symbol;
  u8 second_pilot;

  if (slot<0 || slot>= 20) {
    msg("generate_pilots_slot: slot not in range (%d)\n",slot);
    return(-1);
  }

  Nsymb = (frame_parms->Ncp==0)?7:6;
  second_pilot = (frame_parms->Ncp==0)?4:3;


#ifdef IFFT_FPGA
  slot_offset = slot*frame_parms->N_RB_DL*12*Nsymb;
  samples_per_symbol = frame_parms->N_RB_DL*12;
#else    
  slot_offset = slot*frame_parms->ofdm_symbol_size*Nsymb;
  samples_per_symbol = frame_parms->ofdm_symbol_size;
#endif
    
    //    printf("tti %d : offset %d (slot %d)\n",tti,tti_offset,slot_offset);
    //Generate Pilots
    
    //antenna 0 symbol 0 slot 0
  lte_dl_cell_spec(phy_vars_eNB,
		   &txdataF[0][slot_offset],
		   amp,
		   slot,
		   0,
		   0);

    
    
    //antenna 0 symbol 3 slot 0
  lte_dl_cell_spec(phy_vars_eNB,
		   &txdataF[0][slot_offset+(second_pilot*samples_per_symbol)],
		   amp,
		   slot,
		   1,
		   0);
    

    if (frame_parms->nb_antennas_tx > 1) {
      if (frame_parms->mode1_flag) {
      // antenna 1 symbol 0 slot 0
	lte_dl_cell_spec(phy_vars_eNB,
			 &txdataF[1][slot_offset],
			 amp,
			 slot,
			 0,
			 0);
      
      // antenna 1 symbol 3 slot 0
	lte_dl_cell_spec(phy_vars_eNB,
			 &txdataF[1][slot_offset+(second_pilot*samples_per_symbol)],
			 amp,
			 slot,
			 1,
			 0);

      }
      else {

      // antenna 1 symbol 0 slot 0
	lte_dl_cell_spec(phy_vars_eNB,
			 &txdataF[1][slot_offset],
			 amp,
			 slot,
			 0,
			 1);
      
      // antenna 1 symbol 3 slot 0
	lte_dl_cell_spec(phy_vars_eNB,
			 &txdataF[1][slot_offset+(second_pilot*samples_per_symbol)],
			 amp,
			 slot,
			 1,
			 1);
      }
    }

    return(0);
}

