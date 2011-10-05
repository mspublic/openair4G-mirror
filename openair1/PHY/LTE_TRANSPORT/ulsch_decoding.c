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
#include "UTIL/OCG/OCG.h"
#include "UTIL/OCG/OCG_extern.h"
#endif
//#define DEBUG_ULSCH_DECODING

void free_eNB_ulsch(LTE_eNB_ULSCH_t *ulsch) {

  int i,r;

  if (ulsch) {
    for (i=0;i<ulsch->Mdlharq;i++) {
      if (ulsch->harq_processes[i]) {
	if (ulsch->harq_processes[i]->b)
	  free16(ulsch->harq_processes[i]->b,MAX_ULSCH_PAYLOAD_BYTES);
	if (ulsch->harq_processes[i]->c) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	    free16(ulsch->harq_processes[i]->c[r],((r==0)?8:0) + 768);
	}
	for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++)
	  if (ulsch->harq_processes[i]->d[r])
	    free16(ulsch->harq_processes[i]->d[r],((3*8*6144)+12+96)*sizeof(short));
	free16(ulsch->harq_processes[i],sizeof(LTE_UL_eNB_HARQ_t));
      }
    }
  free16(ulsch,sizeof(LTE_eNB_ULSCH_t));
  }
}

LTE_eNB_ULSCH_t *new_eNB_ulsch(u8 Mdlharq,u8 abstraction_flag) {

  LTE_eNB_ULSCH_t *ulsch;
  u8 exit_flag = 0,i,r;

  ulsch = (LTE_eNB_ULSCH_t *)malloc16(sizeof(LTE_eNB_ULSCH_t));
  if (ulsch) {
    ulsch->Mdlharq = Mdlharq;

    for (i=0;i<Mdlharq;i++) {
      //      msg("new_ue_ulsch: Harq process %d\n",i);
      ulsch->harq_processes[i] = (LTE_UL_eNB_HARQ_t *)malloc16(sizeof(LTE_UL_eNB_HARQ_t));
      if (ulsch->harq_processes[i]) {
	ulsch->harq_processes[i]->b = (u8*)malloc16(MAX_ULSCH_PAYLOAD_BYTES);
	if (!ulsch->harq_processes[i]->b)
	  exit_flag=3;
	if (abstraction_flag==0) {
	  for (r=0;r<MAX_NUM_ULSCH_SEGMENTS;r++) {
	    ulsch->harq_processes[i]->c[r] = (u8*)malloc16(((r==0)?8:0) + 768);	
	    if (!ulsch->harq_processes[i]->c[r])
	      exit_flag=2;
	    ulsch->harq_processes[i]->d[r] = (short*)malloc16(((3*8*6144)+12+96)*sizeof(short));
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
  msg("new_ue_ulsch: exit_flag = %d\n",exit_flag);
  free_eNB_ulsch(ulsch);

  return(NULL);
}

void clean_eNb_ulsch(LTE_eNB_ULSCH_t *ulsch, u8 abstraction_flag) {

  unsigned char Mdlharq;
  unsigned char exit_flag = 0,i,r;

  ulsch = (LTE_eNB_ULSCH_t *)malloc16(sizeof(LTE_eNB_ULSCH_t));
  if (ulsch) {
    Mdlharq = ulsch->Mdlharq;
    ulsch->rnti = 0;
    for (i=0;i<Mdlharq;i++) {
      if (ulsch->harq_processes[i]) {
	  ulsch->harq_processes[i]->Ndi = 0;
	  ulsch->harq_processes[i]->status = 0;
	  ulsch->harq_processes[i]->subframe_scheduling_flag = 0;
	  ulsch->harq_processes[i]->phich_active = 0;
	  ulsch->harq_processes[i]->phich_ACK = 0;
	  ulsch->harq_processes[i]->round = 0;
      }
    }

  }
}


u8 extract_cqi_crc(u8 *cqi,u8 CQI_LENGTH) {

  u8 crc;

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

short dummy_w[8][3*(6144+64)];
u8 dummy_w_cc[3*(MAX_CQI_BITS+8+32)];
short y[6*14*1200];
char ytag[14*1200];

unsigned int  ulsch_decoding(s16 *ulsch_llr,
			     LTE_DL_FRAME_PARMS *frame_parms,
			     LTE_eNB_ULSCH_t *ulsch,
			     u8 subframe,
			     u8 control_only_flag) {


  u8 harq_pid;
  unsigned short nb_rb;
  unsigned int A,E;
  u8 Q_m;
  unsigned int i,q,j;
  int iprime;
  unsigned int ret,offset;
  unsigned short iind;
  //  u8 dummy_channel_output[(3*8*block_length)+12];
  short coded_bits=0;
  unsigned int r,r_offset=0,Kr,Kr_bytes;
  u8 crc_type;
  u8 *columnset;
  unsigned int sumKr=0;
  unsigned int Qprime,L,G,Q_CQI,Q_RI,Q_ACK,H,Hprime,Hpp,Cmux,Rmux,Rmux_prime,O_RCC;
  unsigned int Qprime_ACK,Qprime_CQI,Qprime_RI,len_ACK,len_RI;
  int metric,metric_new;
  u8 o_flip[8];

  //  harq_pid = (ulsch->RRCConnRequest_flag == 0) ? subframe2harq_pid_tdd(frame_parms->tdd_config,subframe) : 0;
  harq_pid = subframe2harq_pid(frame_parms,subframe);

  if (harq_pid==255) {
    msg("ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n");
    return(-1);
  }
  
  nb_rb = ulsch->harq_processes[harq_pid]->nb_rb;
  if (nb_rb>25) {
    msg("ulsch_decoding.c: FATAL ERROR: illegal nb_rb %d\n",nb_rb);
    return(-1);
  }
  A = ulsch->harq_processes[harq_pid]->TBS;

  if (A > 2*6144) {
    msg("ulsch_decoding.c: FATAL ERROR: illegal TBS %d\n",A);
    return(-1);
  }

    
  Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);
  G = nb_rb * (12 * Q_m) * ulsch->Nsymb_pusch;

#ifdef DEBUG_ULSCH_DECODING
  msg("ulsch_decoding: Ndi %d, RV %d, mcs %d, O_RI %d\n",
      ulsch->harq_processes[harq_pid]->Ndi,
      ulsch->harq_processes[harq_pid]->rvidx,
      ulsch->harq_processes[harq_pid]->mcs,
      ulsch->O_RI);
#endif  

  if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
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
  // Compute Q_ri
  Qprime = ulsch->O_RI*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;

  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > 4*nb_rb * 12)
    Qprime = 4*nb_rb * 12;

  Q_RI = Q_m*Qprime;
  Qprime_RI = Qprime;


  // Compute Q_ack

  Qprime = ulsch->O_ACK*ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  if (Qprime > (4*nb_rb * 12))
    Qprime = 4*nb_rb * 12;

  Q_ACK = Qprime * Q_m;
  Qprime_ACK = Qprime;

  // Compute Q_cqi
  L=8;
  Qprime = (ulsch->Or1 + L) * ulsch->harq_processes[harq_pid]->Msc_initial*ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;
  if ((Qprime % (8*sumKr)) > 0)
    Qprime = 1+(Qprime/(8*sumKr));
  else
    Qprime = Qprime/(8*sumKr);

  G = nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);


  if (Qprime > (G - ulsch->O_RI))
    Qprime = G - ulsch->O_RI;

  Q_CQI = Q_m * Qprime;
  Qprime_CQI = Qprime;

  G = G - Q_RI - Q_CQI;

  if ((int)G < 0) {
    msg("[PHY] FATAL: ulsch_decoding.c G < 0 (%d) : Q_RI %d, Q_CQI %d\n",G,Q_RI,Q_CQI);
    return(-1);
  }

  H = G + Q_CQI;
  Hprime = H/Q_m;

  // Demultiplexing/Deinterleaving of PUSCH/ACK/RI/CQI
  Hpp = Hprime + Qprime_RI;
  
  Cmux       = ulsch->Nsymb_pusch;
  Rmux       = Hpp*Q_m/Cmux;
  Rmux_prime = Rmux/Q_m;
  

  // Clear "tag" interleaving matrix to allow for CQI/DATA identification
  memset(ytag,0,Cmux*Rmux_prime);


  // read in buffer
  j=0;
  for (i=0;i<Cmux;i++)
   for (r=0;r<Rmux_prime;r++)
      for (q=0;q<Q_m;q++) {
	y[q+(Q_m*((r*Cmux)+i))] = ulsch_llr[j++];
	//		msg("y[%d] = %d\n",q+(Q_m*((r*Cmux)+i)),y[q+(Q_m*((r*Cmux)+i))]);
      }
  if (j!=(H+Q_RI))
    msg("ulsch_coding.c: Error in input buffer length (j %d, H+Q_RI %d)\n",j,H+Q_RI); 

  // HARQ-ACK Bits (LLRs are nulled in overwritten bits after copying HARQ-ACK LLR)
  r = Rmux_prime-1;

  if (frame_parms->Ncp == 0)
    columnset = cs_ack_normal;
  else
    columnset = cs_ack_extended;

  j=0;

  if (ulsch->O_ACK == 1) {
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
  if (ulsch->O_ACK == 2) {
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
  else{
    msg("ulsch_coding: FATAL, ACK cannot be more than 2 bits yet\n");
    return(-1);
  }

  for (i=0;i<len_ACK;i++)
    ulsch->q_ACK[i] = 0;


  for (i=0;i<Qprime_ACK;i++) {
    for (q=0;q<Q_m;q++) {
      if (y[q+(Q_m*((r*Cmux) + columnset[j]))]!=0)
	ulsch->q_ACK[(q+(Q_m*i))%len_ACK] += y[q+(Q_m*((r*Cmux) + columnset[j]))];
      //            msg("ACK %d => %d (%d,%d,%d)\n",(q+(Q_m*i))%len_ACK,ulsch->q_ACK[(q+(Q_m*i))%len_ACK],q+(Q_m*((r*Cmux) + columnset[j])),r,columnset[j]);
      y[q+(Q_m*((r*Cmux) + columnset[j]))]=0;  // NULL LLRs in ACK positions
    }
    j=(j+3)&3;
    r = Rmux_prime -1 - (i>>2);
  }


  // RI BITS 
  r = Rmux_prime-1;

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
  else {
    msg("ulsch_decoding: FATAL, RI cannot be more than 1 bit yet\n");
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
    for (q=0;q<Q_m;q++) 
      ulsch->q_RI[(q+(Q_m*i))%len_RI] += y[q+(Q_m*((r*Cmux) + columnset[j]))];
    ytag[(r*Cmux) + columnset[j]] = LTE_NULL;
    j=(j+3)&3;
    r = Rmux_prime -1 - (i>>2);
  }

  // CQI and Data bits
  j=0;
  for (i=0,iprime=-Qprime_CQI;i<Hprime;i++,iprime++) {
    while (ytag[j]==LTE_NULL)
      j++;
    if (i<Qprime_CQI) {
      
      for (q=0;q<Q_m;q++) {
	
	if (y[q+(Q_m*j)]>127)
	  ulsch->q[q+(Q_m*i)] = 127;
	else if (y[q+(Q_m*j)]<-128)
	  ulsch->q[q+(Q_m*i)] = -128;
	else 
	  ulsch->q[q+(Q_m*i)] = y[q+(Q_m*j)];
	
	//		msg("CQI %d, y[%d] %d\n",q+(Q_m*i),q+(Q_m*j),y[q+(Q_m*j)]);
      }
    } 
    else {
      for (q=0;q<Q_m;q++) {
	ulsch->e[q+(Q_m*iprime)] = y[q+(Q_m*j)];
	//		msg("e %d, y[%d] %d\n",q+(Q_m*iprime),q+(Q_m*j),ulsch->e[q+(Q_m*iprime)]);
      }
    }
    
    j++;
  }


  // Do CQI/RI/HARQ-ACK Decoding first and pass to MAC

  // HARQ-ACK 
  if (ulsch->O_ACK == 1) {
      ulsch->o_ACK[0] = ((ulsch->q_ACK[0] + ulsch->q_ACK[Q_m/2]) > 0) ? 1 : 0; 
  }
  if (ulsch->O_ACK == 2) {
    switch (Q_m) {
    case 2:
      ulsch->q_ACK[0] += ulsch->q_ACK[3];
      ulsch->q_ACK[1] += ulsch->q_ACK[4];
      ulsch->q_ACK[2] += ulsch->q_ACK[5];

      break;
    case 4:
      ulsch->q_ACK[0] += ulsch->q_ACK[6];
      ulsch->q_ACK[1] = ulsch->q_ACK[2] + ulsch->q_ACK[8];
      ulsch->q_ACK[2] = ulsch->q_ACK[4] + ulsch->q_ACK[10];

      break;
    case 6:
      ulsch->q_ACK[0] += ulsch->q_ACK[9];
      ulsch->q_ACK[1] =  ulsch->q_ACK[3] + ulsch->q_ACK[12]; 
      ulsch->q_ACK[2] =  ulsch->q_ACK[6] + ulsch->q_ACK[15]; 
      break;
    }
    ulsch->o_ACK[0] = 0;
    ulsch->o_ACK[1] = 0;
    metric     = -ulsch->q_ACK[0]-ulsch->q_ACK[1]-ulsch->q_ACK[2];
    metric_new = ulsch->q_ACK[0]-ulsch->q_ACK[1]+ulsch->q_ACK[2];

    if (metric_new > metric) {
      ulsch->o_ACK[0]=1;
      ulsch->o_ACK[1]=0;
      metric = metric_new;
    }
    metric_new = -ulsch->q_ACK[0]+ulsch->q_ACK[1]+ulsch->q_ACK[2];


    if (metric_new > metric) {
      ulsch->o_ACK[0] = 0;
      ulsch->o_ACK[1] = 1;
      metric = metric_new;
    }
    metric_new = ulsch->q_ACK[0]+ulsch->q_ACK[1]-ulsch->q_ACK[2];

    if (metric_new > metric) {
      ulsch->o_ACK[0] = 1;
      ulsch->o_ACK[0] = 1;
      metric = metric_new;
    }
  }
  else{
    msg("ulsch_decoding: FATAL, ACK cannot be more than 2 bits yet\n");
    return(-1);
  }

#ifdef DEBUG_ULSCH_DECODING
  for (i=0;i<ulsch->O_ACK;i++)
    msg("ulsch_decoding: O_ACK[%d] %d\n",i,ulsch->o_ACK[i]);
#endif

  // RI

  if (ulsch->O_RI == 1) {
      ulsch->o_RI[0] = ((ulsch->q_RI[0] + ulsch->q_RI[Q_m/2]) > 0) ? 1 : 0; 
  }
  else {
    msg("ulsch_decoding: FATAL, RI cannot be more than 1 bit yet\n");
    return(-1);
  }
#ifdef DEBUG_ULSCH_DECODING

  for (i=0;i<2*ulsch->O_RI;i++)
    msg("ulsch_decoding: q_RI[%d] %d\n",i,ulsch->q_RI[i]);

  for (i=0;i<ulsch->O_RI;i++)
    msg("ulsch_decoding: O_RI[%d] %d\n",i,ulsch->o_RI[i]);
#endif


  // CQI

  memset((void *)&dummy_w_cc[0],0,3*(ulsch->Or1+8+32));

  O_RCC = generate_dummy_w_cc(ulsch->Or1+8,
			      (char*) &dummy_w_cc[0]);
  


  lte_rate_matching_cc_rx(O_RCC,
			  Q_CQI,
			  ulsch->o_w,
			  dummy_w_cc,
			  ulsch->q);

  sub_block_deinterleaving_cc((unsigned int)(ulsch->Or1+8), 
			      &ulsch->o_d[96], 
			      &ulsch->o_w[0]); 
 
  memset(o_flip,0,1+(ulsch->Or1/8));
  phy_viterbi_lte_sse2(ulsch->o_d+96,o_flip,8+ulsch->Or1);

  if (extract_cqi_crc(o_flip,ulsch->Or1) == (crc8(o_flip,ulsch->Or1)>>24))
    ulsch->cqi_crc_status = 1;
  else
    ulsch->cqi_crc_status = 0;

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
  msg("ulsch_decoding: Or1=%d\n",ulsch->Or1);
  for (i=0;i<1+((8+ulsch->Or1)/8);i++)
    msg("ulsch_decoding: O[%d] %d\n",i,ulsch->o[i]);
  if (ulsch->cqi_crc_status == 1)
    msg("RX CQI CRC OK (%x)\n",crc8(ulsch->o,ulsch->Or1)>>24);
  else
    msg("RX CQI CRC NOT OK (%x)\n",crc8(ulsch->o,ulsch->Or1)>>24);
#endif
  

  //  return(0);
  // Do PUSCH Decoding

  
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
      msg("ulsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      return(-1);
    }
    
#ifdef DEBUG_ULSCH_DECODING     
    msg("f1 %d, f2 %d, F %d\n",f1f2mat[2*iind],f1f2mat[1+(2*iind)],(r==0) ? ulsch->harq_processes[harq_pid]->F : 0);
#endif
    
    memset(&dummy_w[r][0],0,3*(6144+64)*sizeof(short));
    ulsch->harq_processes[harq_pid]->RTC[r] = generate_dummy_w(4+(Kr_bytes*8), 
							       (char*)&dummy_w[r][0],
							       (r==0) ? ulsch->harq_processes[harq_pid]->F : 0);

#ifdef DEBUG_ULSCH_DECODING    
    msg("Rate Matching Segment %d (coded bits (G) %d,unpunctured/repeated bits %d, Q_m %d, nb_rb %d, Nl %d)...\n",
	   r, G,
	   Kr*3,
	   Q_m,
	   nb_rb,
	   ulsch->harq_processes[harq_pid]->Nl);
#endif    

    if (lte_rate_matching_turbo_rx(ulsch->harq_processes[harq_pid]->RTC[r],
				   G,
				   ulsch->harq_processes[harq_pid]->w[r],
				   (u8*) &dummy_w[r][0],
				   ulsch->e,
				   ulsch->harq_processes[harq_pid]->C,
				   NSOFT,
				   ulsch->Mdlharq,
				   1,
				   ulsch->harq_processes[harq_pid]->rvidx,
				   ulsch->harq_processes[harq_pid]->Ndi,
				   get_Qm(ulsch->harq_processes[harq_pid]->mcs),
				   1,
				   r,
				   &E)==-1) {
      msg("ulsch_decoding.c: Problem in rate matching\n");
      return(-1);
    }
    r_offset += E;
    /*
    msg("Subblock deinterleaving, d %p w %p\n",
	   ulsch->harq_processes[harq_pid]->d[r],
	   ulsch->harq_processes[harq_pid]->w);
    */

    sub_block_deinterleaving_turbo(4+Kr, 
				   &ulsch->harq_processes[harq_pid]->d[r][96], 
				   ulsch->harq_processes[harq_pid]->w[r]); 

    /*    
#ifdef DEBUG_ULSCH_DECODING    
    msg("decoder input(segment %d) :",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      msg("%d : %d\n",i,ulsch->harq_processes[harq_pid]->d[r][96+i]);
    msg("\n");
#endif
    */

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
    
    ret = phy_threegpplte_turbo_decoder(&ulsch->harq_processes[harq_pid]->d[r][96],
					ulsch->harq_processes[harq_pid]->c[r],
					Kr,
					f1f2mat[iind*2],   
					f1f2mat[(iind*2)+1], 
					MAX_TURBO_ITERATIONS,
					crc_type,
					(r==0) ? ulsch->harq_processes[harq_pid]->F : 0,
					harq_pid);
    
    if (ret==(1+MAX_TURBO_ITERATIONS)) {// a Code segment is in error so break;
#ifdef DEBUG_ULSCH_DECODING    
      msg("ULSCH harq_pid %d CRC failed\n",harq_pid);
#endif
      return(ret);
    }
#ifdef DEBUG_ULSCH_DECODING    
    else
      msg("ULSCH harq_pid %d CRC OK : %d iterations\n",harq_pid, ret);
#endif

  }
  // Reassembly of Transport block here
  offset = 0;
  //  msg("F %d, Fbytes %d\n",ulsch->harq_processes[harq_pid]->F,ulsch->harq_processes[harq_pid]->F>>3);
  
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    if (r<ulsch->harq_processes[harq_pid]->Cminus)
      Kr = ulsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;

    Kr_bytes = Kr>>3;
    
    if (r==0) {
      memcpy(ulsch->harq_processes[harq_pid]->b,
	     &ulsch->harq_processes[harq_pid]->c[0][(ulsch->harq_processes[harq_pid]->F>>3)],
	     Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3));
      offset = Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3);
      //            msg("copied %d bytes to b sequence\n",
      //      	     Kr_bytes - (ulsch->harq_processes[harq_pid]->F>>3));
    }
    else {
      memcpy(ulsch->harq_processes[harq_pid]->b+offset,
	     ulsch->harq_processes[harq_pid]->c[0],
	     Kr_bytes);
      offset += Kr_bytes;
    }
  }
  
  return(ret);
}

#ifdef PHY_ABSTRACTION
u32 ulsch_decoding_emul(PHY_VARS_eNB *phy_vars_eNB,
			u8 subframe,
			u8 UE_index) {

  u8 UE_id;
  u16 rnti,i;
  u8 harq_pid = subframe2harq_pid(&phy_vars_eNB->lte_frame_parms,subframe);
  
  rnti = phy_vars_eNB->ulsch_eNB[UE_index]->rnti;
#ifdef DEBUG_PHY
  msg("[PHY] EMUL RRC eNB %d ulsch_decoding_emul : subframe %d UE_index %d harq_pid %d rnti %x\n",phy_vars_eNB->Mod_id,subframe,UE_index,harq_pid,rnti);
#endif
  for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
    if (rnti == PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti)
      break;

    msg("[PHY] EMUL eNB %d ulsch_decoding_emul : subframe ue id %d crnti %x nb ue %d\n",
	phy_vars_eNB->Mod_id,
	UE_id,
	PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti,
	NB_UE_INST); 
  }

  if (UE_id==NB_UE_INST) {
    msg("ulsch_decoding_emul: FATAL, didn't find UE with rnti %x\n",rnti);
    return(1+MAX_TURBO_ITERATIONS);
  }
  else {
    msg("found UE with rnti %x => UE_id %d\n",rnti,UE_id);
  }
  // Do abstraction here to determine if packet it in error

  memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->harq_processes[harq_pid]->b,
	 PHY_vars_UE_g[UE_id]->ulsch_ue[0]->harq_processes[harq_pid]->b,
	 phy_vars_eNB->ulsch_eNB[UE_index]->harq_processes[harq_pid]->TBS>>3);
  // get local ue's ack 	 
  if ((UE_index >= oai_emulation.info.first_ue_local) ||(UE_index <(oai_emulation.info.first_ue_local+oai_emulation.info.nb_ue_local))){
    get_ack(&phy_vars_eNB->lte_frame_parms,
	    PHY_vars_UE_g[UE_id]->dlsch_ue[0][0]->harq_ack,
	    subframe,
	    phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK);
  }else { // get remote UEs' ack 
    phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[0] = PHY_vars_UE_g[UE_id]->ulsch_ue[0]->o_ACK[0];
    phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[1] = PHY_vars_UE_g[UE_id]->ulsch_ue[0]->o_ACK[1];
  }
  // Do abstraction of PUSCH feedback
  msg("[PHY] EMUL eNB %d : subframe %d : o_ACK %d %d\n",phy_vars_eNB->Mod_id,subframe,phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[0],phy_vars_eNB->ulsch_eNB[UE_index]->o_ACK[1]); 

  phy_vars_eNB->ulsch_eNB[UE_index]->Or1 = PHY_vars_UE_g[UE_id]->ulsch_ue[0]->O;
  phy_vars_eNB->ulsch_eNB[UE_index]->Or2 = PHY_vars_UE_g[UE_id]->ulsch_ue[0]->O;
  memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->o,PHY_vars_UE_g[UE_id]->ulsch_ue[0]->o,MAX_CQI_BYTES); 
  memcpy(phy_vars_eNB->ulsch_eNB[UE_index]->o_RI,PHY_vars_UE_g[UE_id]->ulsch_ue[0]->o_RI,2); 

  phy_vars_eNB->ulsch_eNB[UE_index]->cqi_crc_status = 1;
  return(1);
}
#endif
