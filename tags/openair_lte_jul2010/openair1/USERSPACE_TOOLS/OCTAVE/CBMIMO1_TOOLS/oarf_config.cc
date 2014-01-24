
// Maxime Guillaud - created Thu May 11 17:19:25 CEST 2006
// see http://www.gnu.org/software/octave/doc/interpreter/Dynamically-Linked-Functions.html#Dynamically-Linked-Functions
// and http://wiki.octave.org/wiki.pl?CodaTutorial
// and http://octave.sourceforge.net/coda/c58.html
// compilation: see Makefile



#include <octave/oct.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern "C" {
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CONFIG/defs.h"
#include "PHY/CONFIG/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
}
#include "PHY/vars.h"
#include "PHY/CONFIG/vars.h"

//#include "oarf_common.c"

#define FCNNAME "oarf_config"

#define TRACE 1

PHY_CONFIG PHY_config_mem;

static bool any_bad_argument(const octave_value_list &args)
{
  octave_value v,w;

  if (args.length()!=4)
  {
    error(FCNNAME);
    error("syntax: oarf_config(freqband,configfile,scenariofile,dual_tx)");
    return true;
  }

  v=args(0);
  if ((!v.is_real_scalar()) || (v.scalar_value() < 0.0) || (floor(v.scalar_value()) != v.scalar_value()) || (v.scalar_value() > 6.0))
  {
    error(FCNNAME);
      error("freqband must be 0-5");
    return true;
  }

  if (!args(1).is_string())
  {
      error(FCNNAME);
      error("configfile (2nd parameter) must be a char string.");
      return true;
  }

  if (!args(2).is_string())
  {
      error(FCNNAME);
      error("scenariofile (3rd parameter) must be a char string.");
      return true;
  }

  w=args(3);
  if (!w.is_real_scalar()) 
  {
    error(FCNNAME);
    error("dual_tx must be 0 or 1");
    return true;
  }
  return false;
}








DEFUN_DLD (oarf_config, args, nargout,"configure the openair interface - returns 0 if successful")
{

  if (any_bad_argument(args))
       return octave_value_list();
       
  const int freq = args(0).int_value();  
  const std::string configfile = args(1).string_value();
  const std::string scenariofile = args(2).string_value();
  const int dual_tx = args(3).int_value();  

  octave_value returnvalue;
  int openair_fd;


  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/openair0");
    return octave_value_list();
  }

  
  if((config = fopen(configfile.c_str(),"r")) == NULL)
  {
    error(FCNNAME);
    error("configuration file could not be opened!");
    return octave_value_list();
  } 

  if((scenario = fopen(scenariofile.c_str(),"r")) == NULL)    // this file is closed by function reconfigure_MACPHY of oarf_common.c
  {
    error(FCNNAME);
    error("scenario file could not be opened!");
    return octave_value_list();
  }
  
  PHY_config = (PHY_CONFIG *)&PHY_config_mem;
  reconfigure_MACPHY(scenario);
  fclose(config);

  PHY_config->dual_tx = dual_tx;
  PHY_config->tdd     = 0;

  lte_frame_parms = &(PHY_config->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ng_times6          = 1;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = NB_ANTENNAS_TX;
  lte_frame_parms->nb_antennas_rx     = NB_ANTENNAS_RX;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->mode1_flag  = 1; //default == SISO
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->symbols_per_tti = 12;
  lte_frame_parms->ofdm_symbol_size = 512;
  lte_frame_parms->log2_symbol_size = 9;
  lte_frame_parms->samples_per_tti = 7680;
  lte_frame_parms->first_carrier_offset = 362;
  lte_frame_parms->nb_prefix_samples=128;
  

  returnvalue=ioctl(openair_fd, openair_DUMP_CONFIG,(char *)PHY_config);

  close(openair_fd);

  return octave_value(returnvalue);
}

