/*________________________phy_procedures_lte.c________________________

 Authors : Hicham Anouar, Raymond Knopp, Florian Kaltenberger
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr, kaltenbe@eurecom.fr
________________________________________________________________*/


// This routine is called periodically by macphy_scheduler to analyse the set of PHY_primitives that were
// Scheduled by MAC and on PHY resources at the appropriate time

/*
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
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softregs.h"
#endif



void phy_procedures_lte(unsigned char last_slot) {

  int ret[2];
  int time_in,time_out;
  int diff;
  int timing_offset;		
  int i,k,l;
  static pbch_errors = 0;
#ifndef USER_MODE
  RTIME  now;            
#endif

  /*
#ifndef OPENAIR2
  static unsigned char dummy_chbch_pdu[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
  static unsigned char dummy_chbch_pdu2[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
#endif
  */

  //msg("[PHY_PROCEDURES_LTE] Calling phy_procedures for frame %d, slot %d\n",mac_xface->frame, last_slot);

  if (mac_xface->is_cluster_head == 0) {
    
    for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
      
      slot_fep(lte_frame_parms,
	       lte_ue_common_vars,
	       l,
	       last_slot,
	       lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size,
	       1);
    }
    
    if (last_slot==1) {

    /*
    lte_adjust_synch(lte_frame_parms,
		     lte_ue_common_vars,
		     1,
		     16384);
    */

      if (rx_pbch(lte_ue_common_vars,
		  lte_ue_pbch_vars,
		  lte_frame_parms,
		  SISO)) {
      
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH decoded sucessfully!\n",mac_xface->frame, last_slot);
	pbch_errors = 0;
	
      }
      else
	pbch_errors++;

      /*      
      if (pbch_errors>20) {
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH lost!\n",mac_xface->frame, last_slot);
	openair_daq_vars.mode = openair_NOT_SYNCHED;
	openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
	openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
	mac_xface->frame = -1;
	openair_daq_vars.synch_wait_cnt=0;
	openair_daq_vars.sched_cnt=-1;
      }
      */
    }
  }
  else {
    msg("[PHY_PROCEDURES_LTE] not yet implemented for eNB\n");
  }
  
}


