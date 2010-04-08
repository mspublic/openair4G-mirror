/*________________________nodeb_scheduler.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/

#include "defs.h"

#include "extern.h"
/*******************************************************************************************************************/
// This function is called by macphy_scheduler at the end of a mini-frame
// It prepares the next mini-frame for PHY and, in the case of DLC Frame boundary
// it retrieves DLC BCCC/CCCH data and scheduling (WIDENS) parameters


/********************************************************************************************************************/
void nodeb_mac_scheduler_tx(unsigned char Mod_id,unsigned char subframe) {
/********************************************************************************************************************/
//  nodeb_scheduler(Mod_id); 
#ifdef DEBUG_NODEB_SCHEDULER
  msg("[MAC enB] : Calling eNB scheduler TX (SF %d)\n",subframe);
#endif

  CH_mac_inst[Mod_id].DCI_pdu.Num_common_dci = 0;
  CH_mac_inst[Mod_id].DCI_pdu.Num_ue_spec_dci = 0;

  if ((subframe == 0))
    nodeb_generate_bcch(Mod_id);

  if (subframe>4)
    nodeb_generate_ccch(Mod_id);

  if ((subframe==0)||(subframe > 4))
    nodeb_generate_dci(Mod_id);

  //  nodeb_generate_dlsch(Mod_id);
  
}
/********************************************************************************************************************/
void nodeb_mac_scheduler_rx(u8 Mod_id) {
/********************************************************************************************************************/
//  nodeb_get_rach(Mod_id,NB_RACH);
//  nodeb_get_ulsch(Mod_id);
}

