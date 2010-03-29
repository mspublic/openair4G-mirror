
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
#include "PHY/TOOLS/defs.h"
#include "PHY/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
}

/*#include "openair_device_proto.h"*/
//#include "config_extern.h"
/*#include "config_proto.h"*/

/* #include "oarf_common.c" */

#define FCNNAME "oarf_send_frame"

#define TRACE 1

extern PHY_CONFIG *PHY_config;

static bool any_bad_argument(const octave_value_list &args)
{
  octave_value v;
  if (args.length()!=2)
  {
    error(FCNNAME);
    error("syntax: oarf_send_frame(freqband,sig)\n      freqband in 0-3, sig is a 2D vector.");
    return true;
  }

  v=args(0);
  if ((!v.is_real_scalar()) || (v.scalar_value() < 0.0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() > 3.0))
  {
    error(FCNNAME);
      error("freqband must be 0, 1, 2, or 3.");
    return true;
  }

  v=args(1);
  printf("signal: R %d, C %d\n",v.rows(),v.columns());
  return false;
}








DEFUN_DLD (oarf_send_frame, args, nargout,"Send frame")
{



  if (any_bad_argument(args))
       return octave_value_list();
       
  const int freq = args(0).int_value();  
  ComplexMatrix dx = args(1).complex_matrix_value();
  
  octave_value returnvalue;
  int openair_fd,i;

  unsigned int length,aa;//mem_base;

  int dummy=0;
  short txsig[NB_ANTENNAS_TX][FRAME_LENGTH_SAMPLES];

 TX_VARS *TX_vars;
 TX_vars = (TX_VARS*)malloc(sizeof(TX_VARS));

  printf("NUMBER_OF_OFDM_CARRIERS = %d\n",NUMBER_OF_OFDM_CARRIERS);
  printf("FRAME_LENGTH_SAMPLES = %d\n",FRAME_LENGTH_SAMPLES);


  PHY_vars = (PHY_VARS *)malloc(sizeof(PHY_VARS));
  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/openair0");
    return octave_value_list();
  }

  ioctl(openair_fd,openair_STOP,(void*)&dummy);

  for (aa=0;aa<NB_ANTENNAS_TX;aa++) 
    TX_vars->TX_DMA_BUFFER[aa] = (mod_sym_t *)&txsig[aa][0];

  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<NB_ANTENNAS_TX;aa++) {
      TX_vars->TX_DMA_BUFFER[aa][2*i]     = (mod_sym_t)short(real(dx(i,aa))); 
      TX_vars->TX_DMA_BUFFER[aa][1+(2*i)] = (mod_sym_t)short(imag(dx(i,aa))); 

    }
  }

  ioctl(openair_fd,openair_START_TX_SIG,(void *)TX_vars);





  close(openair_fd);

  free(PHY_vars);

  return octave_value (dx);
}


