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
	    free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 768);
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

LTE_UE_DLSCH_t *new_ue_dlsch(unsigned char Kmimo,unsigned char Mdlharq,u8 abstraction_flag) {

  LTE_UE_DLSCH_t *dlsch;
  unsigned char exit_flag = 0,i,r;

  dlsch = (LTE_UE_DLSCH_t *)malloc16(sizeof(LTE_UE_DLSCH_t));
  if (dlsch) {
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      //      msg("new_ue_dlsch: Harq process %d\n",i);
      dlsch->harq_processes[i] = (LTE_DL_UE_HARQ_t *)malloc16(sizeof(LTE_DL_UE_HARQ_t));
      if (dlsch->harq_processes[i]) {
	dlsch->harq_processes[i]->b = (unsigned char*)malloc16(MAX_DLSCH_PAYLOAD_BYTES);
	if (abstraction_flag == 0) {
	  if (!dlsch->harq_processes[i]->b)
	    exit_flag=3;
	  for (r=0;r<MAX_NUM_DLSCH_SEGMENTS;r++) {
	    dlsch->harq_processes[i]->c[r] = (unsigned char*)malloc16(((r==0)?8:0) + 768);	
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
  msg("new_ue_dlsch: exit_flag = %d\n",exit_flag);
  free_ue_dlsch(dlsch);

  return(NULL);
}

unsigned int  dlsch_decoding(short *dlsch_llr,
			     LTE_DL_FRAME_PARMS *frame_parms,
			     LTE_UE_DLSCH_t *dlsch,
			     unsigned char subframe,
			     u8 num_pdcch_symbols){
  
  

  unsigned short nb_rb;
  unsigned char harq_pid;
  unsigned int A,E;
  unsigned char mod_order;
  unsigned int G,i;
  unsigned int ret,offset;
  unsigned short iind;
  //  unsigned char dummy_channel_output[(3*8*block_length)+12];
  short coded_bits=0;
  short dummy_w[8][3*(6144+64)];
  unsigned int r,r_offset=0,Kr,Kr_bytes,err_flag=0;
  unsigned char crc_type;

  if (!dlsch_llr) {
    msg("dlsch_decoding.c: NULL dlsch_llr pointer\n");
    return(MAX_TURBO_ITERATIONS);
  }

  if (!dlsch) {
    msg("dlsch_decoding.c: NULL dlsch pointer\n");
    return(MAX_TURBO_ITERATIONS);
  }

  if (!frame_parms) {
    msg("dlsch_decoding.c: NULL frame_parms pointer\n");
    return(MAX_TURBO_ITERATIONS);
  }

  if (subframe>9) {
    msg("dlsch_decoding.c: Illegal subframe index %d\n",subframe);
    return(MAX_TURBO_ITERATIONS);
  }

  nb_rb = dlsch->nb_rb;


  if (nb_rb > 25) {
    msg("dlsch_decoding.c: Illegal nb_rb %d\n",nb_rb);
    return(MAX_TURBO_ITERATIONS);
  }

  harq_pid = dlsch->current_harq_pid;
  if (harq_pid >= 8) {
    msg("dlsch_decoding.c: Illegal harq_pid %d\n",harq_pid);
    return(MAX_TURBO_ITERATIONS);
  }

  A = dlsch->harq_processes[harq_pid]->TBS;

#ifndef USER_MODE
  if (A > 6144) {
    msg("dlsch_decoding.c: Illegal TBS %d\n",A);
    return(MAX_TURBO_ITERATIONS);
  }
#endif

  mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);

  ret = MAX_TURBO_ITERATIONS;


  G = get_G(frame_parms,nb_rb,dlsch->rb_alloc,mod_order,num_pdcch_symbols,subframe);



  //  msg("DLSCH Decoding, harq_pid %d Ndi %d\n",harq_pid,dlsch->harq_processes[harq_pid]->Ndi);

  if (dlsch->harq_processes[harq_pid]->Ndi == 1) {
    // This is a new packet, so compute quantities regarding segmentation
    dlsch->harq_processes[harq_pid]->B = A+24;
    lte_segmentation(NULL,
		     NULL,
		     dlsch->harq_processes[harq_pid]->B,
		     &dlsch->harq_processes[harq_pid]->C,
		     &dlsch->harq_processes[harq_pid]->Cplus,
		     &dlsch->harq_processes[harq_pid]->Cminus,
		     &dlsch->harq_processes[harq_pid]->Kplus,
		     &dlsch->harq_processes[harq_pid]->Kminus,		     
		     &dlsch->harq_processes[harq_pid]->F);
    //  CLEAR LLR's HERE for first packet in process
  }
  /*
  else {
    msg("dlsch_decoding.c: Ndi>0 not checked yet!!\n");
    return(MAX_TURBO_ITERATIONS);
  }
  */
  err_flag = 0;
  r_offset = 0;
  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {

    // Get Turbo interleaver parameters
    if (r<dlsch->harq_processes[harq_pid]->Cminus)
      Kr = dlsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = dlsch->harq_processes[harq_pid]->Kplus;
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
    msg("f1 %d, f2 %d, F %d\n",f1f2mat[2*iind],f1f2mat[1+(2*iind)],(r==0) ? dlsch->harq_processes[harq_pid]->F : 0);
#endif

    memset(&dummy_w[r][0],0,3*(6144+64)*sizeof(short));
    dlsch->harq_processes[harq_pid]->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       (char*) &dummy_w[r][0],
							       (r==0) ? dlsch->harq_processes[harq_pid]->F : 0);

#ifdef DEBUG_DLSCH_DECODING    
    msg("HARQ_PID %d Rate Matching Segment %d (coded bits %d,unpunctured/repeated bits %d, mod_order %d, nb_rb %d, Nl %d)...\n",
	harq_pid,r, G,
	   Kr*3,
	   mod_order,
	   nb_rb,
	   dlsch->harq_processes[harq_pid]->Nl);
#endif    

    if (lte_rate_matching_turbo_rx(dlsch->harq_processes[harq_pid]->RTC[r],
				   G,
				   dlsch->harq_processes[harq_pid]->w[r],
				   (unsigned char*)&dummy_w[r][0],
				   dlsch_llr,
				   dlsch->harq_processes[harq_pid]->C,
				   NSOFT,
				   dlsch->Mdlharq,
				   dlsch->Kmimo,
				   dlsch->harq_processes[harq_pid]->rvidx,
				   dlsch->harq_processes[harq_pid]->Ndi,
				   get_Qm(dlsch->harq_processes[harq_pid]->mcs),
				   dlsch->harq_processes[harq_pid]->Nl,
				   r,
				   &E)==-1) {
      msg("dlsch_decoding.c: Problem in rate_matching\n");
      return(MAX_TURBO_ITERATIONS);
    }
    r_offset += E;

    /*
    msg("Subblock deinterleaving, d %p w %p\n",
	   dlsch->harq_processes[harq_pid]->d[r],
	   dlsch->harq_processes[harq_pid]->w);
    */

    sub_block_deinterleaving_turbo(4+Kr, 
				   &dlsch->harq_processes[harq_pid]->d[r][96], 

				   dlsch->harq_processes[harq_pid]->w[r]); 

    
#ifdef DEBUG_DLSCH_DECODING    
    if (r==0) {
      write_output("decoder_llr.m","decllr",dlsch_llr,G,1,0);
      write_output("decoder_in.m","dec",&dlsch->harq_processes[harq_pid]->d[0][96],(3*8*Kr_bytes)+12,1,0);
    }

    msg("decoder input(segment %d) :",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      msg("%d : %d\n",i,dlsch->harq_processes[harq_pid]->d[r][96+i]);
    msg("\n");
#endif
    

    //    msg("Clearing c, %p\n",dlsch->harq_processes[harq_pid]->c[r]);
    memset(dlsch->harq_processes[harq_pid]->c[r],0,Kr_bytes);
    //    msg("done\n");
    if (dlsch->harq_processes[harq_pid]->C == 1) 
      crc_type = CRC24_A;
    else 
      crc_type = CRC24_B;

    /*            
    msg("decoder input(segment %d)\n",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      if ((dlsch->harq_processes[harq_pid]->d[r][96+i]>7) || 
	  (dlsch->harq_processes[harq_pid]->d[r][96+i] < -8))
	msg("%d : %d\n",i,dlsch->harq_processes[harq_pid]->d[r][96+i]);
    msg("\n");
    */

    if (err_flag == 0) {
      ret = phy_threegpplte_turbo_decoder(&dlsch->harq_processes[harq_pid]->d[r][96],
					  dlsch->harq_processes[harq_pid]->c[r],
					  Kr,
					  f1f2mat[iind*2],   
					  f1f2mat[(iind*2)+1], 
					  MAX_TURBO_ITERATIONS,
					  crc_type,
					  (r==0) ? dlsch->harq_processes[harq_pid]->F : 0,
					  harq_pid+1);
    }

    if (ret>=(1+MAX_TURBO_ITERATIONS)) {// a Code segment is in error so break;
      //      msg("CRC failed\n");
      err_flag = 1;
    }

  }

  if (err_flag == 1) {
    dlsch->harq_ack[subframe].ack = 0;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    dlsch->harq_processes[harq_pid]->round++;
    //    msg("DLSCH: Setting NACK for subframe %d (pid %d, round %d)\n",subframe,harq_pid,dlsch->harq_processes[harq_pid]->round);
    if (dlsch->harq_processes[harq_pid]->round >= dlsch->Mdlharq) {
      dlsch->harq_processes[harq_pid]->status = SCH_IDLE;
    }
    
    return((1+MAX_TURBO_ITERATIONS));
  }
  else {
    dlsch->harq_processes[harq_pid]->status = SCH_IDLE;
    dlsch->harq_processes[harq_pid]->round  = 0;
    dlsch->harq_ack[subframe].ack = 1;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    //    msg("DLSCH decoding: Setting ACK for subframe %d (pid %d)\n",subframe,harq_pid);
  }
  // Reassembly of Transport block here
  offset = 0;
  /*
  msg("harq_pid %d\n",harq_pid);
  msg("F %d, Fbytes %d\n",dlsch->harq_processes[harq_pid]->F,dlsch->harq_processes[harq_pid]->F>>3);
  msg("C %d\n",dlsch->harq_processes[harq_pid]->C);
  */
  for (r=0;r<dlsch->harq_processes[harq_pid]->C;r++) {
    if (r<dlsch->harq_processes[harq_pid]->Cminus)
      Kr = dlsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = dlsch->harq_processes[harq_pid]->Kplus;

    Kr_bytes = Kr>>3;

    if (r==0) {
      memcpy(dlsch->harq_processes[harq_pid]->b,
	     &dlsch->harq_processes[harq_pid]->c[0][(dlsch->harq_processes[harq_pid]->F>>3)],
	     Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3));
      offset = Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3);
      //      msg("copied %d bytes to b sequence (harq_pid %d)\n",
      //	  Kr_bytes - (dlsch->harq_processes[harq_pid]->F>>3),harq_pid); 
    //      msg("b[0] = %x,c[%d] = %x\n",
      //	  dlsch->harq_processes[harq_pid]->b[0],
      //	  dlsch->harq_processes[harq_pid]->F>>3,
      //	  dlsch->harq_processes[harq_pid]->c[0][(dlsch->harq_processes[harq_pid]->F>>3)]);
    }
    else {
      memcpy(dlsch->harq_processes[harq_pid]->b+offset,
	     dlsch->harq_processes[harq_pid]->c[0],
	     Kr_bytes);
      offset += Kr_bytes;
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
#define MCS_COUNT 23

//extern  channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
//extern  double ABS_SINR_eff_BLER_table[MCS_COUNT][9][9];
//extern  double ABS_beta[MCS_COUNT];
double sinr_bler_map[MCS_COUNT][2][9];
double beta[MCS_COUNT] = {1, 1, 1, 1, 1, 0.9459960937499999, 1.2912109374999994, 1.0133789062499998, 1.000390625,
                          1.02392578125, 1.8595703124999998, 2.424389648437498, 2.3946533203124982, 2.5790039062499988,
                          2.4084960937499984, 2.782617187499999, 2.7868652343749996, 3.92099609375, 4.0392578125,
                          4.56109619140625, 5.03338623046875, 5.810888671875, 6.449108886718749};


int dlsch_abstraction(double* sinr_dB, u32 rb_alloc[4], u8 mcs) {

  int index;
  double sinr_eff = 0;
  int rb_count = 0;
  int offset;
  double bler = 0;
  for (offset = 0; offset <= 24; offset++) {
    if (rb_alloc[0] & (1<<offset)) {
      rb_count++;
      sinr_eff += exp(-(pow(10, 0.1*(sinr_dB[offset*2])))/beta[mcs]);
      //printf("sinr_eff1 = %f, power %lf\n",sinr_eff, exp(-pow(10,6.8)));

      sinr_eff += exp(-(pow(10, (sinr_dB[offset*2+1])/10))/beta[mcs]);
      //printf("sinr_dB[%d]=%f\n",offset,sinr_dB[offset*2]);
    }
  }       
  //printf("sinr_eff1 = %f\n",sinr_eff);
  sinr_eff =  -beta[mcs] *log((sinr_eff)/(2*rb_count));
  sinr_eff = 10 * log10(sinr_eff);
  msg("sinr_eff2 = %f\n",sinr_eff);

  // table lookup
  sinr_eff *= 10;
  sinr_eff = floor(sinr_eff);
  if ((int)sinr_eff%2) {
    sinr_eff += 1;
  }
  sinr_eff /= 10;
  msg("sinr_eff after rounding = %f\n",sinr_eff);
  for (index = 0; index < 9; index++) {
    if(index == 0) {
      if (sinr_eff < sinr_bler_map[mcs][0][index]) {
        bler = 1;
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

u32 dlsch_decoding_emul(PHY_VARS_UE *phy_vars_ue,
			u8 subframe,
			u8 dlsch_id,
			u8 eNB_id) {

  LTE_UE_DLSCH_t *dlsch_ue;
  LTE_eNB_DLSCH_t *dlsch_eNB;
  u8 harq_pid;
  u8 eNB_id2,i;

  for (eNB_id2=0;eNB_id2<NB_eNB_INST;eNB_id2++) {
    if (PHY_vars_eNB_g[eNB_id2]->lte_frame_parms.Nid_cell == phy_vars_ue->lte_frame_parms.Nid_cell)
      break;
  }
  if (eNB_id2==NB_eNB_INST) {
    msg("phy_procedures_lte_ue.c: FATAL : Could not find attached eNB for DLSCH emulation !!!!\n");
    mac_xface->macphy_exit("");
  }

  msg("[PHY] EMUL UE dlsch_decoding_emul : subframe %d, eNB_id %d, dlsch_id %d\n",subframe,eNB_id2,dlsch_id);

  //  printf("dlsch_eNB_ra->harq_processes[0] %p\n",PHY_vars_eNB_g[eNB_id]->dlsch_eNB_ra->harq_processes[0]);


  switch (dlsch_id) {
  case 0: // SI
    dlsch_ue = phy_vars_ue->dlsch_ue_SI[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB_SI;
    msg("Doing SI: TBS %d\n",dlsch_ue->harq_processes[0]->TBS>>3);
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
    for (i=0;i<dlsch_ue->harq_processes[0]->TBS>>3;i++)
      msg("%x.",dlsch_eNB->harq_processes[0]->b[i]);
    msg("\n");
    break;
  case 1: // RA
    dlsch_ue  = phy_vars_ue->dlsch_ue_ra[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB_ra;
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
    break;
  case 2: // TB0
    dlsch_ue  = phy_vars_ue->dlsch_ue[eNB_id][0];
    harq_pid = dlsch_ue->current_harq_pid;
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB[find_ue((s16)phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,PHY_vars_eNB_g[eNB_id2])][0];
    for (i=0;i<dlsch_ue->harq_processes[0]->TBS>>3;i++)
      msg("%x.",dlsch_eNB->harq_processes[0]->b[i]);
    msg("\n");

    if (dlsch_abstraction(phy_vars_ue->sinr_dB, dlsch_eNB->rb_alloc, dlsch_eNB->harq_processes[harq_pid]->mcs) == 1) {
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
      return(1+MAX_TURBO_ITERATIONS);
    }

    break;
  case 3: // TB1
    dlsch_ue = phy_vars_ue->dlsch_ue[eNB_id][1];
    harq_pid = dlsch_ue->current_harq_pid;
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2]->dlsch_eNB[find_ue((s16)phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,PHY_vars_eNB_g[eNB_id2])][1];
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
    msg("dlsch_decoding_emul: FATAL, unknown DLSCH_id %d\n",dlsch_id);
    return(1+MAX_TURBO_ITERATIONS);
  }

}
#endif
