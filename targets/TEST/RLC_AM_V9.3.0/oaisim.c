#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#include "LAYER2/RLC/AM_v9.3.0/rlc_am.h"
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "UTIL/LOG/log_if.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
#include "UTIL/OCG/OCG.h"



log_mapping level_names[] =
{
    {"emerg", LOG_EMERG},
    {"alert", LOG_ALERT},
    {"crit", LOG_CRIT},
    {"err", LOG_ERR},
    {"warn", LOG_WARNING},
    {"notice", LOG_NOTICE},
    {"info", LOG_INFO},
    {"debug", LOG_DEBUG},
    {"trace", LOG_TRACE},
    {NULL, -1}
};



void help(void) {
  printf("Usage: physim -h -a -e -x transmission_mode -m target_dl_mcs -r(ate_adaptation) -n n_frames -s snr_dB -k ricean_factor -t max_delay -f forgetting factor -d cooperation_flag\n");
  printf("-h provides this help message!\n");
  printf("-a Activates PHY abstraction mode\n");
  printf("-e Activates extended prefix mode\n");
  printf("-m Gives a fixed DL mcs\n");
  printf("-r Activates rate adaptation (DL for now)\n");
  printf("-n Set the number of frames for the simulation\n");
  printf("-s snr_dB set a fixed (average) SNR\n");
  printf("-k Set the Ricean factor (linear)\n");
  printf("-t Set the delay spread (microseconds)\n");
  printf("-f Set the forgetting factor for time-variation\n");
  printf("-b Set the number of local eNB\n");
  printf("-u Set the number of local UE\n");
  printf("-M Set the machine ID for Ethernet-based emulation\n");
  printf("-p Set the total number of machine in emulation - valid if M is set\n");
  printf("-g Set multicast group ID (0,1,2,3) - valid if M is set\n");
  printf("-l Set the log level (trace, debug, info, warn, err) only valid for MAC layer\n");
  printf("-c Activate the config generator (OCG) - used in conjunction with openair emu web portal\n");
  printf("-x Set the transmission mode (1,2,6 supported for now)\n");
  printf("-d Set the cooperation flag (0 for no cooperation, 1 for delay diversity and 2 for distributed alamouti\n");
}




int main(int argc, char **argv) {


  char c;


  char * g_log_level="trace"; // by default global log level is set to trace


  while ((c = getopt (argc, argv, "haect:k:x:m:rn:s:f:u:b:M:p:g:l:d")) != -1)

    {
       switch (c)
	{
	case 'h':
	  help();
	  exit(1);
	case 'l':
	  g_log_level=optarg;
	  break;
	default:
	  help ();
	  exit (-1);
	  break;
	}
    }

 //initialize the log generator
  logInit(map_str_to_int(level_names, g_log_level));
  LOG_T(LOG,"global log level is set to %s \n",g_log_level );



  //LOG_I(EMU, "total number of UE %d (local %d, remote %d) \n", NB_UE_INST,emu_info.nb_ue_local,emu_info.nb_ue_remote);
  //LOG_I(EMU, "Total number of eNB %d (local %d, remote %d) \n", NB_CH_INST,emu_info.nb_enb_local,emu_info.nb_enb_remote);

  mac_xface = malloc(sizeof(MAC_xface));
  rlc_am_v9_3_0_test();
  return(0);
}


