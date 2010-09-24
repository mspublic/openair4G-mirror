#ifndef __DRIVER_VARS_H__
#define __DRIVER_VARS_H__


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

#ifndef USER_MODE
#include "rtai_posix.h"
int major;

#endif

#include "SIMULATION/PHY_EMULATION/ABSTRACTION/complex.h" 
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "LAYER2/MAC/defs.h"
#include "SIMULATION/simulation_defs.h"

//char master_id;

#ifndef USER_MODE
unsigned int fifo_bypass_phy_kern2user = 52; 
unsigned int fifo_bypass_phy_user2kern = 29;
unsigned int fifo_bypass_phy_kern2user_control = 32; 
unsigned int fifo_bypass_mac = 33; 
unsigned int fifo_mac_bypass = 34; 
#endif //USER_MODE

EMULATION_VARS *Emul_vars;

unsigned char Is_primary_master;
unsigned char TOPOLOGY_OK=0;
unsigned char Emulation_status;
int Rssi[NB_NODE_MAX][NB_NODE_MAX][NUMBER_OF_FREQUENCY_GROUPS];
unsigned char Sync_cnt[NB_MODULES_MAX][NB_CNX_UE];
unsigned char Sync_status[NB_MODULES_MAX][NB_CNX_UE];

unsigned char Phy_sync_cnt[NB_MODULES_MAX][NB_CNX_UE];
unsigned char Phy_sync_status[NB_MODULES_MAX][NB_CNX_UE];



int Rssi_meas[NB_MODULES_MAX][NUMBER_OF_FREQUENCY_GROUPS][NB_TIME_ALLOC];
//unsigned int frame;
//unsigned int mac_debug;

MAC_xface *mac_xface;

//MACPHY_PARAMS MACPHY_params;

//unsigned int mac_registered;

//MACPHY_DATA_REQ_TABLE Macphy_req_table[NB_MODULES_MAX];


pthread_mutex_t Mac_low_mutex;
pthread_cond_t Mac_low_cond;
char Mac_low_mutex_var;
pthread_mutex_t Tx_mutex;
pthread_cond_t Tx_cond;
char Tx_mutex_var;

PHY_CONFIG *PHY_config;


#endif
