
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
#include "PHY/impl_defs_lte.h"
  //#include "PHY/extern.h"
  //#include "PHY/CONFIG/defs.h"
  //#include "PHY/CONFIG/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
//#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
}
#include "PHY/vars.h"
//#include "PHY/CONFIG/vars.h"

//#include "oarf_common.c"

#define FCNNAME "oarf_config_exmimo"

#define TRACE 1

//PHY_CONFIG PHY_config_mem;

static bool any_bad_argument(const octave_value_list &args)
{
  octave_value v,w;
  int i;

  if (args.length()!=10)
  {
    error(FCNNAME);
    error("syntax: oarf_config_exmimo(freq,freq_tx,tdd,dual_tx,rxgain,eNB_flag,rf_mode,rx_dc,rf_local,rf_vcolocal)");
    return true;
  }

  v=args(0);
  for (i=0;i<v.columns();i++) {
    if ((real(v.row_vector_value()(i)) < 0.0) || 
	(floor(real(v.row_vector_value()(i))) != real(v.row_vector_value()(i))) || 
	(real(v.row_vector_value()(i)) > 3.9e9)) {
      error(FCNNAME);
      error("rx freqband %d must be 0-3.9e9",i);
      return true;
    }
  }

  v=args(1);
  for (i=0;i<v.columns();i++) {
    if ((real(v.row_vector_value()(i)) < 0.0) || 
	(floor(real(v.row_vector_value()(i))) != real(v.row_vector_value()(i))) || 
	(real(v.row_vector_value()(i)) > 3.9e9)) {
      error(FCNNAME);
      error("tx freqband %d must be 0-3.9e9",i);
      return true;
    }
  }

  if ((!args(2).is_real_scalar()))// || (args(1).scalar_value()!=0.0) || (args(1).scalar_value()!=1.0))
  {
      error(FCNNAME);
      error("tdd must be 0 or 1.");
      return true;
  }

  if ((!args(3).is_real_scalar()))// || (args(2).scalar_value()!=0.0) || (args(2).scalar_value()!=1.0))
  {
      error(FCNNAME);
      error("dual_tx must be 0 or 1.");
      return true;
  }

  v=args(4);
  if (v.columns() == 4) {
      for (i=0;i<v.columns();i++) {
	if ((real(args(4).row_vector_value()(i))<0.0) || (real(args(4).row_vector_value()(i))>50.0))
	  {
	    error(FCNNAME);
	    error("gain must be between 0 and 50. (got %f).",args(4).row_vector_value()(i));
	    return true;
	  }
      }
  }
  else {
    error(FCNNAME);
    error("number of columns for rxgain must be 4 (got %d)\n",v.columns());
  }
  
  if ((!args(5).is_real_scalar()) || (args(5).scalar_value()<0.0) || (args(5).scalar_value()>1)) {
    error(FCNNAME);
    error("eNB_flag must be between 0 and 1 (got %f).",args(5).scalar_value());
    return true;
  }
  

  v = args(6);
  if (v.columns() == 4) {
    for (i=0;i<v.columns();i++) {
      if ((v.row_vector_value()(i)<0.0) || (v.row_vector_value()(i)>(double)((uint32_t)(1<<21)))) {
	error(FCNNAME);
	error("rf_mode %d must be between 0 and 2^21 (got %f).",i,v.row_vector_value()(i));
	return true;
      }
    }
  }
  else {
    error(FCNNAME);
    error("number of columns for rf_mode must be 4\n");
  }

  v = args(7);
  if (v.columns() == 4) {
    for (i=0;i<v.columns();i++) {
      if ((v.row_vector_value()(i)<0.0) || (v.row_vector_value()(i)>(double)((uint32_t)(1<<16)))) {
	error(FCNNAME);
	error("rx_dc %d must be between 0 and 2^16 (got %f).",i,v.row_vector_value()(i));
	return true;
      }
    }
  }
  else {
    error(FCNNAME);
    error("number of columns for rf_mode must be 4\n");
  }  

  v = args(8);
  if (v.columns() == 4) {
    for (i=0;i<v.columns();i++) {
      if ((v.row_vector_value()(i)<0.0) || (v.row_vector_value()(i)>(double)((uint32_t)(1<<24)))) {
	error(FCNNAME);
	error("rf_local %d must be between 0 and 2^24 (got %f).",i,v.row_vector_value()(i));
	return true;
      }
    }
  }
  else {
    error(FCNNAME);
    error("number of columns for rf_local must be 4\n");
  }

  v = args(9);
  if (v.columns() == 4) {
    for (i=0;i<v.columns();i++) {
      if ((v.row_vector_value()(i)<0.0) || (v.row_vector_value()(i)>(double)((uint32_t)(1<<12)))) {
	error(FCNNAME);
	error("rf_vcocal %d must be between 0 and 2^12 (got %f).",i,v.row_vector_value()(i));
	return true;
      }
    }
  }
  else {
    error(FCNNAME);
    error("number of columns for rf_vcocal must be 4\n");
  }

  return false;
}








DEFUN_DLD (oarf_config_exmimo, args, nargout,"configure the openair interface - returns 0 if successful")
{

  if (any_bad_argument(args))
       return octave_value_list();
       
  //  const int freqrx = args(0).int_value();  
  //  const int freqtx = args(1).int_value();  
  //const std::string configfile = args(1).string_value();
  //const std::string scenariofile = args(2).string_value();
  const int tdd = args(2).int_value();
  const int dual_tx = args(3).int_value();  
  //  const int rxgain = args(4).int_value();
  const int eNB_flag = args(5).int_value();
  //  const int rf_mode0 = args(6).int_value();
  //  const int rf_dc0 = args(7).int_value();
  //  const int rf_local0 = args(8).int_value();
  //  const int rf_vcocal0 = args(9).int_value();
  RowVector freqrx     = args(0).row_vector_value();
  RowVector freqtx     = args(1).row_vector_value();
  RowVector rxgain     = args(4).row_vector_value();
  RowVector rf_mode    = args(6).row_vector_value();
  RowVector rf_dc      = args(7).row_vector_value();
  RowVector rf_local   = args(8).row_vector_value();
  RowVector rf_vcocal  = args(9).row_vector_value();

  octave_value returnvalue;
  int openair_fd;


  LTE_DL_FRAME_PARMS *frame_parms;


  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0)
  {
    error(FCNNAME);
    error("Error opening /dev/openair0");
    return octave_value_list();
  }

  /*  
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
  */

  frame_parms = (LTE_DL_FRAME_PARMS*) malloc(sizeof(LTE_DL_FRAME_PARMS));
  frame_parms->node_id            = (eNB_flag == 1) ? 0 : 1;
  frame_parms->N_RB_DL            = 25;
  frame_parms->N_RB_UL            = 25;
  frame_parms->Ncp                = 1;
  frame_parms->Nid_cell           = 0;
  frame_parms->nushift            = 0;
  frame_parms->nb_antennas_tx     = NB_ANTENNAS_TX;
  frame_parms->nb_antennas_rx     = NB_ANTENNAS_RX;
  frame_parms->mode1_flag         = 1; //default == SISO
  frame_parms->tdd_config         = 3;
  frame_parms->dual_tx            = dual_tx;
  frame_parms->frame_type         = TDD;
  frame_parms->carrier_freq[0]    = freqrx(0);
  frame_parms->carrier_freqtx[0]  = freqtx(0);
  frame_parms->rxgain[0]          = rxgain(0);
  frame_parms->carrier_freq[1]    = freqrx(1);
  frame_parms->carrier_freqtx[1]  = freqtx(1);
  frame_parms->rxgain[1]          = rxgain(1);
  frame_parms->carrier_freq[2]    = freqrx(2);
  frame_parms->carrier_freqtx[2]  = freqtx(2);
  frame_parms->rxgain[2]          = rxgain(2);
  frame_parms->carrier_freq[3]    = freqrx(3);
  frame_parms->carrier_freqtx[3]  = freqtx(3);
  frame_parms->rxgain[3]          = rxgain(3);
  frame_parms->rfmode[0]          = rf_mode(0);
  frame_parms->rflocal[0]         = rf_local(0);
  frame_parms->rfvcolocal[0]      = rf_vcocal(0);
  frame_parms->rxdc[0]            = rf_dc(0);
  frame_parms->rfmode[1]          = rf_mode(1);
  frame_parms->rflocal[1]         = rf_local(1);
  frame_parms->rfvcolocal[1]      = rf_vcocal(1);
  frame_parms->rxdc[1]            = rf_dc(1);
  frame_parms->rfmode[2]          = rf_mode(2);
  frame_parms->rflocal[2]         = rf_local(2);
  frame_parms->rfvcolocal[2]      = rf_vcocal(2);
  frame_parms->rxdc[2]            = rf_dc(2);
  frame_parms->rfmode[3]          = rf_mode(3);
  frame_parms->rflocal[3]         = rf_local(3);
  frame_parms->rfvcolocal[3]      = rf_vcocal(3);
  frame_parms->rxdc[3]            = rf_dc(3);

  init_frame_parms(frame_parms,1);
  //  dump_frame_parms(frame_parms);

  returnvalue=ioctl(openair_fd, openair_DUMP_CONFIG,(char *)frame_parms);

  close(openair_fd);

  return octave_value(returnvalue);
}


