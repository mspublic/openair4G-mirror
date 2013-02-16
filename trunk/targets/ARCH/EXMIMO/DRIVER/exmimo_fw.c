/** exmimo_fw.c
 * 
 *  Initialization routines for
 *  Shared Memory management for ExMIMO and PC(kernel) data exchange
 *  (memory allocation, pointer management)
 * 
 *  There is now one pci_alloc_consistent for each RX and TX buffer
 *  and one for all structures, including pointers, printk, firmware_dump and config
 * 
 *  Authors: Matthias Ihmig <matthias.ihmig@mytum.de>, 2012, 2013
 *           Riadh Ghaddab <riadh.ghaddab@eurecom.fr>
 *           Raymond Knopp <raymond.knopp@eurecom.fr>
 * 
 *  Changelog:
 *  24.01.2013: added memory management functions for bigshm, lots of cleanups
 */

#include <linux/delay.h> 
#include "extern.h"
#include "defs.h"
#include "pcie_interface.h"

/*****************************************************
 *  Private functions within this file
 */

void mem_SetPageReserved(void *kvirt_addr, unsigned int size_pages)
{
    size_t temp_size = size_pages << PAGE_SHIFT;
    void *adr = kvirt_addr;

    while (temp_size > 0)
    {
        SetPageReserved( virt_to_page(adr) );
        adr += PAGE_SIZE;
        temp_size -= PAGE_SIZE;
    } 
}

void mem_ClearPageReserved(void *kvirt_addr, unsigned int size_pages)
{
    size_t temp_size = size_pages << PAGE_SHIFT;
    void *adr = kvirt_addr;
 
    while (temp_size > 0) {
        ClearPageReserved( virt_to_page(adr) );
        adr += PAGE_SIZE;
        temp_size -= PAGE_SIZE;
    } 
}

// allocates big shared memory, to be used for structures, pointers, etc.
// bigshm allocs a single larger memory block for shared structures and pointers, one per card, except: ADC+DAC buffers!
// returns -1 on error, 0 on success
int bigshm_init(int card)
{
    printk("[openair][module] calling pci_alloc_consistent for card %d, bigshm (size: %u*%lu bytes)...\n", card, BIGSHM_SIZE_PAGES, PAGE_SIZE);

    if ( sizeof(dma_addr_t) != 4)
        printk("!!! WARNING: sizeof (dma_addr_t) = %d! Only 32bit mode (= 4) (also: no PAE) is supported at this time!\n", sizeof(dma_addr_t));
    
    if ( bigshm_head[card] == NULL )
        bigshm_head[card] = pci_alloc_consistent( pdev[card], BIGSHM_SIZE_PAGES<<PAGE_SHIFT, &bigshm_head_phys[card] );

    if (bigshm_head[card] == NULL) {
        printk("[openair][MODULE][ERROR] Cannot Allocate Memory (%d bytes) for shared data (bigshm)\n", BIGSHM_SIZE_PAGES<<PAGE_SHIFT);
        return -ENOMEM;
    }
    else {
        printk("[openair][MODULE][INFO] Bigshm at %p (phys %x)\n", bigshm_head[card], (unsigned int) bigshm_head_phys[card]);

        bigshm_currentptr[card] = bigshm_head[card];

        mem_SetPageReserved( bigshm_head[card], BIGSHM_SIZE_PAGES);
        memset(bigshm_head[card], 0, BIGSHM_SIZE_PAGES << PAGE_SHIFT);
    }
    return 0;
}


// use this instead of pci_alloc_consistent to assign memory from the bigshm block
// return kernel virtual pointer and sets physical DMA address in dma_handle
void *bigshm_assign( int card, size_t size_bytes, dma_addr_t *dma_handle_ptr )
{
    void *ret;
    size_t size = size_bytes;

    //size = (size-1) + 4 - ( (size-1) % 4); // round up to keep addresses aligned to DWs
    //size = (size-1) + 16 - ( (size-1) % 16); // round up to keep addresses aligned to 4 DWs

    // round up to the next 64 DW (workaround for current bug in DMA on ExMIMO2) 
    size = (size-1) + 256 - ( (size-1) % 256);

    if ( (bigshm_currentptr[card] - bigshm_head[card]) > (BIGSHM_SIZE_PAGES<<PAGE_SHIFT) -size ) {
        printk("Not enough memory in bigshm! Make BIGSHM_SIZE_PAGES bigger!\n");
        return NULL;
    }

    *dma_handle_ptr = bigshm_head_phys[card] + ( (dma_addr_t)bigshm_currentptr[card] - (dma_addr_t)bigshm_head[card] );
    ret = bigshm_currentptr[card];
    bigshm_currentptr[card] = (char *)bigshm_currentptr[card] + size;

    //printk("bigshm_assign: size %d, virt = %p, dma = %x,    bigshm_phys=%x, _current=%p, _head=%p\n",
    // size, ret, *dma_handle_ptr, bigshm_head_phys[card], bigshm_currentptr[card], bigshm_head[card]);

    return ret;
}


// assign shared memory between card an PC for data exchange: interface structure with pointers,
// firmware- and printk buffers, configuration structure
// returns -1 on error, 0 on success
int exmimo_assign_shm_vars(int card_id)
{
    int j;
    
    if (p_exmimo_pci_phys[card_id] == NULL)
    {
        p_exmimo_pci_phys[card_id] = (exmimo_pci_interface_bot_t *) bigshm_assign( card_id,
                                      sizeof(exmimo_pci_interface_bot_t),
                                      &pphys_exmimo_pci_phys[card_id]);
        printk("Intializing EXMIMO interface support (exmimo_pci_bot at %p, phys %x, size %d bytes)\n",p_exmimo_pci_phys[card_id],(unsigned int)pphys_exmimo_pci_phys[card_id], sizeof(exmimo_pci_interface_bot_t));

        exmimo_pci_kvirt[card_id].firmware_block_ptr = (char *) bigshm_assign( card_id,
                                            MAX_FIRMWARE_BLOCK_SIZE_B,
                                            (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->firmware_block_ptr));
        printk("firmware_code_block_ptr : %p (phys = %08x)\n", exmimo_pci_kvirt[card_id].firmware_block_ptr, p_exmimo_pci_phys[card_id]->firmware_block_ptr);


        exmimo_pci_kvirt[card_id].printk_buffer_ptr = (char *) bigshm_assign( card_id,
                                           MAX_PRINTK_BUFFER_B,
                                           (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->printk_buffer_ptr));
        printk("printk_buffer_ptr : %p (phys = %08x)\n", exmimo_pci_kvirt[card_id].printk_buffer_ptr, p_exmimo_pci_phys[card_id]->printk_buffer_ptr);


        exmimo_pci_kvirt[card_id].exmimo_config_ptr = (exmimo_config_t *) bigshm_assign( card_id,
                                               sizeof(exmimo_config_t),
                                               (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->exmimo_config_ptr));
        printk("exmimo_config_ptr : %p (phys = %08x)\n", exmimo_pci_kvirt[card_id].exmimo_config_ptr, p_exmimo_pci_phys[card_id]->exmimo_config_ptr);  


        exmimo_pci_kvirt[card_id].exmimo_id_ptr = (exmimo_id_t *) bigshm_assign( card_id,
                                               sizeof(exmimo_id_t),
                                               (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->exmimo_id_ptr));
        printk("exmimo_id_ptr : %p (phys = %08x)\n", exmimo_pci_kvirt[card_id].exmimo_id_ptr, p_exmimo_pci_phys[card_id]->exmimo_id_ptr);


        if (    p_exmimo_pci_phys[card_id] == NULL || exmimo_pci_kvirt[card_id].firmware_block_ptr == NULL
             || exmimo_pci_kvirt[card_id].printk_buffer_ptr == NULL || exmimo_pci_kvirt[card_id].exmimo_config_ptr == NULL 
             || exmimo_pci_kvirt[card_id].exmimo_id_ptr == NULL )
                return -1;

        for (j=0; j<MAX_ANTENNAS; j++)
        {
            // size 4*1 should be sufficient, but just to make sure we can also use DMA of size 4DW as fallback
            exmimo_pci_kvirt[card_id].rxcnt_ptr[j] = (uint32_t *) bigshm_assign( card_id, 4*4,
                                               (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->rxcnt_ptr[j]) );
            printk("exmimo_pci_kvirt[%d].rxcnt_ptr[%d] = %p (phys = %08x)\n", card_id, j, exmimo_pci_kvirt[card_id].rxcnt_ptr[j], p_exmimo_pci_phys[card_id]->rxcnt_ptr[j]);
                
            exmimo_pci_kvirt[card_id].txcnt_ptr[j] = (uint32_t *) bigshm_assign( card_id, 4*4,
                                               (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->txcnt_ptr[j]) );
            printk("exmimo_pci_kvirt[%d].txcnt_ptr[%d] = %p (phys = %08x)\n", card_id, j, exmimo_pci_kvirt[card_id].txcnt_ptr[j], p_exmimo_pci_phys[card_id]->txcnt_ptr[j]);
            
            if ( exmimo_pci_kvirt[card_id].rxcnt_ptr[j] == NULL || exmimo_pci_kvirt[card_id].txcnt_ptr[j] == NULL)
                return -1;
        }
    }
    return 0;
}

// allocate memory for RX and TX chains for all antennas
// returns -1 on error, 0 on success
int exmimo_allocate_rx_tx_buffers(int card_id)
{
    size_t size;
    int j;
    
    // Round up to the next PAGE_SIZE (typ. 4096 bytes)
    size = (ADAC_BUFFERSZ_PERCHAN_B >> PAGE_SHIFT) + 1;
    size <<= PAGE_SHIFT;
    
    for (j=0; j<MAX_ANTENNAS; j++)
    {
        exmimo_pci_kvirt[card_id].adc_head[j] = (uint32_t *)pci_alloc_consistent( pdev[card_id], size,
                                                    (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->adc_head[j]) ); 
        
        printk("exmimo_pci_kvirt[%d].adc_head[%d] = %p (phys = %08x)\n", card_id, j, exmimo_pci_kvirt[card_id].adc_head[j], p_exmimo_pci_phys[card_id]->adc_head[j]);
        if ( exmimo_pci_kvirt[card_id].adc_head[j] == NULL)
            return -1;
        
        mem_SetPageReserved( exmimo_pci_kvirt[card_id].adc_head[j], size >> PAGE_SHIFT );
        memset( exmimo_pci_kvirt[card_id].adc_head[j], 0x10+j, size);
        

        exmimo_pci_kvirt[card_id].dac_head[j] = (uint32_t *)pci_alloc_consistent( pdev[card_id], size,
                                                    (dma_addr_t*)&(p_exmimo_pci_phys[card_id]->dac_head[j]) ); 

        printk("exmimo_pci_kvirt[%d].dac_head[%d] = %p (phys = %08x)\n", card_id, j, exmimo_pci_kvirt[card_id].dac_head[j], p_exmimo_pci_phys[card_id]->dac_head[j]);
        if ( exmimo_pci_kvirt[card_id].dac_head[j] == NULL)
            return -ENOMEM;

        mem_SetPageReserved( exmimo_pci_kvirt[card_id].dac_head[j], size >> PAGE_SHIFT );
        memset( exmimo_pci_kvirt[card_id].dac_head[j], 0x20+j, size);
    }
    return 0;
}

/*********************************************
 *  Public functions ExpressMIMO Interface 
 */

/* Allocates buffer and assigns pointers
 * 
 * return 0 on success
 */
int exmimo_memory_alloc(int card)
{
    if ( bigshm_init( card ) ) {
        printk("exmimo_memory_alloc(): bigshm_init failed for card %d.\n", card);
        return -ENOMEM;
    }

    if ( exmimo_assign_shm_vars( card ) ) {
        printk("exmimo_memory_alloc(): exmimo_assign_shm_vars failed to assign enough shared memory for all variables and structures for card %i!\n", card);
        return -ENOMEM;
    }
    
    if ( exmimo_allocate_rx_tx_buffers( card ) ) {
        printk("exmimo_memory_alloc(): exmimo_allocate_rx_tx_buffers() failed to allocate enough memory for RX and TX buffers for card %i!\n", card);
        return -ENOMEM;
    }
    return 0;
}

/*
 * Copies pointer to Leon
 */
int exmimo_firmware_init(int card)
{
    /* pci_dma_sync_single_for_device(pdev[card], 
        pphys_exmimo_pci_phys[card],
        sizeof(exmimo_pci_interface_bot_t), 
        PCI_DMA_TODEVICE); */

    // put DMA pointer to exmimo_pci_interface_bot into LEON register
    iowrite32( pphys_exmimo_pci_phys[card], (bar[card]+0x1c) );  // lower 32bit of address
    iowrite32( 0, (bar[card]+0x20) );                            // higher 32bit of address

    //printk("exmimo_firmware_init(): initializing Leon (EXMIMO_PCIE_INIT)...\n");
    exmimo_send_pccmd(card, EXMIMO_PCIE_INIT);
    
    return 0;
}

/*
 * Free memory on unloading the kernel driver
 */
int exmimo_firmware_cleanup(int card)
{
    size_t size;
    int j;

    // free exmimo_allocate_rx_tx_buffers

    size = (ADAC_BUFFERSZ_PERCHAN_B >> PAGE_SHIFT) + 1;
    size <<= PAGE_SHIFT;
    for (j=0; j<MAX_ANTENNAS; j++)
    {
        if ( exmimo_pci_kvirt[card].adc_head[j] ) {
            mem_ClearPageReserved( exmimo_pci_kvirt[card].adc_head[j], size >> PAGE_SHIFT );
            pci_free_consistent( pdev[card], size, exmimo_pci_kvirt[card].adc_head[j], p_exmimo_pci_phys[card]->adc_head[j] );
        }
       
        if ( exmimo_pci_kvirt[card].dac_head[j] ) {
            mem_ClearPageReserved( exmimo_pci_kvirt[card].dac_head[j], size >> PAGE_SHIFT );
            pci_free_consistent( pdev[card], size, exmimo_pci_kvirt[card].dac_head[j], p_exmimo_pci_phys[card]->dac_head[j] ); 
        }
    }

    if ( bigshm_head[card] ) {
        printk("free bigshm_head[%d] pdev %p, size %u, head %p, phys %x\n", card, pdev[card], BIGSHM_SIZE_PAGES<<PAGE_SHIFT, bigshm_head[card], (unsigned int)bigshm_head_phys[card]);
        mem_ClearPageReserved( bigshm_head[card], BIGSHM_SIZE_PAGES );

        pci_free_consistent( pdev[card], BIGSHM_SIZE_PAGES<<PAGE_SHIFT, bigshm_head[card], bigshm_head_phys[card]);
    }

    return 0;
}


/*
 * Send command to Leon and wait until command is completed
 */
int exmimo_send_pccmd(int card_id, unsigned int cmd)
{
    unsigned int val;
    unsigned int cnt=0;
    
    //printk("Sending command to ExpressMIMO (card %d) : %x\n",card_id, cmd);
    iowrite32(cmd,(bar[card_id]+0x04));
    //    printk("Readback of control1 %x\n",ioread32(bar[0]+0x4));
    val = ioread32(bar[card_id]);
    // set interrupt bit to trigger LEON interrupt
    iowrite32(val|0x1,bar[card_id]);
    //    printk("Readback of control0 %x\n",ioread32(bar[0]));
    
    // workaround until command ack works: wait
    if (cmd == EXMIMO_PCIE_INIT)
        msleep(500);   // give card time to initialize system id structure
    if (cmd == EXMIMO_CONFIG)
        msleep(500);   // give card time to configure
    if (cmd == EXMIMO_STOP)
        msleep(100);
    if (cmd == EXMIMO_FW_START_EXEC || cmd == EXMIMO_REBOOT)
        msleep(100); // wait until code has started before initialization command is sent

    if ( exmimo_pci_kvirt[card_id].exmimo_id_ptr->board_exmimoversion == 2 )   // currently, only exmimo2 implements command ack in bootloader
    {
        if (cmd == EXMIMO_FW_INIT || cmd == EXMIMO_FW_CLEAR_BSS || cmd == EXMIMO_FW_START_EXEC)
        {
            while (cnt<120 && ( ioread32(bar[card_id]+0x4) != EXMIMO_NOP )) {
                msleep(500);
                cnt++;
            }
            if (cnt==120)
                printk("EXMIMO_FW error: Timeout: no EXMIMO_NOP received within 60sec.\n");
        }
    } else
        msleep(500);
        
    return(0);
}

