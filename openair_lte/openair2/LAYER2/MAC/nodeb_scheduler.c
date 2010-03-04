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
void nodeb_mac_scheduler_tx(u8 Mod_id) {
/********************************************************************************************************************/
  nodeb_scheduler(Mod_id); 

  nodeb_generate_chbch(Mod_id);

  nodeb_generate_sach(Mod_id);
  
}
/********************************************************************************************************************/
void nodeb_mac_scheduler_rx(u8 Mod_id) {
/********************************************************************************************************************/
  nodeb_get_rach(Mod_id,NB_RACH);
  nodeb_get_sach(Mod_id);
}

