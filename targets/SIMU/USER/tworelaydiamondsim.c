/*
 * tworelaydiamondsim.c
 *
 *  Created on: Oct 23, 2012
 *      Author: atsan
 */


#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cblas.h>


#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"

#include "SCHED/defs.h"
#include "SCHED/vars.h"


#include "tworelaydiamond.h"
#include "oaisim_config.h"
#include "UTIL/OCG/OCG_extern.h"
#include "cor_SF_sim.h"
#include "UTIL/OMG/omg_constants.h"

#include "../PROC/interface.h"
#include "../PROC/channel_sim_proc.h"
#include "../PROC/Tsync.h"
#include "../PROC/Process.h"

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"

#include "UTIL/LOG/vcd_signal_dumper.h"

#define RF
#define MCS_COUNT 24//added for PHY abstraction
#define N_TRIALS 1

#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,24)
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define RA_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define DLSCH_RB_ALLOC 0x1fff

#define DECOR_DIST 100
#define SF_VAR 10
//constant for OAISIM soft realtime calibration
#define SF_DEVIATION_OFFSET_NS 100000 //= 0.1ms : should be as a number of UE
#define SLEEP_STEP_US       100	//  = 0.01ms could be adaptive, should be as a number of UE
#define K 2                  // averaging coefficient
#define TARGET_SF_TIME_NS 1000000	// 1ms = 1000000 ns

//Channel Descriptors
channel_desc_t *eNB2RN[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
channel_desc_t *RN2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];
channel_desc_t *UE2RN[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];
channel_desc_t *RN2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];

//Added for PHY abstraction
//TODO: What are these data structures. Only for Abstraction? But they are also used for the do_DL_sig and do_UL_sig functions.
node_desc_t *enb_data[NUMBER_OF_eNB_MAX];
node_desc_t *ue_data[NUMBER_OF_UE_MAX];
rn_node_desc_t *rn_data[NUMBER_OF_eNB_MAX];

//TODO: What is this 2 and 16 refers to??
double sinr_bler_map[MCS_COUNT][2][16];


// this should reflect the channel models in openair1/SIMULATION/TOOLS/defs.h
mapping small_scale_names[] = {
    {"custom", 0},
    {"SCM_A", 1},
    {"SCM_B", 2},
    {"SCM_C", 3},
    {"SCM_D", 4},
    {"EPA", 5},
    {"EVA", 6},
    {"ETU", 7},
    {"Rayleigh8", 8},
    {"Rayleigh1", 9},
    {"Rayleigh1_corr", 10},
    {"Rayleigh1_anticorr", 11},
    {"Rice8", 12},
    {"Rice1", 13},
    {"Rice1_corr", 14},
    {"Rice1_anticorr", 15},
    {"AWGN", 16},
    {NULL, -1}
};

static void *sigh(void *arg);
void terminate(void);
void exit_fun(const char* s);

extern int transmission_mode_rrc;//FIXME!!!

void help (void) {
  printf
    ("Usage: oaisim -h -a -F -C tdd_config -V -R N_RB_DL -e -x transmission_mode -m target_dl_mcs -r(ate_adaptation) -n n_frames -s snr_dB -k ricean_factor -t max_delay -f forgetting factor -A channel_model -z cooperation_flag -u nb_local_ue -U UE mobility -b nb_local_enb -B eNB_mobility -M ethernet_flag -p nb_master -g multicast_group -l log_level -c ocg_enable -T traffic model\n");

  printf ("-h provides this help message!\n");
  printf ("-a Activates PHY abstraction mode\n");
  printf ("-F Activates FDD transmission (TDD is default)\n");
  printf ("-C [0-6] Sets TDD configuration\n");
  printf ("-R [6,15,25,50,75,100] Sets N_RB_DL\n");
  printf ("-e Activates extended prefix mode\n");
  printf ("-m Gives a fixed DL mcs\n");
  printf ("-r Activates rate adaptation (DL for now)\n");
  printf ("-n Set the number of frames for the simulation\n");
  printf ("-s snr_dB set a fixed (average) SNR, this deactivates the openair channel model generator (OCM)\n");
  printf ("-S snir_dB set a fixed (average) SNIR, this deactivates the openair channel model generator (OCM)\n");
  printf ("-k Set the Ricean factor (linear)\n");
  printf ("-t Set the delay spread (microseconds)\n");
  printf ("-f Set the forgetting factor for time-variation\n");
  printf ("-A set the multipath channel simulation,  options are: SCM_A, SCM_B, SCM_C, SCM_D, EPA, EVA, ETU, Rayleigh8, Rayleigh1, Rayleigh1_corr,Rayleigh1_anticorr, Rice8,, Rice1, AWGN \n");
  printf ("-b Set the number of local eNB\n");
  printf ("-u Set the number of local UE\n");
  printf ("-j Set the number of local RN (Relay Node)\n");
  printf ("-M Set the machine ID for Ethernet-based emulation\n");
  printf ("-p Set the total number of machine in emulation - valid if M is set\n");
  printf ("-g Set multicast group ID (0,1,2,3) - valid if M is set\n");
  printf ("-l Set the global log level (8:trace, 7:debug, 6:info, 4:warn, 3:error) \n");
  printf
    ("-c [1,2,3,4] Activate the config generator (OCG) to process the scenario descriptor, or give the scenario manually: -c template_1.xml \n");
  printf ("-x Set the transmission mode (1,2,5,6 supported for now)\n");
  printf ("-z Set the cooperation flag (0 for no cooperation, 1 for delay diversity and 2 for distributed alamouti\n");
  printf ("-T activate the traffic generator: 0 for NONE, 1 for CBR, 2 for M2M, 3 for FPS Gaming, 4 for mix\n");
  printf ("-B Set the mobility model for eNB, options are: STATIC, RWP, RWALK, \n");
  printf ("-U Set the mobility model for UE, options are: STATIC, RWP, RWALK \n");
  printf ("-E Random number generator seed\n");
  printf ("-P enable protocol analyzer : 0 for wireshark interface, 1: for pcap , 2 : for tshark \n");
  printf ("-I Enable CLI interface (to connect use telnet localhost 1352)\n");
  printf ("-V Enable VCD dump, file = openair_vcd_dump.vcd\n");
  printf ("-G Enable background traffic \n");
}



// FUNCTION FOR EXITING THE
void exit_fun(const char* s)
{
  fprintf(stderr, "Error: %s. Exiting!\n",s);
  exit (-1);
}

//READ ARGUMENTS from the command line
void read_arguments(ArgumentVars* argvars, int argc, char **argv)
{
	  char c;
	  int temp;
	  //Initialization of some variables for the simulation.
	  argvars->abstraction_flag = 0; //Can be changed via -a option. This is default if no -a specified.
	  argvars->ethernet_flag = 0; // Can be changed via -M option.
	  argvars->set_seed = 0; // Can be changed via -E option.
	  argvars->relay_flag = 0 ; // Default option no relays.
	  argvars->Channel_Flag = 0; // Can be changed via -M option.

	  // get command-line options
	   while ((c = getopt (argc, argv, "haeoFvVIGt:C:N:P:k:x:m:rn:s:S:f:z:u:b:c:M:p:g:l:d:U:B:R:E:X:i:T:A:J:j:")) != -1) {
	     switch (c) {
	     case 'F':			// set FDD
	       oai_emulation.info.frame_type = 0;
	       break;
	     case 'C':
	       oai_emulation.info.tdd_config = atoi (optarg);
	       if (oai_emulation.info.tdd_config > 6) {
	 	LOG_E(EMU,"Illegal tdd_config %d (should be 0-6)\n", oai_emulation.info.tdd_config);
	 	exit (-1);
	       }
	       break;
	     case 'R':
	       oai_emulation.info.N_RB_DL = atoi (optarg);
	       if ((oai_emulation.info.N_RB_DL != 6) && (oai_emulation.info.N_RB_DL != 15) && (oai_emulation.info.N_RB_DL != 25)
	    		   && (oai_emulation.info.N_RB_DL != 50) && (oai_emulation.info.N_RB_DL != 75) && (oai_emulation.info.N_RB_DL != 100)) {
	    	   LOG_E(EMU,"Illegal N_RB_DL %d (should be one of 6,15,25,50,75,100)\n", oai_emulation.info.N_RB_DL);
	    	   exit (-1);
	       }
	        break;
	     case 'N':
	       argvars->Nid_cell = atoi (optarg);
	       if (argvars->Nid_cell > 503) {
	 	LOG_E(EMU,"Illegal Nid_cell %d (should be 0 ... 503)\n", argvars->Nid_cell);
	 	exit(-1);
	       }
	       break;
	     case 'h':
	       help ();
	       exit (1);
	     case 'x':
	       oai_emulation.info.transmission_mode = atoi (optarg);
	       if ((oai_emulation.info.transmission_mode != 1) &&  (oai_emulation.info.transmission_mode != 2) && (oai_emulation.info.transmission_mode != 5) && (oai_emulation.info.transmission_mode != 6)) {
	 	LOG_E(EMU, "Unsupported transmission mode %d\n",oai_emulation.info.transmission_mode);
	 	exit(-1);
	       }
	       break;
	     case 'm':
	       argvars->target_dl_mcs = atoi (optarg);
	       break;
	     case 'r':
	       argvars->rate_adaptation_flag = 1;
	       break;
	     case 'n':
	       oai_emulation.info.n_frames = atoi (optarg);
	       //n_frames = (n_frames >1024) ? 1024: n_frames; // adjust the n_frames if higher that 1024
	       oai_emulation.info.n_frames_flag = 1;
	       break;
	     case 's':
	       argvars->snr_dB = atoi (optarg);
	       //      set_snr = 1;
	       oai_emulation.info.ocm_enabled=0;
	       break;
	     case 'S':
	       argvars->sinr_dB = atoi (optarg);
	       argvars->set_sinr = 1;
	       oai_emulation.info.ocm_enabled=0;
	       break;
	     case 'J':
	       argvars->ue_connection_test=1;
	       oai_emulation.info.ocm_enabled=0;
	       break;
	     case 'k':
	       //ricean_factor = atof (optarg);
	       LOG_E(EMU,"[SIM] Option k is no longer supported on the command line. Please specify your channel model in the xml template\n");
	       exit(-1);
	       break;
	     case 't':
	       //Td = atof (optarg);
	       LOG_E(EMU,"[SIM] Option t is no longer supported on the command line. Please specify your channel model in the xml template\n");
	       exit(-1);
	       break;
	     case 'f':
	       argvars->forgetting_factor = atof (optarg);
	       break;
	     case 'z':
	       argvars->cooperation_flag = atoi (optarg);
	       break;
	     case 'u':
	       oai_emulation.info.nb_ue_local = atoi (optarg);
	       break;
	     case 'b':
	       oai_emulation.info.nb_enb_local = atoi (optarg);
	       break;
	     case 'a':
	       argvars->abstraction_flag = 1;
	       break;
	     case 'A':
	       oai_emulation.info.ocm_enabled=1;
	       if (optarg == NULL)
	 	oai_emulation.environment_system_config.fading.small_scale.selected_option="AWGN";
	       else
	 	oai_emulation.environment_system_config.fading.small_scale.selected_option= optarg;
	       //awgn_flag = 1;
	       break;
	     case 'p':
	       oai_emulation.info.nb_master = atoi (optarg);
	       break;
	     case 'M':
	       argvars->abstraction_flag = 1;
	       argvars->ethernet_flag = 1;
	       oai_emulation.info.ethernet_id = atoi (optarg);
	       oai_emulation.info.master_id = oai_emulation.info.ethernet_id;
	       oai_emulation.info.ethernet_flag = 1;
	       break;
	     case 'e':
	       oai_emulation.info.extended_prefix_flag = 1;
	       break;
	     case 'l':
	       oai_emulation.info.g_log_level = atoi(optarg);
	       break;
	     case 'c':
	       strcpy(oai_emulation.info.local_server, optarg);
	       oai_emulation.info.ocg_enabled=1;
	       break;
	     case 'g':
	       oai_emulation.info.multicast_group = atoi (optarg);
	       break;
	     case 'B':
	       oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option = optarg;
	       break;
	     case 'U':
	       oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = optarg;
	       break;
	     case 'T':
	       oai_emulation.info.otg_enabled = 1;
	       oai_emulation.info.otg_traffic = atoi (optarg);
	       break;
	     case 'P':
	       oai_emulation.info.opt_enabled = 1;
	       oai_emulation.info.opt_mode = atoi (optarg);
	       break;
	     case 'E':
	       argvars->set_seed = 1;
	       oai_emulation.info.seed = atoi (optarg);
	       break;
	     case 'I':
	       oai_emulation.info.cli_enabled = 1;
	       break;
	     case 'X':
	       temp=atoi(optarg);
	       if(temp==0){
	       argvars->port=CHANNEL_PORT; argvars->Channel_Flag=1; argvars->Process_Flag=0; argvars->wgt=0; }
	       else if(temp==1){
	       argvars->port=eNB_PORT; argvars->wgt=0;}
	       else {
	       argvars->port=UE_PORT; argvars->wgt=MAX_eNB;}
	       break;
	     case 'i':
	      argvars->Process_Flag=1;
	      argvars->node_id = argvars->wgt+atoi(optarg);
	      argvars->port+=atoi(optarg);
	      break;
	     case 'v':
	       oai_emulation.info.omv_enabled = 1;
	       break;
	     case 'V':
	       ouput_vcd = 1;
	       oai_emulation.info.vcd_enabled = 1;
	       break;
	     case 'G' :
	       oai_emulation.info.otg_bg_traffic_enabled = 1;
	       break;
	     case 'j' :
	    	 oai_emulation.info.nb_rn_local = atoi(optarg);
	       if( oai_emulation.info.nb_rn_local > 0){
	    	   argvars->relay_flag = 1 ;
	       }
	       break;
	     default:
	       help ();
	       exit (-1);
	       break;
	     }
	   }

}

//MAIN FUNCTION -- SIMULATION STARTS HERE...
int main (int argc, char **argv)
{
	  LTE_DL_FRAME_PARMS *frame_parms;
	  FILE *UE_stats[NUMBER_OF_UE_MAX], *eNB_stats, *eNB_avg_thr;
	  char UE_stats_filename[255];
	  //Definition of Argument Variables defined in tworelaydiamond.h for read_argument() function.
	  ArgumentVars argvars;
	  double **s_re, **s_im, **r_re, **r_im, **r_re0, **r_im0;
	  double forgetting_factor=0.0;
	  int map1,map2;
	  double **ShaF= NULL;
	  int eNB_id, UE_id, RN_id,i;
	  u32 frame=0;

	  // time calibration for soft realtime mode
	  struct timespec time_spec;
	  unsigned long time_last, time_now;
	  int td, td_avg, sleep_time_us;

	  // Framing variables
	  s32 slot, last_slot, next_slot;
	  lte_subframe_t direction;



	  // Initialize OpenAirInterface Emulation environment
	  init_oai_emulation(); // to initialize everything !!!

	  //Read the arguments from the command line
	  read_arguments(&argvars, argc, argv);

	  // Coonfigure oaisim with OCG
	  oaisim_config(); // config OMG and OCG, OPT, OTG, OLG


	  // Initialize VCD LOG module
	  // TODO: Do I really need VCD waveform  dump to a file??
	  //vcd_signal_dumper_init();

	  // ASSERTIONs
	  if (oai_emulation.info.nb_ue_local > NUMBER_OF_UE_MAX ) {
		  LOG_E(EMU,"Enter fewer than %d UEs for the moment or change the NUMBER_OF_UE_MAX\n", NUMBER_OF_UE_MAX);
		  exit (-1);
	  }

	  if (oai_emulation.info.nb_enb_local > NUMBER_OF_eNB_MAX) {
		  LOG_E(EMU,"Enter fewer than %d eNBs for the moment or change the NUMBER_OF_UE_MAX\n", NUMBER_OF_eNB_MAX);
		  exit (-1);
	  }

	  if (oai_emulation.info.nb_rn_local > (NUMBER_OF_UE_MAX - oai_emulation.info.nb_ue_local ) || oai_emulation.info.nb_rn_local > (NUMBER_OF_eNB_MAX - oai_emulation.info.nb_enb_local )  ) {
		  LOG_E(EMU,"Enter fewer than %d or %d RNs for the moment or change the NUMBER_OF_UE_MAX or NUMBER_OF_eNB_MAX\n", NUMBER_OF_UE_MAX,NUMBER_OF_eNB_MAX);
		  exit (-1);
	  }

	  if(argvars.ethernet_flag == 1){
		  LOG_E(EMU,"ETHERNET EMULATION IS NOT SUPPORTED AT THIS POINT IN TWO RELAY SIMULATION \n");
		  exit (-1);
	  }

	  if (argvars.set_sinr == 0){
	    argvars.sinr_dB = argvars.snr_dB - 20;
	  }

	  NB_UE_INST = oai_emulation.info.nb_ue_local;
	  NB_eNB_INST = oai_emulation.info.nb_enb_local;
	  NB_RN_INST =  oai_emulation.info.nb_rn_local;

	  //IGNORED OMV (Assuming that this is mobility visualiser)
	  //TODO: ADD OMV support if necessary


	  #ifdef PRINT_STATS
	  	  	  for (argvars.UE_id=0;argvars.UE_id<NB_UE_INST;argvars.UE_id++) {
				  sprintf(UE_stats_filename,"UE_stats%d.txt",argvars.UE_id);
				  UE_stats[argvars.UE_id] = fopen (UE_stats_filename, "w");
			  }
			  eNB_stats = fopen ("eNB_stats.txt", "w");
			  printf ("UE_stats=%p, eNB_stats=%p\n", UE_stats, eNB_stats);

			  eNB_avg_thr = fopen ("eNB_stats_th.txt", "w");

	  #endif

	 //LOG KEEPING FOR THE INITIAL
	 LOG_I(EMU,"total number of UE %d (local %d, remote %d) mobility %s \n", NB_UE_INST,oai_emulation.info.nb_ue_local,oai_emulation.info.nb_ue_remote, oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option);
	 LOG_I(EMU,"Total number of RN %d (local %d) mobility %s \n", NB_RN_INST,oai_emulation.info.nb_rn_local, oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option);
	 LOG_I(EMU,"Total number of eNB %d (local %d, remote %d) mobility %s \n", NB_eNB_INST,oai_emulation.info.nb_enb_local,oai_emulation.info.nb_enb_remote, oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option);
	 LOG_I(OCM,"Running with frame_type %d, Nid_cell %d, N_RB_DL %d, EP %d, mode %d, target dl_mcs %d, rate adaptation %d, nframes %d, abstraction %d, channel %s\n",
					  oai_emulation.info.frame_type, argvars.Nid_cell, oai_emulation.info.N_RB_DL, oai_emulation.info.extended_prefix_flag, oai_emulation.info.transmission_mode,argvars.target_dl_mcs,argvars.rate_adaptation_flag,oai_emulation.info.n_frames,argvars.abstraction_flag,oai_emulation.environment_system_config.fading.small_scale.selected_option);

	//RANDOM FUNCTION INITIALIZATION
	if(argvars.set_seed){
		randominit (oai_emulation.info.seed);
		set_taus_seed (oai_emulation.info.seed);
	} else {
		randominit (0);
		set_taus_seed (0);
	}

	 //INITIALIZE LTE VARIABLES
     init_lte_vars (&frame_parms, oai_emulation.info.frame_type, oai_emulation.info.tdd_config, oai_emulation.info.tdd_config_S,oai_emulation.info.extended_prefix_flag,oai_emulation.info.N_RB_DL, argvars.Nid_cell, argvars.cooperation_flag, oai_emulation.info.transmission_mode, argvars.abstraction_flag, argvars.relay_flag);


     //INIT eNB, UE and RN data structures...
     //TODO: enb_data ??
     for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
       enb_data[eNB_id] = (node_desc_t *)malloc(sizeof(node_desc_t));
       init_enb(enb_data[eNB_id],oai_emulation.environment_system_config.antenna.eNB_antenna);
     }

     for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
       ue_data[UE_id] = (node_desc_t *)malloc(sizeof(node_desc_t));
       init_ue(ue_data[UE_id],oai_emulation.environment_system_config.antenna.UE_antenna);
     }

     //FIXME: This part does not work with the RN nodes.

     for (RN_id = 0; RN_id < NB_RN_INST; RN_id++) {
       rn_data[RN_id]->ue = (node_desc_t *)malloc(sizeof(node_desc_t));
       rn_data[RN_id]->enb = (node_desc_t *)malloc(sizeof(node_desc_t));
       init_ue(rn_data[RN_id]->ue,oai_emulation.environment_system_config.antenna.UE_antenna);
       init_enb(rn_data[RN_id]->enb,oai_emulation.environment_system_config.antenna.eNB_antenna);
     }


     //SOME OCM RELATED ASSIGNMENTS

     if ((oai_emulation.info.ocm_enabled == 1)&& (argvars.ethernet_flag == 0 ) &&
         (oai_emulation.environment_system_config.fading.shadowing.decorrelation_distance_m>0) &&
         (oai_emulation.environment_system_config.fading.shadowing.variance_dB>0)) {
       // init SF map here!!!
       map1 =(int)oai_emulation.topology_config.area.x_m;
       map2 =(int)oai_emulation.topology_config.area.y_m;
       ShaF = init_SF(map1,map2,oai_emulation.environment_system_config.fading.shadowing.decorrelation_distance_m,oai_emulation.environment_system_config.fading.shadowing.variance_dB);

       // size of area to generate shadow fading map
       LOG_D(EMU,"Simulation area x=%f, y=%f\n",
   	  oai_emulation.topology_config.area.x_m,
   	  oai_emulation.topology_config.area.y_m);
     }


     // INITIALIZE CHANNEL VAIRABLES FOR THE REAL & IMAGINARY PARTS OF RECEIVER AND SOURCE RADIOs.
    //if (argvars.abstraction_flag == 0 && argvars.Process_Flag==0 && argvars.Channel_Flag==0){
   	//  init_channel_vars (frame_parms, &s_re, &s_im, &r_re, &r_im, &r_re0, &r_im0);
    //}

     // initialize channel descriptors
       for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
         for (RN_id = 0; RN_id < NB_RN_INST; RN_id++) {
           LOG_D(OCM,"Initializing channel (%s, %d) from eNB %d to RN %d\n", oai_emulation.environment_system_config.fading.small_scale.selected_option,
     	    map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option), eNB_id, RN_id);

     	eNB2RN[eNB_id][RN_id] = new_channel_desc_scm(PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_tx,
     						     PHY_vars_RN_g[RN_id]->ue->lte_frame_parms.nb_antennas_rx,
     						     map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option),
     						     oai_emulation.environment_system_config.system_bandwidth_MB,
     						     forgetting_factor,
     						     0,
     						     0);
           random_channel(eNB2RN[eNB_id][RN_id]);


           RN2eNB[RN_id][eNB_id] = new_channel_desc_scm(PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.nb_antennas_tx,
                             						     PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_rx,
                             						     map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option),
                             						     oai_emulation.environment_system_config.system_bandwidth_MB,
                             						     forgetting_factor,
                             						     0,
                             						     0);
                        random_channel(RN2eNB[RN_id][eNB_id]);

		   //Generate the frequency response of the channels
           freq_channel(eNB2RN[eNB_id][RN_id],PHY_vars_RN_g[0]->ue->lte_frame_parms.N_RB_DL,PHY_vars_RN_g[0]->ue->lte_frame_parms.N_RB_DL*12+1);
           freq_channel(RN2eNB[RN_id][eNB_id],PHY_vars_eNB_g[0]->lte_frame_parms.N_RB_DL,PHY_vars_eNB_g[0]->lte_frame_parms.N_RB_DL*12+1);


         }
       }


       for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
    	   for (RN_id = 0; RN_id < NB_RN_INST; RN_id++) {

    		   LOG_D(OCM,"[SIM] Initializing channel (%s, %d) from RN %d to eNB %d\n", oai_emulation.environment_system_config.fading.small_scale.selected_option,
    				   map_str_to_int(small_scale_names, oai_emulation.environment_system_config.fading.small_scale.selected_option),UE_id, eNB_id);
    		   UE2RN[UE_id][RN_id] = new_channel_desc_scm(PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_tx,
    				   PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.nb_antennas_rx,
    				   map_str_to_int(small_scale_names, oai_emulation.environment_system_config.fading.small_scale.selected_option),
    				   oai_emulation.environment_system_config.system_bandwidth_MB,
    				   forgetting_factor,
    				   0,
    				   0);
    		   random_channel(UE2RN[UE_id][RN_id]);


               RN2UE[RN_id][UE_id] = new_channel_desc_scm(PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.nb_antennas_tx,
                    						     PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_rx,
                    						     map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option),
                    						     oai_emulation.environment_system_config.system_bandwidth_MB,
                    						     forgetting_factor,
                    						     0,
                    						     0);
               random_channel(RN2UE[RN_id][UE_id]);

    		   //Generate the frequency response of the channel
               freq_channel(UE2RN[UE_id][RN_id],PHY_vars_RN_g[0]->eNB->lte_frame_parms.N_RB_DL,PHY_vars_RN_g[0]->eNB->lte_frame_parms.N_RB_DL*12+1);
               freq_channel(RN2UE[RN_id][UE_id],PHY_vars_RN_g[0]->ue->lte_frame_parms.N_RB_DL,PHY_vars_RN_g[0]->ue->lte_frame_parms.N_RB_DL*12+1);


    	   }
       }


       //TODO: What is this number_of_cards variable??
       //number_of_cards = 1;


       // INITIALIZE UE_MODE STATES FOR EACH UE NODE
       for (UE_id=0; UE_id<NB_UE_INST;UE_id++){
           PHY_vars_UE_g[UE_id]->rx_total_gain_dB=120;
           // update UE_mode for each eNB_id not just 0
           if (argvars.abstraction_flag == 0){
             PHY_vars_UE_g[UE_id]->UE_mode[0] = NOT_SYNCHED;
           }
           else {
             // 0 is the index of the connected eNB
             PHY_vars_UE_g[UE_id]->UE_mode[0] = PRACH;
           }
           PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti = 0x1235 + UE_id;
           PHY_vars_UE_g[UE_id]->current_dlsch_cqi[0] = 10;

           LOG_I(EMU, "UE %d mode is initialized to %d with crnti %d \n", UE_id, PHY_vars_UE_g[UE_id]->UE_mode[0] , PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti);
         }


       // INITIALIZE RN->UE_MODE STATES FOR EACH RN NODE
       for (RN_id=0; RN_id<NB_RN_INST;RN_id++){
                  PHY_vars_RN_g[RN_id]->ue->rx_total_gain_dB=120;
                  // update UE_mode for each eNB_id not just 0
                  if (argvars.abstraction_flag == 0){
                    PHY_vars_RN_g[RN_id]->ue->UE_mode[0] = NOT_SYNCHED;
                  }
                  else {
                    // 0 is the index of the connected eNB
                    PHY_vars_RN_g[RN_id]->ue->UE_mode[0] = PRACH;
                  }
                  PHY_vars_RN_g[RN_id]->ue->lte_ue_pdcch_vars[0]->crnti = 0x1235 +NB_UE_INST+ RN_id;
                  PHY_vars_RN_g[RN_id]->ue->current_dlsch_cqi[0] = 10;

                  LOG_I(EMU, "RN/UE %d mode is initialized to %d with crnti %d \n", RN_id, PHY_vars_RN_g[RN_id]->ue->UE_mode[0] ,PHY_vars_RN_g[RN_id]->ue->lte_ue_pdcch_vars[0]->crnti);
                }

       //===================== LAYER 2 STARTS HERE ====================================
       // Layer 2 Initialization
#ifdef OPENAIR2
       l2_init (&PHY_vars_eNB_g[0]->lte_frame_parms);


       //FIXME
       //TODO: Where is this function mrbch_phy_sync_failure() function defined? What it is used for?
       for (i = 0; i < NB_eNB_INST; i++)
    	   mac_xface->mrbch_phy_sync_failure (i, 0, i);

       //TODO : This function fails with this RN_id values...Correct it.
       //for (RN_id=max(NB_eNB_INST,NB_UE_INST); RN_id < max(NB_eNB_INST,NB_UE_INST)+NB_RN_INST;RN_id++)
       //    	   mac_xface->mrbch_phy_sync_failure (RN_id, 0, RN_id);

       mac_xface->macphy_exit = exit_fun;
#endif

       // time calibration for OAI
       clock_gettime (CLOCK_REALTIME, &time_spec);
       time_now = (unsigned long) time_spec.tv_nsec;
       td_avg = 0;
       sleep_time_us = SLEEP_STEP_US;
       td_avg = TARGET_SF_TIME_NS;

       LOG_I(EMU,">>>>>>>>>>>>>>>>>>>>>>>>>>> OAIEMU initialization done <<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");



       //FRAME SIMULATIONS
       for (frame=0; frame<oai_emulation.info.n_frames; frame++) {


    	   oai_emulation.info.frame = frame;
    	   //oai_emulation.info.time_ms += 1;
    	   oai_emulation.info.time_s += 0.1; // emu time in s, each frame lasts for 10 ms // JNote: TODO check the coherency of the time and frame (I corrected it to 10 (instead of 0.01)
    	   // if n_frames not set by the user or is greater than max num frame then set adjust the frame counter
    	   if ( (oai_emulation.info.n_frames_flag == 0) || (oai_emulation.info.n_frames >= 0xffff) ){
    		   frame %=(oai_emulation.info.n_frames-1);
    	   }

    	   printf("Frame %d\n", frame);


    	   //SET THE PATH LOSS VALUES FOR EACH CHANNEL.
    	   // WE ASSUME THAT CHANNELS BETWEEN A to B and B to A has the same pathloss (Symmetric).

    	   for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
    		   for (RN_id = 0; RN_id < NB_RN_INST; RN_id++) {

    			   printf("RN: %d, eNB: %d \n", RN_id, eNB_id);
    			   //UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + snr_dB;
    			   if (RN_id == 0) {
    				   eNB2RN[eNB_id][RN_id]->path_loss_dB = -105 + argvars.snr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
    				   RN2eNB[RN_id][eNB_id]->path_loss_dB = -105 + argvars.snr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower; //+20 to offset the difference in tx power of the UE wrt eNB
    			   }
    			   else {
    				   eNB2RN[eNB_id][RN_id]->path_loss_dB = -105 + argvars.sinr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
    				   RN2eNB[RN_id][eNB_id]->path_loss_dB = -105 + argvars.sinr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower; //+20 to offset the difference in tx power of the UE wrt eNB
    			   }
    			   LOG_I(OCM,"Path loss from eNB %d to RN %d => %f dB (eNB TX %d)\n",eNB_id,RN_id,eNB2RN[eNB_id][RN_id]->path_loss_dB,
    					   PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower);
    		   }
    	   }


    	   for (RN_id = 0; RN_id < NB_RN_INST; RN_id++) {
    		   for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {

    			   printf("UE: %d, RN: %d \n", UE_id, eNB_id);

    			   //UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + snr_dB;
    			   if (RN_id == (RN_id % NB_RN_INST)) {
    				   RN2UE[RN_id][UE_id]->path_loss_dB = -105 + argvars.snr_dB - PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.pdsch_config_common.referenceSignalPower;
    				   UE2RN[UE_id][RN_id]->path_loss_dB = -105 + argvars.snr_dB - PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.pdsch_config_common.referenceSignalPower; //+20 to offset the difference in tx power of the UE wrt eNB
    			   }
    			   else
    			   {
    				   RN2UE[RN_id][UE_id]->path_loss_dB = -105 + argvars.sinr_dB - PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.pdsch_config_common.referenceSignalPower;
    				   UE2RN[UE_id][RN_id]->path_loss_dB = -105 + argvars.sinr_dB - PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.pdsch_config_common.referenceSignalPower; //+20 to offset the difference in tx power of the UE wrt eNB
    			   }
    			   LOG_I(OCM,"Path loss from RN %d to UE %d => %f dB (RN/eNB TX %d)\n",RN_id,UE_id,RN2UE[RN_id][UE_id]->path_loss_dB,
    					   PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.pdsch_config_common.referenceSignalPower);
    		   }
    	   }

    	   //FOR EACH LTE FRAME (10 ms) , SIMULATE EACH 20 SLOTS one by one...
    	   for (slot=0 ; slot<20 ; slot++) {

    		   last_slot = (slot - 1)%20;

    		   if (last_slot <0){
    			   last_slot+=20;
    		   }
    		   next_slot = (slot + 1)%20;

    		   //INCREASE SIMULATION TIME AT THE BEGINNING OF EACH SLOT by 0.5 ms.
    		   oai_emulation.info.time_ms = frame * 10 + (next_slot>>1) ;

    		   //CHOOSE UPLINK / DOWNLINK DECISION BASED ON the subframe.
    		   //TODO: What is this next_slot>>1 thing? It does not give the current subframe...
    		   direction = subframe_select_HDrelay(frame_parms,next_slot>>1);
    		   printf("(Subframe %d) Direction: %d \n",next_slot>>1, direction);


    		   //TODO: What does it mean to have Channel_Flag=1. What is -X option in args is used for?? What does Channel_Func() function is used for?
#ifdef PROC
    		   if(argvars.Channel_Flag==1)
    			   Channel_Func(s_re2,s_im2,r_re2,r_im2,r_re02,r_im02,r_re0_d,r_im0_d,r_re0_u,r_im0_u,eNB2UE,UE2eNB,enb_data,ue_data,abstraction_flag,frame_parms,slot);
#endif
    		   if(argvars.Channel_Flag==0){


    			   /*******************************************
    			    *
    			    *  eNB procedures and preparation for new subframe
    			    *
    			    *
    			    *********************************************/
    			   // TODO: What next_slot==2 represent?? It does not show that current slot is a new subframe..
    			   // Prepare eNB for the new subframe (next_slot%2 ==0).
    			   if((next_slot %2) ==0)
    				   clear_eNB_transport_info(oai_emulation.info.nb_enb_local);

    			   for (eNB_id=oai_emulation.info.first_enb_local;
    					   (eNB_id<(oai_emulation.info.first_enb_local+oai_emulation.info.nb_enb_local)) && (oai_emulation.info.cli_start_enb[eNB_id]==1);
    					   eNB_id++) {
    				   //printf ("debug: Nid_cell %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell);
    				   //printf ("debug: frame_type %d,tdd_config %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.frame_type,PHY_vars_eNB_g[eNB_id]->lte_frame_parms.tdd_config);
    				   LOG_D(EMU,"PHY procedures eNB %d for frame %d, slot %d (subframe %d) TDD %d/%d Nid_cell %d\n",
    						   eNB_id, frame, slot, next_slot >> 1,
    						   PHY_vars_eNB_g[eNB_id]->lte_frame_parms.frame_type,
    						   PHY_vars_eNB_g[eNB_id]->lte_frame_parms.tdd_config,PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell);

    				   PHY_vars_eNB_g[eNB_id]->frame = frame;

    				   //relay_flag=0 sets this eNB as a standard eNB.
    				   PHY_vars_eNB_g[eNB_id]->relay_flag = 0 ;
    				   phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_eNB_g[eNB_id], argvars.abstraction_flag,argvars.relay_flag);
    				   	  //TODO: What are we going to do with the special subframes? Who is transmitting/receiving during these subframes??

    			   }



     			   /*******************************************
        			    *
        			    *  UE procedures and preparation for new subframe
        			    *
        			    *
        			    *********************************************/

    			   if ((next_slot % 2) == 0)
    				   clear_UE_transport_info (oai_emulation.info.nb_ue_local);

    			   for (UE_id = oai_emulation.info.first_ue_local;
    					   (UE_id < (oai_emulation.info.first_ue_local+oai_emulation.info.nb_ue_local)) && (oai_emulation.info.cli_start_ue[UE_id]==1);
    					   UE_id++)
    				   if (frame >= (UE_id * 20)) {	// activate UE only after 20*UE_id frames so that different UEs turn on separately

    					   LOG_D(EMU,"PHY procedures UE %d for frame %d, slot %d (subframe %d)\n",
    							   UE_id, frame, slot, next_slot >> 1);

    					   //This sets the relay flag = 0, (which means this UE is an ordinary UE).
    					   PHY_vars_UE_g[UE_id]->relay_flag = 0;

    					   if (PHY_vars_UE_g[UE_id]->UE_mode[0] != NOT_SYNCHED) {
    						   if (frame>0) {
    							   PHY_vars_UE_g[UE_id]->frame = frame;
    							   phy_procedures_UE_lte (last_slot, next_slot, PHY_vars_UE_g[UE_id], 0, argvars.abstraction_flag,normal_txrx, argvars.relay_flag);
    						   }
    					   }
    					   else {
    						   if (argvars.abstraction_flag==1){
    							   LOG_E(EMU, "sync not supported in abstraction mode (UE%d,mode%d)\n", UE_id, PHY_vars_UE_g[UE_id]->UE_mode[0]);
    							   exit(-1);
    						   }
    						   if ((frame>0) && (last_slot == (LTE_SLOTS_PER_FRAME-2))) {
    							   initial_sync(PHY_vars_UE_g[UE_id]);

    						   }
    					   }

    				   }



     			   /*******************************************
        			    *
        			    *  Relay Node (RN) procedures and preparation for new subframe
        			    *
        			    *
        			    *********************************************/

    			   //TODO: To be implemented...

    			   //FIXME: What is this transport_info clearing? When does these structures created and filled?
    			   if((next_slot %2) ==0){
    				  // clear_RN_transport_info(oai_emulation.info.nb_rn_local);
    			   }

    			   for (RN_id=oai_emulation.info.first_rn_local;
    					   (RN_id<(oai_emulation.info.first_rn_local+oai_emulation.info.nb_rn_local)) && (oai_emulation.info.cli_start_rn[RN_id]==1);
    					   RN_id++) {
    				   //printf ("debug: Nid_cell %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell);
    				   //printf ("debug: frame_type %d,tdd_config %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.frame_type,PHY_vars_eNB_g[eNB_id]->lte_frame_parms.tdd_config);
    				   LOG_D(EMU,"PHY procedures RN/eNB %d for frame %d, slot %d (subframe %d) TDD %d/%d Nid_cell %d\n",
    						   RN_id, frame, slot, next_slot >> 1,
    						   PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.frame_type,
    						   PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.tdd_config,PHY_vars_RN_g[RN_id]->eNB->lte_frame_parms.Nid_cell);

    				   PHY_vars_RN_g[RN_id]->eNB->frame = frame;

    				   //relay_flag=0 sets this eNB as a standard eNB.
    				   PHY_vars_RN_g[RN_id]->eNB->relay_flag = 1+ (RN_id %2);
    				   PHY_vars_RN_g[RN_id]->ue->relay_flag = 1+ (RN_id %2);

    				   //Procedures for eNB side of RN node
    				   phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_RN_g[RN_id]->eNB, argvars.abstraction_flag,argvars.relay_flag);

    				   //Procedures for UE side of RN node
					   if (PHY_vars_RN_g[RN_id]->ue->UE_mode[0] != NOT_SYNCHED) {
						   if (frame>0) {
							   PHY_vars_RN_g[RN_id]->ue->frame = frame;
							   phy_procedures_UE_lte (last_slot, next_slot, PHY_vars_RN_g[RN_id]->ue, 0, argvars.abstraction_flag,normal_txrx, argvars.relay_flag);
						   }
					   }
					   else {
						   if (argvars.abstraction_flag==1){
							   LOG_E(EMU, "sync not supported in abstraction mode (UE%d,mode%d)\n", RN_id, PHY_vars_RN_g[RN_id]->ue->UE_mode[0]);
							   exit(-1);
						   }
						   if ((frame>0) && (last_slot == (LTE_SLOTS_PER_FRAME-2))) {
							   initial_sync(PHY_vars_RN_g[RN_id]->ue);

						   }
					   }



    			   }

    			   //TODO: Stuff for ethernet emulation. Remove it safely??
    			   //emu_transport (frame, last_slot, next_slot,direction, oai_emulation.info.frame_type, ethernet_flag);

    			   //Create somekind of new function like do_DL_sig() that will not only go over phy_vars_eNB[], but also phy_vars_RN[]->eNB and similarly UE structs.
    			   //  OR add these into the do_DL_sig() function (with a relay flag, otherwise it dlsim etc. will not work).

    			   if ((direction  == SF_DL)|| (frame_parms->frame_type==0)){
    				   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2RN[0][0],enb_data[0],rn_data[0].ue,next_slot,argvars.abstraction_flag,frame_parms);
    				   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,RN2UE[1][0],rn_data.enb,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

    			   }
       			   if ((direction  == SF_DL2)|| (frame_parms->frame_type==0)){
       				   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2RN[0][1],enb_data,rn_data.ue,next_slot,argvars.abstraction_flag,frame_parms);
       				   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,RN2UE[0][0],rn_data.enb,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

        			   }
    			   if ((direction  == SF_UL)|| (frame_parms->frame_type==0)){
    				   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);
    				   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

    			   }
    			   if ((direction  == SF_UL2)|| (frame_parms->frame_type==0)){
    				   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);
    				   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

    			   }

    			   if ((direction == SF_S)) {//it must be a special subframe
    				   if (next_slot%2==0) {//DL part
    					   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);
    					   do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

    				   }
    				   else {// UL part
    					   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);
    					   do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,argvars.abstraction_flag,frame_parms);

    				   }
    			   }



/*
    			   if ((last_slot == 1) && (frame == 0)
    					   && (argvars.abstraction_flag == 0) && (oai_emulation.info.n_frames == 1)) {

    				   write_output ("dlchan0.m", "dlch0",
    						   &(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][0][0]),
    						   (6 * (PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)), 1, 1);
    				   write_output ("dlchan1.m", "dlch1",
    						   &(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[1][0][0]),
    						   (6 * (PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)), 1, 1);
    				   write_output ("dlchan2.m", "dlch2",
    						   &(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[2][0][0]),
    						   (6 * (PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)), 1, 1);
    				   write_output ("pbch_rxF_comp0.m", "pbch_comp0",
    						   PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->rxdataF_comp[0], 6 * 12 * 4, 1, 1);
    				   write_output ("pbch_rxF_llr.m", "pbch_llr",
    						   PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->llr, (frame_parms->Ncp == 0) ? 1920 : 1728, 1, 4);
    			   }
*/
    			   if (next_slot %2 == 0){
    				   clock_gettime (CLOCK_REALTIME, &time_spec);
    				   time_last = time_now;
    				   time_now = (unsigned long) time_spec.tv_nsec;
    				   td = (int) (time_now - time_last);
    				   if (td>0) {
    					   td_avg = (int)(((K*(long)td) + (((1<<3)-K)*((long)td_avg)))>>3); // in us
    					   LOG_D(EMU,"sleep frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",
    							   frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
    				   }
    				   if (td_avg<(TARGET_SF_TIME_NS - SF_DEVIATION_OFFSET_NS)){
    					   sleep_time_us += SLEEP_STEP_US;
    					   LOG_D(EMU,"[TIMING]Fater than realtime increase the avg sleep time for %d us, frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",	sleep_time_us,frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
    				   }
    				   else if (td_avg > (TARGET_SF_TIME_NS + SF_DEVIATION_OFFSET_NS)) {
    					   sleep_time_us-= SLEEP_STEP_US;
    					   LOG_D(EMU,"[TIMING]Slower than realtime reduce the avg sleep time for %d us, frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",	sleep_time_us,frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
    				   }
    			   } // end if next_slot%2



    		   }// if Channel_Flag==0

    	   }


       }


       // added for PHY abstraction
       if (oai_emulation.info.ocm_enabled == 1) {
    	   for (eNB_id = 0; eNB_id < NUMBER_OF_eNB_MAX; eNB_id++)
    		   free(enb_data[eNB_id]);

    	   for (UE_id = 0; UE_id < NUMBER_OF_UE_MAX; UE_id++)
    		   free(ue_data[UE_id]);

    	   for (RN_id = 0; RN_id < NUMBER_OF_eNB_MAX; RN_id++)
    		   free(rn_data[RN_id]);
       } //End of PHY abstraction changes

#ifdef OPENAIR2
       mac_top_cleanup();
#endif

	  return 0;

}

