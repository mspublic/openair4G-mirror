/** openair0_lib : API to interface with ExpressMIMO kernel driver
 * 
 *  Authors: Matthias Ihmig <matthias.ihmig@mytum.de>, 2013
 *           Raymond Knopp <raymond.knopp@eurecom.fr>
 * 
 *  Changelog:
 *  28.01.2013: Initial version
 */

#ifndef __OPENAIR0_LIB_H__
#define __OPENAIR0_LIB_H__

#include "pcie_interface.h"
#include "../openair_device.h"

extern exmimo_pci_interface_bot_virtual_t openair0_exmimo_pci;   // contains userspace pointers

extern int openair0_fd;

extern int openair0_num_antennas;

// opens device and mmaps kernel memory and initializes interface and system_id pointers
// return 0 on success
int openair0_open(void);

// close device and unmaps kernel memory
int openair0_close(void);

#endif
