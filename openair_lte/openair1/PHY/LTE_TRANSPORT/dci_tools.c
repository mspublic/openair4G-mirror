#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#ifdef DEBUG_DCI_TOOLS
#include "PHY/vars.h"
#endif

//#define DEBUG_DCI

unsigned int  localRIV2alloc_LUT25[512];
unsigned int  distRIV2alloc_LUT25[512];
unsigned short RIV2nb_rb_LUT25[512];
unsigned short RIV2first_rb_LUT25[512];
unsigned short RIV_max=0;

unsigned int conv_rballoc(unsigned char ra_header,unsigned int short rb_alloc) {

  unsigned int rb_alloc2=0,i,shift,subset;

  if (ra_header == 0) {// Type 0 Allocation

    for (i=0;i<12;i++) {
      if ((rb_alloc&(1<<i)) != 0)
	rb_alloc2 |= (3<<((2*i)));
      //      printf("rb_alloc2 (type 0) %x\n",rb_alloc2);
    }
    if ((rb_alloc&(1<<12)) != 0)
      rb_alloc2 |= (1<<24);
  }
  else {
    subset = rb_alloc&1;
    shift  = (rb_alloc>>1)&1;
    for (i=0;i<11;i++) {
      if ((rb_alloc&(1<<(i+2))) != 0)
	rb_alloc2 |= (1<<(2*i));
      //      printf("rb_alloc2 (type 1) %x\n",rb_alloc2);
    }
    if ((shift == 0) && (subset == 1))
      rb_alloc2<<=1;
    else if ((shift == 1) && (subset == 0))
      rb_alloc2<<=4;
    else if ((shift == 1) && (subset == 1))
      rb_alloc2<<=3;
  }

  return(rb_alloc2);
}

unsigned int conv_nprb(unsigned char ra_header,unsigned int short rb_alloc) {

  unsigned int nprb=0,i;

  if (ra_header == 0) {// Type 0 Allocation

    for (i=0;i<12;i++) {
      if ((rb_alloc&(1<<i)) != 0)
	nprb += 2;
    }
    if ((rb_alloc&(1<<12)) != 0)
      nprb += 1;
  }
  else {
    for (i=0;i<11;i++) {
      if ((rb_alloc&(1<<(i+2))) != 0)
	nprb += 1;
    }
  }

  return(nprb);
}

unsigned short computeRIV(unsigned short N_RB_DL,unsigned short RBstart,unsigned short Lcrbs) {

  unsigned short RIV;

  if (Lcrbs<=(1+(N_RB_DL>>1)))
    RIV = (N_RB_DL*(Lcrbs-1)) + RBstart;
  else
    RIV = (N_RB_DL*(N_RB_DL+1-Lcrbs)) + (N_RB_DL-1-RBstart);

  return(RIV);
}

void generate_RIV_tables() {

  // 25RBs localized RIV
  unsigned char Lcrbs,RBstart;
  unsigned char distpos;
  unsigned short RIV;
  unsigned int alloc,alloc_dist;

  for (RBstart=0;RBstart<25;RBstart++) {
    alloc = 0;
    alloc_dist = 0;
    for (Lcrbs=1;Lcrbs<=(25-RBstart);Lcrbs++) {
      //printf("RBstart %d, len %d --> ",RBstart,Lcrbs);
      alloc |= (1<<(RBstart+Lcrbs-1));
      // This is the RB<->VRB relationship for N_RB_DL=25
      distpos = ((RBstart+Lcrbs-1)*6)%23;
      if (distpos == 0)
	distpos = 23;
      alloc_dist |= (1<<distpos);
      
      RIV=computeRIV(25,RBstart,Lcrbs);
      if (RIV>RIV_max)
	RIV_max = RIV;

      //printf("RIV %d (%d)\n",RIV,localRIV2alloc_LUT25[RIV]);
      localRIV2alloc_LUT25[RIV] = alloc;
      distRIV2alloc_LUT25[RIV]  = alloc_dist;
      RIV2nb_rb_LUT25[RIV]      = Lcrbs;
      RIV2first_rb_LUT25[RIV]   = RBstart;
    }
  }
}

int generate_eNb_dlsch_params_from_dci(unsigned char subframe,
					void *dci_pdu,
					unsigned short rnti,
					DCI_format_t dci_format,
					LTE_eNb_DLSCH_t **dlsch,
					LTE_DL_FRAME_PARMS *frame_parms,
					unsigned short si_rnti,
					unsigned short ra_rnti,
					unsigned short p_rnti) {

  unsigned char harq_pid;
  unsigned short rballoc;
  unsigned char NPRB,tbswap,tpmi;
  LTE_eNb_DLSCH_t *dlsch0=NULL,*dlsch1;


  switch (dci_format) {

  case format0:   // This is an UL SACH allocation so nothing here, inform MAC
    return(-1);
    break;
  case format1A:  // This is DLSCH SACH allocation for control traffic

    // harq_pid field is reserved
    if ((rnti==si_rnti) || (rnti==ra_rnti) || (rnti==p_rnti)){  // 
      harq_pid=0;
      // see 36-212 V8.6.0 p. 45
      NPRB      = (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->TPC&1) + 2;
    }
    else {
      harq_pid  = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->harq_pid;
      if (harq_pid>8) {
	msg("dci_tools.c: ERROR: harq_pid > 8\n");
	return(-1);
      }
      rballoc = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc;
      if (rballoc>RIV_max) {
	msg("dci_tools.c: ERROR: rb_alloc > RIV_max\n");
	return(-1);
      }
      NPRB      = RIV2nb_rb_LUT25[rballoc];
    }

    if (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->vrb_type == 0)
      dlsch[0]->rb_alloc[0]                       = localRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    else
      dlsch[0]->rb_alloc[0]                       = distRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];

    dlsch[0]->nb_rb                               = RIV2nb_rb_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = (dlsch[0]->mode == 1) ? SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->ndi;
    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB];

    dlsch[0]->current_harq_pid = harq_pid;

    dlsch0 = dlsch[0];
    break;
  case format2_2A_L10PRB:

    return(-1);
    break;
  case format2_2A_M10PRB:
  
    harq_pid  = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>8) {
      msg("dci_tools.c: ERROR: harq_pid > 8\n");
      return(-1);
    }


    tbswap = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->tb_swap;
    if (tbswap == 0) {
      dlsch0 = dlsch[0];
      dlsch1 = dlsch[1];
    }
    else{
      dlsch0 = dlsch[1];
      dlsch1 = dlsch[0];
    }

    dlsch0->current_harq_pid = harq_pid;
    dlsch1->current_harq_pid = harq_pid;
    dlsch0->harq_ids[subframe] = harq_pid;
    dlsch1->harq_ids[subframe] = harq_pid;

    dlsch0->rb_alloc[0]                         = conv_rballoc(((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rah,
							       ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rballoc);

    dlsch1->rb_alloc[0]                         = dlsch0->rb_alloc[0];

    dlsch0->nb_rb                               = conv_nprb(((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rah,
							    ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->nb_rb                               = dlsch0->nb_rb;

    dlsch0->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs1;
    dlsch1->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs2;
    dlsch0->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rv1;
    dlsch1->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rv2;

    // check if either TB is disabled (see 36-213 V8.6 p. 26)
   

    if ((dlsch0->harq_processes[harq_pid]->rvidx == 1) && (dlsch0->harq_processes[harq_pid]->mcs == 0))
	dlsch0->harq_processes[harq_pid]->status = DISABLED;

    if ((dlsch1->harq_processes[harq_pid]->rvidx == 1) && (dlsch1->harq_processes[harq_pid]->mcs == 0))
	dlsch1->harq_processes[harq_pid]->status = DISABLED;
      
    dlsch0->harq_processes[harq_pid]->Nl        = 1;

    dlsch0->layer_index                         = tbswap;
    dlsch1->layer_index                         = 1-tbswap;

    // Fix this
    tpmi = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->tpmi;

    switch (tpmi) {
    case 0 : 
	dlsch0->harq_processes[harq_pid]->mimo_mode   = ALAMOUTI;
	break;
    case 1:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING11;
      return(-1);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      return(-1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      return(-1);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      return(-1);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      return(-1);
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      return(-1);
      break;
    }

    dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->ndi1;
    if (dlsch0->harq_processes[harq_pid]->Ndi == 1)
      dlsch0->harq_processes[harq_pid]->status = ACTIVE;
    dlsch0->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs1;
    if (dlsch0->nb_rb > 0)
#ifdef TBS_FIX
      dlsch0->harq_processes[harq_pid]->TBS         = 3*dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1]/4;
    dlsch0->harq_processes[harq_pid]->TBS = (dlsch0->harq_processes[harq_pid]->TBS>>3)<<3;
#else
      dlsch0->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1];
#endif 
    break;
  default:
    return(-1);
    break;
  }
#ifdef DEBUG_DCI
  if (dlsch0) {
    printf("dlsch0 eNB: NBRB     %d\n",dlsch0->nb_rb);
    printf("dlsch0 eNB: rballoc  %x\n",dlsch0->rb_alloc[0]);
    printf("dlsch0 eNB: harq_pid %d\n",harq_pid);
    printf("dlsch0 eNB: Ndi      %d\n",dlsch0->harq_processes[harq_pid]->Ndi);  
    printf("dlsch0 eNB: rvidx    %d\n",dlsch0->harq_processes[harq_pid]->rvidx);  
    printf("dlsch0 eNB: TBS      %d\n",dlsch0->harq_processes[harq_pid]->TBS);
    printf("dlsch0 eNB: mcs      %d\n",dlsch0->harq_processes[harq_pid]->mcs);
  }
#endif
  return(0);
}


int generate_ue_dlsch_params_from_dci(unsigned char subframe,
				      void *dci_pdu,
				      unsigned short rnti,
				      DCI_format_t dci_format,
				      LTE_UE_DLSCH_t **dlsch,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      unsigned short si_rnti,
				      unsigned short ra_rnti,
				      unsigned short p_rnti) {
  
  unsigned char harq_pid=0;
  unsigned short rballoc;
  unsigned char NPRB,tbswap,tpmi;
  LTE_UE_DLSCH_t *dlsch0=NULL,*dlsch1=NULL;

#ifdef DEBUG_DCI
  msg("dci_tools.c: Filling ue dlsch params -> rnti %x, dci_format %d\n",rnti,dci_format);
#endif
  switch (dci_format) {

  case format0:   // This is an UL SACH allocation so nothing here, inform MAC
    return(-1);
    break;
  case format1A:  // This is DLSCH SACH allocation for control traffic

    // harq_pid field is reserved
    if ((rnti==si_rnti) || (rnti==ra_rnti) || (rnti==p_rnti)){  // 
      harq_pid=0;
      // see 36-212 V8.6.0 p. 45
      NPRB      = (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->TPC&1) + 2;
    }
    else {
      harq_pid  = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->harq_pid;
      if (harq_pid>8) {
	msg("dci_tools.c: ERROR: harq_pid > 8\n");
	return(-1);
      }
      rballoc = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc;
      if (rballoc>RIV_max) {
	msg("dci_tools.c: ERROR: rb_alloc > RIV_max\n");
	return(-1);
      }
      NPRB      = RIV2nb_rb_LUT25[rballoc];
    }

    dlsch[0]->current_harq_pid = harq_pid;
    //    printf("Format 1A: harq_pid %d\n",harq_pid);
    if (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->vrb_type == 0)
      dlsch[0]->rb_alloc[0]                       = localRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    else
      dlsch[0]->rb_alloc[0]                       = distRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];

    dlsch[0]->nb_rb                               = RIV2nb_rb_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = dlsch[0]->mode1_flag == 1 ?SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->ndi;
    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB];


    dlsch0 = dlsch[0];
    break;
  case format2_2A_L10PRB:

    return(-1);
    break;
  case format2_2A_M10PRB:
  
    harq_pid  = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>8) {
      msg("dci_tools.c: ERROR: harq_pid > 8\n");
      return(-1);
    }
    dlsch[0]->current_harq_pid = harq_pid;
    dlsch[0]->harq_ack[subframe].harq_id = harq_pid;

    tbswap = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->tb_swap;
    if (tbswap == 0) {
      dlsch0 = dlsch[0];
      dlsch1 = dlsch[1];
    }
    else{
      dlsch0 = dlsch[1];
      dlsch1 = dlsch[0];
    }

    dlsch0->rb_alloc[0]                         = conv_rballoc(((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rah,
							       ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rballoc);

    dlsch1->rb_alloc[0]                         = dlsch0->rb_alloc[0];

    dlsch0->nb_rb                               = conv_nprb(((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rah,
							    ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->nb_rb                               = dlsch0->nb_rb;

    dlsch0->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs1;
    dlsch1->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs2;
    dlsch0->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rv1;
    dlsch1->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->rv2;

    // check if either TB is disabled (see 36-213 V8.6 p. 26)
   
    if ((dlsch0->harq_processes[harq_pid]->rvidx == 1) && (dlsch0->harq_processes[harq_pid]->mcs == 0)) {
	dlsch0->harq_processes[harq_pid]->status = DISABLED;
    }
    if ((dlsch1->harq_processes[harq_pid]->rvidx == 1) && (dlsch1->harq_processes[harq_pid]->mcs == 0)) {
	dlsch1->harq_processes[harq_pid]->status = DISABLED;
    }
    dlsch0->harq_processes[harq_pid]->Nl        = 1;

    dlsch0->layer_index                         = tbswap;
    dlsch1->layer_index                         = 1-tbswap;

    // Fix this
    tpmi = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->tpmi;

    switch (tpmi) {
    case 0 : 
	dlsch0->harq_processes[harq_pid]->mimo_mode   = ALAMOUTI;
      break;
    case 1:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING11;
      return(-1);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      return(-1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      return(-1);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      return(-1);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      return(-1);
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      return(-1);
      break;
    }


      dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->ndi1;
      if (dlsch0->harq_processes[harq_pid]->Ndi == 1)
	dlsch0->harq_processes[harq_pid]->status = ACTIVE;

      dlsch0->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs1;
      if (dlsch0->nb_rb>1) {
#ifdef TBS_FIX
	dlsch0->harq_processes[harq_pid]->TBS         = 3*dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1]/4;
      dlsch0->harq_processes[harq_pid]->TBS = (dlsch0->harq_processes[harq_pid]->TBS>>3)<<3;
#else
	dlsch0->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1];
#endif
      }
      else
	dlsch0->harq_processes[harq_pid]->TBS         =0;
      
      dlsch1->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->ndi2;
      if (dlsch1->harq_processes[harq_pid]->Ndi == 1)
	dlsch1->harq_processes[harq_pid]->status = ACTIVE;
      dlsch1->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs2;
      if (dlsch1->nb_rb>1) {
#ifdef TBS_FIX
	dlsch1->harq_processes[harq_pid]->TBS       = 3*dlsch_tbs25[dlsch1->harq_processes[harq_pid]->mcs][dlsch1->nb_rb-1]/4;
      dlsch1->harq_processes[harq_pid]->TBS         = (dlsch1->harq_processes[harq_pid]->TBS>>3)<<3;
#else
      dlsch1->harq_processes[harq_pid]->TBS       = dlsch_tbs25[dlsch1->harq_processes[harq_pid]->mcs][dlsch1->nb_rb-1];
#endif
      }
      else
	dlsch1->harq_processes[harq_pid]->TBS         = 0;
      break;
  default:

    return(-1);
    break;
  }

#ifdef DEBUG_DCI
  if (dlsch[0]) {
    printf("dlsch0 UE: NBRB     %d\n",dlsch[0]->nb_rb);
    printf("dlsch0 UE: rballoc  %x\n",dlsch[0]->rb_alloc[0]);
    printf("dlsch0 UE: harq_pid %d\n",harq_pid);
    printf("dlsch0 UE: Ndi      %d\n",dlsch[0]->harq_processes[harq_pid]->Ndi);  
    printf("dlsch0 UE: rvidx    %d\n",dlsch[0]->harq_processes[harq_pid]->rvidx);  
    printf("dlsch0 UE: TBS      %d\n",dlsch[0]->harq_processes[harq_pid]->TBS);
    printf("dlsch0 UE: mcs      %d\n",dlsch[0]->harq_processes[harq_pid]->mcs);
  }
#endif
  return(0);
}

/*
unsigned char subframe2harq_pid_tdd(unsigned char tdd_config,unsigned char subframe) {

  printf("subframe2harq_pid_tdd: tdd_config %d, subframe %d\n",tdd_config,subframe);

  switch (tdd_config) {

  case 2:
    return(subframe-3);
    break;
  case 3:
    if ((subframe>0) && (subframe<8)) {
      msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return((subframe>7) ? subframe-8 : subframe+3);
    break;
  case 4:
    if (subframe<8) {
      msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe-8);
    break;
  case 5:
    if ((subframe<8) || (subframe==9)) {
      msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe-8);
    break;
  default:
    msg("dci_tools.c: subframe2_harq_pid, Unsupported TDD mode\n");
    return(255);
    
  }
}
*/

unsigned char subframe2harq_pid_tdd(unsigned char tdd_config,unsigned char subframe) {

#ifdef DEBUG_DCI
  msg("dci_tools.c: subframe2_harq_pid_tdd, subframe %d for TDD mode %d\n",subframe,tdd_config);
#endif

  switch (tdd_config) {

  case 2:
    if ((subframe!=2) && (subframe!=7)) {
      msg("dci_tools.c: subframe2_harq_pid_tdd, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe/7);
    break;
  case 3:
    if ((subframe<2) || (subframe>4)) {
      msg("dci_tools.c: subframe2_harq_pid_tdd, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe-2);
    break;
  case 4:
    if ((subframe<2) || (subframe>3)) {
      msg("dci_tools.c: subframe2_harq_pid_tdd, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe-2);
    break;
  case 5:
    if (subframe!=2) {
      msg("dci_tools.c: subframe2_harq_pid_tdd, Illegal subframe %d for TDD mode %d\n",subframe,tdd_config);
      return(255);
    }
    return(subframe-2);
    break;
  default:
    msg("dci_tools.c: subframe2_harq_pid, Unsupported TDD mode\n");
    return(255);
    
  }
}


unsigned short quantize_subband_pmi(PHY_MEASUREMENTS *meas,unsigned char eNb_id) {

  unsigned char i;
  unsigned short pmiq;
  unsigned short pmivect = 0;
  unsigned char rank = meas->rank[eNb_id];
  int pmi;
  int pmi_re,pmi_im;

  for (i=0;i<NUMBER_OF_SUBBANDS;i++) {

    if (rank == 0) {
      pmi_re = meas->subband_pmi_re[eNb_id][i][meas->selected_rx_antennas[eNb_id][i]];
      pmi_im = meas->subband_pmi_im[eNb_id][i][meas->selected_rx_antennas[eNb_id][i]];

      if ((pmi_re > pmi_im) && (pmi_re > -pmi_im))
	pmiq = PMI_2A_11;
      else if ((pmi_re < pmi_im) && (pmi_re > -pmi_im))
	pmiq = PMI_2A_1j;
      else if ((pmi_re < pmi_im) && (pmi_re < -pmi_im))
	pmiq = PMI_2A_1m1;
      else if ((pmi_re > pmi_im) && (pmi_re < -pmi_im))
	pmiq = PMI_2A_1mj;
      pmivect |= (pmiq<<(2*i));
    }
    else {
      // This needs to be done properly!!!
      pmivect = 0;
    }
  }

  return(pmivect);
}

unsigned short quantize_wideband_pmi(PHY_MEASUREMENTS *meas,unsigned char eNb_id) {

  unsigned char i;
  unsigned short pmiq;
  unsigned short pmivect = 0;
  unsigned char rank = meas->rank[eNb_id];
  int pmi;
  int pmi_re,pmi_im;

  if (rank == 1) {
    pmi = 
    pmi_re = meas->wideband_pmi_re[eNb_id][meas->selected_rx_antennas[eNb_id][0]];
    pmi_im = meas->wideband_pmi_im[eNb_id][meas->selected_rx_antennas[eNb_id][0]];
    if ((pmi_re > pmi_im) && (pmi_re > -pmi_im))
      pmiq = PMI_2A_11;
    else if ((pmi_re < pmi_im) && (pmi_re > -pmi_im))
      pmiq = PMI_2A_1j;
    else if ((pmi_re < pmi_im) && (pmi_re < -pmi_im))
      pmiq = PMI_2A_1m1;
    else if ((pmi_re > pmi_im) && (pmi_re < -pmi_im))
      pmiq = PMI_2A_1mj;

  }
  else {
    // This needs to be done properly!
    pmiq = PMI_2A_11;
  }
  

  return(pmivect);
}

unsigned char sinr2cqi(int sinr) {

  int i;

  for (i=0;i<15;i++) {
    if (sinr < lte_cqi_snr_dB[i])
      return((i==0) ? 0 : (i-1));
  }
  return(i-1);
}

unsigned char fill_subband_cqi(PHY_MEASUREMENTS *meas,unsigned char eNb_id) {

  unsigned char i;
  unsigned short cqivect = 0;
  unsigned char rank = meas->rank[eNb_id];
  char diff_cqi;

  for (i=0;i<NUMBER_OF_SUBBANDS;i++) {

    diff_cqi = sinr2cqi(meas->wideband_cqi_dB[eNb_id][0]) - sinr2cqi(meas->subband_cqi_dB[eNb_id][i][0]);
    if (diff_cqi<=-1)
      diff_cqi = 3;
    else if (diff_cqi>2)
      diff_cqi = 2;
    cqivect |= (diff_cqi<<(2*i));
    
  }

  return(cqivect);
}

void fill_CQI(void *o,UCI_format fmt,PHY_MEASUREMENTS *meas,unsigned char eNb_id) {

  unsigned char rank;

  rank = meas->rank[eNb_id];

  switch (fmt) {

  case wideband_cqi:

    if (rank == 0) {
      ((wideband_cqi_rank1_2A_5MHz *)o)->cqi1 = sinr2cqi(meas->wideband_cqi_tot[eNb_id]);
      ((wideband_cqi_rank1_2A_5MHz *)o)->pmi  = quantize_subband_pmi(meas,eNb_id);
    }
    else { 
      ((wideband_cqi_rank2_2A_5MHz *)o)->cqi1 = sinr2cqi(meas->wideband_cqi_dB[eNb_id][0]);
      ((wideband_cqi_rank2_2A_5MHz *)o)->cqi2 = sinr2cqi(meas->wideband_cqi_dB[eNb_id][1]);
      ((wideband_cqi_rank2_2A_5MHz *)o)->pmi  = quantize_subband_pmi(meas,eNb_id);
    }
    break;
  case hlc_cqi:
    if (rank == 0) {
      ((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1     = sinr2cqi(meas->wideband_cqi_tot[eNb_id]);
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1 = fill_subband_cqi(meas,eNb_id);
      ((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi      = quantize_wideband_pmi(meas,eNb_id);
    }
    else {

      // This has to be improved!!!
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1     = sinr2cqi(meas->wideband_cqi_dB[eNb_id][0]);
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1 = fill_subband_cqi(meas,eNb_id);
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2     = sinr2cqi(meas->wideband_cqi_dB[eNb_id][0]);
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2 = fill_subband_cqi(meas,eNb_id);
      ((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi      = quantize_subband_pmi(meas,eNb_id);
    }
    break;
  case ue_selected:
    msg("dci_tools.c: fill_CQI ue_selected CQI not supported yet!!!\n");
    break;
  }
}





int generate_ue_ulsch_params_from_dci(void *dci_pdu,
				       unsigned short rnti,
				       unsigned char subframe,
				       DCI_format_t dci_format,
				       LTE_UE_ULSCH_t *ulsch,
				       PHY_MEASUREMENTS *meas,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       unsigned short si_rnti,
				       unsigned short ra_rnti,
				       unsigned short p_rnti,
				       unsigned char eNb_id) {
  
  unsigned char harq_pid;

#ifdef DEBUG_DCI
  msg("dci_tools.c: Filling ue ulsch params -> ulsch %p : rnti %x, dci_format %d,subframe %d\n",ulsch,rnti,dci_format,subframe);
#endif

  if (dci_format == format0) {


    if (rnti == ra_rnti)
      harq_pid = 0;
    else 
      harq_pid = subframe2harq_pid_tdd(3,(subframe+4)%10);
    //    printf("harq_pid = %d\n",harq_pid);

    if (harq_pid == 255) {
      msg("dci_tools.c: frame %d, subframe %d: FATAL ERROR: generate_ue_ulsch_params_from_dci, illegal harq_pid!\n",
	  mac_xface->frame, subframe);
      return(-1);
    }

    // indicate that this process is to be serviced in subframe n+4
    ulsch->harq_processes[harq_pid]->subframe_scheduling_flag = 1;

    ulsch->harq_processes[harq_pid]->TPC                                   = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->TPC;
    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->Ndi         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->ndi;
    if (ulsch->harq_processes[harq_pid]->Ndi == 1)
      ulsch->harq_processes[harq_pid]->status = ACTIVE;

    ulsch->O_RI                                  = 1;
    if (meas->rank[eNb_id] == 1) {
      ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
      ulsch->o_RI[0]                             = 1;
    }
    else {
      ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
      ulsch->o_RI[0]                             = 0;
    }


    fill_CQI(ulsch->o,wideband_cqi,meas,eNb_id);
    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10)) 
      print_CQI(ulsch->o,ulsch->o_RI,wideband_cqi,eNb_id);

    ulsch->O_ACK                                  = 2;

    ulsch->beta_offset_cqi_times8                  = 18;
    ulsch->beta_offset_ri_times8                   = 10;
    ulsch->beta_offset_harqack_times8              = 16;
    
    ulsch->Nsymb_pusch                             = 9;
    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs;
      ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      ulsch->harq_processes[harq_pid]->rvidx = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs - 28;
      ulsch->harq_processes[harq_pid]->round++;
    }
#ifdef DEBUG_DCI
    printf("ulsch (ue): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    printf("ulsch (ue): first_rb %x\n",ulsch->harq_processes[harq_pid]->first_rb);
    printf("ulsch (ue): harq_pid %d\n",harq_pid);
    printf("ulsch (ue): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
    printf("ulsch (ue): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
    printf("ulsch (ue): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
#endif
    return(0);
  }
  else {
    msg("dci_tools.c: frame %d, subframe %d: FATAL ERROR, generate_ue_ulsch_params_from_dci, Illegal dci_format %d\n",
	mac_xface->frame, subframe,dci_format);
    return(-1);
  }

}

int generate_eNb_ulsch_params_from_dci(void *dci_pdu,
				       unsigned short rnti,
				       unsigned char subframe,
				       DCI_format_t dci_format,
				       LTE_eNb_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       unsigned short si_rnti,
				       unsigned short ra_rnti,
				       unsigned short p_rnti) {
  
  unsigned char harq_pid;

  //printf("generate_eNb_ulsch_params_from_dci: subframe %d, rnti %x\n",subframe,rnti);
  if (dci_format == format0) {

    harq_pid = subframe2harq_pid_tdd(3,(subframe+4)%10);
    
    ulsch->harq_processes[harq_pid]->TPC                                   = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->TPC;
    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->Ndi         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->ndi;
    ulsch->Or2                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
    ulsch->Or1                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
    ulsch->O_RI                                   = 1;
    ulsch->O_ACK                                  = 2;
    ulsch->beta_offset_cqi_times8                = 18;
    ulsch->beta_offset_ri_times8                 = 10;
    ulsch->beta_offset_harqack_times8            = 16;

    ulsch->Nsymb_pusch                             = 9;

    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs;
      ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = 9;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      ulsch->harq_processes[harq_pid]->rvidx = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs - 28;
      ulsch->harq_processes[harq_pid]->round++;
    }
#ifdef DEBUG_DCI
    printf("ulsch (eNb): NBRB     %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    printf("ulsch (eNb): rballoc  %x\n",ulsch->harq_processes[harq_pid]->first_rb);
    printf("ulsch (eNb): harq_pid %d\n",harq_pid);
    printf("ulsch (eNb): Ndi      %d\n",ulsch->harq_processes[harq_pid]->Ndi);  
    printf("ulsch (eNb): TBS      %d\n",ulsch->harq_processes[harq_pid]->TBS);
    printf("ulsch (eNb): mcs      %d\n",ulsch->harq_processes[harq_pid]->mcs);
    printf("ulsch (eNb): Or1      %d\n",ulsch->Or1);
#endif
    return(0);
  }
  else {
    msg("dci_tools.c: FATAL ERROR, generate_eNb_ulsch_params_from_dci, Illegal dci_format %d\n",dci_format);
    return(-1);
  }

}


#ifdef DEBUG_DLSCH_TOOLS
 main() {
   
   int i;
   unsigned char rah;
   unsigned short rballoc;

   generate_RIV_tables();

   for (i=0;i<512;i++) {
     printf("RIV %d: nb_rb %d, alloc %x, alloc_dist %x\n",
	    i,
	    RIV2nb_rb_LUT25[i],
	    localRIV2alloc_LUT25[i],
	    distRIV2alloc_LUT25[i]);

   }

   rah = 0;
   rballoc = 0x1fff;
   printf("rballoc 0 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
   rah = 1;

   rballoc = 0x1678;
   printf("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));

   rballoc = 0xfffc;
   printf("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
   rballoc = 0xfffd;
   printf("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
   rballoc = 0xffff;
   printf("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
   rballoc = 0xfffe;
   printf("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
 }

#endif
