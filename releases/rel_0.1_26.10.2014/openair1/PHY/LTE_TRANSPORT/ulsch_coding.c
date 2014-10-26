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

/*! \file PHY/LTE_TRANSPORT/ulsch_coding.c
* \brief Top-level routines for coding the ULSCH transport channel as described in 36.212 V8.6 2009-03
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
#include "extern.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"
#include "UTIL/LOG/vcd_signal_dumper.h"

//#define DEBUG_ULSCH_CODING 
//#define DEBUG_ULSCH_FREE 1

/*
#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \ 
	((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
	((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9)))) \
*/
#define is_not_pilot(pilots,first_pilot,re) (1)




void free_ue_ulsch(LTE_UE_ULSCH_t *ulsch) {
  int i;
  int r;

  if (ulsch) {
#ifdef DEBUG_ULSCH_FREE
    msg("Freeing ulsch %p\n",ulsch);
#endif
    for (i=0;i<ulsch->Mdlharq;i++) {
#ifdef DEBUG_ULSCH_FREE
      msg("Freeing ulsch process %d\n",i);
#endif
      if (ulsch->harq_processes[i]) {
#ifdef DEBUG_ULSCH_FREE
	msg("Freeing ulsch process %d (%p)\n",i,ulsch->harq_processes[i]);
#endif
	if (ulsch->harq_processes[i]->b) {
	  free16(ulsch->harq_processes[i]->b,MAX_ULSCH_PAYLOAD_BYTES);
	  ulsch->harq_processes[i]->b = NULL;
#ifdef DEBUG_ULSCH_FREE
	  msg("Freeing ulsch process %d b (%p)\n",i,ulsch->harq_processes[i]->b);
#endif
	}
	if (ulsch->harq_processes[i]->c) {
#ifdef DEBUG_ULSCH_FREE
	  msg("Freeing ulsch process %d c (%p)\n",i,ulsch->harq_processes[i]->c);
#endif
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {

#ifdef DEBUG_ULSCH_FREE
	    msg("Freeing ulsch process %d c[%d] (%p)\n",i,r,ulsch->harq_processes[i]->c[r]);
#endif
	    if (ulsch->harq_processes[i]->c[r]) {
	      free16(ulsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+768);
	      ulsch->harq_processes[i]->c[r] = NULL;
	    }
	  }
	}
	free16(ulsch->harq_processes[i],sizeof(LTE_UL_UE_HARQ_t));
	ulsch->harq_processes[i] = NULL;
      }
    }
    free16(ulsch,sizeof(LTE_UE_ULSCH_t));
    ulsch = NULL;
  }
  
}

LTE_UE_ULSCH_t *new_ue_ulsch(unsigned char Mdlharq,unsigned char N_RB_UL, uint8_t abstraction_flag) {

  LTE_UE_ULSCH_t *ulsch;
  unsigned char exit_flag = 0,i,j,r;
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
  ulsch = (LTE_UE_ULSCH_t *)malloc16(sizeof(LTE_UE_ULSCH_t));
  if (ulsch) {
    memset(ulsch,0,sizeof(LTE_UE_ULSCH_t));
    ulsch->Mdlharq = Mdlharq;
    for (i=0;i<Mdlharq;i++) {

      ulsch->harq_processes[i] = (LTE_UL_UE_HARQ_t *)malloc16(sizeof(LTE_UL_UE_HARQ_t));
      //      printf("ulsch->harq_processes[%d] %p\n",i,ulsch->harq_processes[i]);
      if (ulsch->harq_processes[i]) {
	memset(ulsch->harq_processes[i], 0, sizeof(LTE_UL_UE_HARQ_t));
	ulsch->harq_processes[i]->b = (unsigned char*)malloc16(MAX_ULSCH_PAYLOAD_BYTES/bw_scaling);
	if (ulsch->harq_processes[i]->b) 
	  memset(ulsch->harq_processes[i]->b,0,MAX_ULSCH_PAYLOAD_BYTES/bw_scaling);
	else {
	  LOG_E(PHY,"Can't get b\n");
	  exit_flag=1;
	}
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {
	    ulsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 3+768);  // account for filler in first segment and CRCs for multiple segment case
	    if (ulsch->harq_processes[i]->c[r]) 
	      memset(ulsch->harq_processes[i]->c[r],0,((r==0)?8:0) + 3+768);
	    else {
	      LOG_E(PHY,"Can't get c\n");
	      exit_flag=2;
	    }
	  }
	}
	ulsch->harq_processes[i]->subframe_scheduling_flag = 0;
	ulsch->harq_processes[i]->first_tx = 1;
      }	else {
	LOG_E(PHY,"Can't get harq_p %d\n",i);
	exit_flag=3;
      }
    }

    if ((abstraction_flag == 0) && (exit_flag==0)) {
      for (i=0;i<Mdlharq;i++)
	for (j=0;j<96;j++)
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	    ulsch->harq_processes[i]->d[r][j] = LTE_NULL;
      return(ulsch);
    }
    else if (abstraction_flag==1)
      return(ulsch);
  }
  LOG_E(PHY,"new_ue_ulsch exit flag, size of  %d ,   %d\n",exit_flag, sizeof(LTE_UE_ULSCH_t));
  free_ue_ulsch(ulsch);
  return(NULL);
  
  
}


uint32_t ulsch_encoding(uint8_t *a,
		   PHY_VARS_UE *phy_vars_ue,
		   uint8_t harq_pid,
		   uint8_t eNB_id,
		   uint8_t tmode,
		   uint8_t control_only_flag,
		   uint8_t Nbundled) {

  time_stats_t *seg_stats=&phy_vars_ue->ulsch_segmentation_stats;
  time_stats_t *rm_stats=&phy_vars_ue->ulsch_rate_matching_stats;
  time_stats_t *te_stats=&phy_vars_ue->ulsch_turbo_encoding_stats;
  time_stats_t *i_stats=&phy_vars_ue->ulsch_interleaving_stats;
  time_stats_t *m_stats=&phy_vars_ue->ulsch_multiplexing_stats;
  
  //  uint16_t offset;
  uint32_t crc=1;
  uint16_t iind;
  uint32_t A;
  uint8_t Q_m=0;
  uint32_t Kr=0,Kr_bytes,r,r_offset=0;
  uint8_t y[6*14*1200],*yptr;;
  uint8_t *columnset;
  uint32_t sumKr=0;
  uint32_t Qprime,L,G,Q_CQI=0,Q_RI=0,Q_ACK=0,H=0,Hprime=0,Hpp=0,Cmux=0,Rmux=0,Rmux_prime=0;
  uint32_t Qprime_ACK=0,Qprime_CQI=0,Qprime_RI=0,len_ACK=0,len_RI=0;
  //  uint32_t E;
  uint8_t ack_parity;
  uint32_t i,q,j,iprime,j2;
  uint16_t o_RCC;
  uint8_t o_flip[8];
  uint32_t wACK_idx;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  PHY_MEASUREMENTS *meas = &phy_vars_ue->PHY_measurements;
  LTE_UE_ULSCH_t *ulsch=phy_vars_ue->ulsch_ue[eNB_id];
  LTE_UE_DLSCH_t **dlsch = phy_vars_ue->dlsch_ue[eNB_id];
  double sinr_eff;
  uint16_t rnti;

  if (!ulsch) {
    LOG_E(PHY,"Null ulsch ptr %p\n",ulsch);
    return(-1);
  }

  if (harq_pid > 7) {
    LOG_E(PHY,"Illegal harq_pid %d\n",harq_pid);
    return(-1);
  }
    
  if (ulsch->harq_processes[harq_pid]->O_ACK > 2)
    {
      LOG_E(PHY,"Illegal O_ACK %d\n",ulsch->harq_processes[harq_pid]->O_ACK);
      return(-1);
    }

  if (ulsch->O_RI > 1)
    {
      LOG_E(PHY,"Illegal O_RI %d\n",ulsch->O_RI);
    return(-1);
  }

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_IN);

  // fill CQI/PMI information
  if (ulsch->O>0) {
    /*  //not correctly done
    if (flag_LA==1)
      sinr_eff = sinr_eff_cqi_calc(phy_vars_ue, 0); //eNB_id is missing here
    else
      sinr_eff = 0;
    */
    rnti = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti;
    fill_CQI(ulsch->o,ulsch->uci_format,meas,0,rnti, tmode,phy_vars_ue->sinr_eff);
   
    LOG_D(PHY,"UE CQI\n");
    print_CQI(ulsch->o,ulsch->uci_format,0);

    // save PUSCH pmi for later (transmission modes 4,5,6)
    if (dlsch[0]) {
      //LOG_I(PHY,"XXX saving pmi for DL %x\n",pmi2hex_2Ar1(((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi));
      dlsch[0]->pmi_alloc = ((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi;
    }
  }

  if (ulsch->O<=32) {
    o_flip[0] = ulsch->o[3];
    o_flip[1] = ulsch->o[2];
    o_flip[2] = ulsch->o[1];
    o_flip[3] = ulsch->o[0];   
  }
  else {
    o_flip[0] = ulsch->o[7];
    o_flip[1] = ulsch->o[6];
    o_flip[2] = ulsch->o[5];
    o_flip[3] = ulsch->o[4];
    o_flip[4] = ulsch->o[3];
    o_flip[5] = ulsch->o[2];
    o_flip[6] = ulsch->o[1];
    o_flip[7] = ulsch->o[0];
  }
  if (control_only_flag == 0) {
    A=ulsch->harq_processes[harq_pid]->TBS;
    Q_m = get_Qm_ul(ulsch->harq_processes[harq_pid]->mcs);

    ulsch->harq_processes[harq_pid]->control_only = 0;
    
#ifdef DEBUG_ULSCH_CODING
  msg("[PHY][UE] ULSCH coding : A %d, Qm %d, mcs %d, harq_pid %d, round %d, RV %d\n",
      ulsch->harq_processes[harq_pid]->TBS,
      Q_m,
      ulsch->harq_processes[harq_pid]->mcs,
      harq_pid,
      ulsch->harq_processes[harq_pid]->round,
      ulsch->harq_processes[harq_pid]->rvidx);

  for (i=0;i<ulsch->harq_processes[harq_pid]->O_ACK;i++)
    msg("ulsch_coding: o_ACK[%d] %d\n",i,ulsch->o_ACK[i]);
  for (i=0;i<ulsch->O_RI;i++)
    msg("ulsch_coding: o_RI[%d] %d\n",i,ulsch->o_RI[i]);
  msg("ulsch_coding: O=%d\n",ulsch->O);
  
  for (i=0;i<1+((8+ulsch->O)/8);i++) {
    //    ulsch->o[i] = i;
    msg("ulsch_coding: O[%d] %d\n",i,ulsch->o[i]);
  }
  if ((tmode != 4))
    print_CQI(ulsch->o,wideband_cqi_rank1_2A,0);
  else
    print_CQI(ulsch->o,HLC_subband_cqi_rank1_2A,0);
#endif
    
    if (ulsch->harq_processes[harq_pid]->round == 0) {  // this is a new packet
      
      start_meas(seg_stats);
      // Add 24-bit crc (polynomial A) to payload
      crc = crc24a(a,
		   A)>>8;
      
      a[A>>3] = ((uint8_t*)&crc)[2];
      a[1+(A>>3)] = ((uint8_t*)&crc)[1];
      a[2+(A>>3)] = ((uint8_t*)&crc)[0];
      
      ulsch->harq_processes[harq_pid]->B = A+24;
      ulsch->harq_processes[harq_pid]->b = a;
      lte_segmentation(ulsch->harq_processes[harq_pid]->b,
		       ulsch->harq_processes[harq_pid]->c,
		       ulsch->harq_processes[harq_pid]->B,
		       &ulsch->harq_processes[harq_pid]->C,
		       &ulsch->harq_processes[harq_pid]->Cplus,
		       &ulsch->harq_processes[harq_pid]->Cminus,
		       &ulsch->harq_processes[harq_pid]->Kplus,
		       &ulsch->harq_processes[harq_pid]->Kminus,		     
		       &ulsch->harq_processes[harq_pid]->F);

      stop_meas(seg_stats);
      
      for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
	if (r<ulsch->harq_processes[harq_pid]->Cminus)
	  Kr = ulsch->harq_processes[harq_pid]->Kminus;
	else
	  Kr = ulsch->harq_processes[harq_pid]->Kplus;
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
	  LOG_E(PHY,"ulsch_coding: Illegal codeword size %d!!!\n",Kr_bytes);
	  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
	  return(-1);
	}
	
	
#ifdef DEBUG_ULSCH_CODING
	msg("Generating Code Segment %d (%d bits)\n",r,Kr);
	// generate codewords
	
	msg("bits_per_codeword (Kr)= %d\n",Kr);
	msg("N_RB = %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
	msg("Ncp %d\n",frame_parms->Ncp);
	msg("Qm %d\n",Q_m);
#endif
	
	//	offset=0;
	
	
#ifdef DEBUG_ULSCH_CODING    
	msg("Encoding ... iind %d f1 %d, f2 %d\n",iind,f1f2mat_old[iind*2],f1f2mat_old[(iind*2)+1]);
#endif
	start_meas(te_stats);
	threegpplte_turbo_encoder(ulsch->harq_processes[harq_pid]->c[r],
				  Kr>>3, 
				  &ulsch->harq_processes[harq_pid]->d[r][96],
				  (r==0) ? ulsch->harq_processes[harq_pid]->F : 0,
				  f1f2mat_old[iind*2],   // f1 (see 36212-820, page 14)
				  f1f2mat_old[(iind*2)+1]  // f2 (see 36212-820, page 14)
				  );
	stop_meas(te_stats);
#ifdef DEBUG_ULSCH_CODING
	if (r==0)
	  write_output("enc_output0.m","enc0",&ulsch->harq_processes[harq_pid]->d[r][96],(3*8*Kr_bytes)+12,1,4);
#endif
	start_meas(i_stats);
	ulsch->harq_processes[harq_pid]->RTC[r] = 
	  sub_block_interleaving_turbo(4+(Kr_bytes*8), 
				       &ulsch->harq_processes[harq_pid]->d[r][96], 
				       ulsch->harq_processes[harq_pid]->w[r]);
	stop_meas(i_stats);
      }
      
    }
    
    if (ulsch->harq_processes[harq_pid]->C == 0) {
      LOG_E(PHY,"null segment\n");
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
      return(-1);
    }
    
    sumKr = 0;
    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
      if (r<ulsch->harq_processes[harq_pid]->Cminus)
	Kr = ulsch->harq_processes[harq_pid]->Kminus;
      else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
      sumKr += Kr;
    }
  }
  else { // This is a control-only PUSCH, set sumKr to O_CQI-MIN
    ulsch->harq_processes[harq_pid]->control_only = 1;
    sumKr = ulsch->O_CQI_MIN;
  }
  ulsch->harq_processes[harq_pid]->sumKr = sumKr;
  // Compute Q_ri (p. 23 36-212)

  Qprime = ulsch->O_RI*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_ri_times8;
  if (Qprime > 0) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
    
    if (Qprime > 4*ulsch->harq_processes[harq_pid]->nb_rb * 12)
      Qprime = 4*ulsch->harq_processes[harq_pid]->nb_rb * 12;
  }

  Q_RI = Q_m*Qprime;
  Qprime_RI = Qprime;

  // Compute Q_ack (p. 23 36-212)
  Qprime = ulsch->harq_processes[harq_pid]->O_ACK*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_harqack_times8;
  if (Qprime > 0) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
    
    if (Qprime > 4*ulsch->harq_processes[harq_pid]->nb_rb * 12)
      Qprime = 4*ulsch->harq_processes[harq_pid]->nb_rb * 12;
  }

  Q_ACK = Qprime * Q_m;
  Qprime_ACK = Qprime;

  // Compute Q_cqi, assume O>11, p. 26 36-212
  if (control_only_flag == 0) {
    
    if (ulsch->O < 12)
      L=0;
    else 
      L=8;
    if (ulsch->O > 0)
      Qprime = (ulsch->O + L) * ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
    else
      Qprime = 0;
    if (Qprime > 0) {
      if ((Qprime % (8*sumKr)) > 0)
	Qprime = 1+(Qprime/(8*sumKr));
      else
	Qprime = Qprime/(8*sumKr);
    }
    
    G = ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);

    if (Qprime > (G - ulsch->O_RI))
      Qprime = G - ulsch->O_RI;
    Q_CQI = Q_m * Qprime;
    Qprime_CQI = Qprime;


  
    G = G - Q_RI - Q_CQI;
    
    if ((int)G < 0) {
      LOG_E(PHY,"FATAL: ulsch_coding.c G < 0 (%d) : Q_RI %d, Q_CQI %d, O %d, betaCQI_times8 %d)\n",G,Q_RI,Q_CQI,ulsch->O,ulsch->beta_offset_cqi_times8);
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);      
      return(-1);
    }


    // Data and control multiplexing (5.2.2.7 36-212)

    H = G + Q_CQI;
    Hprime = H/Q_m;



    // Fill in the "e"-sequence from 36-212, V8.6 2009-03, p. 16-17 (for each "e") and concatenate the
    // outputs for each code segment, see Section 5.1.5 p.20
    
    for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
#ifdef DEBUG_ULSCH_CODING
      msg("Rate Matching, Code segment %d (coded bits (G) %d,unpunctured/repeated bits per code segment %d,mod_order %d, nb_rb %d)...\n",
	  r,
	  G,
	  Kr*3,
	  Q_m,ulsch->harq_processes[harq_pid]->nb_rb);
#endif
      
      start_meas(rm_stats);      
      r_offset += lte_rate_matching_turbo(ulsch->harq_processes[harq_pid]->RTC[r],
					  G,
					  ulsch->harq_processes[harq_pid]->w[r],
					  ulsch->e+r_offset,
					  ulsch->harq_processes[harq_pid]->C, // C
					  NSOFT,                    // Nsoft,
					  ulsch->Mdlharq,
					  1,
					  ulsch->harq_processes[harq_pid]->rvidx,
					  get_Qm_ul(ulsch->harq_processes[harq_pid]->mcs),
					  1,
					  r,
					  ulsch->harq_processes[harq_pid]->nb_rb,
					  ulsch->harq_processes[harq_pid]->mcs);                       // r
      stop_meas(rm_stats);
#ifdef DEBUG_ULSCH_CODING
      if (r==ulsch->harq_processes[harq_pid]->C-1)
	write_output("enc_output.m","enc",ulsch->e,r_offset,1,4);
#endif
    }
  }
  else {  //control-only PUSCH
    Q_CQI = (ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch)) - Q_RI;
    H = Q_CQI;
    Hprime = H/Q_m;
  }


  //  Do CQI coding
  if ((ulsch->O>1) && (ulsch->O < 12)) {
    LOG_E(PHY,"short CQI sizes not supported yet\n");
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
    return(-1);
  }
  else {
    // add 8-bit CRC
    crc = crc8(o_flip,
	       ulsch->O)>>24;
#ifdef DEBUG_ULSCH_CODING
    printf("crc(cqi) tx : %x\n",crc);
#endif
    memset((void *)&ulsch->o_d[0],LTE_NULL,96);
    
    ccodelte_encode(ulsch->O,
		    1,
		    o_flip,
		    &ulsch->o_d[96],
		    0);


    o_RCC = sub_block_interleaving_cc(8+ulsch->O, 
				      &ulsch->o_d[96], 
				      ulsch->o_w);

    lte_rate_matching_cc(o_RCC,
			 Q_CQI,
			 ulsch->o_w,
			 ulsch->q);
    
  }

  i=0;
  //  Do RI coding
  if (ulsch->O_RI == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = PUSCH_y;//ulsch->o_RI[0];
      len_RI=2;
      break;
    case 4:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = PUSCH_y;//1;
      ulsch->q_RI[2] = PUSCH_x;//ulsch->o_RI[0];
      ulsch->q_RI[3] = PUSCH_x;//1;
      len_RI=4;
      break;
    case 6:
      ulsch->q_RI[0] = ulsch->o_RI[0];
      ulsch->q_RI[1] = PUSCH_y;//1;
      ulsch->q_RI[2] = PUSCH_x;//1;
      ulsch->q_RI[3] = PUSCH_x;//ulsch->o_RI[0];
      ulsch->q_RI[4] = PUSCH_x;//1;
      ulsch->q_RI[5] = PUSCH_x;//1;
      len_RI=6;
      break;
    }
  }
  else if (ulsch->O_RI>1){
    LOG_E(PHY,"RI cannot be more than 1 bit yet\n");
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
    return(-1);
  }
  //  Do ACK coding, Section 5.2.2.6 36.213 (p.23-24 in v8.6)
  wACK_idx = (ulsch->bundling==0) ? 4 : ((Nbundled-1)&3);
#ifdef DEBUG_ULSCH_CODING
  msg("ulsch_coding.c: Bundling %d, Nbundled %d, wACK_idx %d\n",
      ulsch->bundling,Nbundled,wACK_idx);
#endif
  // 1-bit ACK/NAK
  if (ulsch->harq_processes[harq_pid]->O_ACK == 1) {
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : ((ulsch->o_ACK[0]+wACK[wACK_idx][1])&1);//ulsch->o_ACK[0];
      len_ACK = 2;
      break;
    case 4:
      ulsch->q_ACK[0] = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : ((ulsch->o_ACK[0]+wACK[wACK_idx][1])&1);
      ulsch->q_ACK[2] = PUSCH_x;
      ulsch->q_ACK[3] = PUSCH_x;
      len_ACK = 4;
      break;
    case 6:
      ulsch->q_ACK[0] = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1] = (ulsch->bundling==0)? PUSCH_y : ((ulsch->o_ACK[0]+wACK[wACK_idx][1])&1);
      ulsch->q_ACK[2] = PUSCH_x;
      ulsch->q_ACK[3] = PUSCH_x;
      ulsch->q_ACK[4] = PUSCH_x;
      ulsch->q_ACK[6] = PUSCH_x;
      len_ACK = 6;
      break;
    }
  }
  // two-bit ACK/NAK
  if (ulsch->harq_processes[harq_pid]->O_ACK == 2) {
    ack_parity = (ulsch->o_ACK[0]+ulsch->o_ACK[1])&1;
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1] = (ulsch->o_ACK[1]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[2] = (ack_parity+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[3] = (ulsch->o_ACK[0]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[4] = (ulsch->o_ACK[1]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[5] = (ack_parity+wACK[wACK_idx][1])&1;
      len_ACK = 6;
      break;
    case 4:
      ulsch->q_ACK[0]  = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1]  = (ulsch->o_ACK[1]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[2]  = PUSCH_x;
      ulsch->q_ACK[3]  = PUSCH_x;//1;
      ulsch->q_ACK[4]  = (ack_parity+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[5]  = (ulsch->o_ACK[0]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[6]  = PUSCH_x;
      ulsch->q_ACK[7]  = PUSCH_x;//1;
      ulsch->q_ACK[8]  = (ulsch->o_ACK[1]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[9]  = (ack_parity+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[10] = PUSCH_x;
      ulsch->q_ACK[11] = PUSCH_x;//1;
      len_ACK = 12;
      break;
    case 6:
      ulsch->q_ACK[0] = (ulsch->o_ACK[0]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[1] = (ulsch->o_ACK[1]+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[2] = PUSCH_x;
      ulsch->q_ACK[3] = PUSCH_x;
      ulsch->q_ACK[4] = PUSCH_x;
      ulsch->q_ACK[5] = PUSCH_x;

      ulsch->q_ACK[6] = (ack_parity+wACK[wACK_idx][0])&1;
      ulsch->q_ACK[7] = (ulsch->o_ACK[0]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[8] = PUSCH_x;
      ulsch->q_ACK[9] = PUSCH_x; 
      ulsch->q_ACK[10] = PUSCH_x;
      ulsch->q_ACK[11] = PUSCH_x;

      ulsch->q_ACK[12] = (ulsch->o_ACK[1]+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[13] = (ack_parity+wACK[wACK_idx][1])&1;
      ulsch->q_ACK[14] = PUSCH_x;
      ulsch->q_ACK[15] = PUSCH_x;
      ulsch->q_ACK[16] = PUSCH_x;
      ulsch->q_ACK[17] = PUSCH_x;
      len_ACK = 18;

      break;
    }
  }
  if (ulsch->harq_processes[harq_pid]->O_ACK > 2) {
    LOG_E(PHY,"ACK cannot be more than 2 bits yet\n");
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
    return(-1);
  }


  // channel multiplexing/interleaving

  start_meas(m_stats);
  Hpp = Hprime + Q_RI;
 
  Cmux       = ulsch->Nsymb_pusch;
  Rmux       = Hpp*Q_m/Cmux;
  Rmux_prime = Rmux/Q_m;

  Qprime_RI  = Q_RI / Q_m;
  Qprime_ACK = Q_ACK / Q_m;
  Qprime_CQI = Q_CQI / Q_m;

  //  printf("Qprime_CQI = %d\n",Qprime_CQI);
  // RI BITS 

  memset(y,LTE_NULL,Q_m*Hpp);

  if (frame_parms->Ncp == 0)
    columnset = cs_ri_normal;
  else
    columnset = cs_ri_extended;
  j=0;   
  for (i=0;i<Qprime_RI;i++) {
    r = Rmux_prime - 1 - (i>>2);
    for (q=0;q<Q_m;q++)  {
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_RI[(q+(Q_m*i))%len_RI];
      //      printf("ri[%d] %d => y[%d]\n",q+(Q_m*i)%len_RI,ulsch->q_RI[(q+(Q_m*i))%len_RI],q+(Q_m*((r*Cmux) + columnset[j])),y[q+(Q_m*((r*Cmux) + columnset[j]))]);
    }
    j=(j+3)&3;

  }


  // CQI and Data bits
  j=0;
  /*
  for (i=0,iprime=-Qprime_CQI;i<Hprime;i++,iprime++) {

    while (y[Q_m*j] != LTE_NULL) j++;
    
    if (i<Qprime_CQI) {
      for (q=0;q<Q_m;q++) {
	y[q+(Q_m*j)] = ulsch->q[q+(Q_m*i)];
	//printf("cqi[%d] %d => y[%d]\n",q+(Q_m*i),ulsch->q[q+(Q_m*i)],q+(Q_m*j));
      }
    }
    else {
      for (q=0;q<Q_m;q++) {
	y[q+(Q_m*j)] = ulsch->e[q+(Q_m*iprime)];
	//	printf("e[%d] %d => y[%d]\n",q+(Q_m*iprime),ulsch->e[q+(Q_m*iprime)],q+(Q_m*j));
      }
    }
    j++;
  }
  */

  for (i=0;i<Qprime_CQI;i++) {

    while (y[Q_m*j] != LTE_NULL) j++;

    for (q=0;q<Q_m;q++) {
      y[q+(Q_m*j)] = ulsch->q[q+(Q_m*i)];
      //        printf("cqi[%d] %d => y[%d] (j %d)\n",q+(Q_m*i),ulsch->q[q+(Q_m*i)],q+(Q_m*j),j);
    }
  
    j++;
  }

  j2 = j*Q_m;
  switch (Q_m) {

  case 2:

    for (iprime=0;iprime<(Hprime-Qprime_CQI)<<1;iprime+=2) {
      while (y[j2] != LTE_NULL) j2+=2;
      y[j2]   = ulsch->e[iprime];
      y[1+j2] = ulsch->e[1+iprime];
      j2+=2;
    }
    break;

  case 4:
    for (iprime=0;iprime<(Hprime-Qprime_CQI)<<2;iprime+=4) {
      while (y[j2] != LTE_NULL) j2+=4;
      y[j2]   = ulsch->e[iprime];
      y[1+j2] = ulsch->e[1+iprime];
      y[2+j2] = ulsch->e[2+iprime];
      y[3+j2] = ulsch->e[3+iprime];
      j2+=4;
    }    
    break;

  case 6:
    for (iprime=0;iprime<(Hprime-Qprime_CQI)*6;iprime+=6) {
      while (y[j2] != LTE_NULL) j2+=6;
      y[j2]   = ulsch->e[iprime];
      y[1+j2] = ulsch->e[1+iprime];
      y[2+j2] = ulsch->e[2+iprime];
      y[3+j2] = ulsch->e[3+iprime];
      y[4+j2] = ulsch->e[4+iprime];
      y[5+j2] = ulsch->e[5+iprime];
      j2+=6;
    }        
    break;

  }
  // HARQ-ACK Bits (Note these overwrite some bits)

  if (frame_parms->Ncp == 0)
    columnset = cs_ack_normal;
  else
    columnset = cs_ack_extended;

  j=0;
  for (i=0;i<Qprime_ACK;i++) {
    r = Rmux_prime - 1 - (i>>2);
    for (q=0;q<Q_m;q++) {
      y[q+(Q_m*((r*Cmux) + columnset[j]))]  = ulsch->q_ACK[(q+(Q_m*i))%len_ACK];
#ifdef DEBUG_ULSCH_CODING
            msg("ulsch_coding.c: ACK %d => y[%d]=%d (i %d, r*Cmux %d, columnset %d)\n",q+(Q_m*i),
		q+(Q_m*((r*Cmux) + columnset[j])),ulsch->q_ACK[(q+(Q_m*i))%len_ACK],
		i,r*Cmux,columnset[j]);
#endif
    }
    j=(j+3)&3;

  }

  // write out buffer
  j=0;
  switch (Q_m) {
  case 2:
    for (i=0;i<Cmux;i++)
      for (r=0;r<Rmux_prime;r++) {
	yptr=&y[((r*Cmux)+i)<<1];
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
      }
    break;
  case 4:
    for (i=0;i<Cmux;i++)
      for (r=0;r<Rmux_prime;r++) {
	yptr = &y[((r*Cmux)+i)<<2];
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
      }
    break;
  case 6:
    for (i=0;i<Cmux;i++)
      for (r=0;r<Rmux_prime;r++) {
	yptr = &y[((r*Cmux)+i)*6];
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
	ulsch->h[j++] = *yptr++;
      }
    break;
  default:
    break;
  }
  stop_meas(m_stats);

  if (j!=(H+Q_RI)) {
    LOG_E(PHY,"Error in output buffer length (j %d, H+Q_RI %d)\n",j,H+Q_RI); 
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
    return(-1);
  }

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING, VCD_FUNCTION_OUT);
  return(0);
}


#ifdef PHY_ABSTRACTION
#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif
int ulsch_encoding_emul(uint8_t *ulsch_buffer,
			PHY_VARS_UE *phy_vars_ue,
			uint8_t eNB_id,
			uint8_t harq_pid,
			uint8_t control_only_flag) {

  LTE_UE_ULSCH_t *ulsch = phy_vars_ue->ulsch_ue[eNB_id];
  LTE_UE_DLSCH_t **dlsch = phy_vars_ue->dlsch_ue[eNB_id];
  PHY_MEASUREMENTS *meas = &phy_vars_ue->PHY_measurements;
  uint8_t tmode = phy_vars_ue->transmission_mode[eNB_id];
  double sinr_eff;  
  uint16_t rnti=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti;
  LOG_D(PHY,"EMUL UE ulsch_encoding for eNB %d,mod_id %d, harq_pid %d rnti %x, ACK(%d,%d) \n",
	eNB_id,phy_vars_ue->Mod_id, harq_pid, rnti,ulsch->o_ACK[0],ulsch->o_ACK[1]);

  if (ulsch->O>0) {
    /*
    if(flag_LA==1)
      sinr_eff = sinr_eff_cqi_calc(phy_vars_ue, eNB_id);
    else
      sinr_eff = meas->wideband_cqi_avg[eNB_id];
    */
   
    fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id,rnti,tmode,phy_vars_ue->sinr_eff);
       //LOG_D(PHY,"UE CQI\n");
    //    print_CQI(ulsch->o,ulsch->uci_format,eNB_id);

    // save PUSCH pmi for later (transmission modes 4,5,6)
    //    msg("ulsch: saving pmi for DL %x\n",pmi2hex_2Ar1(((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi));
    // if (ulsch->uci_format != HLC_subband_cqi_mcs_CBA)
    dlsch[0]->harq_processes[harq_pid]->pmi_alloc = ((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi;
  }

  memcpy(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->b,
	 ulsch_buffer,
	 phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);

     
  //memcpy(&UE_transport_info[phy_vars_ue->Mod_id].transport_blocks[UE_transport_info_TB_index[phy_vars_ue->Mod_id]],
  memcpy(&UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].transport_blocks,
	 ulsch_buffer,
	 phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);  
  //UE_transport_info_TB_index[phy_vars_ue->Mod_id]+=phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3;
  // navid: currently more than one eNB is not supported in the code 
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].num_eNB = 1; 
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].rnti[0] = phy_vars_ue->lte_ue_pdcch_vars[0]->crnti; 
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].eNB_id[0]  = eNB_id;
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].harq_pid[0] = harq_pid;
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].tbs[0]     = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3 ;
  // msg("\nphy_vars_ue->Mod_id%d\n",phy_vars_ue->Mod_id);
  
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.pusch_flag = 1;
  //UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_uci = *(uint32_t *)ulsch->o;
  memcpy(UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.pusch_uci,
	 ulsch->o,
	 MAX_CQI_BYTES); 
  // msg("[UE]cqi is %d \n", ((HLC_subband_cqi_rank1_2A_5MHz *)ulsch->o)->cqi1);
  
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.length_uci = ulsch->O;
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.uci_format = ulsch->uci_format;
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.pusch_ri = (ulsch->o_RI[0]&1)+((ulsch->o_RI[1]&1)<<1);
  UE_transport_info[phy_vars_ue->Mod_id][phy_vars_ue->CC_id].cntl.pusch_ack =   (ulsch->o_ACK[0]&1) + ((ulsch->o_ACK[1]&1)<<1);
  //msg("ack is %d %d %d\n",UE_transport_info[phy_vars_ue->Mod_id].cntl.pusch_ack, (ulsch->o_ACK[1]&1)<<1, ulsch->o_ACK[0]&1);
  return(0);
  
}
#endif
