/*!\brief Initilization and reconfiguration routines for LTE PHY */
#ifndef USER_MODE
#define __NO_VERSION__
#endif

#include "defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"

/*

* @addtogroup _PHY_STRUCTURES_
* Memory Initializaion and Cleanup for LTE MODEM.
* @{
\section _Memory_init_ Memory Initialization for LTE MODEM
Blah Blah
*/

/*
#ifndef USER_MODE
#include "SCHED/defs.h"
#endif //USER_MODE
*/

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softconfig.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_pci.h"
//#include "pci_commands.h"
#endif //CBMIMO1

int phy_init_top(unsigned char nb_antennas_tx) {


  int *tmp_ptr;
  mod_sym_t *tmp_ptr_tx;
  unsigned int tx_dma_buffer_size_bytes;


  int i,j,n,tb;

  bzero((void *)PHY_vars,sizeof(PHY_VARS));



  if (mac_xface->is_cluster_head == 1)
    msg("[openair][PHY][INIT]TERMINODE is preconfigured as a Cluster Head\n");
  msg("[openair][PHY][INIT]OFDM size             : %d\n",NUMBER_OF_OFDM_CARRIERS);
  msg("[openair][PHY][INIT]FRAME_LENGTH_SAMPLES  : %d\n",FRAME_LENGTH_SAMPLES);
  msg("[openair][PHY][INIT]NUMBER_OF_SYMBOLS_PER_FRAME  : %d\n",NUMBER_OF_SYMBOLS_PER_FRAME);
  

    
  crcTableInit();
  
  ccodedot11_init();
  ccodedot11_init_inv();

#ifndef EXPRESSMIMO_TARGET
  phy_generate_viterbi_tables();
#endif //EXPRESSMIMO_TARGET

  for (i=0;i<NB_ANTENNAS_RX;i++) {


    // Allocate memory for TX DMA Buffer

#ifdef IFFT_FPGA
    tx_dma_buffer_size_bytes = NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t);
#else
    tx_dma_buffer_size_bytes = FRAME_LENGTH_BYTES;
#endif

    tmp_ptr_tx = (mod_sym_t *)bigmalloc16(tx_dma_buffer_size_bytes+2*PAGE_SIZE);
					  
    if (tmp_ptr_tx==NULL) {
      msg("[PHY][INIT] Could not allocate TX_DMA %d (%x bytes)\n",i, 
	  (unsigned int)(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t) + 2*PAGE_SIZE));
      return(-1);
    }
    else {
      bzero(tmp_ptr_tx,tx_dma_buffer_size_bytes+2*PAGE_SIZE);
#ifndef USER_MODE
      pci_buffer[(2*i)] = (unsigned int)tmp_ptr_tx;
      tmp_ptr_tx = (mod_sym_t*)(((unsigned int)tmp_ptr_tx + PAGE_SIZE -1) & PAGE_MASK);
      //      reserve_mem(tmp_ptr_tx,FRAME_LENGTH_BYTES+2*PAGE_SIZE);
#endif // //USER_MODE
#ifdef DEBUG_PHY
      msg("[PHY][INIT] TX_DMA_BUFFER %d at %p (%p), size 0x%x\n",i,
	  (void *)tmp_ptr_tx,
	  (void *)virt_to_phys(tmp_ptr_tx),
	     (unsigned int)(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(mod_sym_t)+2*PAGE_SIZE));
#endif
    }


    PHY_vars->tx_vars[i].TX_DMA_BUFFER = tmp_ptr_tx;
#ifdef DEBUG_PHY
    msg("[PHY][INIT] PHY_vars->tx_vars[%d].TX_DMA_BUFFER = %p\n",i,(void *)PHY_vars->tx_vars[i].TX_DMA_BUFFER);
#endif


#ifndef USER_MODE
    TX_DMA_BUFFER[i] = tmp_ptr_tx;
#endif //USER_MODE



#ifndef USER_MODE

#ifdef PLATON
    tx_dma_buffer[i]=tmp_ptr_tx;
    tx_mbox[i] = tmp_ptr_tx + (FRAME_LENGTH_BYTES>>2);
#endif // //USER_MODE
#endif // // PLATON


    // RX DMA Buffers
    tmp_ptr = (int *)bigmalloc16(FRAME_LENGTH_BYTES+OFDM_SYMBOL_SIZE_BYTES+2*PAGE_SIZE);
					  
    if (tmp_ptr==NULL) {
#ifdef DEBUG_PHY
      msg("[PHY][INIT] Could not allocate RX_DMA %d (%x bytes)\n",i, 
	  FRAME_LENGTH_BYTES+2*OFDM_SYMBOL_SIZE_BYTES + 2*PAGE_SIZE);
#endif
      return(-1);
    }
    else {
      bzero(tmp_ptr,FRAME_LENGTH_BYTES+OFDM_SYMBOL_SIZE_BYTES+2*PAGE_SIZE);
#ifndef USER_MODE
      pci_buffer[1+(2*i)] = tmp_ptr;

      tmp_ptr = (((unsigned long)tmp_ptr + PAGE_SIZE -1) & PAGE_MASK);
      //          reserve_mem(tmp_ptr,FRAME_LENGTH_BYTES+2*PAGE_SIZE);

#endif //USER_MODE
#ifdef DEBUG_PHY
      msg("[PHY][INIT] RX_DMA_BUFFER %d at %p (%p), size 0x%x\n",i,
	  (void *)tmp_ptr,
	  (void *)virt_to_phys(tmp_ptr),(unsigned int)(FRAME_LENGTH_BYTES+OFDM_SYMBOL_SIZE_BYTES+2*PAGE_SIZE));
#endif
    }



    PHY_vars->rx_vars[i].RX_DMA_BUFFER = tmp_ptr;
    //#ifdef DEBUG_PHY
    msg("[PHY][INIT] PHY_vars->rx_vars[%d].RX_DMA_BUFFER = %p\n",i,PHY_vars->rx_vars[i].RX_DMA_BUFFER);
    //#endif

#ifdef PLATON
#ifndef USER_MODE
    rx_mbox[i] = tmp_ptr + ((FRAME_LENGTH_BYTES+OFDM_SYMBOL_SIZE_BYTES)>>2);
    rx_dma_buffer[i] = tmp_ptr;
#endif // //USER_MODE
#endif // //PLATON
    //#endif //


#ifndef USER_MODE
    RX_DMA_BUFFER[i] = tmp_ptr;
#endif // //USER_MODE
  }


#ifndef USER_MODE
#ifndef NOCARD_TEST
#ifndef PLATON
  // Allocate memory for PCI interface and store pointers to dma buffers
  msg("[PHY][INIT] Setting up Leon PCI interface structure\n");
  pci_interface = (PCI_interface_t *)((unsigned int)(tmp_ptr + ((OFDM_SYMBOL_SIZE_BYTES+FRAME_LENGTH_BYTES+PAGE_SIZE)>>2)));
  msg("[PHY][INIT] PCI interface at %p\n",pci_interface);
  openair_writel(pdev[0],FROM_GRLIB_CFG_GRPCI_EUR_CTRL0_OFFSET+4,(unsigned int)virt_to_phys((volatile void*)pci_interface));  

  mbox = (unsigned int)(&pci_interface->adac_cnt);
  for (i=0;i<NB_ANTENNAS_RX;i++) {
    pci_interface->adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[i]);
    pci_interface->dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[i]);
  }
#endif //PLATON
#endif //NOCARD_TEST
#endif // USER_MODE


#ifdef PLATON
#ifndef USER_MODE
  PHY_vars->mbox = tx_mbox[0];
#endif //// USER_MODE
#endif // //PLATON
#ifdef CBMIMO1
#ifndef USER_MODE
  PHY_vars->mbox = mbox;
#endif //// USER_MODE 
#endif // // CBMIMO1

  //  printk("[PHY][INIT] mbox = %p,rxgainreg = %p\n",PHY_vars->mbox,rxgainreg);



#ifdef DEBUG_PHY    
  msg("[openair][PHY][INIT] Initializing FFT engine\n");
  msg("[openair][PHY][INIT] Using %d point fft (%d, %p)\n",NUMBER_OF_OFDM_CARRIERS,LOG2_NUMBER_OF_OFDM_CARRIERS,rev );
#endif

#ifndef EXPRESSMIMO_TARGET
  // Initialize fft variables
  init_fft(NUMBER_OF_OFDM_CARRIERS,LOG2_NUMBER_OF_OFDM_CARRIERS,rev);   // TX/RX
  init_fft(4*NUMBER_OF_OFDM_CARRIERS,2+LOG2_NUMBER_OF_OFDM_CARRIERS,rev_times4);   // Synch
  init_fft(NUMBER_OF_OFDM_CARRIERS/2,LOG2_NUMBER_OF_OFDM_CARRIERS-1,rev_half);   // for interpolation of channel est
#endif //EXPRESSMIMO_TARGET


  twiddle_fft = (short *)malloc16(4095*4*2);
  twiddle_ifft = (short *)malloc16(4095*4*2);
  twiddle_fft_times4 = (short*)malloc16(4095*4*2);
  twiddle_ifft_times4 = (short*)malloc16(4095*4*2);
  twiddle_fft_half = (short*)malloc16(4095*4*2);
  twiddle_ifft_half = (short*)malloc16(4095*4*2);

#ifdef DEBUG_PHY    
  msg("[openair][PHY][INIT] twiddle_fft= %p, twiddle_ifft=%p, twiddle_fft_times4=%p,twiddle_ifft_times4=%p\n",
	 (void *)twiddle_fft,(void *)twiddle_ifft,(void *)twiddle_fft_times4,(void *)twiddle_ifft_times4);
#endif

  switch (NUMBER_OF_OFDM_CARRIERS) {
	  
  case 64:
    memcpy(twiddle_fft,&twiddle_fft64[0],63*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft64[0],63*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft256[0],255*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft256[0],255*4*2);
    //memcpy(twiddle_fft_half,&twiddle_fft32[0],31*4*2);
    //memcpy(twiddle_ifft_half,&twiddle_ifft32[0],31*4*2);
    break;
  case 128:
    memcpy(twiddle_fft,&twiddle_fft128[0],127*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft128[0],127*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft512[0],511*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft512[0],511*4*2);
    memcpy(twiddle_fft_half,&twiddle_fft64[0],63*4*2);
    memcpy(twiddle_ifft_half,&twiddle_ifft64[0],63*4*2);
    break;
  case 256:
    memcpy(twiddle_fft,&twiddle_fft256[0],255*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft256[0],255*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft1024[0],1023*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft1024[0],1023*4*2);
    memcpy(twiddle_fft_half,&twiddle_fft128[0],127*4*2);
    memcpy(twiddle_ifft_half,&twiddle_ifft128[0],127*4*2);
    break;
  case 512:
    memcpy(twiddle_fft,&twiddle_fft512[0],511*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft512[0],511*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft2048[0],2047*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft2048[0],2047*4*2);
    memcpy(twiddle_fft_half,&twiddle_fft256[0],255*4*2);
    memcpy(twiddle_ifft_half,&twiddle_ifft256[0],255*4*2);
    break;
  case 1024:
    memcpy(twiddle_fft,&twiddle_fft1024[0],1023*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft1024[0],1023*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft4096[0],4095*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft4096[0],4095*4*2);
    memcpy(twiddle_fft_half,&twiddle_fft512[0],511*4*2);
    memcpy(twiddle_ifft_half,&twiddle_ifft512[0],511*4*2);
    break;
  default:
    memcpy(twiddle_fft,&twiddle_fft64[0],63*4*2);
    memcpy(twiddle_ifft,&twiddle_ifft64[0],63*4*2);
    memcpy(twiddle_fft_times4,&twiddle_fft256[0],255*4*2);
    memcpy(twiddle_ifft_times4,&twiddle_ifft256[0],255*4*2);
    //memcpy(twiddle_fft_half,&twiddle_fft32[0],31*4*2);
    //memcpy(twiddle_ifft_half,&twiddle_ifft32[0],31*4*2);
    break;
  }

  return(1);
}

void phy_cleanup(void) {

  int i,n,tb;
#ifndef USER_MODE
  unsigned int dummy_ptr;
#endif //USER_MODE
  // stop PHY_thread


  msg("[openair][PHY][INIT] cleanup\n");

  for (i=0;i<NB_ANTENNAS_RX;i++) {

#ifndef USER_MODE

    if (pci_buffer[2*i]) {
#ifdef DEBUG_PHY    
      msg("[openair][PHY][INIT] pci_buffer %d\n",2*i);
#endif    
      bigfree(pci_buffer[2*i],FRAME_LENGTH_BYTES+2*PAGE_SIZE);
      //      free_pages(pci_buffer[2*i],8);
#ifdef DEBUG_PHY    
      msg("[openair][PHY][INIT] Freed TX_DMA_BUFFER %d\n",i);
#endif
    }
#ifdef DEBUG_PHY    
    msg("[openair][PHY][INIT] pci_buffer %d\n",1+(2*i));
#endif
    if (pci_buffer[1+(2*i)]) {
#ifdef DEBUG_PHY    
      msg("[openair][PHY][INIT] pci_buffer %d\n",1+(2*i));
#endif
      dummy_ptr = virt_to_phys(pci_buffer[1+(2*i)]);
      bigfree(pci_buffer[1+(2*i)],FRAME_LENGTH_BYTES+2*PAGE_SIZE);

      
      //      free_pages(pci_buffer[1+(2*i)],8);
#ifdef DEBUG_PHY    
      msg("[openair][PHY][INIT] Freed RX_DMA_BUFFER %d\n",i);
#endif
    }
#else
    if (PHY_vars->tx_vars[i].TX_DMA_BUFFER) {
      free(PHY_vars->tx_vars[i].TX_DMA_BUFFER);
#ifdef DEBUG_PHY    
      msg("[openair][PHY] Freed PHY_vars->tx_vars[%d]\n",i);
#endif
    }
#endif // USER_MODE
  }

  msg("[openair][CLEANUP] Done!\n");
}

/*
 * @}*/
