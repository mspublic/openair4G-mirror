#ifndef USER_MODE
#define __NO_VERSION__


#include <rtai.h>
#include <rtai_fifos.h>

#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>
#include <asm/param.h>

#include <linux/init.h>
#include <linux/module.h>
#include <asm/ioctl.h>
//#include <linux/malloc.h>
#endif //


#include "emul_device.h"
#include "defs.h"
#include "extern.h"
#include "SIMULATION/PHY_EMULATION/SCHED/defs.h"
#include "SIMULATION/PHY_EMULATION/SCHED/extern.h"

#include "PHY_INTERFACE/extern.h"




extern int bigphys_ptr;

//-----------------------------------------------------------------------------
int emul_device_open (struct inode *inode,struct file *filp) {
  //-----------------------------------------------------------------------------
  printk("[EMUL][MODULE]  emul_open()\n");
#ifdef KERNEL2_4
 MOD_INC_USE_COUNT;
#endif //
  return 0;
}
//-----------------------------------------------------------------------------
int emul_device_release (struct inode *inode,struct file *filp) {
  //-----------------------------------------------------------------------------
  printk("[EMUL][MODULE]  openair_release(), MODE = %d\n",openair_emul_vars.mode);
#ifdef KERNEL2_4
 MOD_DEC_USE_COUNT;
#endif // KERNEL2_4
  return 0;
}

//-----------------------------------------------------------------------------
int emul_device_ioctl(struct inode *inode,struct file *filp, unsigned int cmd, unsigned long arg) {
  /* arg is not meaningful if no arg is passed in user space */
  //-----------------------------------------------------------------------------
    

  void *arg_ptr = (void *)arg;
  int i,j,ret;




  printk("[EMUL][IOCTL]:  : In ioctl(), ioctl = %x\n",cmd);
  
  switch(cmd) {
    

  case EMUL_DUMP_CONFIG:
    //----------------------
    printk("[openair][IOCTL]     EMUL_DUMP_CONFIG\n");


    copy_from_user((char *)PHY_config,(char *)arg,sizeof(PHY_CONFIG));
    dump_config();
    openair_emul_vars.node_configured = 1;

 
    break;

  case EMUL_START:
    //----------------------

    printk("[openair][IOCTL]     EMUL_START_CLUSTERHEAD\n");

    if ( (openair_emul_vars.node_configured == 1) && 
	 (openair_emul_vars.node_running == 0) && 
	 (openair_emul_vars.mac_registered == 1)) {




      mac_xface->slots_per_frame = SLOTS_PER_FRAME;

      // Start low-level scheduler
      openair_emul_vars.node_running=1;
      emul_sched_init(); 
    }

    else {
      printk("[EMUL][START_CLUSTERHEAD] Radio not configured\n");
      return -1;
    }
    break;

    //----------------------
  case EMUL_STOP:
    //----------------------
    printk("[EMUL][IOCTL]     EMUL_STOP, NODE_CONFIGURED %d\n",openair_emul_vars.node_configured);

    
    if (openair_emul_vars.node_configured == 1) {
      openair_emul_vars.node_running = 0;


      mac_xface->frame = 0;
      mac_xface->is_cluster_head = 0;
      
      udelay(1000);
      //      openair_sched_cleanup();
    } 


    break;
      
    case EMUL_GET_TOPOLOGY:

      printk("[openair][IOCTL]     EMUL_GET_TOPOLOGY ...(%p) (Emul_vars %p,%d)\n",(void *)arg,(void *)Emul_vars,sizeof(EMULATION_VARS));

      if (openair_emul_vars.mac_registered == 1) {

	


	copy_from_user((char *)Emul_vars,(char *)arg,sizeof(EMULATION_VARS));
	TOPOLOGY_OK=1;
	Is_primary_master=0;
	if(!Master_id) 
	  Is_primary_master=1;
	Master_list=0;
	j=1;
	for(i=0;i<NB_MASTER;i++){
	  if(i!=Master_id)
	    Master_list=Master_list+j;
	  //printf("i=%d, MASTER_LIST %d\n",i,Master_list);
	  j*=2;
	}
	printk("[EMUL][GET][TOPOLOGY] Done, Number of Instances %d,NB_MASTER %d,NB_UE_INST %d\n",NB_INST,NB_MASTER,NB_UE_INST);
	
	printk("[EMUL]MAC_INIT IN...\n");
	ret = mac_init();
      }
      else {
	printk("[EMUL][GET][TOPOLOGY] No L2 yet ...\n");
	return -1;
      }

    break;

  default:
    //----------------------
    return -EPERM;
    break;
  }
  
  return 0;
}

