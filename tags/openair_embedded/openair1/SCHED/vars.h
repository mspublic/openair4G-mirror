#ifndef USER_MODE
#define __NO_VERSION__


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/bitops.h>

#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>


#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#include <rtai_sched.h>
#include <rtai_sem.h>

#include "rt_compat.h"

#else
#include <unistd.h>
#endif


//#include "dlc_engine.h"

//#ifndef PHYSIMULATION
/*------------------------------------------------*/
/*---                  THREADS                 ---*/
/*------------------------------------------------*/

//static int                     openair_sched_status;

//static int                     exit_PHY;
//static int                     exit_PHY_ack;
 
//#ifndef USER_MODE
//struct net_device *dev;
//struct DLC_Engine *dlc;
//#endif USER_MODE

//static int PHY_instance_cnt=0;

OPENAIR_DAQ_VARS openair_daq_vars;

#endif
