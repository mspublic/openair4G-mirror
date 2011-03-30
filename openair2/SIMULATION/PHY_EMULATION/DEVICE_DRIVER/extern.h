#ifndef __DRIVER_EXTERN_H__
#define __DRIVER_EXTERN_H__

/*#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif
*/
//extern char master_id;
#include "SIMULATION/PHY_EMULATION/ABSTRACTION/complex.h" 
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/defs.h"
//#include "LAYER2/MAC/defs.h"
#include "SIMULATION/simulation_defs.h"

#ifndef USER_MODE
extern unsigned int fifo_bypass_phy_kern2user; 
extern unsigned int fifo_bypass_phy_user2kern;
extern unsigned int fifo_bypass_phy_kern2user_control; 
extern unsigned int fifo_bypass_mac; 
extern unsigned int fifo_mac_bypass; 
#include "rtai_posix.h"
#endif //USER_MODE

//extern char Master_id;

extern EMULATION_VARS *Emul_vars;

extern unsigned char Is_primary_master;
extern int Rssi[NB_NODE_MAX][NB_NODE_MAX][NUMBER_OF_FREQUENCY_GROUPS];


extern unsigned char TOPOLOGY_OK;
extern unsigned char Emulation_status;

extern unsigned char Sync_cnt[NB_MODULES_MAX][NB_CNX_UE];
extern unsigned char Sync_status[NB_MODULES_MAX][NB_CNX_UE];

extern unsigned char Phy_sync_cnt[NB_MODULES_MAX][NB_CNX_UE];
extern unsigned char Phy_sync_status[NB_MODULES_MAX][NB_CNX_UE];

extern int Rssi_meas[NB_MODULES_MAX][NUMBER_OF_FREQUENCY_GROUPS][NB_TIME_ALLOC];

extern pthread_mutex_t Mac_low_mutex;
extern pthread_cond_t Mac_low_cond;
extern char Mac_low_mutex_var;
extern pthread_mutex_t Tx_mutex;
extern pthread_cond_t Tx_cond;
extern char Tx_mutex_var;

extern MAC_xface *mac_xface;

extern PHY_CONFIG *PHY_config;

#endif

