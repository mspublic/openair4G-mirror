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

/*! \file PHY/LTE_TRANSPORT/dlsch_decoding.c
* \brief Top-level routines for decoding  Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
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
#include "SCHED/extern.h"
#include "SIMULATION/TOOLS/defs.h"
//#define DEBUG_DLSCH_DECODING


void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch) {

  int i,r;

  if (dlsch) {
    for (i=0;i<dlsch->Mdlharq;i++) {
      if (dlsch->harq_processes[i]) {
	if (dlsch->harq_processes[i]->b)
	  free16(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES);
	if (dlsch->harq_processes[i]->c) {
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	    free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+768);
	}
	for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++)
	  if (dlsch->harq_processes[i]->d[r])
	    free16(dlsch->harq_processes[i]->d[r],((3*8*6144)+12+96)*sizeof(short));
	free16(dlsch->harq_processes[i],sizeof(LTE_DL_UE_HARQ_t));
      }
    }
  free16(dlsch,sizeof(LTE_UE_DLSCH_t));
  }
}

LTE_UE_DLSCH_t *new_ue_dlsch(uint8_t Kmimo,uint8_t Mdlharq,uint8_t max_turbo_iterations,uint8_t N_RB_DL, uint8_t abstraction_flag) {

  LTE_UE_DLSCH_t *dlsch;
  uint8_t exit_flag = 0,i,r;
  
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
  dlsch = (LTE_UE_DLSCH_t *)malloc16(sizeof(LTE_UE_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    dlsch->max_turbo_iterations = max_turbo_iterations;

    for (i=0;i<Mdlharq;i++) {
      //      msg("new_ue_dlsch: Harq process %d\n",i);
      dlsch->harq_processes[i] = (LTE_DL_UE_HARQ_t *)malloc16(sizeof(LTE_DL_UE_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->b = (uint8_t*)malloc16(MAX_DLSCH_PAYLOAD_BYTES/bw_scaling);
	if (abstraction_flag == 0) {
	  if (!dlsch->harq_processes[i]->b)
	    exit_flag=3;
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS/bw_scaling;r++) {
	    dlsch->harq_processes[i]->c[r] = (uint8_t*)malloc16(((r==0)?8:0) + 3+ 768);	
	    if (!dlsch->harq_processes[i]->c[r])
	      exit_flag=2;
	    dlsch->harq_processes[i]->d[r] = (short*)malloc16(((3*8*6144)+12+96)*sizeof(short));
	  }
	}
      }	else {
	exit_flag=1;
      }
    }

    if (exit_flag==0)
      return(dlsch);
  }
  msg("new_ue_dlsch with size %d: exit_flag = %d\n",sizeof(LTE_DL_UE_HARQ_t), exit_flag);
  free_ue_dlsch(dlsch);

  return(NULL);
}

uint32_t  dlsch_decoding(PHY_VARS_UE *phy_vars_ue,
                         short *dlsch_llr,
                         LTE_DL_FRAME_PARMS *frame_parms,
                         LTE_UE_DLSCH_t *dlsch,
                         LTE_DL_UE_HARQ_t *harq_process,
                         uint8_t subframe,
                         uint8_t harq_pid,
                         uint8_t is_crnti,
			 uint8_t llr8_flag) {
  
  
  time_stats_t *dlsch_rate_unmatching_stats=&phy_vars_ue->dlsch_rate_unmatching_stats;
  time_stats_t *dlsch_turbo_decoding_stats=&phy_vars_ue->dlsch_turbo_decoding_stats;
  time_stats_t *dlsch_deinterleaving_stats=&phy_vars_ue->dlsch_deinterleaving_stats;
  uint32_t A,E;
  uint32_t G;
  uint32_t ret,offset;
  u16 iind;
 uint8_t llr_clear=0;
  //  uint8_t dummy_channel_output[(3*8*block_length)+12];
  short dummy_w[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)];
  uint32_t r,r_offset=0,Kr,Kr_bytes,err_flag=0;
  uint8_t crc_type;
#ifdef DEBUG_DLSCH_DECODING
  u16 i;
#endif
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

  if (!dlsch_llr) {
    msg("dlsch_decoding.c: NULL dlsch_llr pointer\n");
    return(dlsch->max_turbo_iterations);
  }

  if (!harq_process) {
      msg("dlsch_decoding.c: NULL harq_process pointer\n");
      return(dlsch->max_turbo_iterations);
  }

  if (!frame_parms) {
      msg("dlsch_decoding.c: NULL frame_parms pointer\n");
      return(dlsch->max_turbo_iterations);
  }
  
  if (subframe>9) {
      msg("dlsch_decoding.c: Illegal subframe index %d\n",subframe);
      return(dlsch->max_turbo_iterations);
  }

  if (llr8_flag == 0)
    tc = phy_threegpplte_turbo_decoder16;
  else
    tc = phy_threegpplte_turbo_decoder8;
  //  nb_rb = dlsch->nb_rb;

  /*
  if (nb_rb > frame_parms->N_RB_DL) {
    msg("dlsch_decoding.c: Illegal nb_rb %d\n",nb_rb);
    return(max_turbo_iterations);
    }*/

  /*harq_pid = dlsch->current_harq_pid;
  if (harq_pid >= 8) {
    msg("dlsch_decoding.c: Illegal harq_pid %d\n",harq_pid);
    return(max_turbo_iterations);
  }
  */
  A = harq_process->TBS; //2072 for QPSK 1/3

  //  mod_order = get_Qm(harq_process->mcs);

  ret = dlsch->max_turbo_iterations;


  G = harq_process->G;
  //get_G(frame_parms,nb_rb,dlsch->rb_alloc,mod_order,num_pdcch_symbols,phy_vars_ue->frame,subframe);

  //  msg("DLSCH Decoding, harq_pid %d Ndi %d\n",harq_pid,harq_process->Ndi);

  if (harq_process->Ndi == 1) {
    // This is a new packet, so compute quantities regarding segmentation
    harq_process->B = A+24;
    lte_segmentation(NULL,
		     NULL,
		     harq_process->B,
		     &harq_process->C,
		     &harq_process->Cplus,
		     &harq_process->Cminus,
		     &harq_process->Kplus,
		     &harq_process->Kminus,		     
		     &harq_process->F);
    //  CLEAR LLR's HERE for first packet in process
  }
  /*
  else {
    msg("dlsch_decoding.c: Ndi>0 not checked yet!!\n");
    return(max_turbo_iterations);
  }
  */
  err_flag = 0;
  r_offset = 0;
  for (r=0;r<harq_process->C;r++) {

    // Get Turbo interleaver parameters
    if (r<harq_process->Cminus)
      Kr = harq_process->Kminus;
    else
      Kr = harq_process->Kplus;
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
      msg("dlsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      return(-1);
    }
  
#ifdef DEBUG_DLSCH_DECODING     
    msg("f1 %d, f2 %d, F %d\n",f1f2mat_old[2*iind],f1f2mat_old[1+(2*iind)],(r==0) ? harq_process->F : 0);
#endif

    start_meas(dlsch_rate_unmatching_stats);
    memset(&dummy_w[r][0],0,3*(6144+64)*sizeof(short));
    harq_process->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       (uint8_t*) &dummy_w[r][0],
							       (r==0) ? harq_process->F : 0);

#ifdef DEBUG_DLSCH_DECODING    
    msg("HARQ_PID %d Rate Matching Segment %d (coded bits %d,unpunctured/repeated bits %d, mod_order %d, nb_rb %d, Nl %d)...\n",
	harq_pid,r, G,
	   Kr*3,
	   get_Qm(harq_process->mcs),
	   harq_process->nb_rb,
	   harq_process->Nl);
#endif    
// compute llr_clear
if (harq_process->Ndi==0){
	if (harq_process->first_Qm==6 && get_Qm(harq_process->mcs)==2)
	 llr_clear=1;
	 else
	 llr_clear=0;
 }
 else
	llr_clear=1;

    if (lte_rate_matching_turbo_rx(harq_process->RTC[r],
				   G,
				   harq_process->w[r],
				   (uint8_t*)&dummy_w[r][0],
				   dlsch_llr,
				   harq_process->C,
				   NSOFT,
				   dlsch->Mdlharq,
				   dlsch->Kmimo,
				   harq_process->rvidx,
				   llr_clear,// harq_process->Ndi,  			  
				   get_Qm(harq_process->mcs),
				   harq_process->Nl,
				   r,
				   &E)==-1) {
      stop_meas(dlsch_rate_unmatching_stats);
      msg("dlsch_decoding.c: Problem in rate_matching\n");
      return(dlsch->max_turbo_iterations);
    }
    else
      stop_meas(dlsch_rate_unmatching_stats);
    r_offset += E;

    /*
    msg("Subblock deinterleaving, d %p w %p\n",
	   harq_process->d[r],
	   harq_process->w);
    */
    start_meas(dlsch_deinterleaving_stats);
    sub_block_deinterleaving_turbo(4+Kr, 
				   &harq_process->d[r][96], 

				   harq_process->w[r]); 
    stop_meas(dlsch_deinterleaving_stats);
    
#ifdef DEBUG_DLSCH_DECODING    
    /*
    if (r==0) {
              write_output("decoder_llr.m","decllr",dlsch_llr,G,1,0);
              write_output("decoder_in.m","dec",&harq_process->d[0][96],(3*8*Kr_bytes)+12,1,0);
    }
    
    msg("decoder input(segment %d) :",r);
    int i; for (i=0;i<(3*8*Kr_bytes)+12;i++)
      msg("%d : %d\n",i,harq_process->d[r][96+i]);
      msg("\n");*/
#endif
    

    //    msg("Clearing c, %p\n",harq_process->c[r]);
    memset(harq_process->c[r],0,Kr_bytes);
    //    msg("done\n");
    if (harq_process->C == 1) 
      crc_type = CRC24_A;
    else 
      crc_type = CRC24_B;

    /*            
    msg("decoder input(segment %d)\n",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      if ((harq_process->d[r][96+i]>7) || 
	  (harq_process->d[r][96+i] < -8))
	msg("%d : %d\n",i,harq_process->d[r][96+i]);
    msg("\n");
    */

    if (err_flag == 0) {

      start_meas(dlsch_turbo_decoding_stats);      
#ifdef TURBO_S
      ret = phy_threegpplte_turbo_decoder_scalar
#else
	
      ret = tc
#endif
	(&harq_process->d[r][96],
	 harq_process->c[r],
	 Kr,
	 f1f2mat_old[iind*2],   
	 f1f2mat_old[(iind*2)+1], 
	 dlsch->max_turbo_iterations,
	 crc_type,
	 (r==0) ? harq_process->F : 0,
	 &phy_vars_ue->dlsch_tc_init_stats,
	 &phy_vars_ue->dlsch_tc_alpha_stats,
	 &phy_vars_ue->dlsch_tc_beta_stats,
	 &phy_vars_ue->dlsch_tc_gamma_stats,
	 &phy_vars_ue->dlsch_tc_ext_stats,
	 &phy_vars_ue->dlsch_tc_intl1_stats,
	 &phy_vars_ue->dlsch_tc_intl2_stats); //(is_crnti==0)?harq_pid:harq_pid+1);
      

      stop_meas(dlsch_turbo_decoding_stats);            
    }


      
    if ((err_flag == 0) && (ret>=(1+dlsch->max_turbo_iterations))) {// a Code segment is in error so break;
      //msg("CRC failed, segment %d\n",r);
      err_flag = 1;
    }

  }

  if (err_flag == 1) {
    dlsch->harq_ack[subframe].ack = 0;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    harq_process->round++;
    //    msg("DLSCH: Setting NACK for subframe %d (pid %d, round %d)\n",subframe,harq_pid,harq_process->round);
    if (harq_process->round >= dlsch->Mdlharq) {
      harq_process->status = SCH_IDLE;
    }
    
    return((1+dlsch->max_turbo_iterations));
  }
  else {
    harq_process->status = SCH_IDLE;
    harq_process->round  = 0;
    dlsch->harq_ack[subframe].ack = 1;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    //    msg("DLSCH decoding: Setting ACK for subframe %d (pid %d)\n",subframe,harq_pid);
  }
  // Reassembly of Transport block here
  offset = 0;
  /*  
  msg("harq_pid %d\n",harq_pid);
  msg("F %d, Fbytes %d\n",harq_process->F,harq_process->F>>3);
  msg("C %d\n",harq_process->C);
  */
  for (r=0;r<harq_process->C;r++) {
    if (r<harq_process->Cminus)
      Kr = harq_process->Kminus;
    else
      Kr = harq_process->Kplus;

    Kr_bytes = Kr>>3;
    //    printf("Segment %d : Kr= %d bytes\n",r,Kr_bytes);
    if (r==0) {
      memcpy(harq_process->b,
	     &harq_process->c[0][(harq_process->F>>3)],
	     Kr_bytes - (harq_process->F>>3)- ((harq_process->C>1)?3:0));
      offset = Kr_bytes - (harq_process->F>>3) - ((harq_process->C>1)?3:0);
      //            msg("copied %d bytes to b sequence (harq_pid %d)\n",
      //      	  Kr_bytes - (harq_process->F>>3),harq_pid); 
    //          msg("b[0] = %x,c[%d] = %x\n",
    //  	  harq_process->b[0],
    //  	  harq_process->F>>3,
    //  	  harq_process->c[0][(harq_process->F>>3)]);
    }
    else {
      memcpy(harq_process->b+offset,
	     harq_process->c[r],
	     Kr_bytes- ((harq_process->C>1)?3:0));
      offset += (Kr_bytes - ((harq_process->C>1)?3:0));
    }
  }
  
  return(ret);
}

#ifdef PHY_ABSTRACTION
#include "SIMULATION/TOOLS/defs.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif

 int dlsch_abstraction_EESM(double* sinr_dB, uint8_t TM, uint32_t rb_alloc[4], uint8_t mcs, uint8_t dl_power_off) {

   int index,ii;
  double sinr_eff = 0;
  int rb_count = 0;
  int offset;
  double bler = 0;
  if(TM==5 && dl_power_off==1)
    { //do nothing -- means there is no second UE and TM 5 is behaving like TM 6 for a singal user
}
  else
  TM = TM-1;

  for (offset = 0; offset <= 24; offset++) {
    if (rb_alloc[0] & (1<<offset)) {
      rb_count++;
      for(ii=0;ii<12;ii++)
	{
	  sinr_eff += exp(-(pow(10, 0.1*(sinr_dB[(offset*12)+ii])))/beta1_dlsch[TM][mcs]);
	  //printf("sinr_eff1 = %f, power %lf\n",sinr_eff, exp(-pow(10,6.8)));
	  
	  //  sinr_eff += exp(-(pow(10, (sinr_dB[offset*2+1])/10))/beta1_dlsch[TM][mcs]);
	  //printf("sinr_dB[%d]=%f\n",offset,sinr_dB[offset*2]);
	}
    }
  }       
  //printf("sinr_eff1 = %f\n",sinr_eff);
  sinr_eff =  -beta2_dlsch[TM][mcs]*log((sinr_eff)/(12*rb_count));
  sinr_eff = 10 * log10(sinr_eff);
  LOG_I(OCM,"sinr_eff2 = %f\n",sinr_eff);

  // table lookup
  sinr_eff *= 10;
  sinr_eff = floor(sinr_eff);
  // if ((int)sinr_eff%2) {
  //   sinr_eff += 1;
  // }
  sinr_eff /= 10;
  msg("Imran sinr_eff after rounding = %f\n",sinr_eff);
 for (index = 0; index < table_length[mcs]; index++) {
    if(index == 0) {
      if (sinr_eff < sinr_bler_map[mcs][0][index]) {
        bler = 1;
        break;
      }
      else if(sinr_eff > sinr_bler_map[mcs][0][table_length[mcs]]){
	bler = 0;
	break;
      }
    }
    if (sinr_eff == sinr_bler_map[mcs][0][index]) {
      bler = sinr_bler_map[mcs][1][index];
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

 int dlsch_abstraction_MIESM(double* sinr_dB,uint8_t TM, uint32_t rb_alloc[4], uint8_t mcs,uint8_t dl_power_off) {
  int index,ii;
  double sinr_eff = 0;
  double x = 0;
   double I =0;
  double qpsk_max=12.2;
  double qam16_max=19.2;
  double qam64_max=25.2;
  double sinr_min = -20;
  int rb_count = 0;
  int offset=0;
  double bler = 0;

  if(TM==5 && dl_power_off==1)
    { //do nothing -- means there is no second UE and TM 5 is behaving like TM 6 for a singal user
    }
  else
    TM = TM-1; 
  
  
  for (offset = 0; offset <= 24; offset++) {
    if (rb_alloc[0] & (1<<offset)) {
      rb_count++;
      for(ii=0;ii<12;ii++){
	//x is the sinr_dB in dB
	  x = sinr_dB[(offset*12)+ii] - 10*log10(beta1_dlsch_MI[TM][mcs]);
	  if(x<sinr_min)
	    I +=0;
	  else{
	  if(mcs<10)
	    {
	      if(x>qpsk_max)
		I += 1;
	      else
		I += (q_qpsk[0]*pow(x,7) + q_qpsk[1]*pow(x,6) + q_qpsk[2]*pow(x,5) + q_qpsk[3]*pow(x,4) + q_qpsk[4]*pow(x,3) + q_qpsk[5]*pow(x,2) + q_qpsk[6]*x + q_qpsk[7]);
	    }
	  else if(mcs>9 && mcs<17)
	    {
	      if(x>qam16_max)
		I += 1;
	      else
		I += (q_qam16[0]*pow(x,7) + q_qam16[1]*pow(x,6) + q_qam16[2]*pow(x,5) + q_qam16[3]*pow(x,4) + q_qam16[4]*pow(x,3) + q_qam16[5]*pow(x,2) + q_qam16[6]*x + q_qam16[7]);
	    }
	  else if(mcs>16 && mcs<23)
	    {
	      
	      if(x>qam64_max)
		I += 1;
	      else
		I += (q_qam64[0]*pow(x,7) + q_qam64[1]*pow(x,6) + q_qam64[2]*pow(x,5) + q_qam64[3]*pow(x,4) + q_qam64[4]*pow(x,3) + q_qam64[5]*pow(x,2) + q_qam64[6]*x + q_qam64[7]);
	    }
	  }
      }
    }
  }
  // averaging of accumulated MI 
	  I = I/(12*rb_count);  
	  //Now  I->SINR_effective Mapping
	  
	  if(mcs<10)
	    {
	      sinr_eff = (p_qpsk[0]*pow(I,7) + p_qpsk[1]*pow(I,6) + p_qpsk[2]*pow(I,5) + p_qpsk[3]*pow(I,4) + p_qpsk[4]*pow(I,3) + p_qpsk[5]*pow(I,2) + p_qpsk[6]*I + p_qpsk[7]);
	    }
	  else if(mcs>9 && mcs<17)
	    {
	      sinr_eff = (p_qam16[0]*pow(I,7) + p_qam16[1]*pow(I,6) + p_qam16[2]*pow(I,5) + p_qam16[3]*pow(I,4) + p_qam16[4]*pow(I,3) + p_qam16[5]*pow(I,2) + p_qam16[6]*I + p_qam16[7]);
	    }
	  else if(mcs>16 && mcs<23)
	    {
	      sinr_eff = (p_qam64[0]*pow(I,7) + p_qam64[1]*pow(I,6) + p_qam64[2]*pow(I,5) + p_qam64[3]*pow(I,4) + p_qam64[4]*pow(I,3) + p_qam64[5]*pow(I,2) + p_qam64[6]*I + p_qam64[7]);
	    }	  

 sinr_eff = sinr_eff + 10*log10(beta2_dlsch_MI[TM][mcs]); 
 printf("SINR_Eff = %e\n",sinr_eff);

 sinr_eff *= 10;
  sinr_eff = floor(sinr_eff);
  // if ((int)sinr_eff%2) {
  //   sinr_eff += 1;
  // }
  sinr_eff /= 10;
  msg("sinr_eff after rounding = %f\n",sinr_eff);

  for (index = 0; index < table_length[mcs]; index++) {
    if(index == 0) {
      if (sinr_eff < sinr_bler_map[mcs][0][index]) {
        bler = 1;
        break;
      }
      else if(sinr_eff > sinr_bler_map[mcs][0][table_length[mcs]]){
	bler = 0;
	break;
      }
    }
    if (sinr_eff == sinr_bler_map[mcs][0][index]) {
      bler = sinr_bler_map[mcs][1][index];
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

uint32_t dlsch_decoding_emul(PHY_VARS_UE *phy_vars_ue,
			uint8_t subframe,
			uint8_t dlsch_id,
			uint8_t eNB_id) {

  LTE_UE_DLSCH_t *dlsch_ue;
  LTE_eNB_DLSCH_t *dlsch_eNB;
  uint8_t harq_pid;
  uint32_t eNB_id2;
  uint32_t ue_id;
#ifdef DEBUG_DLSCH_DECODING
  u16 i;
#endif

  for (eNB_id2=0;eNB_id2<NB_eNB_INST;eNB_id2++) {
    if (PHY_vars_eNB_g[eNB_id2]->lte_frame_parms.Nid_cell == phy_vars_ue->lte_frame_parms.Nid_cell)
      break;
  }
  if (eNB_id2==NB_eNB_INST) {
    LOG_E(PHY,"FATAL : Could not find attached eNB for DLSCH emulation !!!!\n");
    mac_xface->macphy_exit("");
  }

  LOG_D(PHY,"[UE] dlsch_decoding_emul : subframe %d, eNB_id %d, dlsch_id %d\n",subframe,eNB_id2,dlsch_id);

  //  printf("dlsch_eNB_ra->harq_processes[0] %p\n",PHY_vars_eNB_g[eNB_id]->dlsch_eNB_ra->harq_processes[0]);


  switch (dlsch_id) {
  case 0: // SI
    dlsch_ue = phy_vars_ue->dlsch_ue_SI[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB_SI;
    //    msg("Doing SI: TBS %d\n",dlsch_ue->harq_processes[0]->TBS>>3);
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
#ifdef DEBUG_DLSCH_DECODING   
    LOG_D(PHY,"SI Decoded\n");
    for (i=0;i<dlsch_ue->harq_processes[0]->TBS>>3;i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[0]->b[i]);
    LOG_T(PHY,"\n");
#endif
    return(1);
    break;
  case 1: // RA
    dlsch_ue  = phy_vars_ue->dlsch_ue_ra[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB_ra;
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
#ifdef DEBUG_DLSCH_DECODING   
    LOG_D(PHY,"RA Decoded\n");
    for (i=0;i<dlsch_ue->harq_processes[0]->TBS>>3;i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[0]->b[i]);
    LOG_T(PHY,"\n");
#endif
    return(1);
    break;
  case 2: // TB0
    dlsch_ue  = phy_vars_ue->dlsch_ue[eNB_id][0];
    harq_pid = dlsch_ue->current_harq_pid;
    ue_id= (uint32_t)find_ue((s16)phy_vars_ue->lte_ue_pdcch_vars[(uint32_t)eNB_id]->crnti,PHY_vars_eNB_g[eNB_id2]);
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB[ue_id][0];

#ifdef DEBUG_DLSCH_DECODING
    for (i=0;i<dlsch_ue->harq_processes[harq_pid]->TBS>>3;i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[harq_pid]->b[i]);
    LOG_T(PHY,"\n current harq pid is %d ue id %d \n", harq_pid, ue_id);
#endif

    if (dlsch_abstraction_EESM(phy_vars_ue->sinr_dB, phy_vars_ue->transmission_mode[eNB_id], dlsch_eNB->rb_alloc, dlsch_eNB->harq_processes[harq_pid]->mcs,PHY_vars_eNB_g[eNB_id]->mu_mimo_mode[ue_id].dl_pow_off) == 1) {
      // reset HARQ 
      dlsch_ue->harq_processes[harq_pid]->status = SCH_IDLE;
      dlsch_ue->harq_processes[harq_pid]->round  = 0;
      dlsch_ue->harq_ack[subframe].ack = 1;
      dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
      dlsch_ue->harq_ack[subframe].send_harq_status = 1;
      if (dlsch_ue->harq_processes[harq_pid]->Ndi == 1)
	memcpy(dlsch_ue->harq_processes[harq_pid]->b,
	       dlsch_eNB->harq_processes[harq_pid]->b,
	       dlsch_ue->harq_processes[harq_pid]->TBS>>3);
      return(1);
    }
    else {
      // retransmission
      dlsch_ue->harq_processes[harq_pid]->status = ACTIVE;
      dlsch_ue->harq_processes[harq_pid]->round++;
      dlsch_ue->harq_ack[subframe].ack = 0;
      dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
      dlsch_ue->harq_ack[subframe].send_harq_status = 1;
      return(1+dlsch_ue->max_turbo_iterations);
      }

    break;
  case 3: // TB1
    dlsch_ue = phy_vars_ue->dlsch_ue[eNB_id][1];
    harq_pid = dlsch_ue->current_harq_pid;
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB[(uint32_t)find_ue((s16)phy_vars_ue->lte_ue_pdcch_vars[(uint32_t)eNB_id]->crnti,
								PHY_vars_eNB_g[eNB_id2])][1];
     // reset HARQ 
    dlsch_ue->harq_processes[harq_pid]->status = SCH_IDLE;
    dlsch_ue->harq_processes[harq_pid]->round  = 0;
    dlsch_ue->harq_ack[subframe].ack = 1;
    dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
    dlsch_ue->harq_ack[subframe].send_harq_status = 1;
    if (dlsch_ue->harq_processes[harq_pid]->Ndi == 1)
      memcpy(dlsch_eNB->harq_processes[harq_pid]->b,dlsch_ue->harq_processes[harq_pid]->b,dlsch_ue->harq_processes[harq_pid]->TBS>>3);
    break;
  default:
    dlsch_ue = phy_vars_ue->dlsch_ue[eNB_id][0];
    LOG_E(PHY,"dlsch_decoding_emul: FATAL, unknown DLSCH_id %d\n",dlsch_id);
    return(1+dlsch_ue->max_turbo_iterations);
  }

  LOG_E(PHY,"[FATAL] dlsch_decoding.c: Should never exit here ...\n");
  return(0);
}
#endif
