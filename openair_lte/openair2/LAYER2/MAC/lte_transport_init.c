#define UL_RB_ALLOC mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,0,24)
#define CCCH_RB_ALLOC mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,0,3)
#define RA_RB_ALLOC mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,0,3)
#define DLSCH_RB_ALLOC 0x1fff
#include "PHY/extern.h"
#include "PHY/LTE_TRANSPORT/extern.h"

void init_transport_channels(unsigned char transmission_mode) {

  // init DCI structures for testing
  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.hopping = 0;
  UL_alloc_pdu.rballoc = UL_RB_ALLOC;
  UL_alloc_pdu.mcs     = 1;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 1;

  CCCH_alloc_pdu.type               = 1;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.rv       = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;
  CCCH_alloc_pdu.TPC      = 1;      // set to 3 PRB

  DLSCH_alloc_pdu1A.type               = 1;
  DLSCH_alloc_pdu1A.vrb_type           = 0;
  DLSCH_alloc_pdu1A.rballoc            = CCCH_RB_ALLOC;
  DLSCH_alloc_pdu1A.ndi      = 1;
  DLSCH_alloc_pdu1A.rv       = 1;
  DLSCH_alloc_pdu1A.mcs      = 2;
  DLSCH_alloc_pdu1A.harq_pid = 0;
  DLSCH_alloc_pdu1A.TPC      = 1;   // set to 3 PRB

  RA_alloc_pdu.type               = 1;
  RA_alloc_pdu.vrb_type           = 0;
  RA_alloc_pdu.rballoc            = RA_RB_ALLOC;
  RA_alloc_pdu.ndi      = 1;
  RA_alloc_pdu.rv       = 1;
  RA_alloc_pdu.mcs      = 1;
  RA_alloc_pdu.harq_pid = 0;
  RA_alloc_pdu.TPC      = 1;


  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 0;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = 4;
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  if (transmission_mode == 6)
    DLSCH_alloc_pdu2.tpmi           = 5;  // PUSCH_PRECODING0
  else
    DLSCH_alloc_pdu2.tpmi             = 0;

}
