#include "extern.h"
#include "defs.h"

void *bigshm_assign( int card, size_t size_bytes, dma_addr_t *dma_handle_ptr )
{
    void *ret;
    size_t size = size_bytes;
   
    size = (size-1) + 4 - ( (size-1) % 4); // round up to the next DW to keep addresses aligned
        
    if ( (bigshm_currentptr[card] - bigshm_head[card]) > (bigshm_size_pages<<PAGE_SHIFT) -size )
        return NULL;
    
    *dma_handle_ptr = bigshm_head_phys[card] + ( (dma_addr_t)bigshm_currentptr[card] - (dma_addr_t)bigshm_head[card] );
    ret = bigshm_currentptr[card];
    bigshm_currentptr[card] = (char *)bigshm_currentptr[card] + size;
    
    //printk("bigshm_assign: size %d, virt = %p, dma = %x,    bigshm_phys=%x, _current=%p, _head=%p\n",
    //    size, ret, *dma_handle_ptr, bigshm_head_phys[card], bigshm_currentptr[card], bigshm_head[card]);
    
    return ret;
}

// allocates buffers for pci_interface_bot. returns 0 on success, -1 on error
int exmimo_firmware_init(int card_id)
{
    size_t size=0;

    if (exmimo_pci_bot_ptr[card_id] == NULL) {
        // increase exmimo_pci_interface_bot to multiple of 128 bytes 
        size = (sizeof(exmimo_pci_interface_bot_t) >> 7 ) +1;
        size <<= 7;
        
        exmimo_pci_bot_ptr[card_id] = (exmimo_pci_interface_bot_t *)pci_alloc_consistent(pdev[card_id], 
                                          size,
                                          &exmimo_pci_bot_physptr[card_id]);
        
        printk("Intializing EXMIMO interface support (exmimo_pci_bot at %p, phys %x)\n",exmimo_pci_bot_ptr[card_id],exmimo_pci_bot_physptr[card_id]);
        
        size = (MAX_FIRMWARE_BLOCK_SIZE_B >> 7 ) +1;
        size <<= 7;
        exmimo_firmware_block_ptr[card_id] = (char *)pci_alloc_consistent(pdev[card_id], 
                                        size,
                                        &exmimo_pci_bot_ptr[card_id]->firmware_block_ptr);
        
        printk("firmware_code_block_ptr : %x\n",exmimo_pci_bot_ptr[card_id]->firmware_block_ptr);
        
        size = (MAX_PRINTK_BUFFER_B >> 7 ) +1;
        size <<= 7;
        exmimo_printk_buffer_ptr[card_id]  = (char *)pci_alloc_consistent(pdev[card_id], 
                                       size,
                                       &exmimo_pci_bot_ptr[card_id]->printk_buffer_ptr);
        
        printk("printk_buffer_ptr : %x\n",exmimo_pci_bot_ptr[card_id]->printk_buffer_ptr);
        
        size = (sizeof(exmimo_pci_interface_t) >> 7 ) +1;
        size <<= 7;
        exmimo_pci_interface_ptr[card_id]  = (exmimo_pci_interface_t *)pci_alloc_consistent(pdev[card_id], 
                                           size,
                                           &exmimo_pci_bot_ptr[card_id]->pci_interface_ptr);
                                           
        printk("pci_interface_ptr : %x\n",exmimo_pci_bot_ptr[card_id]->pci_interface_ptr);  

        if (    exmimo_pci_bot_ptr[card_id] == NULL || exmimo_firmware_block_ptr[card_id] == NULL
             || exmimo_printk_buffer_ptr[card_id] == NULL || exmimo_pci_interface_ptr[card_id] == NULL )
            return -1;
       
    }

    iowrite32( exmimo_pci_bot_physptr[card_id], (bar[card_id]+0x1c) );
    iowrite32( 0, (bar[card_id]+0x20) );
   
    /*
    printk("will fill dma testdata..\n");
    {
    unsigned int DMATEST_SIZE_DW = 4096;
    unsigned int i;

    // Init fw_block with testdata
    for (i=0; i< 3*DMATEST_SIZE_DW; i++)
        ((uint32_t*)exmimo_firmware_block_ptr)[i] = 0x22330002 + i;
    for (i=0; i<DMATEST_SIZE_DW; i++)
        ((uint32_t*)exmimo_firmware_block_ptr)[i+3*DMATEST_SIZE_DW] = 0x22222222;
    }
    */
    openair_send_pccmd(card_id,EXMIMO_PCIE_INIT);
    
    return 0;
}


// assign shared memory between card an PC for data exchange: buffers for ringbuf-indices, TX, RX
// returns -1 on error, 0 on success
int exmimo_assign_shm_vars(int card_id)
{
    int j;
    
    for (j=0; j<MAX_ANTENNA; j++)
    {
        exmimo_shm_vars_kvirt[card_id].rx_cnt_ptr[j] = (unsigned int *)bigshm_assign( card_id, TRX_CNT_SIZE_B, (dma_addr_t*)&( exmimo_shm_vars_phys[card_id].rx_cnt_ptr[j]) ); 
        printk("exmimo_shm_vars_kvirt[%d].rx_cnt_ptr[%d] = %p (phys = %p)\n", card_id, j, exmimo_shm_vars_kvirt[card_id].rx_cnt_ptr[j], exmimo_shm_vars_phys[card_id].rx_cnt_ptr[j]);
        if ( exmimo_shm_vars_kvirt[card_id].rx_cnt_ptr[j] == NULL)
            return -1;
            
        exmimo_shm_vars_kvirt[card_id].tx_cnt_ptr[j] = (unsigned int *)bigshm_assign( card_id, TRX_CNT_SIZE_B, (dma_addr_t*)&exmimo_shm_vars_phys[card_id].tx_cnt_ptr[j] ); 
        printk("exmimo_shm_vars_kvirt[%d].tx_cnt_ptr[%d] = %p (phys = %p)\n", card_id, j, exmimo_shm_vars_kvirt[card_id].tx_cnt_ptr[j], exmimo_shm_vars_phys[card_id].tx_cnt_ptr[j]);
        if ( exmimo_shm_vars_kvirt[card_id].tx_cnt_ptr[j] == NULL)
            return -1;
    }

    for (j=0; j<MAX_ANTENNA; j++)
    {
        exmimo_shm_vars_kvirt[card_id].adc_head[j] = (unsigned int *)bigshm_assign( card_id, ADAC_BUFFERSZ_PERCHAN_B, (dma_addr_t*)&exmimo_shm_vars_phys[card_id].adc_head[j] ); 
        printk("exmimo_shm_vars_kvirt[%d].adc_head[%d] = %p (phys = %p)\n", card_id, j, exmimo_shm_vars_kvirt[card_id].adc_head[j], exmimo_shm_vars_phys[card_id].adc_head[j]);
        if ( exmimo_shm_vars_kvirt[card_id].adc_head[j] == NULL)
            return -1;

        exmimo_shm_vars_kvirt[card_id].dac_head[j] = (unsigned int *)bigshm_assign( card_id, ADAC_BUFFERSZ_PERCHAN_B, (dma_addr_t*)&exmimo_shm_vars_phys[card_id].dac_head[j] ); 
        printk("exmimo_shm_vars_kvirt[%d].dac_head[%d] = %p (phys = %p)\n", card_id, j, exmimo_shm_vars_kvirt[card_id].dac_head[j], exmimo_shm_vars_phys[card_id].dac_head[j]);
        if ( exmimo_shm_vars_kvirt[card_id].dac_head[j] == NULL)
            return -1;
            
        exmimo_pci_interface_ptr[card_id]->rf.adc_head[j]  = (dma_addr_t)exmimo_shm_vars_phys[card_id].adc_head[j];
        exmimo_pci_interface_ptr[card_id]->rf.dac_head[j]  = (dma_addr_t)exmimo_shm_vars_phys[card_id].dac_head[j];
    }
    
    exmimo_pci_interface_ptr[card_id]->rf.mbox_head = (dma_addr_t)exmimo_shm_vars_phys[card_id].rx_cnt_ptr[0];
    
    
    return 0;
}

void exmimo_firmware_cleanup(int card_id) {

    size_t size;
    if ( exmimo_pci_bot_ptr[card_id] ) {
        // increase exmimo_pci_interface_bot to multiple of 128 bytes 
        size = (sizeof(exmimo_pci_interface_t) >> 7 ) +1;
        size <<= 7;
        printk("pci_free pci_interface, size %d\n", size);
        pci_free_consistent( pdev[card_id], size, (void *)exmimo_pci_interface_ptr[card_id], exmimo_pci_bot_ptr[card_id]->pci_interface_ptr);
    }
    if ( exmimo_firmware_block_ptr[card_id] ) {
        size = (MAX_FIRMWARE_BLOCK_SIZE_B >> 7 ) +1;
        size <<= 7;
        pci_free_consistent( pdev[card_id], size, (void *)exmimo_firmware_block_ptr[card_id], exmimo_pci_bot_ptr[card_id]->firmware_block_ptr);
    }
    if ( exmimo_printk_buffer_ptr[card_id] ) {
        size = (MAX_PRINTK_BUFFER_B >> 7 ) +1;
        size <<= 7;
        printk("pci_free printk_buf, size %d\n", size);
        pci_free_consistent( pdev[card_id], size, (void *)exmimo_printk_buffer_ptr[card_id], exmimo_pci_bot_ptr[card_id]->printk_buffer_ptr);
    }    
    if ( exmimo_pci_bot_ptr[card_id] ) {
        size = (sizeof(exmimo_pci_interface_bot_t) >> 7 ) +1;
        size <<= 7;
        printk("pci_free pci_bot, size %d\n", size);
        pci_free_consistent( pdev[card_id], size, (void *)exmimo_pci_bot_ptr[card_id], exmimo_pci_bot_physptr[card_id]);
    }
}

void pcie_printk(void)
{
    char *buffer = exmimo_printk_buffer_ptr[0];
    unsigned int len = ((unsigned int *)buffer)[0];
    unsigned int off=0,i;
    unsigned char *dword;
    unsigned char tmp;

    //printk("In pci_fifo_printk : buffer %p, len %d: \n",buffer,len);
    printk("[LEON]: ");

    if (len<256)
    {
        if ( (len&3) >0 )
            off=1;
    
        for (i=0; i<(off+(len>>2)); i++)
        {
            dword = &((unsigned char *)buffer)[(1+i)<<2];
            tmp = dword[3];
            dword[3] = dword[0];
            dword[0] = tmp;
            tmp = dword[2];
            dword[2] = dword[1];
            dword[1] = tmp;
        }
        for (i=0; i<len; i++)
            printk("%c",((char*)&buffer[4])[i]);
    }
}


int openair_send_pccmd(int card_id, unsigned int cmd)
{
    unsigned int val;

    //    printk("Sending command to ExpressMIMO : %x\n",cmd);
    //write cmd to be executed by
    iowrite32(cmd,(bar[card_id]+0x04));
    //    printk("Readback of control1 %x\n",ioread32(bar[0]+0x4));
    val = ioread32(bar[card_id]);
    // set interrupt bit to trigger LEON interrupt
    iowrite32(val|0x1,bar[card_id]);
    //    printk("Readback of control0 %x\n",ioread32(bar[0]));

    return(0);
}
