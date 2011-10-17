#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#ifdef DEBUG_DCI_TOOLS
#include "PHY/vars.h"
#endif

//#define DEBUG_DCI

u32  localRIV2alloc_LUT25[512];
u32  distRIV2alloc_LUT25[512];
u16 RIV2nb_rb_LUT25[512];
u16 RIV2first_rb_LUT25[512];
u16 RIV_max=0;

extern u32 current_dlsch_cqi;

u32 conv_rballoc(u8 ra_header,u32 rb_alloc) {

  u32 rb_alloc2=0,i,shift,subset;

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



u32 conv_nprb(u8 ra_header,u32 rb_alloc) {

  u32 nprb=0,i;

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

u16 computeRIV(u16 N_RB_DL,u16 RBstart,u16 Lcrbs) {

  u16 RIV;

  if (Lcrbs<=(1+(N_RB_DL>>1)))
    RIV = (N_RB_DL*(Lcrbs-1)) + RBstart;
  else
    RIV = (N_RB_DL*(N_RB_DL+1-Lcrbs)) + (N_RB_DL-1-RBstart);

  return(RIV);
}

void generate_RIV_tables() {

  // 25RBs localized RIV
  u8 Lcrbs,RBstart;
  u8 distpos;
  u16 RIV;
  u32 alloc,alloc_dist;

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

      //      printf("RIV %d (%d) : first_rb %d NBRB %d\n",RIV,localRIV2alloc_LUT25[RIV],RBstart,Lcrbs);
      localRIV2alloc_LUT25[RIV] = alloc;
      distRIV2alloc_LUT25[RIV]  = alloc_dist;
      RIV2nb_rb_LUT25[RIV]      = Lcrbs;
      RIV2first_rb_LUT25[RIV]   = RBstart;
    }
  }
}

u32 get_rballoc(u8 vrb_type,u16 rb_alloc_dci) {

  if (vrb_type == 0)
    return(localRIV2alloc_LUT25[rb_alloc_dci]);
  else
    return(distRIV2alloc_LUT25[rb_alloc_dci]);

}

u8 get_transmission_mode(u16 Mod_id, u16 rnti) {
  unsigned char UE_id;

  // find the UE_index corresponding to rnti
  UE_id = find_ue(rnti,PHY_vars_eNB_g[Mod_id]);

  return(PHY_vars_eNB_g[Mod_id]->transmission_mode[UE_id]);
}

int generate_eNB_dlsch_params_from_dci(u8 subframe,
				       void *dci_pdu,
				       u16 rnti,
				       DCI_format_t dci_format,
				       LTE_eNB_DLSCH_t **dlsch,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti,
				       u16 DL_pmi_single) {

  u8 harq_pid;
  u8 dl_power_off;
  u16 rballoc;
  u8 NPRB,tbswap,tpmi=0;
  LTE_eNB_DLSCH_t *dlsch0=NULL,*dlsch1;


  //  printf("Generate eNB DCI, format %d, rnti %x (pdu %p)\n",dci_format,rnti,dci_pdu);

  switch (dci_format) {

  case format0:
    return(-1);
    break;
  case format1A:  // This is DLSCH allocation for control traffic

    // harq_pid field is reserved
    if ((rnti==si_rnti) || (rnti==ra_rnti) || (rnti==p_rnti)){  //
      harq_pid=0;
      // see 36-212 V8.6.0 p. 45
      NPRB      = (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->TPC&1) + 2;

      //printf("TPC %d\n",((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->TPC);
      //printf("RV %d\n",((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rv);
      //printf("NDI %d\n",((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->ndi);
    }
    else {
      harq_pid  = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->harq_pid;

      if (harq_pid>1) {
	msg("dci_tools.c: ERROR: Format 1A: harq_pid > 1\n");
	return(-1);
      }
      rballoc = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc;
      if (rballoc>RIV_max) {
	msg("dci_tools.c: ERROR: Format 1A: rb_alloc (%x) > RIV_max (%x)\n",rballoc,RIV_max);
	return(-1);
      }
      NPRB      = RIV2nb_rb_LUT25[rballoc];
    }

    if (NPRB==0)
      return(-1);

    dlsch[0]->subframe_tx[subframe] = 1;

    if (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->vrb_type == 0)
      dlsch[0]->rb_alloc[0]                       = localRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    else
      dlsch[0]->rb_alloc[0]                       = distRIV2alloc_LUT25[((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];

    dlsch[0]->nb_rb                               = NPRB;
    //    printf("NPRB %d\n",NPRB);
    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->ndi;

    if (dlsch[0]->harq_processes[harq_pid]->Ndi == 1) {
      dlsch[0]->harq_processes[harq_pid]->status = ACTIVE;
      //            printf("Setting DLSCH process %d to ACTIVE\n",harq_pid);
    }

    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB-1];

    dlsch[0]->current_harq_pid = harq_pid;
    dlsch[0]->harq_ids[subframe] = harq_pid;

    dlsch[0]->active = 1;
    dlsch0 = dlsch[0];

    dlsch[0]->rnti = rnti;

    dlsch[0]->harq_ids[subframe] = harq_pid;
    if (dlsch[0]->harq_processes[harq_pid]->Ndi == 1)
      dlsch[0]->harq_processes[harq_pid]->status = ACTIVE;

    break;
  case format1:
    harq_pid  = ((DCI1_5MHz_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 1: harq_pid >= 8\n");
      return(-1);
    }

    // msg("DCI: Setting subframe_tx for subframe %d\n",subframe);
    dlsch[0]->subframe_tx[subframe] = 1;

    dlsch[0]->rb_alloc[0]                         = conv_rballoc(((DCI1_5MHz_TDD_t *)dci_pdu)->rah,
								 ((DCI1_5MHz_TDD_t *)dci_pdu)->rballoc);

    dlsch[0]->nb_rb                               = conv_nprb(((DCI1_5MHz_TDD_t *)dci_pdu)->rah,
							      ((DCI1_5MHz_TDD_t *)dci_pdu)->rballoc);

    NPRB      = dlsch[0]->nb_rb;

    if (NPRB==0)
      return(-1);

    //    printf("NPRB %d\n",NPRB);
    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1_5MHz_TDD_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1_5MHz_TDD_t *)dci_pdu)->ndi;

    dlsch[0]->active = 1;

    if (dlsch[0]->harq_processes[harq_pid]->Ndi == 1) {
      dlsch[0]->harq_processes[harq_pid]->status = ACTIVE;
      //            printf("Setting DLSCH process %d to ACTIVE\n",harq_pid);

    }

    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1_5MHz_TDD_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB-1];

    dlsch[0]->current_harq_pid = harq_pid;
    dlsch[0]->harq_ids[subframe] = harq_pid;



    dlsch0 = dlsch[0];

    dlsch[0]->rnti = rnti;

    break;
  case format2_2A_L10PRB:

    return(-1);
    break;
  case format2_2A_M10PRB:

    harq_pid  = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 2_2A_M10PRB: harq_pid >= 8\n");
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

    dlsch0->subframe_tx[subframe] = 1;

    dlsch0->current_harq_pid = harq_pid;
    dlsch1->current_harq_pid = harq_pid;
    dlsch0->harq_ids[subframe] = harq_pid;
    dlsch1->harq_ids[subframe] = harq_pid;
    //    printf("Setting DLSCH harq id %d to subframe %d\n",harq_pid,subframe);

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
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,0);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,2);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,3);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      dlsch0->pmi_alloc                             = DL_pmi_single;
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      return(-1);
      break;
    }

    //    printf("Set pmi %x (tpmi %d)\n",dlsch0->pmi_alloc,tpmi);


    if (frame_parms->mode1_flag == 1)
      dlsch0->harq_processes[harq_pid]->mimo_mode   = SISO;

    dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->ndi1;
    if (dlsch0->harq_processes[harq_pid]->Ndi == 1) {
      dlsch0->harq_processes[harq_pid]->status = ACTIVE;
      //      printf("Setting DLSCH process %d to ACTIVE\n",harq_pid);
    }

    dlsch0->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->mcs1;
    if (dlsch0->nb_rb > 0) {
#ifdef TBS_FIX
      dlsch0->harq_processes[harq_pid]->TBS         = 3*dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1]/4;
      dlsch0->harq_processes[harq_pid]->TBS = (dlsch0->harq_processes[harq_pid]->TBS>>3)<<3;
#else
      dlsch0->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1];
#endif 
    }
    else {
      dlsch0->harq_processes[harq_pid]->TBS = 0;
    }

    dlsch0->active = 1;

    dlsch0->rnti = rnti;
    dlsch1->rnti = rnti;

    break;
    /*  case format2_2D_L10PRB:
    
    return(-1);
    break;*/
  case format2_2D_M10PRB:

    harq_pid  = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 2_2D_M10PRB: harq_pid >= 8\n");
      return(-1);
    }


    tbswap = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->tb_swap;
    if (tbswap == 0) {
      dlsch0 = dlsch[0];
      dlsch1 = dlsch[1];
    }
    else{
      dlsch0 = dlsch[1];
      dlsch1 = dlsch[0];
    }

    dlsch0->subframe_tx[subframe] = 1;

    dlsch0->current_harq_pid = harq_pid;
    dlsch1->current_harq_pid = harq_pid;
    dlsch0->harq_ids[subframe] = harq_pid;
    dlsch1->harq_ids[subframe] = harq_pid;



    //    printf("Setting DLSCH harq id %d to subframe %d\n",harq_pid,subframe);

    dlsch0->rb_alloc[0]                         = conv_rballoc(((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rah,
							       ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->rb_alloc[0]                         = dlsch0->rb_alloc[0];

    dlsch0->nb_rb                               = conv_nprb(((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rah,
							    ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->nb_rb                               = dlsch0->nb_rb;

    dlsch0->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs1;
    dlsch1->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs2;
    dlsch0->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rv1;
    dlsch1->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rv2;

    // check if either TB is disabled (see 36-213 V8.6 p. 26)


    if ((dlsch0->harq_processes[harq_pid]->rvidx == 1) && (dlsch0->harq_processes[harq_pid]->mcs == 0))
      dlsch0->harq_processes[harq_pid]->status = DISABLED;

    if ((dlsch1->harq_processes[harq_pid]->rvidx == 1) && (dlsch1->harq_processes[harq_pid]->mcs == 0))
      dlsch1->harq_processes[harq_pid]->status = DISABLED;

    dlsch0->harq_processes[harq_pid]->Nl        = 1;

    dlsch0->layer_index                         = tbswap;
    dlsch1->layer_index                         = 1-tbswap;

    // Fix this
    tpmi = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->tpmi;

    switch (tpmi) {
    case 0 :
      dlsch0->harq_processes[harq_pid]->mimo_mode   = ALAMOUTI;
      break;
    case 1:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING11;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,0);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,2);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,3);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      dlsch0->pmi_alloc                             = DL_pmi_single;
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      return(-1);
      break;
    }

    //    printf("Set pmi %x (tpmi %d)\n",dlsch0->pmi_alloc,tpmi);


    if (frame_parms->mode1_flag == 1)
      dlsch0->harq_processes[harq_pid]->mimo_mode   = SISO;

    dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->ndi1;
    if (dlsch0->harq_processes[harq_pid]->Ndi == 1) {
      dlsch0->harq_processes[harq_pid]->status = ACTIVE;
      //      printf("Setting DLSCH process %d to ACTIVE\n",harq_pid);
    }

    dlsch0->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs1;
    if (dlsch0->nb_rb > 0) {
#ifdef TBS_FIX
      dlsch0->harq_processes[harq_pid]->TBS         = 3*dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1]/4;
      dlsch0->harq_processes[harq_pid]->TBS = (dlsch0->harq_processes[harq_pid]->TBS>>3)<<3;
#else
      dlsch0->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch0->harq_processes[harq_pid]->mcs)][dlsch0->nb_rb-1];
#endif 
    }
    else {
      dlsch0->harq_processes[harq_pid]->TBS = 0;
    }

    dlsch0->active = 1;

    dlsch0->rnti = rnti;
    dlsch1->rnti = rnti;

    dlsch0->dl_power_off = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->dl_power_off;
    dlsch1->dl_power_off = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->dl_power_off;

    break;
  default:
    msg("dci_tools.c: Unknown DCI format\n");
    return(-1);
    break;
  }
#ifdef DEBUG_DCI
  if (dlsch0) {
    msg("dlsch0 eNB: NBRB     %d\n",dlsch0->nb_rb);
    msg("dlsch0 eNB: rballoc  %x\n",dlsch0->rb_alloc[0]);
    msg("dlsch0 eNB: harq_pid %d\n",harq_pid);
    msg("dlsch0 eNB: Ndi      %d\n",dlsch0->harq_processes[harq_pid]->Ndi);
    msg("dlsch0 eNB: rvidx    %d\n",dlsch0->harq_processes[harq_pid]->rvidx);
    msg("dlsch0 eNB: TBS      %d\n",dlsch0->harq_processes[harq_pid]->TBS);
    msg("dlsch0 eNB: mcs      %d\n",dlsch0->harq_processes[harq_pid]->mcs);
    msg("dlsch0 eNB: tpmi %d\n",tpmi);
    msg("dlsch0 eNB: mimo_mode %d\n",dlsch0->harq_processes[harq_pid]->mimo_mode);
  }
#endif
  return(0);
}


int dump_dci(LTE_DL_FRAME_PARMS *frame_parms, DCI_ALLOC_t *dci) {

  switch (dci->format) {

  case format0:   // This is an UL SACH allocation so nothing here, inform MAC
    if (frame_parms->tdd_config>0)
      msg("DCI format0, rnti %x (%x): hopping %d, rb_alloc %x, mcs %d, ndi %d, TPC %d, cshift %d, dai %d, cqi_req %d\n",
	  dci->rnti,
	  ((u32*)&dci->dci_pdu[0])[0],
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->hopping,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->rballoc,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->mcs,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->ndi,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->TPC,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->cshift,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->dai,
	  ((DCI0_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->cqi_req);
    else
      msg("Don't know how to handle TDD format 0 yet\n");
    break;

  case format1:
    if (frame_parms->tdd_config>0)
      msg("DCI format1, rnti %x (%x): rah %d, rb_alloc %x, mcs %d, harq_pid %d, ndi %d, RV %d, TPC %d, dai %d\n",
	  dci->rnti,
	  ((u32*)&dci->dci_pdu)[0],
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->rah,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->rballoc,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->mcs,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->harq_pid,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->ndi,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->rv,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->TPC,
	  ((DCI1_5MHz_TDD_t *)&dci->dci_pdu[0])->dai);

    else
      msg("Don't know how to handle TDD format 0 yet\n");
    break;
  case format1A:  // This is DLSCH allocation for control traffic
    msg("DCI format1A, rnti %x (%x)\n",dci->rnti,((u32*)&dci->dci_pdu[0])[0]);
    msg("VRB_TYPE %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->vrb_type);
    msg("RB_ALLOC %x (NB_RB %d)\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->rballoc,RIV2nb_rb_LUT25[((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->rballoc]);
    msg("MCS %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->mcs);
    msg("HARQ_PID %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->harq_pid);
    msg("NDI %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->ndi);
    msg("RV %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->rv);
    msg("TPC %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->TPC);
    msg("DAI %d\n",((DCI1A_5MHz_TDD_1_6_t *)&dci->dci_pdu[0])->dai);
    break;
  case format2_2A_L10PRB:
    break;
  case format2_2A_M10PRB:
    msg("DCI format2_2A_M10PRB, rnti %x (%8x %8x): harq_pid %d, tb_swap %d, rah %d, rb_alloc %x, mcs1 %d, mcs2 %d, rv1 %d, rv2 %d, tpmi %d, ndi1 %d, ndi2 %d\n",
	dci->rnti,
	((u32 *)&dci->dci_pdu)[1],
	((u32 *)&dci->dci_pdu)[0],
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->harq_pid,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->tb_swap,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->rah,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->rballoc,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->mcs1,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->mcs2,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->rv1,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->rv2,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->tpmi,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->ndi1,
	((DCI2_5MHz_2A_M10PRB_TDD_t *)&dci->dci_pdu[0])->ndi2
	);
    break;
  case format2_2D_M10PRB:
    msg("DCI format2_2D_M10PRB, rnti %x (%8x %8x): harq_pid %d, tb_swap %d, rah %d, rb_alloc %x, mcs1 %d, mcs2 %d, rv1 %d, rv2 %d, tpmi %d, ndi1 %d, ndi2 %d, dl_power_offset %d\n",
	dci->rnti,
	((u32 *)&dci->dci_pdu)[1],
	((u32 *)&dci->dci_pdu)[0],
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->harq_pid,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->tb_swap,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->rah,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->rballoc,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->mcs1,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->mcs2,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->rv1,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->rv2,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->tpmi,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->ndi1,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->ndi2,
	((DCI2_5MHz_2D_M10PRB_TDD_t *)&dci->dci_pdu[0])->dl_power_off
	);
    break;
  default:
    return(-1);
    break;
  }
  return(0);
}


int generate_ue_dlsch_params_from_dci(u8 subframe,
				      void *dci_pdu,
				      u16 rnti,
				      DCI_format_t dci_format,
				      LTE_UE_DLSCH_t **dlsch,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti) {

  u8 harq_pid=0;
  u8 dl_power_off;
  u16 rballoc;
  u8 NPRB,tbswap,tpmi;
  LTE_UE_DLSCH_t *dlsch0=NULL,*dlsch1=NULL;

#ifdef DEBUG_DCI
  msg("dci_tools.c: Filling ue dlsch params -> rnti %x, dci_format %d\n",rnti,dci_format);
#endif
  switch (dci_format) {

  case format0:   // This is an UL SACH allocation so nothing here, inform MAC
    msg("dci_tools.c: format0 not possible\n");
    return(-1);
    break;
  case format1A:

    // harq_pid field is reserved
    rballoc = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rballoc;
    if (rballoc>RIV_max) {
      msg("dci_tools.c: ERROR: Format 1A: rb_alloc > RIV_max\n");
      return(-1);
    }

    if ((rnti==si_rnti) || (rnti==ra_rnti) || (rnti==p_rnti)){  //
      harq_pid=0;
      // see 36-212 V8.6.0 p. 45
      NPRB      = (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->TPC&1) + 2;
    }
    else {
      harq_pid  = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->harq_pid;
      if (harq_pid>1) {
	msg("dci_tools.c: ERROR: Format 1A: harq_pid > 1\n");
	return(-1);
      }

      NPRB      = RIV2nb_rb_LUT25[rballoc];
    }

    if (NPRB==0) {
      msg("dci_tools.c: ERROR: Format 1A: NPRB=0\n");
      return(-1);
    }

    if (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs > 7) {
      msg("dci_tools.c: ERROR: Format 1A: unlikely mcs for format 1A (%d)\n",((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs);
      return(-1);
    }

    dlsch[0]->current_harq_pid = harq_pid;
    //    msg("Format 1A: harq_pid %d\n",harq_pid);
    if (((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->vrb_type == 0)
      dlsch[0]->rb_alloc[0]                       = localRIV2alloc_LUT25[rballoc];
    else
      dlsch[0]->rb_alloc[0]                       = distRIV2alloc_LUT25[rballoc];

    dlsch[0]->nb_rb                               = NPRB; //RIV2nb_rb_LUT25[rballoc];
    //printf("DCI 1A : nb_rb %d\n",dlsch[0]->nb_rb);
    if ((dlsch[0]->nb_rb<=0) || (dlsch[0]->nb_rb > 3)) {
      msg("dci_tools.c: ERROR:  Format 1A: unlikely nb_rb for format 1A (%d)\n",dlsch[0]->nb_rb);
      return(-1);
    }

    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = frame_parms->mode1_flag == 1 ?SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->ndi;
    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1A_5MHz_TDD_1_6_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB-1];

    dlsch[0]->rnti = rnti;

    dlsch0 = dlsch[0];
    break;

  case format1:
    harq_pid  = ((DCI1_5MHz_TDD_t *)dci_pdu)->harq_pid;

    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 1: harq_pid >= 8\n");
      return(-1);
    }
    dlsch[0]->current_harq_pid = harq_pid;
    dlsch[0]->harq_ack[subframe].harq_id = harq_pid;

    dlsch[0]->rb_alloc[0]                         = conv_rballoc(((DCI1_5MHz_TDD_t *)dci_pdu)->rah,
								 ((DCI1_5MHz_TDD_t *)dci_pdu)->rballoc);

    dlsch[0]->nb_rb                               = conv_nprb(((DCI1_5MHz_TDD_t *)dci_pdu)->rah,
							      ((DCI1_5MHz_TDD_t *)dci_pdu)->rballoc);

    NPRB      = dlsch[0]->nb_rb;

    if (NPRB==0) {
      msg("dci_tools.c: ERROR: Format 1: NPRB=0\n");
      return(-1);
    }

    //    printf("NPRB %d\n",NPRB);
    dlsch[0]->harq_processes[harq_pid]->rvidx     = ((DCI1_5MHz_TDD_t *)dci_pdu)->rv;

    dlsch[0]->harq_processes[harq_pid]->Nl          = 1;
    dlsch[0]->layer_index = 0;
    dlsch[0]->harq_processes[harq_pid]->mimo_mode   = (frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI;
    dlsch[0]->harq_processes[harq_pid]->Ndi         = ((DCI1_5MHz_TDD_t *)dci_pdu)->ndi;

    if (dlsch[0]->harq_processes[harq_pid]->Ndi == 1) {
      dlsch[0]->harq_processes[harq_pid]->status = ACTIVE;
      //      printf("Setting DLSCH process %d to ACTIVE\n",harq_pid);
    }
    else if (dlsch[0]->harq_processes[harq_pid]->status == SCH_IDLE) {  // we got an Ndi = 0 for a previously decoded process,
      // this happens if either another harq process in the same
      // is NAK or an ACK was not received

      dlsch[0]->harq_ack[subframe].ack              = 1;
      dlsch[0]->harq_ack[subframe].harq_id          = harq_pid;
      dlsch[0]->harq_ack[subframe].send_harq_status = 1;
      dlsch[0]->active = 0;
      return(0);
    }
    dlsch[0]->harq_processes[harq_pid]->mcs         = ((DCI1_5MHz_TDD_t *)dci_pdu)->mcs;

    dlsch[0]->harq_processes[harq_pid]->TBS         = dlsch_tbs25[get_I_TBS(dlsch[0]->harq_processes[harq_pid]->mcs)][NPRB-1];

    dlsch[0]->current_harq_pid = harq_pid;

    dlsch[0]->active = 1;

    dlsch[0]->rnti = rnti;

    dlsch0 = dlsch[0];

    break;

  case format2_2A_L10PRB:
    msg("dci_tools.c: format2_2A_L10PRB not yet implemented\n");
    return(-1);
    break;
  case format2_2A_M10PRB:

    harq_pid  = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 2_2A_M10PRB: harq_pid >= 8\n");
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

    /*
      if (dlsch0->harq_processes[harq_pid]->mcs>20) {
      msg("dci_tools.c: mcs > 20 disabled for now (asked %d)\n",dlsch0->harq_processes[harq_pid]->mcs);
      return(-1);
      }
    */

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
    //    msg("ue: tpmi %d\n",tpmi);

    switch (tpmi) {
    case 0 :
      dlsch0->harq_processes[harq_pid]->mimo_mode   = ALAMOUTI;
      break;
    case 1:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING11;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,0);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,2);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,3);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      // pmi stored from ulsch allocation routine
      //msg("using PMI %x\n",pmi2hex_2Ar1(dlsch0->pmi_alloc));
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      msg("dci_tools.c: ERROR: Unsupported TPMI\n");
      return(-1);
      break;
    }


    if (frame_parms->mode1_flag == 1)
      dlsch0->harq_processes[harq_pid]->mimo_mode   = SISO;

    dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2A_M10PRB_TDD_t *)dci_pdu)->ndi1;
    if (dlsch0->harq_processes[harq_pid]->Ndi == 1)
      dlsch0->harq_processes[harq_pid]->status = ACTIVE;
    else if (dlsch0->harq_processes[harq_pid]->status == SCH_IDLE) {  // we got an Ndi = 0 for a previously decoded process,
      // this happens if either another harq process in the same
      // is NAK or an ACK was not received

      dlsch0->harq_ack[subframe].ack              = 1;
      dlsch0->harq_ack[subframe].harq_id          = harq_pid;
      dlsch0->harq_ack[subframe].send_harq_status = 1;
      dlsch0->active = 0;
      return(0);
    }
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
    /*
      if (dlsch0->harq_processes[harq_pid]->mcs > 18)
      printf("mcs %d, TBS %d\n",dlsch0->harq_processes[harq_pid]->mcs,dlsch0->harq_processes[harq_pid]->TBS);
    */
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

    dlsch0->rnti = rnti;
    dlsch1->rnti = rnti;

    dlsch0->active = 1;
    dlsch1->active = 1;

    break;
    /*  case format2_2D_L10PRB:
    msg("dci_tools.c: format2_2D_L10PRB not yet implemented\n");
    return(-1);
    break;*/
  case format2_2D_M10PRB:

    harq_pid  = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->harq_pid;
    if (harq_pid>=8) {
      msg("dci_tools.c: ERROR: Format 2_2D_M10PRB: harq_pid >= 8\n");
      return(-1);
    }
    dlsch[0]->current_harq_pid = harq_pid;
    dlsch[0]->harq_ack[subframe].harq_id = harq_pid;

    tbswap = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->tb_swap;
    if (tbswap == 0) {
      dlsch0 = dlsch[0];
      dlsch1 = dlsch[1];
    }
    else{
      dlsch0 = dlsch[1];
      dlsch1 = dlsch[0];
    }

    dlsch0->rb_alloc[0]                         = conv_rballoc(((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rah,
							       ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->rb_alloc[0]                         = dlsch0->rb_alloc[0];

    dlsch0->nb_rb                               = conv_nprb(((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rah,
							    ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rballoc);
    dlsch1->nb_rb                               = dlsch0->nb_rb;

    dlsch0->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs1;




    /*
      if (dlsch0->harq_processes[harq_pid]->mcs>20) {
      msg("dci_tools.c: mcs > 20 disabled for now (asked %d)\n",dlsch0->harq_processes[harq_pid]->mcs);
      return(-1);
      }
    */

    dlsch1->harq_processes[harq_pid]->mcs       = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs2;
    dlsch0->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rv1;
    dlsch1->harq_processes[harq_pid]->rvidx     = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->rv2;

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
    tpmi = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->tpmi;
    //    msg("ue: tpmi %d\n",tpmi);

    switch (tpmi) {
    case 0 :
      dlsch0->harq_processes[harq_pid]->mimo_mode   = ALAMOUTI;
      break;
    case 1:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING11;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,0);
      break;
    case 2:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1m1;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,1);
      break;
    case 3:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1j;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,2);
      break;
    case 4:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = UNIFORM_PRECODING1mj;
      dlsch0->pmi_alloc                             = pmi_extend(frame_parms,3);
      break;
    case 5:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING0;
      // pmi stored from ulsch allocation routine
      break;
    case 6:
      dlsch0->harq_processes[harq_pid]->mimo_mode   = PUSCH_PRECODING1;
      msg("dci_tools.c: ERROR: Unsupported TPMI\n");
      return(-1);
      break;
    }


    if (frame_parms->mode1_flag == 1)
      dlsch0->harq_processes[harq_pid]->mimo_mode   = SISO;

    dlsch0->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->ndi1;
    if (dlsch0->harq_processes[harq_pid]->Ndi == 1)
      dlsch0->harq_processes[harq_pid]->status = ACTIVE;
    else if (dlsch0->harq_processes[harq_pid]->status == SCH_IDLE) {  // we got an Ndi = 0 for a previously decoded process,
      // this happens if either another harq process in the same
      // is NAK or an ACK was not received

      dlsch0->harq_ack[subframe].ack              = 1;
      dlsch0->harq_ack[subframe].harq_id          = harq_pid;
      dlsch0->harq_ack[subframe].send_harq_status = 1;
      dlsch0->active = 0;
      return(0);
    }
    dlsch0->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs1;
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
    /*
      if (dlsch0->harq_processes[harq_pid]->mcs > 18)
      printf("mcs %d, TBS %d\n",dlsch0->harq_processes[harq_pid]->mcs,dlsch0->harq_processes[harq_pid]->TBS);
    */
    dlsch1->harq_processes[harq_pid]->Ndi         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->ndi2;
    if (dlsch1->harq_processes[harq_pid]->Ndi == 1)
      dlsch1->harq_processes[harq_pid]->status = ACTIVE;
    dlsch1->harq_processes[harq_pid]->mcs         = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->mcs2;
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

    dlsch0->rnti = rnti;
    dlsch1->rnti = rnti;

    dlsch0->active = 1;
    dlsch1->active = 1;

    dlsch0->dl_power_off = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->dl_power_off;
    dlsch1->dl_power_off = ((DCI2_5MHz_2D_M10PRB_TDD_t *)dci_pdu)->dl_power_off;


    break;
  default:
    msg("dci_tools.c: format %d not yet implemented\n",dci_format);
    return(-1);
    break;
  }


#ifdef DEBUG_DCI
  if (dlsch[0]) {
    msg("dlsch0 UE: %p\n",dlsch[0]);
    msg("dlsch0 UE: NBRB     %d\n",dlsch[0]->nb_rb);
    msg("dlsch0 UE: rballoc  %x\n",dlsch[0]->rb_alloc[0]);
    msg("dlsch0 UE: harq_pid %d\n",harq_pid);
    msg("dlsch0 UE: Ndi      %d\n",dlsch[0]->harq_processes[harq_pid]->Ndi);
    msg("dlsch0 UE: rvidx    %d\n",dlsch[0]->harq_processes[harq_pid]->rvidx);
    msg("dlsch0 UE: TBS      %d\n",dlsch[0]->harq_processes[harq_pid]->TBS);
    msg("dlsch0 UE: mcs      %d\n",dlsch[0]->harq_processes[harq_pid]->mcs);
  }
#endif
  dlsch[0]->active=1;
  return(0);
}

/*
  u8 subframe2harq_pid_tdd(u8 tdd_config,u8 subframe) {

  msg("subframe2harq_pid_tdd: tdd_config %d, subframe %d\n",tdd_config,subframe);

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

u8 subframe2harq_pid(LTE_DL_FRAME_PARMS *frame_parms,u8 subframe) {

#ifdef DEBUG_DCI
  if (frame_parms->frame_type == 1)
    msg("dci_tools.c: subframe2_harq_pid, subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
#endif

  if (frame_parms->frame_type == 0) {
    return(0);
  }
  else {

    switch (frame_parms->tdd_config) {

    case 2:
      if ((subframe!=2) && (subframe!=7)) {
	msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
#ifdef USER_MODE
	exit(-1);
#endif
	return(255);
      }
      return(subframe/7);
      break;
    case 3:
      if ((subframe<2) || (subframe>4)) {
	msg("dci_tools.c: subframe2_harq_pid_tdd, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
#ifdef USER_MODE
	exit(-1);
#endif
	return(255);
      }
      return(subframe-2);
      break;
    case 4:
      if ((subframe<2) || (subframe>3)) {
	msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
#ifdef USER_MODE
	exit(-1);
#endif
	return(255);
      }
      return(subframe-2);
      break;
    case 5:
      if (subframe!=2) {
	msg("dci_tools.c: subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
#ifdef USER_MODE
	exit(-1);
#endif
	return(255);
      }
      return(subframe-2);
      break;
    default:
      msg("dci_tools.c: subframe2_harq_pid, Unsupported TDD mode\n");
      return(255);

    }
  }
}

u16 quantize_subband_pmi(PHY_MEASUREMENTS *meas,u8 eNB_id) {

  int i;
  u16 pmiq=0;
  u16 pmivect = 0;
  u8 rank = meas->rank[eNB_id];
  int pmi_re,pmi_im;

  for (i=0;i<NUMBER_OF_SUBBANDS;i++) {

    if (rank == 0) {
      pmi_re = meas->subband_pmi_re[eNB_id][i][meas->selected_rx_antennas[eNB_id][i]];
      pmi_im = meas->subband_pmi_im[eNB_id][i][meas->selected_rx_antennas[eNB_id][i]];

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
      msg("PMI feedback for rank>1 not supported!\n");
      pmivect = 0;
    }
  }

  return(pmivect);
}

u16 quantize_subband_pmi2(PHY_MEASUREMENTS *meas,u8 eNB_id,u8 a_id) {

  u8 i;
  u16 pmiq=0;
  u16 pmivect = 0;
  u8 rank = meas->rank[eNB_id];
  int pmi_re,pmi_im;

  for (i=0;i<NUMBER_OF_SUBBANDS;i++) {

    if (rank == 0) {
      pmi_re = meas->subband_pmi_re[eNB_id][i][a_id];
      pmi_im = meas->subband_pmi_im[eNB_id][i][a_id];

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

u16 quantize_wideband_pmi(PHY_MEASUREMENTS *meas,u8 eNB_id) {

  u16 pmiq;
  u16 pmivect = 0;
  u8 rank = meas->rank[eNB_id];
  //int pmi;
  int pmi_re,pmi_im;

  if (rank == 1) {
    //pmi =
    pmi_re = meas->wideband_pmi_re[eNB_id][meas->selected_rx_antennas[eNB_id][0]];
    pmi_im = meas->wideband_pmi_im[eNB_id][meas->selected_rx_antennas[eNB_id][0]];
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

u8 sinr2cqi(int sinr) {
  if (sinr<-3)
    return(0);
  if (sinr>14)
    return(10);
  else
    return(3+(sinr>>1));
}

//u8 sinr2cqi(int sinr) {
//
//  return(sinr+4.5);
//
//}

//u32 fill_subband_cqi(PHY_MEASUREMENTS *meas,u8 eNB_id) {
//
//	u8 i;
////	u16 cqivect = 0;
//	u32 cqivect = 0;
//
////	char diff_cqi;
//	int diff_cqi=0;
//
//	for (i=0;i<NUMBER_OF_SUBBANDS;i++) {
//
//		diff_cqi = -sinr2cqi(meas->wideband_cqi_dB[eNB_id][0]) + sinr2cqi(meas->subband_cqi_dB[eNB_id][0][i]);
//
//		// Note, this is Table 7.2.1-2 from 36.213
//		if (diff_cqi<=-1)
//			diff_cqi = 3;
//		else if (diff_cqi>2)
//			diff_cqi = 2;
//		cqivect |= (diff_cqi<<(2*i));
//
//	}
//
//	return(cqivect);
//}


u32 fill_subband_cqi(PHY_MEASUREMENTS *meas,u8 eNB_id) {

  u8 i;

  u32 cqivect = 0,offset=0;


  int diff_cqi=0;

  for (i=0;i<NUMBER_OF_SUBBANDS;i++) {

    diff_cqi = -sinr2cqi(meas->wideband_cqi_dB[eNB_id][0]) + sinr2cqi(meas->subband_cqi_dB[eNB_id][0][i]);

    // Note, this is Table 7.2.1-2 from 36.213
    if (diff_cqi<=-1)
      offset = 3;
    else
      if (diff_cqi>=2)
	offset = 2;
      else
	offset=(u32)diff_cqi;

    cqivect |= (offset<<(2*i));

  }

  return(cqivect);
}

void fill_CQI(void *o,UCI_format_t uci_format,PHY_MEASUREMENTS *meas,u8 eNB_id) {
  
  msg("[PHY][UE] Filling CQI for eNB %d, meas->wideband_cqi_tot[%d] %d\n",
      eNB_id,eNB_id,meas->wideband_cqi_tot[eNB_id]);
  

  switch (uci_format) {
  case wideband_cqi_rank1_2A:
    ((wideband_cqi_rank1_2A_5MHz *)o)->cqi1 = sinr2cqi(meas->wideband_cqi_tot[eNB_id]);
    ((wideband_cqi_rank1_2A_5MHz *)o)->pmi  = quantize_subband_pmi(meas,eNB_id);
    break;
  case wideband_cqi_rank2_2A:
    ((wideband_cqi_rank2_2A_5MHz *)o)->cqi1 = sinr2cqi(meas->wideband_cqi_dB[eNB_id][0]);
    ((wideband_cqi_rank2_2A_5MHz *)o)->cqi2 = sinr2cqi(meas->wideband_cqi_dB[eNB_id][1]);
    ((wideband_cqi_rank2_2A_5MHz *)o)->pmi  = quantize_subband_pmi(meas,eNB_id);
    break;
  case HLC_subband_cqi_nopmi:
    ((HLC_subband_cqi_nopmi_5MHz *)o)->cqi1     = sinr2cqi(meas->wideband_cqi_tot[eNB_id]);
    ((HLC_subband_cqi_nopmi_5MHz *)o)->diffcqi1 = fill_subband_cqi(meas,eNB_id);
    break;
  case HLC_subband_cqi_rank1_2A:
    ((HLC_subband_cqi_rank1_2A_5MHz *)o)->cqi1     = sinr2cqi(meas->wideband_cqi_tot[eNB_id]);
    ((HLC_subband_cqi_rank1_2A_5MHz *)o)->diffcqi1 = fill_subband_cqi(meas,eNB_id);
    ((HLC_subband_cqi_rank1_2A_5MHz *)o)->pmi      = quantize_wideband_pmi(meas,eNB_id);
    break;
  case HLC_subband_cqi_rank2_2A:
    // This has to be improved!!!
    ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi1     = sinr2cqi(meas->wideband_cqi_dB[eNB_id][0]);
    ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi1 = fill_subband_cqi(meas,eNB_id);
    ((HLC_subband_cqi_rank2_2A_5MHz *)o)->cqi2     = sinr2cqi(meas->wideband_cqi_dB[eNB_id][0]);
    ((HLC_subband_cqi_rank2_2A_5MHz *)o)->diffcqi2 = fill_subband_cqi(meas,eNB_id);
    ((HLC_subband_cqi_rank2_2A_5MHz *)o)->pmi      = quantize_subband_pmi(meas,eNB_id);
    break;
  case ue_selected:
    msg("dci_tools.c: fill_CQI ue_selected CQI not supported yet!!!\n");
    break;
  }
}


u32 pmi_extend(LTE_DL_FRAME_PARMS *frame_parms,u8 wideband_pmi) {

  u8 i,wideband_pmi2=wideband_pmi&3;
  u32 pmi_ex = 0;

  for (i=0;i<14;i+=2)
    pmi_ex|=(wideband_pmi2<<i);

  return(pmi_ex);
}


int generate_ue_ulsch_params_from_dci(void *dci_pdu,
				      u16 rnti,
				      u8 subframe,
				      u8 transmission_mode,
				      DCI_format_t dci_format,
				      LTE_UE_ULSCH_t *ulsch,
				      LTE_UE_DLSCH_t **dlsch,
				      PHY_MEASUREMENTS *meas,
				      LTE_DL_FRAME_PARMS *frame_parms,
				      u16 si_rnti,
				      u16 ra_rnti,
				      u16 p_rnti,
				      u8 eNB_id,
				      u32 current_dlsch_cqi,
				      u8 generate_srs) {

  u8 harq_pid;

#ifdef DEBUG_DCI
  msg("dci_tools.c: Filling ue ulsch params for rnti %x, dci_format %d, dci %x, subframe %d\n",
      rnti,dci_format,*(u32 *)dci_pdu,subframe);
  msg("type %d, hopping %d, rballoc %d, mcs %d, ndi %d, TPC %d, cshift %d, dai %d, cqi_req %d\n",
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->type,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->hopping,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->ndi,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->TPC,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->dai,
      ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cqi_req);
#endif

  if (dci_format == format0) {

    //printf("RIV %d\n",((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc);

    if (rnti == ra_rnti)
      harq_pid = 0;
    else
      harq_pid = subframe2harq_pid(frame_parms,(subframe+4)%10);
    //    msg("harq_pid = %d\n",harq_pid);

    if (harq_pid == 255) {
      msg("dci_tools.c: frame %d, subframe %d, rnti %x, format %d: FATAL ERROR: generate_ue_ulsch_params_from_dci, illegal harq_pid!\n",
	  mac_xface->frame, subframe, rnti, dci_format);
      return(-1);
    }
    if (((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc > RIV_max) {
      msg("dci_tools.c: frame %d, subframe %d, rnti %x, format %d: FATAL ERROR: generate_ue_ulsch_params_from_dci, rb_alloc > RIV_max\n", 	  
	  mac_xface->frame, subframe, rnti, dci_format);
      return(-1);
    }


    // indicate that this process is to be serviced in subframe n+4
    ulsch->harq_processes[harq_pid]->subframe_scheduling_flag = 1;

    ulsch->harq_processes[harq_pid]->TPC                                   = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->TPC;

    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->Ndi                                   = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->ndi;

    msg("[PHY][UE] DCI format 0: harq_pid %d nb_rb %d, rballoc %d\n",harq_pid,ulsch->harq_processes[harq_pid]->nb_rb,
	   ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc);
    //Mapping of cyclic shift field in DCI format0 to n_DMRS2 (3GPP 36.211, Table 5.5.2.1.1-1)
    if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 0)
      ulsch->n_DMRS2 = 0;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 1)
      ulsch->n_DMRS2 = 6;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 2)
      ulsch->n_DMRS2 = 3;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 3)
      ulsch->n_DMRS2 = 4;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 4)
      ulsch->n_DMRS2 = 2;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 5)
      ulsch->n_DMRS2 = 8;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 6)
      ulsch->n_DMRS2 = 10;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 7)
      ulsch->n_DMRS2 = 9;

    //reserved for cooperative communication
    if(ulsch->n_DMRS2 == 6)
      ulsch->cooperation_flag = 2;
    else
      ulsch->cooperation_flag = 0;


    if ((ulsch->harq_processes[harq_pid]->nb_rb>0) && (ulsch->harq_processes[harq_pid]->nb_rb < 25))
      ulsch->power_offset = ue_power_offsets[ulsch->harq_processes[harq_pid]->nb_rb-1];

    if (ulsch->harq_processes[harq_pid]->Ndi == 1)
      ulsch->harq_processes[harq_pid]->status = ACTIVE;


    if (((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cqi_req == 1) {
      ulsch->O_RI = 1; //we only support 2 antenna ports, so this is always 1 according to 3GPP 36.213 Table 
      switch(transmission_mode){ 
	// The aperiodic CQI reporting mode is fixed for every transmission mode instead of being configured by higher layer signaling
      case 1:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 2:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 3:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 4:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank1_2A;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank2_2A;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 5:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank1_2A;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank2_2A;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 6:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_wideband_cqi_rank1_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank1_2A;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_wideband_cqi_rank2_2A_5MHz;
	  ulsch->uci_format                          = wideband_cqi_rank2_2A;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      case 7:
	if(meas->rank[eNB_id] == 0){
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 0;
	}
	else{
	  ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	  ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	  ulsch->o_RI[0]                             = 1;
	}
	break;
      default:
	msg("Incorrect Transmission Mode \n");
	break;
      }
    }
    else {
      ulsch->O_RI = 1;
      if(meas->rank[eNB_id] == 0){
	ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	ulsch->o_RI[0]                             = 0;
      }
      else{
	ulsch->O                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                          = HLC_subband_cqi_nopmi;
	ulsch->o_RI[0]                             = 1;
      }
      //ulsch->O = sizeof_HLC_subband_cqi_nopmi_5MHz;
    }



    fill_CQI(ulsch->o,ulsch->uci_format,meas,eNB_id);
    print_CQI(ulsch->o,ulsch->uci_format,eNB_id);
    // save PUSCH pmi for later (transmission modes 4,5,6)

    //    msg("ulsch: saving pmi for DL %x\n",pmi2hex_2Ar1(((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi));
    dlsch[0]->pmi_alloc = ((wideband_cqi_rank1_2A_5MHz *)ulsch->o)->pmi;

#ifdef DEBUG_PHY
    if (((mac_xface->frame % 100) == 0) || (mac_xface->frame < 10))
      print_CQI(ulsch->o,ulsch->uci_format,eNB_id);
#endif

    ulsch->O_ACK                                  = 2;

    ulsch->beta_offset_cqi_times8                  = 18;
    ulsch->beta_offset_ri_times8                   = 10;
    ulsch->beta_offset_harqack_times8              = 16;


    ulsch->Nsymb_pusch                             = 12-(frame_parms->Ncp<<1)-(generate_srs==0?0:1);

    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs;
      if (ulsch->harq_processes[harq_pid]->mcs < 28)
	ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = ulsch->Nsymb_pusch;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      //      printf("Ndi = 0 : Setting RVidx from mcs %d\n",((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs);
      if (((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs >= 28)
	ulsch->harq_processes[harq_pid]->rvidx = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs - 28;
      else
	ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->round++;
    }

    // ulsch->n_DMRS2 = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift;

#ifdef DEBUG_DCI
    msg("ulsch (ue): NBRB        %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch (ue): first_rb    %d\n",ulsch->harq_processes[harq_pid]->first_rb);
    msg("ulsch (ue): harq_pid    %d\n",harq_pid);
    msg("ulsch (ue): Ndi         %d\n",ulsch->harq_processes[harq_pid]->Ndi);
    msg("ulsch (ue): TBS         %d\n",ulsch->harq_processes[harq_pid]->TBS);
    msg("ulsch (ue): mcs         %d\n",ulsch->harq_processes[harq_pid]->mcs);
    msg("ulsch (ue): Nsymb_pusch %d\n",ulsch->Nsymb_pusch);
    msg("ulsch (ue): cshift        %d\n",ulsch->n_DMRS2);
#endif
    return(0);
  }
  else {
    msg("dci_tools.c: frame %d, subframe %d: FATAL ERROR, generate_ue_ulsch_params_from_dci, Illegal dci_format %d\n",
	mac_xface->frame, subframe,dci_format);
    return(-1);
  }

}

int generate_eNB_ulsch_params_from_dci(void *dci_pdu,
				       u16 rnti,
				       u8 subframe,
				       u8 transmission_mode,
				       DCI_format_t dci_format,
				       LTE_eNB_ULSCH_t *ulsch,
				       LTE_DL_FRAME_PARMS *frame_parms,
				       u16 si_rnti,
				       u16 ra_rnti,
				       u16 p_rnti,
				       u8 use_srs) {

  u8 harq_pid;
  u32 rb_alloc;

#ifdef DEBUG_DCI
  msg("dci_tools.c: filling eNB ulsch params for rnti %x, dci format %d, dci %x, subframe %d\n",
      rnti,dci_format,*(u32*)dci_pdu,subframe);
#endif

  if (dci_format == format0) {


    harq_pid = subframe2harq_pid(frame_parms,(subframe+4)%10);
    rb_alloc = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc;
    if (rb_alloc>RIV_max) {
      msg("dci_tools.c: ERROR: Format 0: rb_alloc > RIV_max\n");
      return(-1);
    }

#ifdef DEBUG_DCI
    msg("generate_eNB_ulsch_params_from_dci: subframe %d, rnti %x,harq_pid %d\n",subframe,rnti,harq_pid);
#endif

    ulsch->harq_processes[harq_pid]->TPC                                   = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->TPC;
    ulsch->harq_processes[harq_pid]->first_rb                              = RIV2first_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->nb_rb                                 = RIV2nb_rb_LUT25[((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->rballoc];
    ulsch->harq_processes[harq_pid]->Ndi         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->ndi;




    if (((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cqi_req == 1) {
      ulsch->O_RI = 1; //we only support 2 antenna ports, so this is always 1 according to 3GPP 36.213 Table 
      switch(transmission_mode){ 
	// The aperiodic CQI reporting mode is fixed for every transmission mode instead of being configured by higher layer signaling
      case 1:
	ulsch->Or2                                   = 0;
	ulsch->Or1                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                            = HLC_subband_cqi_nopmi;
	break;
      case 2:
	ulsch->Or2                                   = 0;
	ulsch->Or1                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                            = HLC_subband_cqi_nopmi;
	break;
      case 3:
	ulsch->Or2                                   = 0;
	ulsch->Or1                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                            = HLC_subband_cqi_nopmi;
	break;
      case 4:
	ulsch->Or2                                 = sizeof_wideband_cqi_rank2_2A_5MHz;
	ulsch->Or1                                 = sizeof_wideband_cqi_rank1_2A_5MHz;
	ulsch->uci_format                          = wideband_cqi_rank1_2A;
	break;
      case 5:
	ulsch->Or2                                 = sizeof_wideband_cqi_rank2_2A_5MHz;
	ulsch->Or1                                 = sizeof_wideband_cqi_rank1_2A_5MHz;
	ulsch->uci_format                          = wideband_cqi_rank1_2A;
	break;
      case 6:
	ulsch->Or2                                 = sizeof_wideband_cqi_rank2_2A_5MHz;
	ulsch->Or1                                 = sizeof_wideband_cqi_rank1_2A_5MHz;
	ulsch->uci_format                          = wideband_cqi_rank1_2A;
	break;
      case 7:
	ulsch->Or2                                   = 0;
	ulsch->Or1                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
	ulsch->uci_format                            = HLC_subband_cqi_nopmi;
	break;
      default:
	msg("Incorrect Transmission Mode \n");
	break;
      }
    }
    else {
      ulsch->O_RI = 1;
      ulsch->Or2                                   = 0;
      ulsch->Or1                                   = sizeof_HLC_subband_cqi_nopmi_5MHz;
      ulsch->uci_format                            = HLC_subband_cqi_nopmi;
    }



    ulsch->O_ACK                                  = 2;
    ulsch->beta_offset_cqi_times8                = 18;
    ulsch->beta_offset_ri_times8                 = 10;
    ulsch->beta_offset_harqack_times8            = 16;

    ulsch->Nsymb_pusch                             = 12-(frame_parms->Ncp<<1)-(use_srs==0?0:1);


    //Mapping of cyclic shift field in DCI format0 to n_DMRS2 (3GPP 36.211, Table 5.5.2.1.1-1)
    if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 0)
      ulsch->n_DMRS2 = 0;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 1)
      ulsch->n_DMRS2 = 6;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 2)
      ulsch->n_DMRS2 = 3;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 3)
      ulsch->n_DMRS2 = 4;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 4)
      ulsch->n_DMRS2 = 2;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 5)
      ulsch->n_DMRS2 = 8;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 6)
      ulsch->n_DMRS2 = 10;
    else if(((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift == 7)
      ulsch->n_DMRS2 = 9;






    if (ulsch->harq_processes[harq_pid]->Ndi == 1) {
      ulsch->harq_processes[harq_pid]->status = ACTIVE;
      ulsch->harq_processes[harq_pid]->rvidx = 0;
      ulsch->harq_processes[harq_pid]->mcs         = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs;
      //if (ulsch->harq_processes[harq_pid]->mcs)
      ulsch->harq_processes[harq_pid]->TBS         = dlsch_tbs25[ulsch->harq_processes[harq_pid]->mcs][ulsch->harq_processes[harq_pid]->nb_rb-1];
      ulsch->harq_processes[harq_pid]->Msc_initial   = 12*ulsch->harq_processes[harq_pid]->nb_rb;
      ulsch->harq_processes[harq_pid]->Nsymb_initial = ulsch->Nsymb_pusch;
      ulsch->harq_processes[harq_pid]->round = 0;
    }
    else {
      if (((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs>28)
	ulsch->harq_processes[harq_pid]->rvidx = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs - 28;
      else {
	ulsch->harq_processes[harq_pid]->rvidx = 0;
	ulsch->harq_processes[harq_pid]->mcs = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->mcs;
      }
      ulsch->harq_processes[harq_pid]->round++;
    }
    ulsch->rnti = rnti;

    //ulsch->n_DMRS2 = ((DCI0_5MHz_TDD_1_6_t *)dci_pdu)->cshift;

#ifdef DEBUG_DCI
    msg("ulsch (eNB): NBRB          %d\n",ulsch->harq_processes[harq_pid]->nb_rb);
    msg("ulsch (eNB): first_rb      %d\n",ulsch->harq_processes[harq_pid]->first_rb);
    msg("ulsch (eNB): harq_pid      %d\n",harq_pid);
    msg("ulsch (eNB): Ndi           %d\n",ulsch->harq_processes[harq_pid]->Ndi);
    msg("ulsch (eNB): TBS           %d\n",ulsch->harq_processes[harq_pid]->TBS);
    msg("ulsch (eNB): mcs           %d\n",ulsch->harq_processes[harq_pid]->mcs);
    msg("ulsch (eNB): Or1           %d\n",ulsch->Or1);
    msg("ulsch (eNB): Nsymb_pusch   %d\n",ulsch->Nsymb_pusch);
    msg("ulsch (eNB): cshift        %d\n",ulsch->n_DMRS2);
#endif
    return(0);
  }
  else {
    msg("dci_tools.c: FATAL ERROR, generate_eNB_ulsch_params_from_dci, Illegal dci_format %d\n",dci_format);
    return(-1);
  }

}


#ifdef DEBUG_DLSCH_TOOLS
main() {

  int i;
  u8 rah;
  u16 rballoc;

  generate_RIV_tables();

  for (i=0;i<512;i++) {
    msg("RIV %d: nb_rb %d, alloc %x, alloc_dist %x\n",
	i,
	RIV2nb_rb_LUT25[i],
	localRIV2alloc_LUT25[i],
	distRIV2alloc_LUT25[i]);

  }

  rah = 0;
  rballoc = 0x1fff;
  msg("rballoc 0 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
  rah = 1;

  rballoc = 0x1678;
  msg("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));

  rballoc = 0xfffc;
  msg("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
  rballoc = 0xfffd;
  msg("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
  rballoc = 0xffff;
  msg("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
  rballoc = 0xfffe;
  msg("rballoc 1 %x => %x\n",rballoc,conv_rballoc(rah,rballoc));
}

#endif
