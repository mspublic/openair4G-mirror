/** openair0_lib : API to interface with ExpressMIMO-1&2 kernel driver
 * 
 *  Authors: Matthias Ihmig <matthias.ihmig@mytum.de>, 2013
 *           Raymond Knopp <raymond.knopp@eurecom.fr>
 * 
 *  Changelog:
 *  28.01.2013: Initial version
 */

#include <fcntl.h> 
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "openair0_lib.h"
#include "../openair_device.h"

exmimo_pci_interface_bot_virtual_t openair0_exmimo_pci; // contains userspace pointers
int openair0_fd;
int openair0_num_antennas=2;

unsigned int PAGE_SHIFT;
char *bigshm_top;


unsigned int log2_int( unsigned int x )
{
  unsigned int ans = 0 ;
  while( x>>=1 ) ans++;
  return ans ;
}

int openair0_open(void)
{
    unsigned int bigshm_top_kvirtptr;
    int ant;

    exmimo_pci_interface_bot_virtual_t exmimo_pci_kvirt;
    
    PAGE_SHIFT = log2_int( sysconf( _SC_PAGESIZE ) );
    
    bigshm_top = NULL;
    memset( &openair0_exmimo_pci, 0, sizeof( exmimo_pci_interface_bot_virtual_t )  );
    
    if ((openair0_fd = open("/dev/openair0", O_RDWR,0)) <0)
    {
        return -1;
    }

    ioctl(openair0_fd, openair_GET_BIGSHMTOP_KVIRT, &bigshm_top_kvirtptr);
    ioctl(openair0_fd, openair_GET_PCI_INTERFACE_BOT_KVIRT, &exmimo_pci_kvirt);

    bigshm_top = (char *)mmap( NULL,
                       BIGSHM_SIZE_PAGES<<PAGE_SHIFT,
                       PROT_READ|PROT_WRITE,
                       MAP_SHARED, //|MAP_FIXED,//MAP_SHARED,
                       openair0_fd,
                       openair_mmap_BIGSHM<<PAGE_SHIFT);

    if (bigshm_top == MAP_FAILED) {
        openair0_close();
        return -2;
    }

    // calculate userspace addresses
    openair0_exmimo_pci.firmware_block_ptr = (char*) (bigshm_top +  (unsigned int)exmimo_pci_kvirt.firmware_block_ptr - bigshm_top_kvirtptr);
    openair0_exmimo_pci.printk_buffer_ptr  = (char*) (bigshm_top +  (unsigned int)exmimo_pci_kvirt.printk_buffer_ptr  - bigshm_top_kvirtptr);
    openair0_exmimo_pci.exmimo_config_ptr  = (exmimo_config_t*) (bigshm_top +  (unsigned int)exmimo_pci_kvirt.exmimo_config_ptr  - bigshm_top_kvirtptr);
    openair0_exmimo_pci.exmimo_id_ptr      = (exmimo_id_t*)     (bigshm_top +  (unsigned int)exmimo_pci_kvirt.exmimo_id_ptr      - bigshm_top_kvirtptr);

    /*printf("openair0_exmimo_pci.firmware_block_ptr (%p) =  bigshm_top(%p) + exmimo_pci_kvirt.firmware_block_ptr(%p) - bigshm_top_kvirtptr(%x)\n",
        openair0_exmimo_pci.firmware_block_ptr, bigshm_top, exmimo_pci_kvirt.firmware_block_ptr, bigshm_top_kvirtptr);
    printf("openair0_exmimo_pci.exmimo_id_ptr      (%p) =  bigshm_top(%p) + exmimo_pci_kvirt.exmimo_id_ptr     (%p) - bigshm_top_kvirtptr(%x)\n",
        openair0_exmimo_pci.exmimo_id_ptr, bigshm_top, exmimo_pci_kvirt.exmimo_id_ptr, bigshm_top_kvirtptr);
    */
    
    if ( openair0_exmimo_pci.exmimo_id_ptr->board_exmimoversion == 1)
        openair0_num_antennas = 2;

    if ( openair0_exmimo_pci.exmimo_id_ptr->board_exmimoversion == 2)
        openair0_num_antennas = 4;
    

    for (ant=0; ant<openair0_num_antennas; ant++)
    {
        openair0_exmimo_pci.rxcnt_ptr[ant] = (unsigned int *) (bigshm_top +  (unsigned int)exmimo_pci_kvirt.rxcnt_ptr[ant] - bigshm_top_kvirtptr);
        openair0_exmimo_pci.txcnt_ptr[ant] = (unsigned int *) (bigshm_top +  (unsigned int)exmimo_pci_kvirt.txcnt_ptr[ant] - bigshm_top_kvirtptr);
    }

    for (ant=0; ant<openair0_num_antennas; ant++)
    {
        openair0_exmimo_pci.adc_head[ant] = mmap( NULL,
                   ADAC_BUFFERSZ_PERCHAN_B,
                   PROT_READ|PROT_WRITE,
                   MAP_SHARED, //|MAP_FIXED,//MAP_SHARED,
                   openair0_fd,
                   openair_mmap_RX(ant)<<PAGE_SHIFT );

        openair0_exmimo_pci.dac_head[ant] = mmap( NULL,
                   ADAC_BUFFERSZ_PERCHAN_B,
                   PROT_READ|PROT_WRITE,
                   MAP_SHARED, //|MAP_FIXED,//MAP_SHARED,
                   openair0_fd,
                   openair_mmap_TX(ant)<<PAGE_SHIFT );
                   
        if (openair0_exmimo_pci.adc_head[ant] == MAP_FAILED || openair0_exmimo_pci.dac_head[ant] == MAP_FAILED) {
            openair0_close();
            return -3;
        }
    }
    
    //printf("p_exmimo_config = %p, p_exmimo_id = %p\n", openair0_exmimo_pci.exmimo_config_ptr, openair0_exmimo_pci.exmimo_id_ptr);
    //printf("ExpressMIMO %d, SW Rev 0x%d\n", openair0_exmimo_pci.exmimo_id_ptr->board_exmimoversion, openair0_exmimo_pci.exmimo_id_ptr->board_swrev);
      
    return 0;
}
    
    
int openair0_close(void)
{
    int ant;
    close(openair0_fd);
    
    if (bigshm_top != NULL)
        munmap(bigshm_top, BIGSHM_SIZE_PAGES<<PAGE_SHIFT);
    
    for (ant=0; ant<openair0_num_antennas; ant++)
    {
        if (openair0_exmimo_pci.adc_head[ant] != NULL)
            munmap(openair0_exmimo_pci.adc_head[ant], ADAC_BUFFERSZ_PERCHAN_B);

        if (openair0_exmimo_pci.dac_head[ant] != NULL)
            munmap(openair0_exmimo_pci.dac_head[ant], ADAC_BUFFERSZ_PERCHAN_B);
    }
    return 0;
}
