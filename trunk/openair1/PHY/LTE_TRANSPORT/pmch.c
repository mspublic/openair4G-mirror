#include "PHY/defs.h"
#include "PHY/extern.h"

// Mask for identifying subframe for MBMS 
#define MBSFN_TDD_SF3 0x80// for TDD
#define MBSFN_TDD_SF4 0x40
#define MBSFN_TDD_SF7 0x20
#define MBSFN_TDD_SF8 0x10
#define MBSFN_TDD_SF9 0x08

#include "PHY/defs.h"
 
#define MBSFN_FDD_SF1 0x80// for FDD
#define MBSFN_FDD_SF2 0x40
#define MBSFN_FDD_SF3 0x20
#define MBSFN_FDD_SF6 0x10
#define MBSFN_FDD_SF7 0x08
#define MBSFN_FDD_SF8 0x04

 
#ifndef __SSE3__
#warning SSE3 instruction set not preset
__m128i zeroM;//,tmp_over_sqrt_10,tmp_sum_4_over_sqrt_10,tmp_sign,tmp_sign_3_over_sqrt_10;
//#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_abs_epi16(xmmx) _mm_add_epi16(_mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroM,(xmmx))),_mm_srli_epi16(_mm_cmpgt_epi16(zeroM,(xmmx)),15))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroM,(xmmy)))
#endif


void dump_mch(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint16_t coded_bits_per_codeword,int subframe) {

  unsigned int nsymb_pmch=12;
  char fname[32],vname[32];
  int N_RB_DL=phy_vars_ue->lte_frame_parms.N_RB_DL;
  
  sprintf(fname,"mch_rxF_ext0.m");
  sprintf(vname,"pmch_rxF_ext0");
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->rxdataF_ext[0],12*N_RB_DL*nsymb_pmch,1,1);
  sprintf(fname,"mch_ch_ext00.m");
  sprintf(vname,"pmch_ch_ext00");
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->dl_ch_estimates_ext[0],12*N_RB_DL*nsymb_pmch,1,1);
  /*
    write_output("dlsch%d_ch_ext01.m","dl01_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[1],12*N_RB_DL*nsymb_pmch,1,1);
    write_output("dlsch%d_ch_ext10.m","dl10_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[2],12*N_RB_DL*nsymb_pmch,1,1);
    write_output("dlsch%d_ch_ext11.m","dl11_ch0_ext",lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext[3],12*N_RB_DL*nsymb_pmch,1,1);
    write_output("dlsch%d_rho.m","dl_rho",lte_ue_pdsch_vars[eNB_id]->rho[0],12*N_RB_DL*nsymb_pmch,1,1);
  */
  sprintf(fname,"mch_rxF_comp0.m");
  sprintf(vname,"pmch_rxF_comp0");
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->rxdataF_comp[0],12*N_RB_DL*nsymb_pmch,1,1);
  sprintf(fname,"mch_rxF_llr.m");
  sprintf(vname,"pmch_llr");
  write_output(fname,vname, phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->llr[0],coded_bits_per_codeword,1,0);
  sprintf(fname,"mch_mag1.m");
  sprintf(vname,"pmch_mag1");
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->dl_ch_mag[0],12*N_RB_DL*nsymb_pmch,1,1);
  sprintf(fname,"mch_mag2.m");
  sprintf(vname,"pmch_mag2");
  write_output(fname,vname,phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id]->dl_ch_magb[0],12*N_RB_DL*nsymb_pmch,1,1);

  write_output("mch00_ch0.m","pmch00_ch0",
	       &(phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0]),
	       phy_vars_ue->lte_frame_parms.ofdm_symbol_size*12,1,1);

  write_output("rxsig_mch.m","rxs_mch",
	       &phy_vars_ue->lte_ue_common_vars.rxdata[0][subframe*phy_vars_ue->lte_frame_parms.samples_per_tti],
	       phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);
  if (PHY_vars_eNB_g)
    write_output("txsig_mch.m","txs_mch",
   	         &PHY_vars_eNB_g[0][0]->lte_eNB_common_vars.txdata[0][0][subframe*phy_vars_ue->lte_frame_parms.samples_per_tti],
	         phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);
}

int is_pmch_subframe(uint32_t frame, int subframe, LTE_DL_FRAME_PARMS *frame_parms) {

  uint32_t period;
  uint8_t i;


  for (i=0; i<frame_parms->num_MBSFN_config; i++) {  // we have at least one MBSFN configuration
    period = 1<<frame_parms->MBSFN_config[i].radioframeAllocationPeriod;
    if ((frame % period) == frame_parms->MBSFN_config[i].radioframeAllocationOffset) {
      if (frame_parms->MBSFN_config[i].fourFrames_flag == 0) {
	if (frame_parms->frame_type == FDD) {
	  switch (subframe) {
	    
	  case 1:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF1) > 0)
	      return(1);
	    break;
	  case 2:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF2) > 0)
	      return(1);
	    break;
	  case 3:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF3) > 0)
	      return(1);
	    break;
	  case 6:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF6) > 0)
	      return(1);
	    break;
	  case 7:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF7) > 0)
	      return(1);
	    break;
	  case 8:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_FDD_SF8) > 0)
	      return(1);
	    break;
	  }
	}
	else  {
	  switch (subframe) {
	  case 3:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_TDD_SF3) > 0)
	      return(1);
	    break;
	  case 4:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_TDD_SF4) > 0)
	      return(1);
	    break;
	  case 7:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_TDD_SF7) > 0)
	      return(1);
	    break;
	  case 8:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_TDD_SF8) > 0)
	      return(1);
	    break;
	  case 9:
	    if ((frame_parms->MBSFN_config[i].mbsfn_SubframeConfig & MBSFN_TDD_SF9) > 0)
	      return(1);
	    break;
	  }
	}

      }
      else {  // handle 4 frames case

      }
    } 
  }
  return(0);
} 

void fill_eNB_dlsch_MCH(PHY_VARS_eNB *phy_vars_eNB,int mcs,int ndi,int rvidx, int abstraction_flag) {

  LTE_eNB_DLSCH_t *dlsch = phy_vars_eNB->dlsch_eNB_MCH;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_eNB->lte_frame_parms;
  
  //  dlsch->rnti   = M_RNTI;
  dlsch->harq_processes[0]->mcs   = mcs;
  //  dlsch->harq_processes[0]->Ndi   = ndi;
  dlsch->harq_processes[0]->rvidx = rvidx;
  dlsch->harq_processes[0]->Nl    = 1;
  dlsch->harq_processes[0]->TBS   = TBStable[get_I_TBS(dlsch->harq_processes[0]->mcs)][frame_parms->N_RB_DL-1];
  dlsch->current_harq_pid = 0;
  dlsch->nb_rb = frame_parms->N_RB_DL;

  switch(frame_parms->N_RB_DL) {
  case 6:
    dlsch->rb_alloc[0] = 0x3f;
    break;
  case 25:
    dlsch->rb_alloc[0] = 0x1ffffff;
    break;
  case 50:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0x3ffff;
    break;
  case 100:
    dlsch->rb_alloc[0] = 0xffffffff;
    dlsch->rb_alloc[1] = 0xffffffff;
    dlsch->rb_alloc[2] = 0xffffffff;
    dlsch->rb_alloc[3] = 0xf;
    break;
  }

  if (abstraction_flag){
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].cntl.pmch_flag=1;
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_pmch=1; // assumption: there is always one pmch in each SF
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_common_dci=0;
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].num_ue_spec_dci=0;
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].dlsch_type[0]=5;// put at the reserved position for PMCH
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].harq_pid[0]=0;
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].ue_id[0]=255;//broadcast
    eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].tbs[0]=dlsch->harq_processes[0]->TBS>>3;
  }

}

void fill_UE_dlsch_MCH(PHY_VARS_UE *phy_vars_ue,int mcs,int ndi,int rvidx,int eNB_id) {

  LTE_UE_DLSCH_t *dlsch = phy_vars_ue->dlsch_ue_MCH[eNB_id];
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  
  //  dlsch->rnti   = M_RNTI;
  dlsch->harq_processes[0]->mcs   = mcs;
  dlsch->harq_processes[0]->rvidx = rvidx;
  //  dlsch->harq_processes[0]->Ndi   = ndi;
  dlsch->harq_processes[0]->Nl    = 1;
  dlsch->harq_processes[0]->TBS = TBStable[get_I_TBS(dlsch->harq_processes[0]->mcs)][frame_parms->N_RB_DL-1];
  dlsch->current_harq_pid = 0;
  dlsch->harq_processes[0]->nb_rb = frame_parms->N_RB_DL;
  
  switch(frame_parms->N_RB_DL) {
  case 6:
    dlsch->harq_processes[0]->rb_alloc[0] = 0x3f;
    break;
  case 25:
    dlsch->harq_processes[0]->rb_alloc[0] = 0x1ffffff;
    break;
  case 50:
    dlsch->harq_processes[0]->rb_alloc[0] = 0xffffffff;
    dlsch->harq_processes[0]->rb_alloc[1] = 0x3ffff;
    break;
  case 100:
    dlsch->harq_processes[0]->rb_alloc[0] = 0xffffffff;
    dlsch->harq_processes[0]->rb_alloc[1] = 0xffffffff;
    dlsch->harq_processes[0]->rb_alloc[2] = 0xffffffff;
    dlsch->harq_processes[0]->rb_alloc[3] = 0xf;
    break;
  }
}

 void generate_mch(PHY_VARS_eNB *phy_vars_eNB,int sched_subframe,uint8_t *a,int abstraction_flag) {

  int G;
  int subframe = phy_vars_eNB->proc[sched_subframe].subframe_tx;

  if (abstraction_flag != 0) {
    if (eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]!=0)
      printf("[PHY][EMU] PMCH transport block position is different than zero %d \n", eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]);
    
    memcpy(phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->b,
	   a,
	   phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->TBS>>3);
    LOG_D(PHY, "[eNB %d] dlsch_encoding_emul pmch , tbs is %d \n", 
	  phy_vars_eNB->Mod_id,
	  phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->TBS>>3);

    memcpy(&eNB_transport_info[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id].transport_blocks[eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]],
    	   a,
	   phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->TBS>>3);
    eNB_transport_info_TB_index[phy_vars_eNB->Mod_id][phy_vars_eNB->CC_id]+= phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->TBS>>3;//=eNB_transport_info[phy_vars_eNB->Mod_id].tbs[0];
  }else {
    G = get_G(&phy_vars_eNB->lte_frame_parms,
	      phy_vars_eNB->lte_frame_parms.N_RB_DL,
	      phy_vars_eNB->dlsch_eNB_MCH->rb_alloc,
	      get_Qm(phy_vars_eNB->dlsch_eNB_MCH->harq_processes[0]->mcs),1,
	      2,phy_vars_eNB->proc[sched_subframe].frame_tx,subframe);
    
    generate_mbsfn_pilot(phy_vars_eNB,
			 phy_vars_eNB->lte_eNB_common_vars.txdataF[0],
			 AMP,
			 subframe);
    
    if (dlsch_encoding(a,
		       &phy_vars_eNB->lte_frame_parms,
		       1,
		       phy_vars_eNB->dlsch_eNB_MCH,
		       phy_vars_eNB->proc[sched_subframe].frame_tx,
		       subframe,
		       &phy_vars_eNB->dlsch_rate_matching_stats,
		       &phy_vars_eNB->dlsch_turbo_encoding_stats,
		       &phy_vars_eNB->dlsch_interleaving_stats
		       )<0)
      exit(-1);
    
    dlsch_scrambling(&phy_vars_eNB->lte_frame_parms,1,phy_vars_eNB->dlsch_eNB_MCH,G,0,subframe<<1);
    
    
    mch_modulation(phy_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   subframe,
		   &phy_vars_eNB->lte_frame_parms,
		   phy_vars_eNB->dlsch_eNB_MCH);
  }
  
 }

void mch_extract_rbs(int **rxdataF,
		     int **dl_ch_estimates,
		     int **rxdataF_ext,
		     int **dl_ch_estimates_ext,
		     unsigned char symbol,
		     unsigned char subframe,
		     LTE_DL_FRAME_PARMS *frame_parms) {

  int pilots=0,i,j,offset,aarx;

  //  printf("Extracting PMCH: symbol %d\n",symbol);
  if ((symbol==2)||
      (symbol==10)) {
    pilots = 1;
    offset = 1;
  }
  else if (symbol==6) {
    pilots = 1;
    offset = 0;
  }


  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    if (pilots==1) {
      for (i=offset,j=0;i<frame_parms->N_RB_DL*6;i+=2,j++) {
	/*	printf("MCH with pilots: i %d, j %d => %d,%d\n",i,j,
	       *(int16_t*)&rxdataF[aarx][i+frame_parms->first_carrier_offset + (symbol*frame_parms->ofdm_symbol_size)],
	       *(int16_t*)(1+&rxdataF[aarx][i+frame_parms->first_carrier_offset + (symbol*frame_parms->ofdm_symbol_size)])); 
	       */
	rxdataF_ext[aarx][j+symbol*(frame_parms->N_RB_DL*12)]                                  = rxdataF[aarx][i+frame_parms->first_carrier_offset + (symbol*frame_parms->ofdm_symbol_size)]; 
	rxdataF_ext[aarx][(frame_parms->N_RB_DL*3)+j+symbol*(frame_parms->N_RB_DL*12)]         = rxdataF[aarx][i+1+ (symbol*frame_parms->ofdm_symbol_size)]; 
	dl_ch_estimates_ext[aarx][j+symbol*(frame_parms->N_RB_DL*12)]                          = dl_ch_estimates[aarx][i+(symbol*frame_parms->ofdm_symbol_size)]; 
	dl_ch_estimates_ext[aarx][(frame_parms->N_RB_DL*3)+j+symbol*(frame_parms->N_RB_DL*12)] = dl_ch_estimates[aarx][i+(frame_parms->N_RB_DL*6)+(symbol*frame_parms->ofdm_symbol_size)]; 
      }
    }
    else {
      memcpy((void*)&rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)],
	     (void*)&rxdataF[aarx][frame_parms->first_carrier_offset + (symbol*frame_parms->ofdm_symbol_size)],
	     frame_parms->N_RB_DL*24);
      memcpy((void*)&rxdataF_ext[aarx][(frame_parms->N_RB_DL*6) + symbol*(frame_parms->N_RB_DL*12)],
	     (void*)&rxdataF[aarx][1 + (symbol*frame_parms->ofdm_symbol_size)],
	     frame_parms->N_RB_DL*24);
      memcpy((void*)&dl_ch_estimates_ext[aarx][symbol*(frame_parms->N_RB_DL*12)],
	     (void*)&dl_ch_estimates[aarx][(symbol*frame_parms->ofdm_symbol_size)],
	     frame_parms->N_RB_DL*48);
    }

  }



}

void mch_channel_level(int **dl_ch_estimates_ext,
		       LTE_DL_FRAME_PARMS *frame_parms,
		       int *avg,
		       uint8_t symbol,
		       unsigned short nb_rb){

  int i,aarx,nre;
  __m128i *dl_ch128,avg128;
  
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    //clear average level
    avg128 = _mm_xor_si128(avg128,avg128);
    // 5 is always a symbol with no pilots for both normal and extended prefix
    
    dl_ch128=(__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];

    if ((symbol == 2) || (symbol == 6) || (symbol == 10))
      nre = (frame_parms->N_RB_DL*6);
    else
      nre = (frame_parms->N_RB_DL*12);

    for (i=0;i<(nre>>2);i++) {
      avg128 = _mm_add_epi32(avg128,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
    }
    
    avg[aarx] = (((int*)&avg128)[0] + 
		 ((int*)&avg128)[1] + 
		 ((int*)&avg128)[2] + 
		 ((int*)&avg128)[3])/nre;
    
      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

void mch_channel_compensation(int **rxdataF_ext,
			      int **dl_ch_estimates_ext,
			      int **dl_ch_mag,
			      int **dl_ch_magb,
			      int **rxdataF_comp,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char symbol,
			      unsigned char mod_order,
			      unsigned char output_shift) {

  int aarx,nre,i;
  __m128i *dl_ch128,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  __m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3,QAM_amp128,QAM_amp128b;

  if ((symbol == 2) || (symbol == 6) || (symbol == 10))
    nre = frame_parms->N_RB_DL*6;
  else
    nre = frame_parms->N_RB_DL*12;

  if (mod_order == 4) {
    QAM_amp128 = _mm_set1_epi16(QAM16_n1);  // 2/sqrt(10)
    QAM_amp128b = _mm_setzero_si128();
  }    
  else if (mod_order == 6) {
    QAM_amp128  = _mm_set1_epi16(QAM64_n1); // 
    QAM_amp128b = _mm_set1_epi16(QAM64_n2);
  }
  
  

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    dl_ch128          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_mag128      = (__m128i *)&dl_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_mag128b     = (__m128i *)&dl_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];
    
    
    for (i=0;i<(nre>>2);i+=2) {
      if (mod_order>2) {  
	// get channel amplitude if not QPSK
        
	mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128[0]);
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
        
	mmtmpD1 = _mm_madd_epi16(dl_ch128[1],dl_ch128[1]);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
        
	mmtmpD0 = _mm_packs_epi32(mmtmpD0,mmtmpD1);
        
	// store channel magnitude here in a new field of dlsch
        
	dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[0] = dl_ch_mag128[0];
	dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);
	dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128[0],1);
        
	dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[1] = dl_ch_mag128[1];
	dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);
        
        
	dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
        
        
	dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
        
      }
      
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpD0);
      
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpD1);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      //	print_ints("re(shift)",&mmtmpD0);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      //	print_ints("im(shift)",&mmtmpD1);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      //       	print_ints("c0",&mmtmpD2);
      //	print_ints("c1",&mmtmpD3);
      rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128);
      //	print_shorts("ch:",dl_ch128);
      //	print_shorts("pack:",rxdataF_comp128);
      
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[1],rxdataF128[1]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[1]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      
      rxdataF_comp128[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128+1);
      //	print_shorts("ch:",dl_ch128+1);
      //	print_shorts("pack:",rxdataF_comp128+1);	
      
      dl_ch128+=2;
      dl_ch_mag128+=2;
      dl_ch_mag128b+=2;
      rxdataF128+=2;
      rxdataF_comp128+=2;
    
      
    }
  }
  
  _mm_empty();
  _m_empty();
}

void mch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
                         int **rxdataF_comp,
                         int **dl_ch_mag,
                         int **dl_ch_magb,
                         unsigned char symbol) {

    
  int i;
  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*dl_ch_mag128_0,*dl_ch_mag128_1,*dl_ch_mag128_0b,*dl_ch_mag128_1b;

  if (frame_parms->nb_antennas_rx>1) {
      
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12];  
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[1][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_0      = (__m128i *)&dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_1      = (__m128i *)&dl_ch_mag[1][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_0b     = (__m128i *)&dl_ch_magb[0][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_1b     = (__m128i *)&dl_ch_magb[1][symbol*frame_parms->N_RB_DL*12];  
    
    // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
    for (i=0;i<frame_parms->N_RB_DL*3;i++) {
      rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      dl_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0[i],1),_mm_srai_epi16(dl_ch_mag128_1[i],1));
      dl_ch_mag128_0b[i]   = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0b[i],1),_mm_srai_epi16(dl_ch_mag128_1b[i],1));
    }
  }

  _mm_empty();
  _m_empty();
}

int mch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		 int **rxdataF_comp,
		 short *dlsch_llr,
		 unsigned char symbol,
		 short **llr32p) {

  uint32_t *rxF = (uint32_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  uint32_t *llr32;
  int i,len;

  if (symbol==2) {
      llr32 = (uint32_t*)dlsch_llr;
  }
  else {
      llr32 = (uint32_t*)(*llr32p);
  }
 
  if (!llr32) {
      msg("dlsch_qpsk_llr: llr is null, symbol %d, llr32=%p\n",symbol, llr32);
      return(-1);
  } 


  if ((symbol==2) || (symbol==6) || (symbol==10)) {
    len = frame_parms->N_RB_DL*6;
  }
  else {
    len = frame_parms->N_RB_DL*12;
  }
  //  printf("dlsch_qpsk_llr: symbol %d,len %d,pbch_pss_sss_adjust %d\n",symbol,len,pbch_pss_sss_adjust);
  for (i=0;i<len;i++) {
      *llr32 = *rxF;
      rxF++;
      llr32++;
  }

  *llr32p = (short *)llr32;

  _mm_empty();
  _m_empty();

  return(0);
}

//----------------------------------------------------------------------------------------------
// 16-QAM
//----------------------------------------------------------------------------------------------

void mch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   short *dlsch_llr,
		   int **dl_ch_mag,
		   unsigned char symbol,
		   int16_t **llr32p) {

    __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
    __m128i *ch_mag;
    __m128i llr128[2],xmm0;
    int i,len;
    unsigned char len_mod4=0;
    uint32_t *llr32;
    
    if (symbol==2) {
        llr32 = (uint32_t*)dlsch_llr;
    }
    else {
        llr32 = (uint32_t*)*llr32p;
    }
  
    
    ch_mag = (__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
    
    if ((symbol==2) || (symbol==6) || (symbol==10)) {
      len = frame_parms->N_RB_DL*6;
    }
    else {
      len = frame_parms->N_RB_DL*12;
    }
    

    
  // update output pointer according to number of REs in this symbol (<<2 because 4 bits per RE)
    if (symbol==2)
        *llr32p = dlsch_llr + (len<<2);
    else
        *llr32p += (len<<2);
    
    len_mod4 = len&3;
    len>>=2;  // length in quad words (4 REs)
    len+=(len_mod4==0 ? 0 : 1);
    
    for (i=0;i<len;i++) {
        
        xmm0 = _mm_abs_epi16(rxF[i]);
        xmm0 = _mm_subs_epi16(ch_mag[i],xmm0);

        // lambda_1=y_R, lambda_2=|y_R|-|h|^2, lamda_3=y_I, lambda_4=|y_I|-|h|^2
        llr128[0] = _mm_unpacklo_epi32(rxF[i],xmm0); 
        llr128[1] = _mm_unpackhi_epi32(rxF[i],xmm0);
        llr32[0] = ((uint32_t *)&llr128[0])[0];
        llr32[1] = ((uint32_t *)&llr128[0])[1];
        llr32[2] = ((uint32_t *)&llr128[0])[2];
        llr32[3] = ((uint32_t *)&llr128[0])[3];
        llr32[4] = ((uint32_t *)&llr128[1])[0];
        llr32[5] = ((uint32_t *)&llr128[1])[1];
        llr32[6] = ((uint32_t *)&llr128[1])[2];
        llr32[7] = ((uint32_t *)&llr128[1])[3];
        llr32+=8;
  }
  _mm_empty();
  _m_empty();
}

//----------------------------------------------------------------------------------------------
// 64-QAM
//----------------------------------------------------------------------------------------------

void mch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                     int **rxdataF_comp,
                     short *dlsch_llr,
                     int **dl_ch_mag,
                     int **dl_ch_magb,
                     unsigned char symbol,
                     short **llr_save) {

  __m128i xmm1,xmm2,*ch_mag,*ch_magb;
  __m128i *rxF = (__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  
  int i,len,len2;
//   int j=0;
  unsigned char len_mod4;
  short *llr;
  int16_t *llr2;
  
  if (symbol==2)
    llr = dlsch_llr;
  else
    llr = *llr_save;
  
  ch_mag = (__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb = (__m128i*)&dl_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];
  
  if ((symbol==2) || (symbol==6) || (symbol==10)) {
    len = frame_parms->N_RB_DL*6;
  }
  else {
    len = frame_parms->N_RB_DL*12;
  }
  
  
  llr2 = llr;
  llr += (len*6);
  
  len_mod4 =len&3;
  len2=len>>2;  // length in quad words (4 REs)
  len2+=(len_mod4?0:1);
  
  
  for (i=0;i<len2;i++) {
    
    xmm1 = _mm_abs_epi16(rxF[i]);
    xmm1  = _mm_subs_epi16(ch_mag[i],xmm1);
    xmm2 = _mm_abs_epi16(xmm1);
    xmm2 = _mm_subs_epi16(ch_magb[i],xmm2);
    /*
      printf("pmch i: %d => mag (%d,%d) (%d,%d)\n",i,((short *)&ch_mag[i])[0],((short *)&ch_magb[i])[0],
      ((short *)&rxF[i])[0],((short *)&rxF[i])[1]);
    */
    // loop over all LLRs in quad word (24 coded bits)
    /* 
   for (j=0;j<8;j+=2) {
      llr2[0] = ((short *)&rxF[i])[j];
      llr2[1] = ((short *)&rxF[i])[j+1];
      llr2[2] = _mm_extract_epi16(xmm1,j);
      llr2[3] = _mm_extract_epi16(xmm1,j+1);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,j);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,j+1);//((short *)&xmm2)[j+1];
      
      llr2+=6;
    }
    */
      llr2[0] = ((short *)&rxF[i])[0];
      llr2[1] = ((short *)&rxF[i])[1];
      llr2[2] = _mm_extract_epi16(xmm1,0);
      llr2[3] = _mm_extract_epi16(xmm1,1);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,0);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,1);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[2];
      llr2[1] = ((short *)&rxF[i])[3];
      llr2[2] = _mm_extract_epi16(xmm1,2);
      llr2[3] = _mm_extract_epi16(xmm1,3);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,2);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,3);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[4];
      llr2[1] = ((short *)&rxF[i])[5];
      llr2[2] = _mm_extract_epi16(xmm1,4);
      llr2[3] = _mm_extract_epi16(xmm1,5);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,4);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,5);//((short *)&xmm2)[j+1];
      
      llr2+=6;
      llr2[0] = ((short *)&rxF[i])[6];
      llr2[1] = ((short *)&rxF[i])[7];
      llr2[2] = _mm_extract_epi16(xmm1,6);
      llr2[3] = _mm_extract_epi16(xmm1,7);//((short *)&xmm1)[j+1];
      llr2[4] = _mm_extract_epi16(xmm2,6);//((short *)&xmm2)[j];
      llr2[5] = _mm_extract_epi16(xmm2,7);//((short *)&xmm2)[j+1];
      
      llr2+=6;
  }
  *llr_save = llr;
  _mm_empty();
  _m_empty();
}

int avg_pmch[4];
int rx_pmch(PHY_VARS_UE *phy_vars_ue,
	   unsigned char eNB_id,
	   uint8_t subframe,
	   unsigned char symbol) {

  LTE_UE_COMMON *lte_ue_common_vars  = &phy_vars_ue->lte_ue_common_vars;
  LTE_UE_PDSCH **lte_ue_pdsch_vars   = &phy_vars_ue->lte_ue_pdsch_vars_MCH[eNB_id];
  LTE_DL_FRAME_PARMS *frame_parms    = &phy_vars_ue->lte_frame_parms;
  LTE_UE_DLSCH_t   **dlsch_ue        = &phy_vars_ue->dlsch_ue_MCH[eNB_id];
  int avgs,aarx;

  //printf("*********************mch: symbol %d\n",symbol);

  mch_extract_rbs(lte_ue_common_vars->rxdataF,
		  lte_ue_common_vars->dl_ch_estimates[eNB_id],
		  lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
		  lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
		  symbol,
		  subframe,
		  frame_parms);
  if (symbol == 2) {
    mch_channel_level(lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
		      frame_parms,
		      avg_pmch,
		      symbol,
		      frame_parms->N_RB_DL);
  }

  avgs = 0;  
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
    avgs = cmax(avgs,avg_pmch[aarx]);  
 
  if (get_Qm(dlsch_ue[0]->harq_processes[0]->mcs)==2)
    lte_ue_pdsch_vars[eNB_id]->log2_maxh = (log2_approx(avgs)/2) ;// + 2
  else
    lte_ue_pdsch_vars[eNB_id]->log2_maxh = (log2_approx(avgs)/2); // + 5;// + 2

  mch_channel_compensation(lte_ue_pdsch_vars[eNB_id]->rxdataF_ext,
			   lte_ue_pdsch_vars[eNB_id]->dl_ch_estimates_ext,
			   lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
			   lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
			   lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
			   frame_parms,
			   symbol,
			   get_Qm(dlsch_ue[0]->harq_processes[0]->mcs),
			   lte_ue_pdsch_vars[eNB_id]->log2_maxh);


  if (frame_parms->nb_antennas_rx > 1)
    mch_detection_mrc(frame_parms,
		      lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
		      lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
		      lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
		      symbol);

    switch (get_Qm(dlsch_ue[0]->harq_processes[0]->mcs)) {
    case 2 : 
      mch_qpsk_llr(frame_parms,
		   lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
		   lte_ue_pdsch_vars[eNB_id]->llr[0],
		   symbol,
		   lte_ue_pdsch_vars[eNB_id]->llr128);
      break;
    case 4:
      mch_16qam_llr(frame_parms,
		    lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
		    lte_ue_pdsch_vars[eNB_id]->llr[0],
		    lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
		    symbol,
		    lte_ue_pdsch_vars[eNB_id]->llr128);
      break;
    case 6:
      mch_64qam_llr(frame_parms,
		    lte_ue_pdsch_vars[eNB_id]->rxdataF_comp,
		    lte_ue_pdsch_vars[eNB_id]->llr[0],
		    lte_ue_pdsch_vars[eNB_id]->dl_ch_mag,
		    lte_ue_pdsch_vars[eNB_id]->dl_ch_magb,
		    symbol,
		    lte_ue_pdsch_vars[eNB_id]->llr128);
      break;
    }
    return(0);
}

