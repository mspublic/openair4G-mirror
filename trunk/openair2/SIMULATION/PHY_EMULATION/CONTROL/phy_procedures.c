/*________________________phy_procedures.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

#ifndef USER_MODE
//#include "rt_compat.h"


#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/mman.h>

#include <linux/slab.h>
//#include <linux/config.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#ifdef KERNEL2_4
#include <linux/malloc.h>
#include <linux/wrapper.h>
#endif
#endif //USER_MODE

#include "defs.h"
#include "extern.h"

#ifdef PHY_EMUL
#ifdef USER_MODE
#include "w3g4free_extern.h"
#endif //USER_MDOE
#ifdef EMULATION_MASTER
//#include "rg_bypass_session_layer_tx.h"
//#include "rg_bypass_session_layer_rx.h"
//#include "rg_bypass_session_layer.h"
#define NUM_20US_SLEEP 5000
#else
//#include "mt_bypass_session_layer_tx.h"
//#include "mt_bypass_session_layer_rx.h"
//#include "mt_bypass_session_layer.h"
#endif //EMULATION_MASTER
#endif //PHY_EMUL

int k;
#ifndef USER_MODE
RTIME  now;            /* time when we started waiting        */
struct timespec timeout;        /* timeout value for the wait function */
#endif

#define openair_get_mbox() (*(unsigned int *)mbox)

void phy_procedures(unsigned char last_slot) {

  unsigned short i;
  int ret;
  int time_in,time_out;
  int mrbch_tx_power;
  int mrbch_crc;
  static char dummy_mrbch_pdu[MRBCH_SIZE] __attribute__ ((aligned(16)));		

  unsigned short j;
  if (last_slot == SLOTS_PER_FRAME-2) {

    emulation_tx_rx();

    //   msg("[OPENAIR][EMULATION]: Cleaning IND/REQ Table\n");
    for(i=0;i<NB_INST;i++){
      for(j=0;j<NB_REQ_MAX;j++){
	if(Macphy_req_table[i].Macphy_req_table_entry[j].Active==1){
	  //msg("cleaning entry %d\n",j);
	  //if(Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Direction == RX
	  // && Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type!=RACH && mac_xface->frame >50){
	    //msg("INST %d requestiong pdu of TYPE %d on lchan_%d, RX_REQ NOT FILLED!!!",i,Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Pdu_type,Macphy_req_table[i].Macphy_req_table_entry[j].Macphy_data_req.Lchan_id.Index);
	    // exit(0);
	  // }
	  
	  Macphy_req_table[i].Macphy_req_table_entry[j].Active=0;
	  Macphy_req_table[i].Macphy_req_cnt = (Macphy_req_table[i].Macphy_req_cnt - 1)%NB_REQ_MAX;
	} 
      }
    }
  }
  
  
}  



#ifndef USER_MODE
EXPORT_SYMBOL(phy_procedures);
#endif

