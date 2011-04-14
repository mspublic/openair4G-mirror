
// Maxime Guillaud - created Fri May 12 16:20:04 CEST 2006
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://wiki.octave.org/wiki.pl?CodaTutorial
// and http://octave.sourceforge.net/coda/c58.html
// compilation: see Makefile
// Update: Wed May 23 17:25:39 CEST 2007, fifo acquisition of signal buffer (RK)


#include <octave/oct.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

extern "C" {
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/impl_defs_lte.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
}
#include "PHY/vars.h"

#define FCNNAME "oarf_get_frame"

#define TRACE 1

//extern PHY_CONFIG *PHY_config;

static bool any_bad_argument(const octave_value_list &args)
{
  octave_value v;
  if (args.length()!=1)
  {
    error(FCNNAME);
    error("syntax: oarf_get_frame(freqband)\n      freqband in 0-5.");
    return true;
  }

  v=args(0);
  if ((!v.is_real_scalar()) || (v.scalar_value() < 0.0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() > 6.0))
  {
    error(FCNNAME);
      error("freqband must be 0-5.");
    return true;
  }

  return false;
}








DEFUN_DLD (oarf_get_frame, args, nargout,"Get frame (Action 5)")
{

  if (any_bad_argument(args))
       return octave_value_list();
       
  const int freq = args(0).int_value();  

  octave_value returnvalue;
  int openair_fd,i,rx_sig_fifo_fd,rf_cntl_fifo_fd;
  unsigned int length;//mem_base;
  short *rx_sig[4];

  //PHY_VARS *PHY_vars;
  //PHY_CONFIG *PHY_config;

  //PHY_vars = (PHY_VARS *)malloc(sizeof(PHY_VARS));
  //PHY_config = (PHY_CONFIG *)malloc(sizeof(PHY_CONFIG));


  LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));

  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/openair0");
    return octave_value_list();
  }

  if ((rx_sig_fifo_fd = open("/dev/rtf59", O_RDONLY,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/rtf59");
    return octave_value_list();
  }

  if ((rf_cntl_fifo_fd = open("/dev/rtf60", O_RDONLY,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/rtf60");
    return octave_value_list();
  }

  /*
  printf("Getting PHY_vars ...\n");

  ioctl(openair_fd,openair_GET_VARS,PHY_vars);

  //printf("Getting PHY_vars->tx_vars[0].TX_DMA_BUFFER=%p\n",PHY_vars->tx_vars[0].TX_DMA_BUFFER);
  //printf("Getting PHY_vars->rx_vars[0].RX_DMA_BUFFER = %p\n",PHY_vars->rx_vars[0].RX_DMA_BUFFER);
  */

  printf("Getting PHY_config ...\n");

  ioctl(openair_fd,openair_GET_CONFIG,frame_parms);

  dump_frame_parms(frame_parms);

  ComplexMatrix dx (FRAME_LENGTH_COMPLEX_SAMPLES,NB_ANTENNAS_RX);
  short dma_buffer_local[2*NB_ANTENNAS_RX*FRAME_LENGTH_COMPLEX_SAMPLES];

  // Flush RX sig fifo
  ((unsigned int *)&dma_buffer_local[0])[0] = 1 | ((freq&7)<<1) | ((freq&7)<<4);
  ioctl(openair_fd,openair_GET_BUFFER,(void *)dma_buffer_local);


  // wait for indication from RT process that a new frame is ready
  read(rf_cntl_fifo_fd,(void *)dma_buffer_local,4);
  printf("Sched count %d\n",((int *)dma_buffer_local)[0]);

  length = read(rx_sig_fifo_fd,(void *)dma_buffer_local,NB_ANTENNAS_RX*FRAME_LENGTH_BYTES);
  printf("Got %d bytes from sig fifo\n",length);

  for (i=0;i<NB_ANTENNAS_RX;i++)
    rx_sig[i] = (short *)(&dma_buffer_local[2*i*FRAME_LENGTH_COMPLEX_SAMPLES]);

  /*
  mem_base = (unsigned int)mmap(0,
		  2048*4096,
		  PROT_READ,
		  MAP_PRIVATE,
		  openair_fd,
		  0);

  if (mem_base != -1)
    msg("MEM base= %p\n",mem_base);
  else
    msg("Could not map physical memory\n");

  

  

  for (i=0;i<NBANTENNAS;i++)
      rx_sig[i] = (short *)(mem_base + (unsigned int)PHY_vars->rx_vars[i].RX_DMA_BUFFER-(unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);
  */

  
  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++)
  {
    dx(i,0)=Complex( rx_sig[0][i*2], rx_sig[0][i*2+1] );
    dx(i,1)=Complex( rx_sig[1][i*2], rx_sig[1][i*2+1] );
  }  

  close(openair_fd);

  close(rx_sig_fifo_fd);
  close(rf_cntl_fifo_fd);

  //free(PHY_vars);
  //free(PHY_config);
  free(frame_parms);
  return octave_value (dx);
}


