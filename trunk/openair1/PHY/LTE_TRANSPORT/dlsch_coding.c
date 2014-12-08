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

/*! \file PHY/LTE_TRANSPORT/dlsch_coding.c
* \brief Top-level routines for implementing Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "PHY/LTE_TRANSPORT/defs.h"
#include "defs.h"
#include "UTIL/LOG/vcd_signal_dumper.h"

//#define DEBUG_DLSCH_CODING 
//#define DEBUG_DLSCH_FREE 1

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)


void free_eNB_dlsch(LTE_eNB_DLSCH_t *dlsch) {
  int i;
  int r;

  if (dlsch) {
#ifdef DEBUG_DLSCH_FREE
    msg("Freeing dlsch %p\n",dlsch);
#endif
    for (i=0;i<dlsch->Mdlharq;i++) {
#ifdef DEBUG_DLSCH_FREE
      msg("Freeing dlsch process %d\n",i);
#endif
      if (dlsch->harq_processes[i]) {
#ifdef DEBUG_DLSCH_FREE
	msg("Freeing dlsch process %d (%p)\n",i,dlsch->harq_processes[i]);
#endif
	if (dlsch->harq_processes[i]->b) {
	  free16(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES);
	  dlsch->harq_processes[i]->b = NULL;
#ifdef DEBUG_DLSCH_FREE
	  msg("Freeing dlsch process %d b (%p)\n",i,dlsch->harq_processes[i]->b);
#endif
	}
	if (dlsch->harq_processes[i]->c) {
#ifdef DEBUG_DLSCH_FREE
	  msg("Freeing dlsch process %d c (%p)\n",i,dlsch->harq_processes[i]->c);
#endif
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++) {

#ifdef DEBUG_DLSCH_FREE
	    msg("Freeing dlsch process %d c[%d] (%p)\n",i,r,dlsch->harq_processes[i]->c[r]);
#endif
	    if (dlsch->harq_processes[i]->c[r]) {
	      free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+768);
	      dlsch->harq_processes[i]->c[r] = NULL;
	    }
	  }
	}
	free16(dlsch->harq_processes[i],sizeof(LTE_DL_eNB_HARQ_t));
	dlsch->harq_processes[i] = NULL;
      }
    }
    free16(dlsch,sizeof(LTE_eNB_DLSCH_t));
    dlsch = NULL;
  }
  
}

LTE_eNB_DLSCH_t *new_eNB_dlsch(unsigned char Kmimo,unsigned char Mdlharq,unsigned char N_RB_DL, uint8_t abstraction_flag) {

  LTE_eNB_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,j,r;
  unsigned char bw_scaling =1;
  
  switch (N_RB_DL){
  case 6: 
    bw_scaling =16;
    break;
  case 25:
    bw_scaling =4;
    break;
  case 50: 
    bw_scaling =2;
    break;
  default:
    bw_scaling =1;
    break;
  }
  
  dlsch = (LTE_eNB_DLSCH_t *)malloc16(sizeof(LTE_eNB_DLSCH_t));
  if (dlsch) {
    bzero(dlsch,sizeof(LTE_eNB_DLSCH_t));
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    for (i=0;i<10;i++)
      dlsch->harq_ids[i] = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      dlsch->harq_processes[i] = (LTE_DL_eNB_HARQ_t *)malloc16(sizeof(LTE_DL_eNB_HARQ_t));
      LOG_T(PHY, "Required mem size %d (bw scaling %d), dlsch->harq_processes[%d] %p\n",
	     MAX_DLSCH_PAYLOAD_BYTES/bw_scaling,bw_scaling, i,dlsch->harq_processes[i]);
      if (dlsch->harq_processes[i]) {
          bzero(dlsch->harq_processes[i],sizeof(LTE_DL_eNB_HARQ_t));
	  //	  dlsch->harq_processes[i]->first_tx=1;
          dlsch->harq_processes[i]->b = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES/bw_scaling);
          if (dlsch->harq_processes[i]->b) {
	    bzero(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES/bw_scaling);
	  }
	  else {
              msg("Can't get b\n");
              exit_flag=1;
          }
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS/bw_scaling;r++) {
	    // account for filler in first segment and CRCs for multiple segment case
	    dlsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+ 768);  
	    if (dlsch->harq_processes[i]->c[r]) {
	      bzero(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+ 768);
	    } 
	    else {
	      msg("Can't get c\n");
	      exit_flag=2;
	    }
	  }
	}
      }	else {
	msg("Can't get harq_p %d\n",i);
	exit_flag=3;
      }
    }

    if ((exit_flag==0)) {
      for (i=0;i<Mdlharq;i++) {
	dlsch->harq_processes[i]->round=0;
	if (abstraction_flag==0) {
	  for (j=0;j<96;j++)
	    for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	      dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
	}
      }
      return(dlsch);
    }
  }
  LOG_D(PHY, "new_eNB_dlsch exit flag %d, size of  %ld\n",
        exit_flag, sizeof(LTE_eNB_DLSCH_t));
  free_eNB_dlsch(dlsch);
  return(NULL);
  
  
}

void clean_eNb_dlsch(LTE_eNB_DLSCH_t *dlsch, uint8_t abstraction_flag) {

  unsigned char Mdlharq;
  unsigned char i,j,r;
  
  if (dlsch) {
    Mdlharq = dlsch->Mdlharq;
    dlsch->rnti = 0;
    dlsch->active = 0;
    for (i=0;i<10;i++)
      dlsch->harq_ids[i] = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	//	dlsch->harq_processes[i]->Ndi    = 0;
	dlsch->harq_processes[i]->status = 0;
	dlsch->harq_processes[i]->round  = 0;
	if (abstraction_flag==0) {
	  for (j=0;j<96;j++)
	    for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	      dlsch->harq_processes[i]->d[r][j] = LTE_NULL;
	}
      }
    }
  }
}

 
int dlsch_encoding(unsigned char *a,
		   LTE_DL_FRAME_PARMS *frame_parms,
		   uint8_t num_pdcch_symbols,
		   LTE_eNB_DLSCH_t *dlsch,
		   int frame,
		   uint8_t subframe,
		   time_stats_t *rm_stats,
		   time_stats_t *te_stats,
		   time_stats_t *i_stats) {
  
  unsigned int G;
  unsigned int crc=1;
  unsigned short iind;

  unsigned char harq_pid = dlsch->current_harq_pid;
  unsigned short nb_rb = dlsch->harq_processes[harq_pid]->nb_rb;
  unsigned int A; 
  unsigned char mod_order;
  unsigned int Kr=0,Kr_bytes,r,r_offset=0;
  unsigned short m=dlsch->harq_processes[harq_pid]->mcs;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ENCODING, VCD_FUNCTION_IN);

  A = dlsch->harq_processes[harq_pid]->TBS; //6228
  // printf("Encoder: A: %d\n",A);
  mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);

  G = get_G(frame_parms,nb_rb,dlsch->harq_processes[harq_pid]->rb_alloc,mod_order,dlsch->harq_processes[harq_pid]->Nl,num_pdcch_symbols,frame,subframe);

   
  //  if (dlsch->harq_processes[harq_pid]->Ndi == 1) {  // this is a new packet
  if (dlsch->harq_processes[harq_pid]->round == 0) {  // this is a new packet

    /*
    int i;
    printf("dlsch (tx): \n");
    for (i=0;i<(A>>3);i++)
      printf("%02x.",a[i]);
    printf("\n");
    */
    // Add 24-bit crc (polynomial A) to payload
    crc = crc24a(a,
		 A)>>8;
    a[A>>3] = ((uint8_t*)&crc)[2];
    a[1+(A>>3)] = ((uint8_t*)&crc)[1];
    a[2+(A>>3)] = ((uint8_t*)&crc)[0];
    //    printf("CRC %x (A %d)\n",crc,A);

    dlsch->harq_processes[harq_pid]->B = A+24;
    //    dlsch->harq_processes[harq_pid]->b = a;
    memcpy(dlsch->harq_processes[harq_pid]->b,a,(A/8)+4);
    if (lte_segmentation(dlsch->harq_processes[harq_pid]->b,
			 dlsch->harq_processes[harq_pid]->c,
			 dlsch->harq_processes[harq_pid]->B,
			 &dlsch->harq_processes[harq_pid]->C,
			 &dlsch->harq_processes[harq_pid]->Cplus,
			 &dlsch->harq_processes[harq_pid]->Cminus,
			 &dlsch->harq_processes[harq_pid]->Kplus,
			 &dlsch->harq_processes[harq_pid]->Kminus,		     
			 &dlsch->harq_processes[harq_pid]->F)<0)
      return(-1);
    
    for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
      if (r<dlsch->harq_processes[harq_pid]->Cminus)
	Kr = dlsch->harq_processes[harq_pid]->Kminus;
      else
	Kr = dlsch->harq_processes[harq_pid]->Kplus;
      Kr_bytes = Kr>>3;
      
      // get interleaver index for Turbo code (lookup in Table 5.1.3-3 36-212, V8.6 2009-03, p. 13-14)
      if (Kr_bytes<=64)
	iind = (Kr_bytes-5);
      else if (Kr_bytes <=128)
	iind = 59 + ((Kr_bytes-64)>>1);
      else if (Kr_bytes <= 256)
	iind = 91 + ((Kr_bytes-128)>>2);
      else if (Kr_bytes <= 768)
	iind = 123 + ((Kr_bytes-256)>>3);
      else {
	msg("dlsch_coding: Illegal codeword size %d!!!\n",Kr_bytes);
	return(-1);
      }
      
      
#ifdef DEBUG_DLSCH_CODING
      printf("Generating Code Segment %d (%d bits)\n",r,Kr);
      // generate codewords
      
      msg("bits_per_codeword (Kr)= %d, A %d\n",Kr,A);
      msg("N_RB = %d\n",nb_rb);
      msg("Ncp %d\n",frame_parms->Ncp);
      msg("mod_order %d\n",mod_order);
#endif
      
      
#ifdef DEBUG_DLSCH_CODING    
      msg("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat_old[iind*2],f1f2mat_old[(iind*2)+1]);
#endif
      start_meas(te_stats);
      threegpplte_turbo_encoder(dlsch->harq_processes[harq_pid]->c[r],
				Kr>>3, 
				&dlsch->harq_processes[harq_pid]->d[r][96],
				(r==0) ? dlsch->harq_processes[harq_pid]->F : 0,
				f1f2mat_old[iind*2],   // f1 (see 36121-820, page 14)
				f1f2mat_old[(iind*2)+1]  // f2 (see 36121-820, page 14)
				);
      stop_meas(te_stats);
#ifdef DEBUG_DLSCH_CODING
      if (r==0)
	write_output("enc_output0.m","enc0",&dlsch->harq_processes[harq_pid]->d[r][96],(3*8*Kr_bytes)+12,1,4);
#endif
      start_meas(i_stats);
      dlsch->harq_processes[harq_pid]->RTC[r] = 
	sub_block_interleaving_turbo(4+(Kr_bytes*8), 
				     &dlsch->harq_processes[harq_pid]->d[r][96], 
				     dlsch->harq_processes[harq_pid]->w[r]);
      stop_meas(i_stats);
    }
    
  }

  // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
  // outputs for each code segment, see Section 5.1.5 p.20

  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_DLSCH_CODING
    msg("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	   r,
	   G,
	   Kr*3,
	   mod_order,nb_rb);
#endif

    start_meas(rm_stats);
    r_offset += lte_rate_matching_turbo(dlsch->harq_processes[harq_pid]->RTC[r],
					G,  //G
					dlsch->harq_processes[harq_pid]->w[r],
					dlsch->harq_processes[harq_pid]->e+r_offset,
					dlsch->harq_processes[harq_pid]->C, // C
					NSOFT,                    // Nsoft,
					dlsch->Mdlharq,
					dlsch->Kmimo,
					dlsch->harq_processes[harq_pid]->rvidx,
					get_Qm(dlsch->harq_processes[harq_pid]->mcs),
					dlsch->harq_processes[harq_pid]->Nl,
					r,
					nb_rb,
					m);                       // r
    stop_meas(rm_stats);
#ifdef DEBUG_DLSCH_CODING
    if (r==dlsch->harq_processes[harq_pid]->C-1)
      write_output("enc_output.m","enc",dlsch->harq_processes[harq_pid]->e,r_offset,1,4);
#endif
  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ENCODING, VCD_FUNCTION_OUT);

  return(0);
}

#ifdef PHY_ABSTRACTION
void dlsch_encoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			 uint8_t *DLSCH_pdu,
			 LTE_eNB_DLSCH_t *dlsch) {

  //int payload_offset = 0;
  unsigned char harq_pid = dlsch->current_harq_pid;
  unsigned short i;

  //  if (dlsch->harq_processes[harq_pid]->Ndi == 1) {
  if (dlsch->harq_processes[harq_pid]->round == 0) {
    memcpy(dlsch->harq_processes[harq_pid]->b,
	   DLSCH_pdu,
	   dlsch->harq_processes[harq_pid]->TBS>>3);
    LOG_D(PHY, "eNB %d dlsch_encoding_emul, tbs is %d harq pid %d \n", 
	phy_vars_eNB->Mod_id,
	  dlsch->harq_processes[harq_pid]->TBS>>3,
	  harq_pid);

    for (i=0;i<dlsch->harq_processes[harq_pid]->TBS>>3;i++)
      LOG_T(PHY,"%x.",DLSCH_pdu[i]);
    LOG_T(PHY,"\n");

    memcpy(&eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].transport_blocks[eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]],
	   //	    memcpy(&eNB_transport_info[phy_vars_eNB->Mod_id].transport_blocks[payload_offset],
    	   DLSCH_pdu,
	   dlsch->harq_processes[harq_pid]->TBS>>3);
  }  
  eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]+=dlsch->harq_processes[harq_pid]->TBS>>3;
  //payload_offset +=dlsch->harq_processes[harq_pid]->TBS>>3;
  
}
#endif
