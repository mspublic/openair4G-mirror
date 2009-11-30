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



int phy_procedures_lte(unsigned char last_slot, unsigned char next_slot) {

  int ret[2];
  int time_in,time_out;
  int diff;
  int timing_offset;		
  int i,k,l;
  unsigned char pbch_pdu[6];
#ifndef USER_MODE
  RTIME  now;            
#endif

  /*
#ifndef OPENAIR2
  static unsigned char dummy_chbch_pdu[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
  static unsigned char dummy_chbch_pdu2[CHBCH_PDU_SIZE] __attribute__ ((aligned(16)));
#endif
  */

  if (last_slot<0 || last_slot>=20) {
    msg("[PHY_PROCEDURES_LTE] Frame %d, Error: last_slot =%d!\n",mac_xface->frame, last_slot);
    return(-1);
  }

  //if (mac_xface->frame%100 == 0)
  //  msg("[PHY_PROCEDURES_LTE] Calling phy_procedures for frame %d, slot %d\n",mac_xface->frame, last_slot);

  if (mac_xface->is_cluster_head == 0) {
    
    for (l=0;l<lte_frame_parms->symbols_per_tti/2;l++) {
      
      slot_fep(lte_frame_parms,
	       lte_ue_common_vars,
	       l,
	       last_slot,
	       (last_slot>>1)*lte_frame_parms->symbols_per_tti*lte_frame_parms->ofdm_symbol_size,
	       1);
    }

    if (last_slot==0) {
      // Measurements
      lte_ue_measurements(lte_ue_common_vars,
			  lte_frame_parms,
			  &PHY_vars->PHY_measurements);
      
      // AGC
      if (mac_xface->frame % 100 == 0)
	phy_adjust_gain (0,16384,0);
    }
    
    if (last_slot==1) {

      lte_adjust_synch(lte_frame_parms,
		       lte_ue_common_vars,
		       1,
		       16384);

      if (rx_pbch(lte_ue_common_vars,
		  lte_ue_pbch_vars,
		  lte_frame_parms,
		  SISO))
	lte_ue_pbch_vars->pdu_errors_conseq = 0;
      else {
	lte_ue_pbch_vars->pdu_errors_conseq++;
	lte_ue_pbch_vars->pdu_errors++;
      }

      if (mac_xface->frame % 100 == 0) {
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
	    mac_xface->frame, last_slot, lte_ue_pbch_vars->pdu_errors, lte_ue_pbch_vars->pdu_errors_conseq);
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH received frame = %d!\n",
	    mac_xface->frame, last_slot,*((unsigned int*) lte_ue_pbch_vars->decoded_output));
      }

      if (lte_ue_pbch_vars->pdu_errors_conseq>20) {
	msg("[PHY_PROCEDURES_LTE] frame %d, slot %d, PBCH consecutive errors > 20, going out of sync!\n",mac_xface->frame, last_slot);
	openair_daq_vars.mode = openair_NOT_SYNCHED;
	openair_daq_vars.sync_state=0;
#ifdef CBMIMO1
	openair_dma(FROM_GRLIB_IRQ_FROM_PCI_IS_ACQ_DMA_STOP);
#endif //CBMIMO1
	mac_xface->frame = -1;
	openair_daq_vars.synch_wait_cnt=0;
	openair_daq_vars.sched_cnt=-1;

	lte_ue_pbch_vars->pdu_errors_conseq=0;
	lte_ue_pbch_vars->pdu_errors=0;

      }
    }
  }
  else {

    generate_pilots_slot(lte_eNB_common_vars->txdataF,
			 AMP,
			 lte_frame_parms,
			 next_slot);

    if (next_slot == 0)
      generate_pss(lte_eNB_common_vars->txdataF,
		   1024,
		   lte_frame_parms,
		   1);

    if (next_slot == 1) {

      if (mac_xface->frame%100 == 0)
	msg("[PHY_PROCEDURES_LTE] Calling generate_pbch for frame %d, slot %d\n",mac_xface->frame, next_slot);
      
      *((unsigned int*) pbch_pdu) = mac_xface->frame;
      
      generate_pbch(lte_eNB_common_vars->txdataF,
		    AMP,
		    lte_frame_parms,
		    pbch_pdu);
    }

  }

  return(0);
  
}


