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

/*! \file PHY/LTE_TRANSPORT/ulsch_decoding.c
* \brief Top-level routines for decoding  the ULSCH transport channel from 36.212 V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/

//#include "defs.h"

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/extern.h"
#include "extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/extern.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/extern.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"
#endif

#ifdef OMP
#include <omp.h>
#endif

#ifdef PHY_ABSTRACTION
#include "UTIL/OCG/OCG.h"
#include "UTIL/OCG/OCG_extern.h"
#endif

#include "UTIL/LOG/vcd_signal_dumper.h"
//#define DEBUG_ULSCH_DECODING

void free_eNB_ulsch(LTE_eNB_ULSCH_t *ulsch) {

  int i,r;

  if (ulsch) {
    for (i=0;i<ulsch->Mdlharq;i++) {
      if (ulsch->harq_processes[i]) {
	if (ulsch->harq_processes[i]->b) {
	  free16(ulsch->harq_processes[i]->b,MAX_ULSCH_PAYLOAD_BYTES);
	  ulsch->harq_processes[i]->b = NULL;
	}
	if (ulsch->harq_processes[i]->c) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {
	    free16(ulsch->harq_processes[i]->c[r],((r==0)?8:0) + 768);
	    ulsch->harq_processes[i]->c[r] = NULL;
	  }
	}
	for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	  if (ulsch->harq_processes[i]->d[r]) {
	    free16(ulsch->harq_processes[i]->d[r],((3*8*6144)+12+96)*sizeof(short));
	    ulsch->harq_processes[i]->d[r] = NULL;
	  }
	free16(ulsch->harq_processes[i],sizeof(LTE_UL_eNB_HARQ_t));
	ulsch->harq_processes[i] = NULL;
      }
    }
  free16(ulsch,sizeof(LTE_eNB_ULSCH_t));
  ulsch = NULL;
  }
}

LTE_eNB_ULSCH_t *new_eNB_ulsch(uint8_t Mdlharq,uint8_t max_turbo_iterations,uint8_t N_RB_UL, uint8_t abstraction_flag) {

  LTE_eNB_ULSCH_t *ulsch;
  uint8_t exit_flag = 0,i,r;
  unsigned char bw_scaling =1;
  
  switch (N_RB_UL){
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
  
  ulsch = (LTE_eNB_ULSCH_t *)malloc16(sizeof(LTE_eNB_ULSCH_t));
  if (ulsch) {
    memset(ulsch,0,sizeof(LTE_eNB_ULSCH_t));
    ulsch->Mdlharq = Mdlharq;
    ulsch->max_turbo_iterations = max_turbo_iterations;

    for (i=0;i<Mdlharq;i++) {
      //      msg("new_ue_ulsch: Harq process %d\n",i);
      ulsch->harq_processes[i] = (LTE_UL_eNB_HARQ_t *)malloc16(sizeof(LTE_UL_eNB_HARQ_t));
      if (ulsch->harq_processes[i]) {
	memset(ulsch->harq_processes[i],0,sizeof(LTE_UL_eNB_HARQ_t));
	ulsch->harq_processes[i]->b = (uint8_t*)malloc16(MAX_ULSCH_PAYLOAD_BYTES/bw_scaling);
	if (ulsch->harq_processes[i]->b)
	  memset(ulsch->harq_processes[i]->b,0,MAX_ULSCH_PAYLOAD_BYTES/bw_scaling);
	else
	  exit_flag=3;
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS/bw_scaling;r++) {
	    ulsch->harq_processes[i]->c[r] = (uint8_t*)malloc16(((r==0)?8:0) + 3+768);	
	    if (ulsch->harq_processes[i]->c[r])
	      memset(ulsch->harq_processes[i]->c[r],0,((r==0)?8:0) + 3+768);
	    else
	      exit_flag=2;
	    ulsch->harq_processes[i]->d[r] = (short*)malloc16(((3*8*6144)+12+96)*sizeof(short));
	    if (ulsch->harq_processes[i]->d[r])
	      memset(ulsch->harq_processes[i]->d[r],0,((3*8*6144)+12+96)*sizeof(short));
	    else
	      exit_flag=2;
	  }
	  ulsch->harq_processes[i]->subframe_scheduling_flag = 0;
	}
      }	else {
	exit_flag=1;
      }
    }

    if (exit_flag==0)
      return(ulsch);
  }
  LOG_E(PHY,"new_ue_ulsch: exit_flag = %d\n",exit_flag);
  free_eNB_ulsch(ulsch);

  return(NULL);
}

void clean_eNb_ulsch(LTE_eNB_ULSCH_t *ulsch, uint8_t abstraction_flag) {

  unsigned char Mdlharq;
  unsigned char i;

  //ulsch = (LTE_eNB_ULSCH_t *)malloc16(sizeof(LTE_eNB_ULSCH_t));
  if (ulsch) {
    Mdlharq = ulsch->Mdlharq;
    ulsch->rnti = 0;
    for (i=0;i<Mdlharq;i++) {
      if (ulsch->harq_processes[i]) {
	//	  ulsch->harq_processes[i]->Ndi = 0;
	  ulsch->harq_processes[i]->status = 0;
	  ulsch->harq_processes[i]->subframe_scheduling_flag = 0;
	  //ulsch->harq_processes[i]->phich_active = 0; //this will be done later after transmission of PHICH
	  ulsch->harq_processes[i]->phich_ACK = 0;
	  ulsch->harq_processes[i]->round = 0;
      }
    }

  }
}


uint8_t extract_cqi_crc(uint8_t *cqi,uint8_t CQI_LENGTH) {

  uint8_t crc;

  crc = cqi[CQI_LENGTH>>3];
  //  msg("crc1: %x, shift %d\n",crc,CQI_LENGTH&0x7);
  crc = (crc<<(CQI_LENGTH&0x7));
  // clear crc bits
  //  ((char *)cqi)[CQI_LENGTH>>3] &= 0xff>>(8-(CQI_LENGTH&0x7));
  //  msg("crc2: %x, cqi0 %x\n",crc,cqi[1+(CQI_LENGTH>>3)]);
  crc |= (cqi[1+(CQI_LENGTH>>3)])>>(8-(CQI_LENGTH&0x7));
  // clear crc bits
  //(((char *)cqi)[1+(CQI_LENGTH>>3)]) = 0;

  //  printf("crc : %x\n",crc);
  return(crc);

}



unsigned int  ulsch_decoding(PHY_VARS_eNB *phy_vars_eNB,
			     uint8_t UE_id,
			     uint8_t sched_subframe,
			     uint8_t control_only_flag,
			     uint8_t Nbundled,
			     uint8_t llr8_flag) {


  int16_t *ulsch_llr = phy_vars_eNB->lte_eNB_pusch_vars[UE_id]->llr;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNB->lte_frame_parms;
  LTE_eNB_ULSCH_t *ulsch = phy_vars_eNB->ulsch_eNB[UE_id];
  uint8_t harq_pid;
  unsigned short nb_rb;
  unsigned int A,E;
  uint8_t Q_m;
  unsigned int i,i2,q,j,j2;
  int iprime;
  unsigned int ret=0,offset;
  unsigned short iind;
  //  uint8_t dummy_channel_output[(3*8*block_length)+12];

  unsigned int r,r_offset=0,Kr,Kr_bytes;
  uint8_t crc_type;
  uint8_t *columnset;
  unsigned int sumKr=0;
  unsigned int Qprime,L,G,Q_CQI,Q_RI,H,Hprime,Hpp,Cmux,Rmux_prime,O_RCC;
  unsigned int Qprime_ACK,Qprime_CQI,Qprime_RI,len_ACK=0,len_RI=0;
  //  uint8_t q_ACK[MAX_ACK_PAYLOAD],q_RI[MAX_RI_PAYLOAD];
  int metric,metric_new;
  uint8_t o_flip[8];
  uint32_t x1, x2, s=0;
  int16_t ys,c;
  uint32_t wACK_idx;
  int16_t dummy_w[MAX_NUM_ULSCH_SEGMENTS][3*(6144+64)];
  uint8_t dummy_w_cc[3*(MAX_CQI_BITS+8+32)];
  int16_t y[6*14*1200];
  uint8_t ytag[14*1200];
  //  uint8_t ytag2[6*14*1200],*ytag2_ptr;
  int16_t cseq[6*14*1200];
  int off;
  int status[20];
  int subframe = phy_vars_eNB->proc[sched_subframe].subframe_rx;

  uint8_t (*tc)(int16_t *y,
		uint8_t *,
		uint16_t,			       
		uint16_t,
		uint16_t,
		uint8_t,
		uint8_t,
		uint8_t,
		time_stats_t *,
		time_stats_t *,
		time_stats_t *,
		time_stats_t *,
		time_stats_t *,
		time_stats_t *,
		time_stats_t *);

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING,1);

  // x1 is set in lte_gold_generic
  x2 = ((uint32_t)ulsch->rnti<<14) + ((uint32_t)subframe<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.3.1
  
  //  harq_pid = (ulsch->RRCConnRequest_flag == 0) ? subframe2harq_pid_tdd(frame_parms->tdd_config,subframe) : 0;
  harq_pid = subframe2harq_pid(frame_parms,phy_vars_eNB->proc[sched_subframe].frame_rx,subframe);

  if (harq_pid==255) {
    LOG_E(PHY, "ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
    return(-1);
  }

  if (llr8_flag == 0)
    tc = phy_threegpplte_turbo_decoder16;
  else
    tc = phy_threegpplte_turbo_decoder8;
  
  nb_rb = ulsch->harq_processes[harq_pid]->nb_rb;

  A = ulsch->harq_processes[harq_pid]->TBS;

    
  Q_m = get_Qm_ul(ulsch->harq_processes[harq_pid]->mcs);
  G = nb_rb * (12 * Q_m) * ulsch->Nsymb_pusch;


#ifdef DEBUG_ULSCH_DECODING
  LOG_D(PHY,"ulsch_decoding (Nid_cell %d, rnti %x, x2 %x): round %d, RV %d, mcs %d, O_RI %d, O_ACK %d, G %d, subframe %d\n",
      frame_parms->Nid_cell,ulsch->rnti,x2,
      ulsch->harq_processes[harq_pid]->round,
      ulsch->harq_processes[harq_pid]->rvidx,
      ulsch->harq_processes[harq_pid]->mcs,
      ulsch->O_RI,
      ulsch->harq_processes[harq_pid]->O_ACK,
      G,
      subframe);
#endif  

  if (ulsch->harq_processes[harq_pid]->round == 0) {
    // This is a new packet, so compute quantities regarding segmentation
    ulsch->harq_processes[harq_pid]->B = A+24;
    lte_segmentation(NULL,
		     NULL,
		     ulsch->harq_processes[harq_pid]->B,
		     &ulsch->harq_processes[harq_pid]->C,
		     &ulsch->harq_processes[harq_pid]->Cplus,
		     &ulsch->harq_processes[harq_pid]->Cminus,
		     &ulsch->harq_processes[harq_pid]->Kplus,
		     &ulsch->harq_processes[harq_pid]->Kminus,		     
		     &ulsch->harq_processes[harq_pid]->F);
    //  CLEAR LLR's HERE for first packet in process
  }


  sumKr = 0;
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    if (r<ulsch->harq_processes[harq_pid]->Cminus)
      Kr = ulsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
    sumKr += Kr;
  }
  if (sumKr==0) {
    LOG_N(PHY,"[eNB %d] ulsch_decoding.c: FATAL sumKr is 0!\n",phy_vars_eNB->Mod_id);
    LOG_D(PHY,"ulsch_decoding (Nid_cell %d, rnti %x, x2 %x): harq_pid %d round %d, RV %d, mcs %d, O_RI %d, O_ACK %d, G %d, subframe %d\n",
	frame_parms->Nid_cell,ulsch->rnti,x2,
	harq_pid,
	ulsch->harq_processes[harq_pid]->round,
	ulsch->harq_processes[harq_pid]->rvidx,
	ulsch->harq_processes[harq_pid]->mcs,
	ulsch->O_RI,
	ulsch->harq_processes[harq_pid]->O_ACK,
	G,
	subframe);
    mac_xface->macphy_exit("ulsch_decoding.c: FATAL sumKr is 0!");
    return(-1);
  }
    
  // Compute Q_ri
  Qprime = ulsch->O_RI*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_ri_times8;

  if (Qprime > 0 ) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
    
    if (Qprime > 4*nb_rb * 12)
      Qprime = 4*nb_rb * 12;
  }

  Q_RI = Q_m*Qprime;
  Qprime_RI = Qprime;


  // Compute Q_ack

  Qprime = ulsch->harq_processes[harq_pid]->O_ACK*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_harqack_times8;
  if (Qprime > 0) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);

    if (Qprime > (4*nb_rb * 12))
      Qprime = 4*nb_rb * 12;
  }

  //  Q_ACK = Qprime * Q_m;
  Qprime_ACK = Qprime;
#ifdef DEBUG_ULSCH_DECODING
  LOG_D(PHY,"ulsch_decoding.c: Qprime_ACK %d, Msc_initial %d, Nsymb_initial %d, sumKr %d\n",
      Qprime_ACK,ulsch->harq_processes[harq_pid]->Msc_initial,ulsch->harq_processes[harq_pid]->Nsymb_initial,sumKr);
#endif
  // Compute Q_cqi
  if (ulsch->Or1 < 12)
    L=0;
  else 
    L=8;
  if (ulsch->Or1 > 0)
    Qprime = (ulsch->Or1 + L) * ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  else
    Qprime=0;

  if (Qprime > 0) { 
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
  }

  G = nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);

 

  if (Qprime > (G - ulsch->O_RI))
    Qprime = G - ulsch->O_RI;

  Q_CQI = Q_m * Qprime;
  //#ifdef DEBUG_ULSCH_DECODING
    LOG_D(PHY,"ulsch_decoding: G %d, Q_RI %d, Q_CQI %d (L %d, Or1 %d) O_ACK %d\n",G,Q_RI,Q_CQI,L,ulsch->Or1,ulsch->harq_processes[harq_pid]->O_ACK); 
  //#endif
  Qprime_CQI = Qprime;

  G = G - Q_RI - Q_CQI;

  if ((int)G < 0) {
    LOG_E(PHY,"FATAL: ulsch_decoding.c G < 0 (%d) : Q_RI %d, Q_CQI %d\n",G,Q_RI,Q_CQI);
    return(-1);
  }

  H = G + Q_CQI;
  Hprime = H/Q_m;

  // Demultiplexing/Deinterleaving of PUSCH/ACK/RI/CQI
  Hpp = Hprime + Qprime_RI;
  
  Cmux       = ulsch->Nsymb_pusch;
  //  Rmux       = Hpp*Q_m/Cmux;
  Rmux_prime = Hpp/Cmux;
  
#ifdef DEBUG_ULSCH_DECODING
  LOG_D(PHY,"ulsch_decoding.c: G raw %d (%d symb), Hpp %d, Cmux %d, Rmux_prime %d\n",G,ulsch->Nsymb_pusch,Hpp,Cmux,Rmux_prime);
#endif
  // Clear "tag" interleaving matrix to allow for CQI/DATA identification
  memset(ytag,0,Cmux*Rmux_prime);

  start_meas(&phy_vars_eNB->ulsch_demultiplexing_stats);

  i=0;
  memset(y,LTE_NULL,Q_m*Hpp);
  /*
  //  Do RI coding
  if (ulsch->O_RI == 1) {
    switch (Q_m) {
    case 2:
      q_RI[0] = 0;
      q_RI[1] = PUSCH_y;
      len_RI=2;
      break;
    case 4:
      q_RI[0] = 0;
      q_RI[1] = PUSCH_y;//1;
      q_RI[2] = PUSCH_x;//o_RI[0];
      q_RI[3] = PUSCH_x;//1;
      len_RI=4;
      break;
    case 6:
      q_RI[0] = 0;
      q_RI[1] = PUSCH_y;//1;
      q_RI[2] = PUSCH_x;//1;
      q_RI[3] = PUSCH_x;//ulsch->o_RI[0];
      q_RI[4] = PUSCH_x;//1;
      q_RI[5] = PUSCH_x;//1;
      len_RI=6;
      break;
    }
  }
  else if (ulsch->O_RI > 1){
    LOG_E(PHY,"ulsch_decoding: FATAL, RI cannot be more than 1 bit yet\n");
    return(-1);
  }
 
  // 1-bit ACK/NAK
  if (ulsch->harq_processes[harq_pid]->O_ACK == 1) {
    switch (Q_m) {
    case 2:
      q_ACK[0] = 0;
      q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : 0;
      len_ACK = 2;
      break;
    case 4:
      q_ACK[0] = 0;
      q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : 0;
      q_ACK[2] = PUSCH_x;
      q_ACK[3] = PUSCH_x;
      len_ACK = 4;
      break;
    case 6:
      q_ACK[0] = 0;
      q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : 0;
      q_ACK[2] = PUSCH_x;
      q_ACK[3] = PUSCH_x;
      q_ACK[4] = PUSCH_x;
      q_ACK[6] = PUSCH_x;
      len_ACK = 6;
      break;
    }
  }
  // two-bit ACK/NAK
  if (ulsch->harq_processes[harq_pid]->O_ACK == 2) {
    switch (Q_m) {
    case 2:
      q_ACK[0] = 0;
      q_ACK[1] = 0;
      q_ACK[2] = 0;
      q_ACK[3] = 0;
      q_ACK[4] = 0;
      q_ACK[5] = 0;
      len_ACK = 6;
      break;
    case 4:
      q_ACK[0]  = 0;
      q_ACK[1]  = 0;
      q_ACK[2]  = PUSCH_x;
      q_ACK[3]  = PUSCH_x;//1;
      q_ACK[4]  = 0;
      q_ACK[5]  = 0;
      q_ACK[6]  = PUSCH_x;
      q_ACK[7]  = PUSCH_x;//1;
      q_ACK[8]  = 0;
      q_ACK[9]  = 0;
      q_ACK[10] = PUSCH_x;
      q_ACK[11] = PUSCH_x;//1;
      len_ACK = 12;
      break;
    case 6:
      q_ACK[0] = 0;
      q_ACK[1] = 0;
      q_ACK[2] = PUSCH_x;
      q_ACK[3] = PUSCH_x;
      q_ACK[4] = PUSCH_x;
      q_ACK[5] = PUSCH_x;

      q_ACK[6] = 0;
      q_ACK[7] = 0;
      q_ACK[8] = PUSCH_x;
      q_ACK[9] = PUSCH_x; 
      q_ACK[10] = PUSCH_x;
      q_ACK[11] = PUSCH_x;

      q_ACK[12] = 0;
      q_ACK[13] = 0;
      q_ACK[14] = PUSCH_x;
      q_ACK[15] = PUSCH_x;
      q_ACK[16] = PUSCH_x;
      q_ACK[17] = PUSCH_x;
      len_ACK = 18;

      break;
    }
  }
  if (ulsch->harq_processes[harq_pid]->O_ACK > 2) {
    LOG_E(PHY,"ulsch_decoding: FATAL, ACK cannot be more than 2 bits yet\n");
    return(-1);
  }


  // RI BITS 

  //  memset(ytag2,0,Q_m*Hpp);

  */
  // read in buffer and unscramble llrs for everything but placeholder bits
  // llrs stored per symbol correspond to columns of interleaving matrix


  s = lte_gold_generic(&x1, &x2, 1);
  i2=0;
  for (i=0;i<((Hpp*Q_m)>>5);i++) {
    for (j=0;j<32;j++) {
      cseq[i2++] = (int16_t)((((s>>j)&1)<<1)-1);
    }
    s = lte_gold_generic(&x1, &x2, 0);
  }

  if (frame_parms->Ncp == 0)
    columnset = cs_ri_normal;
  else
    columnset = cs_ri_extended;
  j=0;   
  for (i=0;i<Qprime_RI;i++) {
    r = Rmux_prime - 1 - (i>>2);
    /*    
    for (q=0;q<Q_m;q++) 
      ytag2[q+(Q_m*((r*Cmux) + columnset[j]))]  = q_RI[(q+(Q_m*i))%len_RI];
    */
    off =((Rmux_prime*Q_m*columnset[j])+(r*Q_m)); 
    cseq[off+1] = cseq[off];  // PUSCH_y
    for (q=2;q<Q_m;q++)
      cseq[off+q] = -1;    // PUSCH_x

    j=(j+3)&3;

  }

  // HARQ-ACK Bits (Note these overwrite some bits)
  if (frame_parms->Ncp == 0)
    columnset = cs_ack_normal;
  else
    columnset = cs_ack_extended;

  j=0;
  for (i=0;i<Qprime_ACK;i++) {
    r = Rmux_prime - 1 - (i>>2);
    /*
    for (q=0;q<Q_m;q++) {
      ytag2[q+(Q_m*((r*Cmux) + columnset[j]))]  = q_ACK[(q+(Q_m*i))%len_ACK];
    }
    */
    off =((Rmux_prime*Q_m*columnset[j])+(r*Q_m)); 
    if (ulsch->harq_processes[harq_pid]->O_ACK == 1) {
      if (ulsch->bundling==0)
	cseq[off+1] = cseq[off];  // PUSCH_y
      for (q=2;q<Q_m;q++)
	cseq[off+q] = -1;    // PUSCH_x      
    }
    else if (ulsch->harq_processes[harq_pid]->O_ACK == 2) {
      for (q=2;q<Q_m;q++)
	cseq[off+q] = -1;    // PUSCH_x 
    }
#ifdef DEBUG_ULSCH_DECODING
    LOG_D(PHY,"ulsch_decoding.c: ACK i %d, r %d, j %d, ColumnSet[j] %d\n",i,r,j,columnset[j]); 
#endif
    j=(j+3)&3;
  }



  i=0;
  switch (Q_m) {
  case 2:
    for (j=0;j<Cmux;j++) {
      i2=j<<1;
      for (r=0;r<Rmux_prime;r++) {
	c = cseq[i];
	//	printf("ulsch %d: %d * ",i,c);
	y[i2++] = c*ulsch_llr[i++];
	//	printf("%d\n",ulsch_llr[i-1]);
	c = cseq[i];
	//	printf("ulsch %d: %d * ",i,c);
	y[i2] = c*ulsch_llr[i++];
	//	printf("%d\n",ulsch_llr[i-1]);
	i2=(i2+(Cmux<<1)-1);
      }
    }
    break;
  case 4:
    for (j=0;j<Cmux;j++) {
      i2=j<<2;
      for (r=0;r<Rmux_prime;r++) {
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2] = c*ulsch_llr[i++];
	i2=(i2+(Cmux<<2)-3);
      }
    }
    break; 
  case 6:
    for (j=0;j<Cmux;j++) {
      i2=j*6;
      for (r=0;r<Rmux_prime;r++) {
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2++] = c*ulsch_llr[i++];
	c = cseq[i];
	y[i2] = c*ulsch_llr[i++];
	i2=(i2+(Cmux*6)-5);
      }
    }
    break;
  }

  
	//      for (q=0;q<Q_m;q++) {
	/*
	if ((i&0x1f)==0) {
	  s = lte_gold_generic(&x1, &x2, reset);
	  //	  msg("lte_gold[%d]=%x\n",i,s);
	  reset = 0;
	}
	c = (uint8_t)((s>>(i&0x1f))&1);
	


	// if bits are tagged as placeholders (RI,ACK)

	if (ytag2[q+(Q_m*((r*Cmux)+j))] == PUSCH_y) {

	  c=c_prev;
	}
	else if (ytag2[q+(Q_m*((r*Cmux)+j))] == PUSCH_x) {
	  c = 0;
#ifdef DEBUG_ULSCH_DECODING
	  //	  msg("ulsch_decoding.c: PUSCH_x in row %d, col %d: llr %d\n",r,j,ulsch_llr[i]);
#endif
	}
	c_prev = c;
#ifdef DEBUG_ULSCH_DECODING
	//	msg("llr[%d] = %d (c %d, ytag2 %d) ==> ",i,ulsch_llr[i],c,ytag2[q+(Q_m*((r*Cmux)+j))]);
#endif
	// note flipped here for reverse polarity in 3GPP bit mapping
	y[q+(Q_m*((r*Cmux)+j))] = (c==0) ? -ulsch_llr[i] : ulsch_llr[i];
	i++;
#ifdef DEBUG_ULSCH_DECODING
	//	msg("%d\n",y[q+(Q_m*((r*Cmux)+j))]);
#endif
	*/
  stop_meas(&phy_vars_eNB->ulsch_demultiplexing_stats);

  if (i!=(H+Q_RI))
    LOG_D(PHY,"ulsch_decoding.c: Error in input buffer length (j %d, H+Q_RI %d)\n",i,H+Q_RI); 

  // HARQ-ACK Bits (LLRs are nulled in overwritten bits after copying HARQ-ACK LLR)

  if (frame_parms->Ncp == 0)
    columnset = cs_ack_normal;
  else
    columnset = cs_ack_extended;

  j=0;

  if (ulsch->harq_processes[harq_pid]->O_ACK == 1) {
    switch (Q_m) {
    case 2:
      len_ACK = 2;
      break;
    case 4:
      len_ACK = 4;
      break;
    case 6:
      len_ACK = 6;
      break;
    }
  }
  if (ulsch->harq_processes[harq_pid]->O_ACK == 2) {
    switch (Q_m) {
    case 2:
      len_ACK = 6;
      break;
    case 4:
      len_ACK = 12;
      break;
    case 6:
      len_ACK = 18;
      break;
    }
  }
  if (ulsch->harq_processes[harq_pid]->O_ACK > 2) {
    LOG_E(PHY,"ulsch_decoding: FATAL, ACK cannot be more than 2 bits yet\n");
    return(-1);
  }

  for (i=0;i<len_ACK;i++)
    ulsch->q_ACK[i] = 0;


  for (i=0;i<Qprime_ACK;i++) {
    r = Rmux_prime -1 - (i>>2);
    for (q=0;q<Q_m;q++) {
      if (y[q+(Q_m*((r*Cmux) + columnset[j]))]!=0)
	ulsch->q_ACK[(q+(Q_m*i))%len_ACK] += y[q+(Q_m*((r*Cmux) + columnset[j]))];
#ifdef DEBUG_ULSCH_DECODING
      //      LOG_D(PHY,"ACK %d => %d (%d,%d,%d)\n",(q+(Q_m*i))%len_ACK,ulsch->q_ACK[(q+(Q_m*i))%len_ACK],q+(Q_m*((r*Cmux) + columnset[j])),r,columnset[j]);
      printf("ACK %d => %d (%d,%d,%d)\n",(q+(Q_m*i))%len_ACK,ulsch->q_ACK[(q+(Q_m*i))%len_ACK],q+(Q_m*((r*Cmux) + columnset[j])),r,columnset[j]);
#endif
      y[q+(Q_m*((r*Cmux) + columnset[j]))]=0;  // NULL LLRs in ACK positions
    }
    j=(j+3)&3;
  }


  // RI BITS 

  if (ulsch->O_RI == 1) {
    switch (Q_m) {
    case 2:
        len_RI=2;
      break;
    case 4:
      len_RI=4;
      break;
    case 6:
      len_RI=6;
      break;
    }
  }

  if (ulsch->O_RI > 1) {
    LOG_E(PHY,"ulsch_decoding: FATAL, RI cannot be more than 1 bit yet\n");
    return(-1);
  }

  for (i=0;i<len_RI;i++)
    ulsch->q_RI[i] = 0;
 
  if (frame_parms->Ncp == 0)
    columnset = cs_ri_normal;
  else
    columnset = cs_ri_extended;
  j=0;   
  for (i=0;i<Qprime_RI;i++) {
    r = Rmux_prime -1 - (i>>2);
    for (q=0;q<Q_m;q++) 
      ulsch->q_RI[(q+(Q_m*i))%len_RI] += y[q+(Q_m*((r*Cmux) + columnset[j]))];
    ytag[(r*Cmux) + columnset[j]] = LTE_NULL;
    j=(j+3)&3;
  }

  // CQI and Data bits
  j=0;j2=0;
  //  r=0;
  for (i=0;i<Qprime_CQI;i++) {

    /*
    while (ytag[(r*Cmux)+j]==LTE_NULL) {
#ifdef DEBUG_ULSCH_DECODING	
      msg("ulsch_decoding.c: r %d, j %d: LTE_NULL\n",r,j);
#endif
      j++;
      if (j==Cmux) {
	j=0;
	r++;
      }
    }
    for (q=0;q<Q_m;q++) {
      ys = y[q+(Q_m*((r*Cmux)+j))];
      if (ys>127)
	ulsch->q[q+(Q_m*i)] = 127;
      else if (ys<-128)
	ulsch->q[q+(Q_m*i)] = -128;
      else 
	ulsch->q[q+(Q_m*i)] = ys;
#ifdef DEBUG_ULSCH_DECODING	
      msg("ulsch_decoding.c: CQI %d, r %d, j %d, y[%d] %d\n",q+(Q_m*i),r,j, q+(Q_m*((r*Cmux) + j)),ys);
#endif
    } 
    */
    while (ytag[j]==LTE_NULL){j++;j2+=Q_m;}

    for (q=0;q<Q_m;q++) {
      //      ys = y[q+(Q_m*((r*Cmux)+j))];
      ys = y[q+j2];
      if (ys>127)
	ulsch->q[q+(Q_m*i)] = 127;
      else if (ys<-128)
	ulsch->q[q+(Q_m*i)] = -128;
      else 
	ulsch->q[q+(Q_m*i)] = ys;
 #ifdef DEBUG_ULSCH_DECODING	
      LOG_D(PHY,"ulsch_decoding.c: CQI %d, q %d, y[%d] %d\n",q+(Q_m*i),q,j2, q+j2,ys);
 #endif
    } 
    j2+=Q_m;
  }

  // j2=j*Q_m;

  switch (Q_m) {
  case 2:
    for (iprime=0;iprime<(Hprime-Qprime_CQI)<<1;) {
      while (ytag[j]==LTE_NULL) { j++;j2+=2; }

      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
#ifdef DEBUG_ULSCH_DECODING	
	//	msg("ulsch_decoding.c: e %d, r %d, j %d, y[%d] %d\n",g,r,j,q+(Q_m*((r*Cmux) + j)),y[q+(Q_m*((r*Cmux)+j))]);
#endif
      
    }
    //    write_output("/tmp/ulsch_e.m","ulsch_e",ulsch->e,iprime,1,0);
    break;
  case 4:
    for (iprime=0;iprime<(Hprime-Qprime_CQI)<<2;) {
      while (ytag[j]==LTE_NULL) { j++;j2+=4; }

      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
#ifdef DEBUG_ULSCH_DECODING	
	//	msg("ulsch_decoding.c: e %d, r %d, j %d, y[%d] %d\n",g,r,j,q+(Q_m*((r*Cmux) + j)),y[q+(Q_m*((r*Cmux)+j))]);
#endif
      
    }
    break;
  case 6:
    for (iprime=0;iprime<(Hprime-Qprime_CQI)*6;) {
      while (ytag[j]==LTE_NULL) { j++;j2+=6; }

      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
      ulsch->e[iprime++] = y[j2++];
#ifdef DEBUG_ULSCH_DECODING	
	//	msg("ulsch_decoding.c: e %d, r %d, j %d, y[%d] %d\n",g,r,j,q+(Q_m*((r*Cmux) + j)),y[q+(Q_m*((r*Cmux)+j))]);
#endif
      
    }
    break;

  }
  /*
  for (i=0,iprime=-Qprime_CQI;i<Hprime;i++,iprime++) {

    while (ytag[(r*Cmux)+j]==LTE_NULL) {
#ifdef DEBUG_ULSCH_DECODING	
      msg("ulsch_decoding.c: r %d, j %d: LTE_NULL\n",r,j);
#endif
      j++;
      if (j==Cmux) {
	j=0;
	r++;
      }
    }
    
    if (i<Qprime_CQI) {
      
      for (q=0;q<Q_m;q++) {
	ys = y[q+(Q_m*((r*Cmux)+j))];
	if (ys>127)
	  ulsch->q[q+(Q_m*i)] = 127;
	else if (ys<-128)
	  ulsch->q[q+(Q_m*i)] = -128;
	else 
	  ulsch->q[q+(Q_m*i)] = ys;
#ifdef DEBUG_ULSCH_DECODING	
	msg("ulsch_decoding.c: CQI %d, r %d, j %d, y[%d] %d\n",q+(Q_m*i),r,j, q+(Q_m*((r*Cmux) + j)),ys);
#endif
      }
    } 
    else {
      for (q=0;q<Q_m;q++) {
	g = q+(Q_m*iprime);
	ulsch->e[g] = y[q+(Q_m*((r*Cmux)+j))];
#ifdef DEBUG_ULSCH_DECODING	
	//	msg("ulsch_decoding.c: e %d, r %d, j %d, y[%d] %d\n",g,r,j,q+(Q_m*((r*Cmux) + j)),y[q+(Q_m*((r*Cmux)+j))]);
#endif
      }
    }
    j++;
    if (j==Cmux) {
      j=0;
      r++;
    }
  }
*/

  // Do CQI/RI/HARQ-ACK Decoding first and pass to MAC

  // HARQ-ACK 
  wACK_idx = (ulsch->bundling==0) ? 4 : ((Nbundled-1)&3);
#ifdef DEBUG_ULSCH_DECODING
  LOG_D(PHY,"ulsch_decoding.c: Bundling %d, Nbundled %d, wACK_idx %d\n",
      ulsch->bundling,Nbundled,wACK_idx);
#endif
  if (ulsch->harq_processes[harq_pid]->O_ACK == 1) {
      ulsch->q_ACK[0] *= wACK_RX[wACK_idx][0]; 
      ulsch->q_ACK[0] += (ulsch->bundling==0) ? ulsch->q_ACK[1]*wACK_RX[wACK_idx][0] : ulsch->q_ACK[1]*wACK_RX[wACK_idx][1];
  
      if (ulsch->q_ACK[0] < 0)
	ulsch->o_ACK[0] = 0;
      else
	ulsch->o_ACK[0] = 1;
#ifdef DEBUG_ULSCH_DECODING
      LOG_D(PHY,"ulsch_decoding.c: ulsch_q_ACK[0] %d (%d,%d)\n",ulsch->q_ACK[0],wACK_RX[wACK_idx][0],wACK_RX[wACK_idx][1]);
#endif
  }
  if (ulsch->harq_processes[harq_pid]->O_ACK == 2) {
    switch (Q_m) {

    case 2:
      ulsch->q_ACK[0] = ulsch->q_ACK[0]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[3]*wACK_RX[wACK_idx][1];
      ulsch->q_ACK[1] = ulsch->q_ACK[1]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[4]*wACK_RX[wACK_idx][1];
      ulsch->q_ACK[2] = ulsch->q_ACK[2]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[5]*wACK_RX[wACK_idx][1];

      break;
    case 4:
      ulsch->q_ACK[0] = ulsch->q_ACK[0]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[5]*wACK_RX[wACK_idx][1];
      ulsch->q_ACK[1] = ulsch->q_ACK[1]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[8]*wACK_RX[wACK_idx][1];
      ulsch->q_ACK[2] = ulsch->q_ACK[4]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[9]*wACK_RX[wACK_idx][1];

      break;
    case 6:
      ulsch->q_ACK[0] =  ulsch->q_ACK[0]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[7]*wACK_RX[wACK_idx][1];
      ulsch->q_ACK[1] =  ulsch->q_ACK[1]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[12]*wACK_RX[wACK_idx][1]; 
      ulsch->q_ACK[2] =  ulsch->q_ACK[6]*wACK_RX[wACK_idx][0] + ulsch->q_ACK[13]*wACK_RX[wACK_idx][1]; 
      break;
    }
    ulsch->o_ACK[0] = 1;
    ulsch->o_ACK[1] = 1;
    metric     = ulsch->q_ACK[0]+ulsch->q_ACK[1]-ulsch->q_ACK[2];
    metric_new = -ulsch->q_ACK[0]+ulsch->q_ACK[1]+ulsch->q_ACK[2];

    if (metric_new > metric) {
      ulsch->o_ACK[0]=0;
      ulsch->o_ACK[1]=1;
      metric = metric_new;
    }
    metric_new = ulsch->q_ACK[0]-ulsch->q_ACK[1]+ulsch->q_ACK[2];


    if (metric_new > metric) {
      ulsch->o_ACK[0] = 1;
      ulsch->o_ACK[1] = 0;
      metric = metric_new;
    }
    metric_new = -ulsch->q_ACK[0]-ulsch->q_ACK[1]-ulsch->q_ACK[2];

    if (metric_new > metric) {
      ulsch->o_ACK[0] = 0;
      ulsch->o_ACK[1] = 0;
      metric = metric_new;
    }
  }

#ifdef DEBUG_ULSCH_DECODING
  for (i=0;i<ulsch->harq_processes[harq_pid]->O_ACK;i++)
    LOG_D(PHY,"ulsch_decoding: O_ACK[%d] %d, q_ACK => (%d,%d,%d)\n",i,ulsch->o_ACK[i],ulsch->q_ACK[0],ulsch->q_ACK[1],ulsch->q_ACK[2]);
#endif

  // RI

  if ((ulsch->O_RI == 1) && (Qprime_RI > 0)) {
    ulsch->o_RI[0] = ((ulsch->q_RI[0] + ulsch->q_RI[Q_m/2]) > 0) ? 0 : 1; 
  }
#ifdef DEBUG_ULSCH_DECODING

  if (Qprime_RI > 0) {
    for (i=0;i<2*ulsch->O_RI;i++)
      LOG_D(PHY,"ulsch_decoding: q_RI[%d] %d\n",i,ulsch->q_RI[i]);
  }
  
  if (Qprime_CQI > 0) {
    for (i=0;i<ulsch->O_RI;i++)
      LOG_D(PHY,"ulsch_decoding: O_RI[%d] %d\n",i,ulsch->o_RI[i]);
  }
#endif


  // CQI

  if (Qprime_CQI>0) {
    memset((void *)&dummy_w_cc[0],0,3*(ulsch->Or1+8+32));
    
    O_RCC = generate_dummy_w_cc(ulsch->Or1+8,
				&dummy_w_cc[0]);
    
    
    lte_rate_matching_cc_rx(O_RCC,
			    Q_CQI,
			    ulsch->o_w,
			    dummy_w_cc,
			    ulsch->q);
    
    sub_block_deinterleaving_cc((unsigned int)(ulsch->Or1+8),
				&ulsch->o_d[96], 
				&ulsch->o_w[0]); 
    
    memset(o_flip,0,1+((8+ulsch->Or1)/8));
    phy_viterbi_lte_sse2(ulsch->o_d+96,o_flip,8+ulsch->Or1);
    
    if (extract_cqi_crc(o_flip,ulsch->Or1) == (crc8(o_flip,ulsch->Or1)>>24))
      ulsch->cqi_crc_status = 1;
    else
      ulsch->cqi_crc_status = 0;

    //printf("crc(cqi) rx: %x\n",(crc8(o_flip,ulsch->Or1)>>24));

    if (ulsch->Or1<=32) {
      ulsch->o[3] = o_flip[0] ;
      ulsch->o[2] = o_flip[1] ;
      ulsch->o[1] = o_flip[2] ;
      ulsch->o[0] = o_flip[3] ;   
    }
    else {
      ulsch->o[7] = o_flip[0] ;
      ulsch->o[6] = o_flip[1] ;
      ulsch->o[5] = o_flip[2] ;
      ulsch->o[4] = o_flip[3] ;   
      ulsch->o[3] = o_flip[4] ;
      ulsch->o[2] = o_flip[5] ;
      ulsch->o[1] = o_flip[6] ;
      ulsch->o[0] = o_flip[7] ;   
      
    }
    
#ifdef DEBUG_ULSCH_DECODING
    LOG_D(PHY,"ulsch_decoding: Or1=%d\n",ulsch->Or1);
    for (i=0;i<1+((8+ulsch->Or1)/8);i++)
      msg("ulsch_decoding: O[%d] %d\n",i,ulsch->o[i]);
    if (ulsch->cqi_crc_status == 1)
      msg("RX CQI CRC OK (%x)\n",extract_cqi_crc(o_flip,ulsch->Or1));
    else
      msg("RX CQI CRC NOT OK (%x)\n",extract_cqi_crc(o_flip,ulsch->Or1));
#endif
  }

  //  return(0);
  // Do PUSCH Decoding

  //  stop_meas(&phy_vars_eNB->ulsch_demultiplexing_stats);


  r_offset = 0;
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    
    // Get Turbo interleaver parameters
    if (r<ulsch->harq_processes[harq_pid]->Cminus)
      Kr = ulsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
    Kr_bytes = Kr>>3;
    
    if (Kr_bytes<=64)
      iind = (Kr_bytes-5);
    else if (Kr_bytes <=128)
      iind = 59 + ((Kr_bytes-64)>>1);
    else if (Kr_bytes <= 256)
      iind = 91 + ((Kr_bytes-128)>>2);
    else if (Kr_bytes <= 768)
      iind = 123 + ((Kr_bytes-256)>>3);
    else {
      LOG_E(PHY,"ulsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      return(-1);
    }
    
#ifdef DEBUG_ULSCH_DECODING     
    msg("f1 %d, f2 %d, F %d\n",f1f2mat_old[2*iind],f1f2mat_old[1+(2*iind)],(r==0) ? ulsch->harq_processes[harq_pid]->F : 0);
#endif
    
    memset(&dummy_w[r][0],0,3*(6144+64)*sizeof(short));
    ulsch->harq_processes[harq_pid]->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       (uint8_t*)&dummy_w[r][0],
							       (r==0) ? ulsch->harq_processes[harq_pid]->F : 0);

#ifdef DEBUG_ULSCH_DECODING    
    msg("Rate Matching Segment %d (coded bits (G) %d,unpunctured/repeated bits %d, Q_m %d, nb_rb %d, Nl %d)...\n",
	   r, G,
	   Kr*3,
	   Q_m,
	   nb_rb,
	   ulsch->harq_processes[harq_pid]->Nl);
#endif    

    start_meas(&phy_vars_eNB->ulsch_rate_unmatching_stats);

    if (lte_rate_matching_turbo_rx(ulsch->harq_processes[harq_pid]->RTC[r],
				   G,
				   ulsch->harq_processes[harq_pid]->w[r],
				   (uint8_t*) &dummy_w[r][0],
				   ulsch->e+r_offset,
				   ulsch->harq_processes[harq_pid]->C,
				   NSOFT,
				   ulsch->Mdlharq,
				   1,
				   ulsch->harq_processes[harq_pid]->rvidx,
				   (ulsch->harq_processes[harq_pid]->round==0)?1:0,  // clear
				   get_Qm_ul(ulsch->harq_processes[harq_pid]->mcs),
				   1,
				   r,
				   &E)==-1) {
      LOG_E(PHY,"ulsch_decoding.c: Problem in rate matching\n");
      return(-1);
    }
    stop_meas(&phy_vars_eNB->ulsch_rate_unmatching_stats);
    r_offset += E;
    /*
    msg("Subblock deinterleaving, d %p w %p\n",
	   ulsch->harq_processes[harq_pid]->d[r],
	   ulsch->harq_processes[harq_pid]->w);
    */
    start_meas(&phy_vars_eNB->ulsch_deinterleaving_stats);
    sub_block_deinterleaving_turbo(4+Kr, 
				   &ulsch->harq_processes[harq_pid]->d[r][96], 
				   ulsch->harq_processes[harq_pid]->w[r]); 
    stop_meas(&phy_vars_eNB->ulsch_deinterleaving_stats);
    /*        
#ifdef DEBUG_ULSCH_DECODING    
    msg("decoder input(segment %d) :",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      msg("%d : %d\n",i,ulsch->harq_processes[harq_pid]->d[r][96+i]);
    msg("\n");
#endif
    */
  }

#ifdef OMP
#pragma omp parallel private(r,ret) shared(ulsch,harq_pid,crc_type,Kr,f1f2mat_old,phy_vars_eNB,status,iind,)
  {
#pragma omp for nowait
#endif
    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    //    msg("Clearing c, %p\n",ulsch->harq_processes[harq_pid]->c[r]);
    //    memset(ulsch->harq_processes[harq_pid]->c[r],0,16);//block_length);
    //    msg("done\n");
      if (ulsch->harq_processes[harq_pid]->C == 1) 
	crc_type = CRC24_A;
      else 
	crc_type = CRC24_B;
      
      /*            
		    msg("decoder input(segment %d)\n",r);
		    for (i=0;i<(3*8*Kr_bytes)+12;i++)
		    if ((ulsch->harq_processes[harq_pid]->d[r][96+i]>7) || 
		    (ulsch->harq_processes[harq_pid]->d[r][96+i] < -8))
		    msg("%d : %d\n",i,ulsch->harq_processes[harq_pid]->d[r][96+i]);
		    msg("\n");
      */
      
      start_meas(&phy_vars_eNB->ulsch_turbo_decoding_stats);
      
      ret = tc(&ulsch->harq_processes[harq_pid]->d[r][96],
	       ulsch->harq_processes[harq_pid]->c[r],
	       Kr,
	       f1f2mat_old[iind*2],   
	       f1f2mat_old[(iind*2)+1], 
	       ulsch->max_turbo_iterations,//MAX_TURBO_ITERATIONS,
	       crc_type,
	       (r==0) ? ulsch->harq_processes[harq_pid]->F : 0,
	       &phy_vars_eNB->ulsch_tc_init_stats,
	       &phy_vars_eNB->ulsch_tc_alpha_stats,
	       &phy_vars_eNB->ulsch_tc_beta_stats,
	       &phy_vars_eNB->ulsch_tc_gamma_stats,
	       &phy_vars_eNB->ulsch_tc_ext_stats,
	       &phy_vars_eNB->ulsch_tc_intl1_stats,
	       &phy_vars_eNB->ulsch_tc_intl2_stats);
      
      stop_meas(&phy_vars_eNB->ulsch_turbo_decoding_stats);

      status[r] = ret;
      if (ret==(1+ulsch->max_turbo_iterations)) {// a Code segment is in error so break;
#ifdef DEBUG_ULSCH_DECODING    
	msg("ULSCH harq_pid %d CRC failed\n",harq_pid);
#endif
	/*
	  for (i=0;i<Kr_bytes;i++)
	  printf("segment %d : byte %d => %d\n",r,i,ulsch->harq_processes[harq_pid]->c[r][i]);
	  return(ret);
	*/
      }
#ifdef DEBUG_ULSCH_DECODING    
      else
	msg("ULSCH harq_pid %d CRC OK : %d iterations\n",harq_pid, ret);
#endif

    }
#ifdef OMP
  }
#endif
  // Reassembly of Transport block here
  offset = 0;
  //  msg("F %d, Fbytes %d\n",ulsch->harq_processes[harq_pid]->F,ulsch->harq_processes[harq_pid]->F>>3);

  ret = 1;
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    if (status[r] != (1+ulsch->max_turbo_iterations)) {
      if (r<ulsch->harq_processes[harq_pid]->Cminus)
	Kr = ulsch->harq_processes[harq_pid]->Kminus;
      else
	Kr = ulsch->harq_processes[harq_pid]->Kplus;
      
      Kr_bytes = Kr>>3;
      
      if (r==0) {
	memcpy(ulsch->harq_processes[harq_pid]->b,
	       &ulsch->harq_processes[harq_pid]->c[0][(ulsch->harq_processes[harq_pid]->F>>3)],
	       Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3) - ((ulsch->harq_processes[harq_pid]->C>1)?3:0));
	offset = Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3) - ((ulsch->harq_processes[harq_pid]->C>1)?3:0);
	//            msg("copied %d bytes to b sequence\n",
	//      	     Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3));
      }
      else {
	memcpy(ulsch->harq_processes[harq_pid]->b+offset,
	       ulsch->harq_processes[harq_pid]->c[r],
	       Kr_bytes - ((ulsch->harq_processes[harq_pid]->C>1)?3:0));
	offset += (Kr_bytes- ((ulsch->harq_processes[harq_pid]->C>1)?3:0));
      }
      if (ret != (1+ulsch->max_turbo_iterations))
	ret = status[r];
    }
    else {
      ret = 1+ulsch->max_turbo_iterations;
    }
    
  }
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING,0);
  
  return(ret);
}

#ifdef PHY_ABSTRACTION

#ifdef PHY_ABSTRACTION_UL
 int ulsch_abstraction(double* sinr_dB, uint8_t TM, uint8_t mcs,uint16_t nrb, uint16_t frb) {

   int index,ii;
  double sinr_eff = 0;
  int rb_count = 0;
  int offset;
  double bler = 0;
  TM = TM-1;
  sinr_eff = sinr_dB[frb]; //the single sinr_eff value we calculated with MMSE FDE formula in init_snr_up function
  
   
  sinr_eff *= 10;
  sinr_eff = floor(sinr_eff);
  sinr_eff /= 10;
  
  LOG_D(PHY,"[ABSTRACTION] sinr_eff after rounding = %f\n",sinr_eff);
  for (index = 0; index < 16; index++) {
    if(index == 0) {
      if (sinr_eff < sinr_bler_map_up[mcs][0][index]) {
        bler = 1;
        break;
      }
    }
    if (sinr_eff == sinr_bler_map_up[mcs][0][index]) {
        bler = sinr_bler_map_up[mcs][1][index];
    }
  }
#ifdef USER_MODE // need to be adapted for the emulation in the kernel space 
   if (uniformrandom() < bler) {
     LOG_I(OCM,"abstraction_decoding failed (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(0);
  }
  else {
    LOG_I(OCM,"abstraction_decoding successful (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(1);
  }
#endif
}







int ulsch_abstraction_MIESM(double* sinr_dB,uint8_t TM, uint8_t mcs,uint16_t nrb, uint16_t frb) {
  int index;
  double sinr_eff = 0;
  double sinr_db1 = 0;
  double sinr_db2 = 0;
  double SI=0;
  double RBIR=0;
  int rb_count = 0;
  int offset, M=0;
  double bler = 0;
  int start,middle,end;
  TM = TM-1;
  for (offset = frb; offset <= (frb + nrb -1); offset++) {

      rb_count++;
     
      //we need to do the table lookups here for the mutual information corresponding to the certain sinr_dB. 
      
      sinr_db1 = sinr_dB[offset*2];
      sinr_db2 = sinr_dB[offset*2+1];

      msg("sinr_db1=%f\n,sinr_db2=%f\n",sinr_db1,sinr_db2);

      //rounding up for the table lookup
      sinr_db1 *= 10;
      sinr_db2 *= 10;

      sinr_db1 = floor(sinr_db1);
      sinr_db2 = floor(sinr_db2);

      if ((int)sinr_db1%2) {
	sinr_db1 += 1;
      }
      if ((int)sinr_db2%2) {
	sinr_db2 += 1;
      }

      sinr_db1 /= 10;
      sinr_db2 /= 10;
      
      if(mcs<10){
	//for sinr_db1
	for (index = 0; index < 162; index++) {
	    if (sinr_db1 < MI_map_4qam[0][0]) {
	      SI += (MI_map_4qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	      M +=2;
	    break;
	    }
	     if (sinr_db1 > MI_map_4qam[0][161]) {
	       SI += (MI_map_4qam[1][161]/beta1_dlsch_MI[TM][mcs]);
	        M +=2;
	    break;
	    }
	  
	  if (sinr_db1 == MI_map_4qam[0][index]) {
	    SI += (MI_map_4qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	     M +=2;
	    break;
	  }
	}
      
      //for sinr_db2
	for (index = 0; index < 162; index++) {
	    if (sinr_db2 < MI_map_4qam[0][0]) {
	      SI += (MI_map_4qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	       M +=2;
	    break;
	    }
	     if (sinr_db2 > MI_map_4qam[0][161]) {
	       SI += (MI_map_4qam[1][161]/beta1_dlsch_MI[TM][mcs]);
	        M +=2;
	    break;
	    }
	  
	  if (sinr_db2 == MI_map_4qam[0][index]) {
	    SI += (MI_map_4qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	     M +=2;
	    break;
	  }
	}
	
      }
      else if(mcs>9 && mcs<17)
	{
	  //for sinr_db1
	  for (index = 0; index < 197; index++) {
	    if (sinr_db1 < MI_map_16qam[0][0]) {
	      SI += (MI_map_16qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	       M +=4;
	      break;
	    }
	    if (sinr_db1 > MI_map_16qam[0][196]) {
	      SI += (MI_map_16qam[1][196]/beta1_dlsch_MI[TM][mcs]);
	      M +=4;
	      break;
	    }
	    
	    if (sinr_db1 == MI_map_16qam[0][index]) {
	      SI += (MI_map_16qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	      M +=4;
	    break;
	  }
	  }
	  
	  //for sinr_db2
	  for (index = 0; index < 197; index++) {
	    if (sinr_db2 < MI_map_16qam[0][0]) {
	      SI += (MI_map_16qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	      M +=4;
	      break;
	    }
	    if (sinr_db2 > MI_map_16qam[0][196]) {
	      SI += (MI_map_16qam[1][196]/beta1_dlsch_MI[TM][mcs]);
	      M +=4;
	      break;
	    }
	    
	    if (sinr_db2 == MI_map_16qam[0][index]) {
	      SI += (MI_map_16qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	      M +=4;
	    break;
	    }
	  }
	  
	}
      else if(mcs>16 && mcs<22)
	{
	  	//for sinr_db1
	for (index = 0; index < 227; index++) {
	    if (sinr_db1 < MI_map_64qam[0][0]) {
	      SI += (MI_map_64qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	      M +=6;
	    break;
	    }
	     if (sinr_db1 > MI_map_64qam[0][226]) {
	       SI += (MI_map_64qam[1][226]/beta1_dlsch_MI[TM][mcs]);
	       M +=6;
	    break;
	    }
	  
	     if (sinr_db1 == MI_map_64qam[0][index]) {
	       SI += (MI_map_64qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	       M +=6;
	       break;
	     }
	}
	
	//for sinr_db2
	for (index = 0; index < 227; index++) {
	  if (sinr_db2 < MI_map_64qam[0][0]) {
	    SI += (MI_map_64qam[1][0]/beta1_dlsch_MI[TM][mcs]);
	    M +=6;
	    break;
	  }
	  if (sinr_db2 > MI_map_64qam[0][226]) {
	    SI += (MI_map_64qam[1][226]/beta1_dlsch_MI[TM][mcs]);
	    M +=6;
	    break;
	  }
	  
	  if (sinr_db2 == MI_map_64qam[0][index]) {
	    SI += (MI_map_64qam[1][index]/beta1_dlsch_MI[TM][mcs]);
	    M +=6;
	    break;
	  }
	}
	}
    }
 // }

  RBIR = SI/M;
  
  //Now RBIR->SINR_effective Mapping
  //binary search method is performed here
  if(mcs<10){
    start = 0;
    end = 161;
    middle = end/2;
    if (RBIR <= MI_map_4qam[2][start])
      {
      sinr_eff =  MI_map_4qam[0][start];
      }
    else
      {
      if (RBIR >= MI_map_4qam[2][end])
	sinr_eff =  MI_map_4qam[0][end];
      else
	{//while((end-start > 1) && (RBIR >= MI_map_4qam[2])) 
	if (RBIR < MI_map_4qam[2][middle]){
	  end = middle;
	  middle = end/2;
	}
	else{ 
	    start = middle;
	  middle = (end-middle)/2;
	}
	}
    for (; end>start; end--){
      if ((RBIR < MI_map_4qam[2][end]) && (RBIR >  MI_map_4qam[2][end-2])){
      sinr_eff = MI_map_4qam[0][end-1];
      break;
      }
    }
      }
    sinr_eff = sinr_eff * beta2_dlsch_MI[TM][mcs]; 
  }


  
  else
    if (mcs>9 && mcs<17)
      {
	
	start = 0;
	end = 196;
	middle = end/2;
	if (RBIR <= MI_map_16qam[2][start])
	  {
	  sinr_eff =  MI_map_16qam[0][start];
	  }
	else
	  {
	  if (RBIR >= MI_map_16qam[2][end])
	    sinr_eff =  MI_map_16qam[0][end];
	  else
	    {
	//while((end-start > 1) && (RBIR >= MI_map_4qam[2])) 
	if (RBIR < MI_map_16qam[2][middle]){
	  end = middle;
	  middle = end/2;
	}
	else{ 
	  start = middle;
	  middle = (end-middle)/2;
	}
	    }
	for (; end>start; end--){
	  if ((RBIR < MI_map_16qam[2][end]) && (RBIR >  MI_map_16qam[2][end-2])){
	    sinr_eff = MI_map_16qam[0][end-1];
	    break;
	  }
	}
      }
	sinr_eff = sinr_eff * beta2_dlsch_MI[TM][mcs];
      } 
    else
      if (mcs>16)
	{
	  start = 0;
	  end = 226;
    middle = end/2;
    if (RBIR <= MI_map_64qam[2][start])
      {
      sinr_eff =  MI_map_64qam[0][start];
      }
    else
      {
      if (RBIR >= MI_map_64qam[2][end])
	sinr_eff =  MI_map_64qam[0][end];
      else
	{
	//while((end-start > 1) && (RBIR >= MI_map_4qam[2])) 
	if (RBIR < MI_map_64qam[2][middle]){
	  end = middle;
	  middle = end/2;
	}
	else{ 
	  start = middle;
	  middle = (end-middle)/2;
	}
	}
    for (; end>start; end--){
      if ((RBIR < MI_map_64qam[2][end]) && (RBIR >  MI_map_64qam[2][end-2])){
	sinr_eff = MI_map_64qam[0][end-1];
      break;
      }
    } 
      }
    sinr_eff = sinr_eff * beta2_dlsch_MI[TM][mcs]; 
	}

  msg("SINR_Eff = %e\n",sinr_eff);

 sinr_eff *= 10;
  sinr_eff = floor(sinr_eff);
  // if ((int)sinr_eff%2) {
  //   sinr_eff += 1;
  // }
  sinr_eff /= 10;
  msg("sinr_eff after rounding = %f\n",sinr_eff);

   for (index = 0; index < 16; index++) {
    if(index == 0) {
      if (sinr_eff < sinr_bler_map_up[mcs][0][index]) {
        bler = 1;
        break;
      }
    }
    if (sinr_eff == sinr_bler_map_up[mcs][0][index]) {
        bler = sinr_bler_map_up[mcs][1][index];
    }
   }
   
#ifdef USER_MODE // need to be adapted for the emulation in the kernel space 
   if (uniformrandom() < bler) {
    msg("abstraction_decoding failed (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(0);
  }
  else {
    msg("abstraction_decoding successful (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(1);
  }
#endif

}

#endif

uint32_t ulsch_decoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			     uint8_t sched_subframe,
			     uint8_t UE_index,
			     uint16_t *crnti) {

  uint8_t UE_id;
  uint16_t rnti;
  int subframe = phy_vars_eNB->proc[sched_subframe].subframe_rx;
  uint8_t harq_pid;
  uint8_t CC_id = phy_vars_eNB->CC_id;

  harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,phy_vars_eNB->proc[sched_subframe].frame_rx,subframe);
  
  rnti = phy_vars_eNB->ulsch_eNB[UE_index]->rnti;
#ifdef DEBUG_PHY
  LOG_D(PHY,"[eNB %d] ulsch_decoding_emul : subframe %d UE_index %d harq_pid %d rnti %x\n",phy_vars_eNB->Mod_id,subframe,UE_index,harq_pid,rnti);
#endif
  for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
    if (rnti == PHY_vars_UE_g[UE_id][CC_id]->lte_ue_pdcch_vars[0]->crnti)
      break;
    /*
    msg("[PHY] EMUL eNB %d ulsch_decoding_emul : subframe ue id %d crnti %x nb ue %d\n",
	phy_vars_eNB->Mod_id,
	UE_id,
	PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti,
	NB_UE_INST); 
    */
  }

  if (UE_id==NB_UE_INST) {
    LOG_W(PHY,"[eNB %d] ulsch_decoding_emul: FATAL, didn't find UE with rnti %x (UE index %d)\n",
	  phy_vars_eNB->Mod_id, rnti, UE_index);
    return(1+phy_vars_eNB->ulsch_eNB[UE_id]->max_turbo_iterations);
  }
  else {
    LOG_D(PHY,"[eNB %d] Found UE with rnti %x => UE_id %d\n",phy_vars_eNB->Mod_id, rnti, UE_id);
  }

  if (PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->harq_processes[harq_pid]->status == CBA_ACTIVE){
    *crnti = rnti;
    PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->harq_processes[harq_pid]->status=IDLE;
  } else 
    *crnti = 0x0;
  
  // Do abstraction here to determine if packet it in error
 /* if (ulsch_abstraction_MIESM(phy_vars_eNB->sinr_dB_eNB,1, phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs,phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb, phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb) == 1) 
  flag = 1;
  else flag = 0;*/
  

  /*
 //SINRdbPost = phy_vars_eNB->sinr_dB_eNB;
 mcsPost = phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs,
 nrbPost = phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb;
 frbPost = phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb; 
 

 if(nrbPost > 0)
 {
 SINRdbPost = phy_vars_eNB->sinr_dB_eNB;
 ULflag1 = 1;
}
 else
 {
	 SINRdbPost = NULL  ;
	 ULflag1 = 0 ;
}*/

  //
  // write_output("postprocSINR.m","SINReNB",phy_vars_eNB->sinr_dB,301,1,7);
 
 
 //Yazdir buraya her frame icin 300 eNb 
 // fprintf(SINRrx,"%e,%e,%e,%e;\n",SINRdbPost);
 //fprintf(SINRrx,"%e\n",SINRdbPost);
 
 // fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
  
 // if (ulsch_abstraction(phy_vars_eNB->sinr_dB,1, phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs,phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb, phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->first_rb) == 1) {
  if (1) {
    LOG_D(PHY,"ulsch_decoding_emul abstraction successful\n");

    memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->harq_processes[harq_pid]->b,
	   PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->harq_processes[harq_pid]->b,
	   phy_vars_eNB->ulsch_eNB[UE_index]->harq_processes[harq_pid]->TBS>>3);
    // get local ue's ack 	 
    if ((UE_index >= oai_emulation.info.first_ue_local) ||(UE_index <(oai_emulation.info.first_ue_local+oai_emulation.info.nb_ue_local))){
      get_ack(&phy_vars_eNB->lte_frame_parms,
	      PHY_vars_UE_g[UE_id][CC_id]->dlsch_ue[0][0]->harq_ack,
	      subframe,
	      phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK);
    }else { // get remote UEs' ack 
      phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[0] = PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->o_ACK[0];
      phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[1] = PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->o_ACK[1];
    }

    // Do abstraction of PUSCH feedback
#ifdef DEBUG_PHY    
    LOG_D(PHY,"[eNB %d][EMUL] ue index %d UE_id %d: subframe %d : o_ACK (%d %d), cqi (val %d, len %d)\n",
	  phy_vars_eNB->Mod_id,UE_index, UE_id, subframe,phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[0],
	  phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[1],
	  ((HLC_subband_cqi_rank1_2A_5MHz *)PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->o)->cqi1,
	  PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->O); 
#endif 

    phy_vars_eNB->ulsch_eNB[UE_index]->Or1 = PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->O;
    phy_vars_eNB->ulsch_eNB[UE_index]->Or2 = PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->O;
   
    phy_vars_eNB->ulsch_eNB[UE_index]->uci_format = PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->uci_format;    
    memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->o,PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->o,MAX_CQI_BYTES); 
    memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->o_RI,PHY_vars_UE_g[UE_id][CC_id]->ulsch_ue[0]->o_RI,2); 

    phy_vars_eNB->ulsch_eNB[UE_index]->cqi_crc_status = 1;
    
    return(1);   
  }
  else {
    LOG_W(PHY,"[eNB %d] ulsch_decoding_emul abstraction failed for UE %d\n",phy_vars_eNB->Mod_id,UE_index);

    phy_vars_eNB->ulsch_eNB[UE_index]->cqi_crc_status = 0;

    // retransmission
    return(1+phy_vars_eNB->ulsch_eNB[UE_index]->max_turbo_iterations);
  }

}
#endif
