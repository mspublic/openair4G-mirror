#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cblas.h>
#include <execinfo.h>

 
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

//#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
//#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"

#ifdef IFFT_FPGA
//#include "PHY/LTE_REFSIG/mod_table.h"
#endif //IFFT_FPGA

#include "SCHED/defs.h"
#include "SCHED/vars.h"

#ifdef XFORMS
#include "PHY/TOOLS/lte_phy_scope.h"
#endif

#include "oaisim.h"
#include "oaisim_config.h"
#include "UTIL/OCG/OCG_extern.h"
#include "cor_SF_sim.h"
#include "UTIL/OMG/omg_constants.h"


//#ifdef PROC
#include "../PROC/interface.h"
#include "../PROC/channel_sim_proc.h"
#include "../PROC/Tsync.h"
#include "../PROC/Process.h"
//#endif

#include "UTIL/LOG/vcd_signal_dumper.h"

#define RF

//#define DEBUG_SIM

#define MCS_COUNT 24//added for PHY abstraction
#define N_TRIALS 1

/*
  DCI0_5MHz_TDD0_t          UL_alloc_pdu;
  DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
  DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
  DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
 */

#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,24)
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define RA_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,3)
#define DLSCH_RB_ALLOC 0x1fff

#define DECOR_DIST 100
#define SF_VAR 10

//constant for OAISIM soft realtime calibration
#define SF_DEVIATION_OFFSET_NS 100000 //= 0.1ms : should be as a number of UE
#define SLEEP_STEP_US       100 //  = 0.01ms could be adaptive, should be as a number of UE
#define K 2                  // averaging coefficient 
#define TARGET_SF_TIME_NS 1000000       // 1ms = 1000000 ns

//#ifdef OPENAIR2
//u16 NODE_ID[1];
//u8 NB_INST = 2;
//#endif //OPENAIR2
char stats_buffer[16384];
channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];
//Added for PHY abstraction
node_desc_t *enb_data[NUMBER_OF_eNB_MAX]; 
node_desc_t *ue_data[NUMBER_OF_UE_MAX];
//double sinr_bler_map[MCS_COUNT][2][16];
//double sinr_bler_map_up[MCS_COUNT][2][16];
//extern double SINRpost_eff[301];
//extern int mcsPost; 
//extern int  nrbPost; 
//extern int frbPost;
//extern void kpi_gen();

// this should reflect the channel models in openair1/SIMULATION/TOOLS/defs.h
mapping small_scale_names[] = {
    {"custom", custom},
    {"SCM_A", SCM_A},
    {"SCM_B", SCM_B},
    {"SCM_C", SCM_C},
    {"SCM_D", SCM_D},
    {"EPA", EPA},
    {"EVA", EVA},
    {"ETU", ETU},
    {"Rayleigh8", Rayleigh8},
    {"Rayleigh1", Rayleigh1},
    {"Rayleigh1_800", Rayleigh1_800},
    {"Rayleigh1_corr", Rayleigh1_corr},
    {"Rayleigh1_anticorr", Rayleigh1_anticorr},
    {"Rice8", Rice8},
    {"Rice1", Rice1},
    {"Rice1_corr", Rice1_corr},
    {"Rice1_anticorr", Rice1_anticorr},
    {"AWGN", AWGN},
    {NULL, -1}
};

//static void *sigh(void *arg);
void terminate(void);
void exit_fun(const char* s);

void 
help (void) {
  printf
    ("Usage: oaisim -h -a -F -C tdd_config -V -R N_RB_DL -e -x transmission_mode -m target_dl_mcs -r(ate_adaptation) -n n_frames -s snr_dB -k ricean_factor -t max_delay -f forgetting factor -A channel_model -z cooperation_flag -u nb_local_ue -U UE mobility -b nb_local_enb -B eNB_mobility -M ethernet_flag -p nb_master -g multicast_group -l log_level -c ocg_enable -T traffic model -D multicast network device\n");

  printf ("-h provides this help message!\n");
  printf ("-a Activates PHY abstraction mode\n");
  printf ("-F Activates FDD transmission (TDD is default)\n");
  printf ("-C [0-6] Sets TDD configuration\n");
  printf ("-L [0-1] 0 to disable new link adaptation, 1 to enable new link adapatation\n");
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
  printf ("-O [mme ipv4 address] Enable MME mode\n");
  printf ("-Z Reserved\n");
}

#ifdef OPENAIR2
int omv_write (int pfd,  Node_list enb_node_list, Node_list ue_node_list, Data_Flow_Unit omv_data){
  int i,j;
  omv_data.end=0;
  //omv_data.total_num_nodes = NB_UE_INST + NB_eNB_INST;
  for (i=0;i<NB_eNB_INST;i++) {
    if (enb_node_list != NULL) {
      omv_data.geo[i].x = (enb_node_list->node->X_pos < 0.0)? 0.0 : enb_node_list->node->X_pos;
      omv_data.geo[i].y = (enb_node_list->node->Y_pos < 0.0)? 0.0 : enb_node_list->node->Y_pos;
      omv_data.geo[i].z = 1.0;
      omv_data.geo[i].mobility_type = oai_emulation.info.omg_model_enb;
      omv_data.geo[i].node_type = 0; //eNB
      enb_node_list = enb_node_list->next;
      omv_data.geo[i].Neighbors=0;
      for (j=NB_eNB_INST; j< NB_UE_INST + NB_eNB_INST ; j++){
        if (is_UE_active(i,j - NB_eNB_INST ) == 1) {
          omv_data.geo[i].Neighbor[omv_data.geo[i].Neighbors]=  j; 
          omv_data.geo[i].Neighbors++; 
          LOG_D(OMG,"[eNB %d][UE %d] is_UE_active(i,j) %d geo (x%d, y%d) num neighbors %d\n", i,j-NB_eNB_INST, is_UE_active(i,j-NB_eNB_INST), 
                omv_data.geo[i].x, omv_data.geo[i].y, omv_data.geo[i].Neighbors);
        } 
      } 
    }
  }
  for (i=NB_eNB_INST;i<NB_UE_INST+NB_eNB_INST;i++) {
    if (ue_node_list != NULL) {
      omv_data.geo[i].x = (ue_node_list->node->X_pos < 0.0) ? 0.0 : ue_node_list->node->X_pos;
      omv_data.geo[i].y = (ue_node_list->node->Y_pos < 0.0) ? 0.0 : ue_node_list->node->Y_pos;
      omv_data.geo[i].z = 1.0;
      omv_data.geo[i].mobility_type = oai_emulation.info.omg_model_ue;
      omv_data.geo[i].node_type = 1; //UE
      //trial
      omv_data.geo[i].state = 1;
      omv_data.geo[i].rnti = 88;
      omv_data.geo[i].connected_eNB = 0;
      omv_data.geo[i].RSRP = 66;
      omv_data.geo[i].RSRQ = 55;
      omv_data.geo[i].Pathloss = 44;
      omv_data.geo[i].RSSI[0] = 33;
      omv_data.geo[i].RSSI[1] = 22;
      omv_data.geo[i].RSSI[2] = 11;
      
      ue_node_list = ue_node_list->next;
      omv_data.geo[i].Neighbors=0;
         for (j=0; j< NB_eNB_INST ; j++){
        if (is_UE_active(j,i-NB_eNB_INST) == 1) {
          omv_data.geo[i].Neighbor[ omv_data.geo[i].Neighbors]=j;       
          omv_data.geo[i].Neighbors++; 
          LOG_D(OMG,"[UE %d][eNB %d] is_UE_active  %d geo (x%d, y%d) num neighbors %d\n", i-NB_eNB_INST,j, is_UE_active(j,i-NB_eNB_INST), 
                omv_data.geo[i].x, omv_data.geo[i].y, omv_data.geo[i].Neighbors);
        } 
        }
    }
  }
 
  if( write( pfd, &omv_data, sizeof(struct Data_Flow_Unit) ) == -1 )
   perror( "write omv failed" );
  return 1;
}

void omv_end (int pfd, Data_Flow_Unit omv_data) {
  omv_data.end=1;
  if( write( pfd, &omv_data, sizeof(struct Data_Flow_Unit) ) == -1 )
    perror( "write omv failed" );
}
#endif 
int
main (int argc, char **argv)
{
  char c;
  s32 i, j;
  int new_omg_model; // goto ocg in oai_emulation.info.
  // pointers signal buffers (s = transmit, r,r0 = receive)
  double **s_re, **s_im, **r_re, **r_im, **r_re0, **r_im0;
  double forgetting_factor=0.0;
  int map1,map2;
  double **ShaF= NULL;
  u32 frame=0;

  // Framing variables
  s32 slot, last_slot, next_slot;

  //FILE *SINRpost;
  //char SINRpost_fname[512];
  // sprintf(SINRpost_fname,"postprocSINR.m");
  //SINRpost = fopen(SINRpost_fname,"w");
  // variables/flags which are set by user on command-line
  double snr_dB, sinr_dB,snr_direction,snr_step=1.0;//,sinr_direction;
  u8 set_sinr=0;//,set_snr=0;
  u8 ue_connection_test=0;
  u8 set_seed=0;
  u8 cooperation_flag;          // for cooperative communication
  u8 target_dl_mcs = 4;
  u8 target_ul_mcs = 2;
  u8 rate_adaptation_flag;

  u8 abstraction_flag = 0, ethernet_flag = 0;
  u16 Nid_cell = 0;
  s32 UE_id, eNB_id,ret;

  // time calibration for soft realtime mode  
  struct timespec time_spec;
  unsigned long time_last, time_now;
  int td, td_avg, sleep_time_us;
  u8 beta_ACK=0,beta_RI=0,beta_CQI=2;
  lte_subframe_t direction;
#ifdef OPENAIR2
  // omv related info
  //pid_t omv_pid;
  char full_name[200];
  int pfd[2]; // fd for omv : fixme: this could be a local var
  char fdstr[10];
  char frames[10];
  char num_enb[10];
  char num_ue[10];
  //area_x, area_y and area_z for omv
  char x_area[20];
  char y_area[20];  
  char z_area[20];
  char nb_antenna[20];
  char frame_type[10];
  char tdd_config[10];
#endif   
  char fname[64],vname[64];
  // u8 awgn_flag = 0;
#ifdef XFORMS
  // current status is that every UE has a DL scope for a SINGLE eNB (eNB_id=0)
  // at eNB 0, an UL scope for every UE 
  FD_lte_phy_scope_ue  *form_ue[NUMBER_OF_UE_MAX];
  FD_lte_phy_scope_enb *form_enb[NUMBER_OF_UE_MAX];
  char title[255];
#endif
  LTE_DL_FRAME_PARMS *frame_parms;

#ifdef PRINT_STATS
  int len;

  FILE *UE_stats[NUMBER_OF_UE_MAX], *UE_stats_th[NUMBER_OF_UE_MAX], *eNB_stats, *eNB_avg_thr;
  char UE_stats_filename[255];

  char UE_stats_th_filename[255];
 


  char eNB_stats_th_filename[255];
 #endif

  int nb_antennas_rx=1;

  //time_t t0,t1;
  //clock_t start, stop;
  
  // Added for PHY abstraction
  Node_list ue_node_list = NULL;
  Node_list enb_node_list = NULL;
#ifdef OPENAIR2
  Data_Flow_Unit omv_data ;
#endif //ALU

#ifdef PROC
  int node_id;
  int port,Process_Flag=0,wgt,Channel_Flag=0,temp;
#endif
  //double **s_re2[MAX_eNB+MAX_UE], **s_im2[MAX_eNB+MAX_UE], **r_re2[MAX_eNB+MAX_UE], **r_im2[MAX_eNB+MAX_UE], **r_re02, **r_im02;
  //double **r_re0_d[MAX_UE][MAX_eNB], **r_im0_d[MAX_UE][MAX_eNB], **r_re0_u[MAX_eNB][MAX_UE],**r_im0_u[MAX_eNB][MAX_UE];
  //default parameters
  rate_adaptation_flag = 0;
  oai_emulation.info.n_frames = 0xffff;//1024;          //10;
  oai_emulation.info.n_frames_flag = 0;//fixme
  snr_dB = 30;
  cooperation_flag = 0;         // default value 0 for no cooperation, 1 for Delay diversity, 2 for Distributed Alamouti
  int eMBMS_active = 0;

  init_oai_emulation(); // to initialize everything !!!
  
   // get command-line options
  while ((c = getopt (argc, argv, "aA:b:B:c:C:d:eE:f:FGg:hi:IJ:k:L:l:m:M:n:N:oO:p:P:QrR:s:S:t:T:u:U:vVx:y:X:z:Z:")) != -1) {

    switch (c) {
    case 'L':                   // set FDD
      flag_LA = atoi(optarg);
      break;
    case 'F':                   // set FDD
      printf("Setting Frame to FDD\n");
      oai_emulation.info.frame_type = 0;
      break;
    case 'C':
      oai_emulation.info.tdd_config = atoi (optarg);
      if (oai_emulation.info.tdd_config > 6) {
        LOG_E(EMU,"Illegal tdd_config %d (should be 0-6)\n", oai_emulation.info.tdd_config);
        exit (-1);
      }
      break;
    case 'Q':
      eMBMS_active=1;
      break;
    case 'R':
      oai_emulation.info.N_RB_DL = atoi (optarg);
      if ((oai_emulation.info.N_RB_DL != 6) && (oai_emulation.info.N_RB_DL != 15) && (oai_emulation.info.N_RB_DL != 25)
          && (oai_emulation.info.N_RB_DL != 50) && (oai_emulation.info.N_RB_DL != 75) && (oai_emulation.info.N_RB_DL != 100)) {
        LOG_E(EMU,"Illegal N_RB_DL %d (should be one of 6,15,25,50,75,100)\n", oai_emulation.info.N_RB_DL);
        exit (-1);
      }
    case 'N':
      Nid_cell = atoi (optarg);
      if (Nid_cell > 503) {
        LOG_E(EMU,"Illegal Nid_cell %d (should be 0 ... 503)\n", Nid_cell);
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
    case 'y':
      nb_antennas_rx=atoi(optarg);
      if (nb_antennas_rx>4) {
	printf("Cannot have more than 4 antennas\n");
	exit(-1);
      }
      break;
    case 'm':
      target_dl_mcs = atoi (optarg);
      break;
    case 'r':
      rate_adaptation_flag = 1;
      break;
    case 'n':
      oai_emulation.info.n_frames = atoi (optarg);
      //n_frames = (n_frames >1024) ? 1024: n_frames; // adjust the n_frames if higher that 1024
      oai_emulation.info.n_frames_flag = 1;
      break;
    case 's':
      snr_dB = atoi (optarg);
      //      set_snr = 1;
      oai_emulation.info.ocm_enabled=0;
      break;
    case 'S':
      sinr_dB = atoi (optarg);
      set_sinr = 1;
      oai_emulation.info.ocm_enabled=0;
      break;
    case 'J':
      ue_connection_test=1;
      oai_emulation.info.ocm_enabled=0;
      snr_step = atof(optarg);
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
      forgetting_factor = atof (optarg);
      break;
    case 'z':
      cooperation_flag = atoi (optarg);
      break;
    case 'u':
      oai_emulation.info.nb_ue_local = atoi (optarg);
      break;
    case 'b':
      oai_emulation.info.nb_enb_local = atoi (optarg);
      break;
    case 'a':
      abstraction_flag = 1;
      break;
    case 'A':
      //oai_emulation.info.ocm_enabled=1;
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
      abstraction_flag = 1;
      ethernet_flag = 1;
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
    case 'D':
      oai_emulation.info.multicast_ifname = malloc(4*sizeof(char)); // allocate 4 byte for the interface name
      oai_emulation.info.multicast_ifname=optarg;
      break;
    case 'B':
      oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option = optarg;
      //oai_emulation.info.omg_model_enb = atoi (optarg);
      break;
    case 'U':
      oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = optarg;
      /*oai_emulation.info.omg_model_ue = atoi (optarg);
      switch (oai_emulation.info.omg_model_ue){
      case STATIC:
        oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "STATIC";
        break;
      case RWP:
        oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "RWP";
        break;
      case RWALK:
        oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "RWALK";
        break;
      case TRACE:
        oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "TRACE";
        break;
      case SUMO:
        oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "SUMO";
        break;
      default:
        LOG_N(OMG, "Unsupported generator %d \n", oai_emulation.info.omg_model_ue);
        }*/
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
      set_seed = 1;
      oai_emulation.info.seed = atoi (optarg);
      break;
    case 'I':
      oai_emulation.info.cli_enabled = 1;
      break;
    case 'X':
#ifdef PROC
      temp=atoi(optarg);
      if(temp==0){ 
	port=CHANNEL_PORT; Channel_Flag=1; Process_Flag=0; wgt=0; 
      }
      else if(temp==1){
	port=eNB_PORT; wgt=0;
      }
      else {
	port=UE_PORT; wgt=MAX_eNB;
      }
#endif
      break;
    case 'i':
#ifdef PROC
     Process_Flag=1;
     node_id = wgt+atoi(optarg);
     port+=atoi(optarg);
#endif
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
    case 'Z':
      /* Sebastien ROUX: Reserved for future use (currently used in ltenow branch) */
      break;
    case 'O':
#if defined(ENABLE_USE_MME)
      oai_emulation.info.mme_enabled = 1;
      if (optarg == NULL) /* No IP address provided: use localhost */
      {
        memcpy(&oai_emulation.info.mme_ip_address[0], "127.0.0.1", 10);
      } else {
        u8 ip_length = strlen(optarg) + 1;
        memcpy(&oai_emulation.info.mme_ip_address[0], optarg, ip_length > 16 ? 16 : ip_length);
      }
#else
      LOG_E(EMU, "You enabled MME mode without MME support...\n");
#endif
      break;
    default:
      help ();
      exit (-1);
      break;
    }
  }
  /*  pthread_t sigth;
  sigset_t sigblock;
  sigemptyset(&sigblock);
  sigaddset(&sigblock, SIGHUP);
  sigaddset(&sigblock, SIGINT);
  sigaddset(&sigblock, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigblock, NULL);
  if (pthread_create(&sigth, NULL, sigh, NULL)) {
    msg("Pthread for tracing Signals is not created!\n");
    return -1;
  } else {
    msg("Pthread for tracing Signals is created!\n");
    }*/
   // configure oaisim with OCG
  oaisim_config(); // config OMG and OCG, OPT, OTG, OLG
  // Initialize VCD LOG module
  vcd_signal_dumper_init();
  
  if (oai_emulation.info.nb_ue_local > NUMBER_OF_UE_MAX ) {
    LOG_E(EMU,"Enter fewer than %d UEs for the moment or change the NUMBER_OF_UE_MAX\n", NUMBER_OF_UE_MAX);
    exit (-1);
  }
  if (oai_emulation.info.nb_enb_local > NUMBER_OF_eNB_MAX) {
    LOG_E(EMU,"Enter fewer than %d eNBs for the moment or change the NUMBER_OF_UE_MAX\n", NUMBER_OF_eNB_MAX);
    exit (-1);
  }
      
  // fix ethernet and abstraction with RRC_CELLULAR Flag
#ifdef RRC_CELLULAR
  abstraction_flag = 1;
  ethernet_flag = 1;
#endif

  if (set_sinr == 0)
    sinr_dB = snr_dB - 20;

  // setup ntedevice interface (netlink socket)
  //#ifdef NAS_NETLINK  
  LOG_I(EMU,"[INIT] Starting NAS netlink interface\n");
  ret = netlink_init ();
  if (ret < 0)
    LOG_E(EMU,"[INIT] Netlink not available, careful ...\n");
  //#endif

  if (ethernet_flag == 1) {
    oai_emulation.info.master[oai_emulation.info.master_id].nb_ue = oai_emulation.info.nb_ue_local;
    oai_emulation.info.master[oai_emulation.info.master_id].nb_enb = oai_emulation.info.nb_enb_local;

    if (!oai_emulation.info.master_id)
      oai_emulation.info.is_primary_master = 1;
    j = 1;
    for (i = 0; i < oai_emulation.info.nb_master; i++) {
      if (i != oai_emulation.info.master_id)
        oai_emulation.info.master_list = oai_emulation.info.master_list + j;
      LOG_I (EMU, "Index of master id i=%d  MASTER_LIST %d\n", i, oai_emulation.info.master_list);
      j *= 2;
    }
    LOG_I (EMU, " Total number of master %d my master id %d\n", oai_emulation.info.nb_master, oai_emulation.info.master_id);
    init_bypass ();

    while (emu_tx_status != SYNCED_TRANSPORT) {
      LOG_I (EMU, " Waiting for EMU Transport to be synced\n");
      emu_transport_sync ();    //emulation_tx_rx();
    }
  }                             // ethernet flag


  NB_UE_INST = oai_emulation.info.nb_ue_local + oai_emulation.info.nb_ue_remote;
  NB_eNB_INST = oai_emulation.info.nb_enb_local + oai_emulation.info.nb_enb_remote;
#ifdef OPENAIR2
  if (oai_emulation.info.omv_enabled == 1) {
    
    if(pipe(pfd) == -1)
      perror("pipe error \n");
    
    sprintf(full_name, "%s/UTIL/OMV/OMV",getenv("OPENAIR2_DIR"));
    LOG_I(EMU,"Stating the OMV path %s pfd[0] %d pfd[1] %d \n", full_name, pfd[0],pfd[1]);
    
      switch(fork()) {
      case -1 :
        perror("fork failed \n");
        break;
      case 0 : /* child is going to be the omv, it is the reader */
        if(close(pfd[1]) == -1 ) /* we close the write desc. */
          perror("close on write\n" );
        sprintf(fdstr, "%d", pfd[0] );
        sprintf(num_enb, "%d", NB_eNB_INST);
        sprintf(num_ue, "%d", NB_UE_INST);
        sprintf(x_area, "%f", oai_emulation.topology_config.area.x_m );
        sprintf(y_area, "%f", oai_emulation.topology_config.area.y_m );
        sprintf(z_area, "%f", 200.0 );
        sprintf(frames, "%d", oai_emulation.info.n_frames);
        sprintf(nb_antenna, "%d", 4);
        sprintf(frame_type, "%s", (oai_emulation.info.frame_type == 0) ? "FDD" : "TDD");
        sprintf(tdd_config, "%d", oai_emulation.info.tdd_config);
        /* execl is used to launch the visualisor */
        execl(full_name,"OMV", fdstr, frames, num_enb, num_ue, x_area, y_area, z_area, nb_antenna, frame_type, tdd_config,NULL );
        perror( "error in execl the OMV" );
      }
    //parent
    if(close( pfd[0] ) == -1 ) /* we close the write desc. */
      perror("close on read\n" );
  }
#endif 
#ifdef PRINT_STATS
for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
    sprintf(UE_stats_filename,"UE_stats%d_tx%d.txt",UE_id,oai_emulation.info.transmission_mode);
    UE_stats[UE_id] = fopen (UE_stats_filename, "w");
   }
  eNB_stats = fopen ("eNB_stats.txt", "w");
  printf ("UE_stats=%p, eNB_stats=%p\n", UE_stats, eNB_stats);
  
  if(abstraction_flag==0){
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
      sprintf(UE_stats_th_filename,"UE_stats_th%d_tx%d.txt",UE_id,oai_emulation.info.transmission_mode);
    UE_stats_th[UE_id] = fopen (UE_stats_th_filename, "w");
    }
      sprintf(eNB_stats_th_filename,"eNB_stats_th_tx%d.txt",oai_emulation.info.transmission_mode);
    eNB_avg_thr = fopen (eNB_stats_th_filename, "w"); 
  }
  else
    {
      for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
	sprintf(UE_stats_th_filename,"UE_stats_abs_th%d_tx%d.txt",UE_id,oai_emulation.info.transmission_mode);
	UE_stats_th[UE_id] = fopen (UE_stats_th_filename, "w");
      }
        sprintf(eNB_stats_th_filename,"eNB_stats_abs_th_tx%d.txt",oai_emulation.info.transmission_mode);
    eNB_avg_thr = fopen (eNB_stats_th_filename, "w"); 
    } 
#endif
      
  LOG_I(EMU,"total number of UE %d (local %d, remote %d) mobility %s \n", NB_UE_INST,oai_emulation.info.nb_ue_local,oai_emulation.info.nb_ue_remote, oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option);
  LOG_I(EMU,"Total number of eNB %d (local %d, remote %d) mobility %s \n", NB_eNB_INST,oai_emulation.info.nb_enb_local,oai_emulation.info.nb_enb_remote, oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option);
  LOG_I(OCM,"Running with frame_type %d, Nid_cell %d, N_RB_DL %d, EP %d, mode %d, target dl_mcs %d, rate adaptation %d, nframes %d, abstraction %d, channel %s\n",
         oai_emulation.info.frame_type, Nid_cell, oai_emulation.info.N_RB_DL, oai_emulation.info.extended_prefix_flag, oai_emulation.info.transmission_mode,target_dl_mcs,rate_adaptation_flag,oai_emulation.info.n_frames,abstraction_flag,oai_emulation.environment_system_config.fading.small_scale.selected_option);
  
  // if(set_seed){
    randominit (oai_emulation.info.seed);
    set_taus_seed (oai_emulation.info.seed);
    //  } else {
    //  randominit (0);
    // set_taus_seed (0);
    // }
  // change the nb_connected_eNB
    init_lte_vars (&frame_parms, oai_emulation.info.frame_type, oai_emulation.info.tdd_config, oai_emulation.info.tdd_config_S,oai_emulation.info.extended_prefix_flag,oai_emulation.info.N_RB_DL, Nid_cell, cooperation_flag, oai_emulation.info.transmission_mode, abstraction_flag,nb_antennas_rx);
  
    for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++){ 
        for (UE_id=0; UE_id<NB_UE_INST;UE_id++){
            PHY_vars_eNB_g[eNB_id]->pusch_config_dedicated[UE_id].betaOffset_ACK_Index = beta_ACK;
            PHY_vars_eNB_g[eNB_id]->pusch_config_dedicated[UE_id].betaOffset_RI_Index  = beta_RI;
            PHY_vars_eNB_g[eNB_id]->pusch_config_dedicated[UE_id].betaOffset_CQI_Index = beta_CQI;                    
            PHY_vars_UE_g[UE_id]->pusch_config_dedicated[eNB_id].betaOffset_ACK_Index = beta_ACK;
            PHY_vars_UE_g[UE_id]->pusch_config_dedicated[eNB_id].betaOffset_RI_Index  = beta_RI;
            PHY_vars_UE_g[UE_id]->pusch_config_dedicated[eNB_id].betaOffset_CQI_Index = beta_CQI;
            ((PHY_vars_UE_g[UE_id]->lte_frame_parms).pdsch_config_common).p_b = (frame_parms->nb_antennas_tx_eNB>1) ? 1 : 0; // rho_a = rhob
            ((PHY_vars_eNB_g[eNB_id]->lte_frame_parms).pdsch_config_common).p_b = (frame_parms->nb_antennas_tx_eNB>1) ? 1 : 0; // rho_a = rhob

        }
    }



  printf ("AFTER init: Nid_cell %d\n", PHY_vars_eNB_g[0]->lte_frame_parms.Nid_cell);
  printf ("AFTER init: frame_type %d,tdd_config %d\n", 
          PHY_vars_eNB_g[0]->lte_frame_parms.frame_type,
          PHY_vars_eNB_g[0]->lte_frame_parms.tdd_config);


  /* Added for PHY abstraction */
  if (abstraction_flag) {
    if (0) { //the values of beta and awgn tables are hard coded in PHY/vars.h
    get_beta_map();
#ifdef PHY_ABSTRACTION_UL
        get_beta_map_up();
#endif
          get_MIESM_param();
        }
  }


  for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
    enb_data[eNB_id] = (node_desc_t *)malloc(sizeof(node_desc_t)); 
    init_enb(enb_data[eNB_id],oai_emulation.environment_system_config.antenna.eNB_antenna);
  }
  
  for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
    ue_data[UE_id] = (node_desc_t *)malloc(sizeof(node_desc_t));
    init_ue(ue_data[UE_id],oai_emulation.environment_system_config.antenna.UE_antenna);
  } 


  if ((oai_emulation.info.ocm_enabled == 1)&& (ethernet_flag == 0 ) &&
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

#ifdef PROC  
  if (abstraction_flag == 0 && Process_Flag==0 && Channel_Flag==0)
    init_channel_vars (frame_parms, &s_re, &s_im, &r_re, &r_im, &r_re0, &r_im0);
#else
  if (abstraction_flag == 0)
    init_channel_vars (frame_parms, &s_re, &s_im, &r_re, &r_im, &r_re0, &r_im0);
#endif
  // initialize channel descriptors
  for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
    for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
      LOG_D(OCM,"Initializing channel (%s, %d) from eNB %d to UE %d\n", oai_emulation.environment_system_config.fading.small_scale.selected_option,
            map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option), eNB_id, UE_id);
      /*
      if (oai_emulation.info.transmission_mode == 5) 
        eNB2UE[eNB_id][UE_id] = new_channel_desc_scm(PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_tx,
                                                     PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_rx,
                                                     (UE_id == 0)? Rice1_corr : Rice1_anticorr,
                                                     oai_emulation.environment_system_config.system_bandwidth_MB,
                                                     forgetting_factor,
                                                     0,
                                                     0);
      
      else
      */
        eNB2UE[eNB_id][UE_id] = new_channel_desc_scm(PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_tx,
                                                     PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_rx,
                                                     map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option),
                                                     oai_emulation.environment_system_config.system_bandwidth_MB,
                                                     forgetting_factor,
                                                     0,
                                                     0);
      random_channel(eNB2UE[eNB_id][UE_id]);      
      LOG_D(OCM,"[SIM] Initializing channel (%s, %d) from UE %d to eNB %d\n", oai_emulation.environment_system_config.fading.small_scale.selected_option,
            map_str_to_int(small_scale_names, oai_emulation.environment_system_config.fading.small_scale.selected_option),UE_id, eNB_id);
      UE2eNB[UE_id][eNB_id] = new_channel_desc_scm(PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_tx,
                                                   PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_rx,
                                                   map_str_to_int(small_scale_names, oai_emulation.environment_system_config.fading.small_scale.selected_option),
                                                   //                                                   map_str_to_int(small_scale_names,"AWGN"),
                                                   oai_emulation.environment_system_config.system_bandwidth_MB,
                                                   forgetting_factor,
                                                   0,
                                                   0);
      random_channel(UE2eNB[UE_id][eNB_id]);
    }
  }

// Not needed anymore, done automatically in init_freq_channel upon first call to the function

//  if (abstraction_flag==1)
//    init_freq_channel(eNB2UE[0][0],PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL,PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL*12+1);  
  freq_channel(eNB2UE[0][0],PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL,PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL*12+1);  
  number_of_cards = 1;

  openair_daq_vars.rx_rf_mode = 1;
  openair_daq_vars.tdd = 1;
  openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;

  openair_daq_vars.dlsch_transmission_mode = oai_emulation.info.transmission_mode;

  openair_daq_vars.target_ue_dl_mcs = target_dl_mcs;
  openair_daq_vars.target_ue_ul_mcs = target_ul_mcs;
  openair_daq_vars.ue_dl_rb_alloc=0x1fff;
  openair_daq_vars.ue_ul_nb_rb=6;
  openair_daq_vars.dlsch_rate_adaptation = rate_adaptation_flag;
  openair_daq_vars.use_ia_receiver = 0;


  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ 
    //PHY_vars_UE_g[UE_id]->rx_total_gain_dB=145;
    PHY_vars_UE_g[UE_id]->rx_total_gain_dB=130;
    // update UE_mode for each eNB_id not just 0
    if (abstraction_flag == 0)
      PHY_vars_UE_g[UE_id]->UE_mode[0] = NOT_SYNCHED;
    else {
      // 0 is the index of the connected eNB
      PHY_vars_UE_g[UE_id]->UE_mode[0] = PRACH;
    }
    PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti = 0x1235 + UE_id;
    PHY_vars_UE_g[UE_id]->current_dlsch_cqi[0] = 10;

    LOG_I(EMU, "UE %d mode is initialized to %d\n", UE_id, PHY_vars_UE_g[UE_id]->UE_mode[0] );
  }

  
#ifdef XFORMS
  eNB_id = 0;
  for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
      // DL scope at UEs
      fl_initialize (&argc, argv, NULL, 0, 0);
      form_ue[UE_id] = create_lte_phy_scope_ue();
      sprintf (title, "LTE DL SCOPE eNB %d to UE %d", eNB_id, UE_id);
      fl_show_form (form_ue[UE_id]->lte_phy_scope_ue, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);

      // UL scope at eNB 0
      fl_initialize (&argc, argv, NULL, 0, 0);
      form_enb[UE_id] = create_lte_phy_scope_enb();
      sprintf (title, "LTE UL SCOPE UE %d to eNB %d", UE_id, eNB_id);
      fl_show_form (form_enb[UE_id]->lte_phy_scope_enb, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);

      if (openair_daq_vars.use_ia_receiver == 1) {
          fl_set_button(form_ue[UE_id]->button_0,1);
          fl_set_object_label(form_ue[UE_id]->button_0, "IA Receiver ON");
          fl_set_object_color(form_ue[UE_id]->button_0, FL_GREEN, FL_GREEN);
      }

  }
#endif

  printf ("before L2 init: Nid_cell %d\n", PHY_vars_eNB_g[0]->lte_frame_parms.Nid_cell);
  printf ("before L2 init: frame_type %d,tdd_config %d\n", 
          PHY_vars_eNB_g[0]->lte_frame_parms.frame_type,
          PHY_vars_eNB_g[0]->lte_frame_parms.tdd_config);



#ifdef OPENAIR2
  l2_init (&PHY_vars_eNB_g[0]->lte_frame_parms,eMBMS_active);
  printf ("after L2 init: Nid_cell %d\n", PHY_vars_eNB_g[0]->lte_frame_parms.Nid_cell);
  printf ("after L2 init: frame_type %d,tdd_config %d\n", 
          PHY_vars_eNB_g[0]->lte_frame_parms.frame_type,
          PHY_vars_eNB_g[0]->lte_frame_parms.tdd_config);


  for (i = 0; i < NB_eNB_INST; i++)
    mac_xface->mrbch_phy_sync_failure (i, 0, i);
  if (abstraction_flag == 1) {
    for (UE_id = 0; UE_id < NB_UE_INST; UE_id++)
      mac_xface->dl_phy_sync_success (UE_id, 0, 0,1);   //UE_id%NB_eNB_INST);
      }
#endif

  mac_xface->macphy_exit = exit_fun;

  // time calibration for OAI 
  clock_gettime (CLOCK_REALTIME, &time_spec);
  time_now = (unsigned long) time_spec.tv_nsec;
  td_avg = 0;
  sleep_time_us = SLEEP_STEP_US;
  td_avg = TARGET_SF_TIME_NS;

#ifdef PROC
  if(Channel_Flag==1)
    Channel_Inst(node_id,port,s_re2,s_im2,r_re2,r_im2,r_re02,r_im02,r_re0_d,r_im0_d,r_re0_u,r_im0_u,eNB2UE,UE2eNB,enb_data,ue_data,abstraction_flag,frame_parms);
  if(Process_Flag==1)
    Process_Func(node_id,port,r_re02,r_im02,r_re2[0],r_im2[0],s_re2[0],s_im2[0],enb_data,ue_data,abstraction_flag,frame_parms);
#endif 
  
  LOG_I(EMU,">>>>>>>>>>>>>>>>>>>>>>>>>>> OAIEMU initialization done <<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
  printf ("after init: Nid_cell %d\n", PHY_vars_eNB_g[0]->lte_frame_parms.Nid_cell);
  printf ("after init: frame_type %d,tdd_config %d\n", 
          PHY_vars_eNB_g[0]->lte_frame_parms.frame_type,
          PHY_vars_eNB_g[0]->lte_frame_parms.tdd_config);


  if (ue_connection_test == 1) {
    snr_direction = -snr_step;
    snr_dB=20;
    sinr_dB=-20;
  }
  for (frame=0; frame<oai_emulation.info.n_frames; frame++) {
    /*
    // Handling the cooperation Flag
    if (cooperation_flag == 2)
      {
        if ((PHY_vars_eNB_g[0]->eNB_UE_stats[0].mode == PUSCH) && (PHY_vars_eNB_g[0]->eNB_UE_stats[1].mode == PUSCH))
          PHY_vars_eNB_g[0]->cooperation_flag = 2;
      }
    */
    if (ue_connection_test==1) {
      if ((frame%20) == 0) {
        snr_dB += snr_direction;
        sinr_dB -= snr_direction;
      }
      if (snr_dB == -20) {
        snr_direction=snr_step;
      }
      else if (snr_dB==20) {
        snr_direction=-snr_step;
      }
    }
      
    oai_emulation.info.frame = frame; 
    //oai_emulation.info.time_ms += 1;  
    oai_emulation.info.time_s += 0.1; // emu time in s, each frame lasts for 10 ms // JNote: TODO check the coherency of the time and frame (I corrected it to 10 (instead of 0.01)
    // if n_frames not set by the user or is greater than max num frame then set adjust the frame counter
    if ( (oai_emulation.info.n_frames_flag == 0) || (oai_emulation.info.n_frames >= 0xffff) ){ 
      frame %=(oai_emulation.info.n_frames-1);
    } 
    
    if ((frame % 10) == 0 ) { // call OMG every 10ms 
      update_nodes(oai_emulation.info.time_s); 
      display_node_list(enb_node_list);
      display_node_list(ue_node_list);
      if (oai_emulation.info.omg_model_ue >= MAX_NUM_MOB_TYPES){ // mix mobility model
        for(UE_id=oai_emulation.info.first_ue_local; UE_id<(oai_emulation.info.first_ue_local+oai_emulation.info.nb_ue_local);UE_id++){
          new_omg_model = randomGen(STATIC,RWALK); 
          LOG_D(OMG, "[UE] Node of ID %d is changing mobility generator ->%d \n", UE_id, new_omg_model);
          // reset the mobility model for a specific node
          set_new_mob_type (UE_id, UE, new_omg_model, oai_emulation.info.time_s);
        }
      }
      if (oai_emulation.info.omg_model_enb >= MAX_NUM_MOB_TYPES) {      // mix mobility model
        for (eNB_id = oai_emulation.info.first_enb_local; eNB_id < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local); eNB_id++) {
          new_omg_model = randomGen (STATIC, RWALK);
          LOG_D (OMG,"[eNB] Node of ID %d is changing mobility generator ->%d \n", UE_id, new_omg_model);
          // reset the mobility model for a specific node
          set_new_mob_type (eNB_id, eNB, new_omg_model, oai_emulation.info.time_s);
        }
      }
    }
    enb_node_list = get_current_positions(oai_emulation.info.omg_model_enb, eNB, oai_emulation.info.time_s);
    ue_node_list = get_current_positions(oai_emulation.info.omg_model_ue, UE, oai_emulation.info.time_s);
#ifdef OPENAIR2
    // check if pipe is still open
    if ((oai_emulation.info.omv_enabled == 1) ){
      omv_write(pfd[1], enb_node_list, ue_node_list, omv_data);
    }
#endif    
#ifdef DEBUG_OMG
    if ((((int) oai_emulation.info.time_s) % 100) == 0) {
      for (UE_id = oai_emulation.info.first_ue_local; UE_id < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local); UE_id++) {
        get_node_position (UE, UE_id);
      }
    }
#endif 

    for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) 
      enb_data[eNB_id]->tx_power_dBm = PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
    for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) 
      ue_data[UE_id]->tx_power_dBm = PHY_vars_UE_g[UE_id]->tx_power_dBm;
    
    /* check if the openair channel model is activated used for PHY abstraction : path loss*/
    if ((oai_emulation.info.ocm_enabled == 1)&& (ethernet_flag == 0 )) {
      //LOG_D(OMG," extracting position of eNb...\n");
      extract_position(enb_node_list, enb_data, NB_eNB_INST);
      //LOG_D(OMG," extracting position of UE...\n");
      //if (oai_emulation.info.omg_model_ue == TRACE)
       extract_position(ue_node_list, ue_data, NB_UE_INST); 
      // extract_position_fixed_ue(ue_data, NB_UE_INST, frame);
      for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
        for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
          calc_path_loss (enb_data[eNB_id], ue_data[UE_id], eNB2UE[eNB_id][UE_id], oai_emulation.environment_system_config,ShaF);
          //calc_path_loss (enb_data[eNB_id], ue_data[UE_id], eNB2UE[eNB_id][UE_id], oai_emulation.environment_system_config,0);
          UE2eNB[UE_id][eNB_id]->path_loss_dB = eNB2UE[eNB_id][UE_id]->path_loss_dB;
          LOG_I(OCM,"Path loss between eNB %d at (%f,%f) and UE %d at (%f,%f) is %f, angle %f\n",
                eNB_id,enb_data[eNB_id]->x,enb_data[eNB_id]->y,UE_id,ue_data[UE_id]->x,ue_data[UE_id]->y,
                eNB2UE[eNB_id][UE_id]->path_loss_dB, eNB2UE[eNB_id][UE_id]->aoa);
        }
      }
    }

    else {
      for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
        for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {

          //UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + snr_dB;
          if (eNB_id == (UE_id % NB_eNB_INST)) {
            eNB2UE[eNB_id][UE_id]->path_loss_dB = -105 + snr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
            UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + snr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower; //+20 to offset the difference in tx power of the UE wrt eNB
          }
          else {
            eNB2UE[eNB_id][UE_id]->path_loss_dB = -105 + sinr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
            UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + sinr_dB - PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
          }
          LOG_I(OCM,"Path loss from eNB %d to UE %d => %f dB (eNB TX %d)\n",eNB_id,UE_id,eNB2UE[eNB_id][UE_id]->path_loss_dB,
                PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower);
          //      printf("[SIM] Path loss from UE %d to eNB %d => %f dB\n",UE_id,eNB_id,UE2eNB[UE_id][eNB_id]->path_loss_dB);
        }
      }
    }

    for (slot=0 ; slot<20 ; slot++) {

      last_slot = (slot - 1)%20;
      if (last_slot <0)
        last_slot+=20;
      next_slot = (slot + 1)%20;
      
      oai_emulation.info.time_ms = frame * 10 + (next_slot>>1) ;
      
      direction = subframe_select(frame_parms,next_slot>>1);
#ifdef PROC      
      if(Channel_Flag==1)
          Channel_Func(s_re2,s_im2,r_re2,r_im2,r_re02,r_im02,r_re0_d,r_im0_d,r_re0_u,r_im0_u,eNB2UE,UE2eNB,enb_data,ue_data,abstraction_flag,frame_parms,slot);

     if(Channel_Flag==0)
#endif
       {
      if((next_slot %2) ==0)
        clear_eNB_transport_info(oai_emulation.info.nb_enb_local);

      for (eNB_id=oai_emulation.info.first_enb_local;
           (eNB_id<(oai_emulation.info.first_enb_local+oai_emulation.info.nb_enb_local)) && (oai_emulation.info.cli_start_enb[eNB_id]==1);
           eNB_id++) {
                    //printf ("debug: Nid_cell %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell);
                    //printf ("debug: frame_type %d,tdd_config %d\n", PHY_vars_eNB_g[eNB_id]->lte_frame_parms.frame_type,PHY_vars_eNB_g[eNB_id]->lte_frame_parms.tdd_config);
        LOG_D(EMU,"PHY procedures eNB %d for frame %d, slot %d (subframe TX %d, RX %d) TDD %d/%d Nid_cell %d\n",
              eNB_id, frame, slot, next_slot >> 1,last_slot>>1,
              PHY_vars_eNB_g[eNB_id]->lte_frame_parms.frame_type,
              PHY_vars_eNB_g[eNB_id]->lte_frame_parms.tdd_config,PHY_vars_eNB_g[eNB_id]->lte_frame_parms.Nid_cell);
        
        //PHY_vars_eNB_g[eNB_id]->frame = frame;
        phy_procedures_eNB_lte (last_slot, next_slot, PHY_vars_eNB_g[eNB_id], abstraction_flag);
        
#ifdef PRINT_STATS
      	if(last_slot==9 && frame%10==0)
	if(eNB_avg_thr)
	  fprintf(eNB_avg_thr,"%d %d\n",PHY_vars_eNB_g[eNB_id]->frame,(PHY_vars_eNB_g[eNB_id]->total_system_throughput)/((PHY_vars_eNB_g[eNB_id]->frame+1)*10));
	if (eNB_stats) {
	  len = dump_eNB_stats (PHY_vars_eNB_g[eNB_id], stats_buffer, 0);
	  rewind (eNB_stats);
	  fwrite (stats_buffer, 1, len, eNB_stats);
	  fflush(eNB_stats);
	}
#endif
      }
      // Call ETHERNET emulation here
      //emu_transport (frame, last_slot, next_slot, direction, oai_emulation.info.frame_type, ethernet_flag);
      
       if ((next_slot % 2) == 0)
        clear_UE_transport_info (oai_emulation.info.nb_ue_local);
      
      for (UE_id = oai_emulation.info.first_ue_local; 
           (UE_id < (oai_emulation.info.first_ue_local+oai_emulation.info.nb_ue_local)) && (oai_emulation.info.cli_start_ue[UE_id]==1); 
           UE_id++) 
        if (frame >= (UE_id * 20)) {    // activate UE only after 20*UE_id frames so that different UEs turn on separately

          LOG_D(EMU,"PHY procedures UE %d for frame %d, slot %d (subframe TX %d, RX %d)\n",
                UE_id, frame, slot, next_slot >> 1,last_slot>>1);

          if (PHY_vars_UE_g[UE_id]->UE_mode[0] != NOT_SYNCHED) {
            if (frame>0) {
              PHY_vars_UE_g[UE_id]->frame = frame;
              phy_procedures_UE_lte (last_slot, next_slot, PHY_vars_UE_g[UE_id], 0, abstraction_flag,normal_txrx);
                  ue_data[UE_id]->tx_power_dBm = PHY_vars_UE_g[UE_id]->tx_power_dBm;
            }
          }
          else {
            if (abstraction_flag==1){
              LOG_E(EMU, "sync not supported in abstraction mode (UE%d,mode%d)\n", UE_id, PHY_vars_UE_g[UE_id]->UE_mode[0]);
              exit(-1);
            }
            if ((frame>0) && (last_slot == (LTE_SLOTS_PER_FRAME-2))) {
              initial_sync(PHY_vars_UE_g[UE_id],normal_txrx);
              
              /*
              write_output("dlchan00.m","dlch00",&(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][0][0]),(6*(PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)),1,1);
              if (PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx>1)
                write_output("dlchan01.m","dlch01",&(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][1][0]),(6*(PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)),1,1);
              write_output("dlchan10.m","dlch10",&(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][2][0]),(6*(PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)),1,1);
              if (PHY_vars_UE_g[0]->lte_frame_parms.nb_antennas_rx>1)
                write_output("dlchan11.m","dlch11",&(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][3][0]),(6*(PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)),1,1);
              write_output("rxsig.m","rxs",PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[0],PHY_vars_UE_g[0]->lte_frame_parms.samples_per_tti*10,1,1);
              write_output("rxsigF.m","rxsF",PHY_vars_UE_g[0]->lte_ue_common_vars.rxdataF[0],2*PHY_vars_UE_g[0]->lte_frame_parms.symbols_per_tti*PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size,2,1);
              write_output("pbch_rxF_ext0.m","pbch_ext0",PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->rxdataF_ext[0],6*12*4,1,1);
              write_output("pbch_rxF_comp0.m","pbch_comp0",PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->rxdataF_comp[0],6*12*4,1,1);
              write_output("pbch_rxF_llr.m","pbch_llr",PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->llr,(frame_parms->Ncp==0) ? 1920 : 1728,1,4);
              */
            }
          }
#ifdef PRINT_STATS
       	  if(last_slot==2 && frame%10==0)
	  if (UE_stats_th[UE_id])
	    fprintf(UE_stats_th[UE_id],"%d %d\n",frame, PHY_vars_UE_g[UE_id]->bitrate[0]/1000);
	  if (UE_stats[UE_id]) {
	    len = dump_ue_stats (PHY_vars_UE_g[UE_id], stats_buffer, 0, normal_txrx, 0);
	    rewind (UE_stats[UE_id]);
	    fwrite (stats_buffer, 1, len, UE_stats[UE_id]);
	    fflush(UE_stats[UE_id]);
	  }
#endif
        }
      emu_transport (frame, last_slot, next_slot,direction, oai_emulation.info.frame_type, ethernet_flag);
      if ((direction  == SF_DL)|| (frame_parms->frame_type==0)){
          do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,enb_data,ue_data,next_slot,abstraction_flag,frame_parms);
      }
      if ((direction  == SF_UL)|| (frame_parms->frame_type==0)){//if ((subframe<2) || (subframe>4))
          do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,abstraction_flag,frame_parms,frame);
	/*   
                int ccc;
                fprintf(SINRpost,"SINRdb For eNB New Subframe : \n ");
                for(ccc = 0 ; ccc<301; ccc++)
                {
                        fprintf(SINRpost,"_ %f ", SINRpost_eff[ccc]);
                }
                fprintf(SINRpost,"SINRdb For eNB : %f \n ", SINRpost_eff[ccc]);
   */   }
      if ((direction == SF_S)) {//it must be a special subframe
        if (next_slot%2==0) {//DL part
          do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,enb_data,ue_data,next_slot,abstraction_flag,frame_parms);
          /*
            for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
            for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
            for (k=0;k<UE2eNB[1][0]->channel_length;k++)
            printf("SB(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
          */
        }
        else {// UL part
            do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,enb_data,ue_data,next_slot,abstraction_flag,frame_parms,frame);
	          
	  /*        int ccc;
                fprintf(SINRpost,"SINRdb For eNB New Subframe : \n ");
                for(ccc = 0 ; ccc<301; ccc++)
                {
                        fprintf(SINRpost,"_ %f ", SINRpost_eff[ccc]);
                }
                fprintf(SINRpost,"SINRdb For eNB : %f \n ", SINRpost_eff[ccc]);
  */      }
      }
      if ((last_slot == 1) && (frame == 0)
          && (abstraction_flag == 0) && (oai_emulation.info.n_frames == 1)) {

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
      /*
         if ((last_slot==1) && (frame==1)) {
         write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->rxdataF_comp[0],300*(-(PHY_vars_UE->lte_frame_parms.Ncp*2)+14),1,1);
         write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0],4*300,1,1);
         }
       */

      if (next_slot %2 == 0){
        clock_gettime (CLOCK_REALTIME, &time_spec);
        time_last = time_now;
        time_now = (unsigned long) time_spec.tv_nsec;
        td = (int) (time_now - time_last);
        if (td>0) {
          td_avg = (int)(((K*(long)td) + (((1<<3)-K)*((long)td_avg)))>>3); // in us
                        LOG_T(EMU,"sleep frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",
                              frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
        }  
        if (td_avg<(TARGET_SF_TIME_NS - SF_DEVIATION_OFFSET_NS)){
          sleep_time_us += SLEEP_STEP_US; 
          LOG_D(EMU,"Faster than realtime increase the avg sleep time for %d us, frame %d\n",    
		sleep_time_us,frame);
	  // LOG_D(EMU,"Faster than realtime increase the avg sleep time for %d us, frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",    sleep_time_us,frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
        }
        else if (td_avg > (TARGET_SF_TIME_NS + SF_DEVIATION_OFFSET_NS)) {
          sleep_time_us-= SLEEP_STEP_US; 
          LOG_D(EMU,"Slower than realtime reduce the avg sleep time for %d us, frame %d, time_now\n",     
		sleep_time_us,frame);
	  //LOG_T(EMU,"Slower than realtime reduce the avg sleep time for %d us, frame %d, time_now %ldus,time_last %ldus,average time difference %ldns, CURRENT TIME DIFF %dus, avgerage difference from the target %dus\n",     sleep_time_us,frame, time_now,time_last,td_avg, td/1000,(td_avg-TARGET_SF_TIME_NS)/1000);
        }
      } // end if next_slot%2
     }// if Channel_Flag==0

     }                          //end of slot
    
    if ((frame>=1)&&(frame<=9)&&(abstraction_flag==0)
#ifdef PROC
	&&(Channel_Flag==0) 
#endif
	){
      write_output("UEtxsig0.m","txs0", PHY_vars_UE_g[0]->lte_ue_common_vars.txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      sprintf(fname,"eNBtxsig%d.m",frame);
      sprintf(vname,"txs%d",frame);
      write_output(fname,vname, PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      write_output("eNBtxsigF0.m","txsF0",PHY_vars_eNB_g[0]->lte_eNB_common_vars.txdataF[0][0],PHY_vars_eNB_g[0]->lte_frame_parms.symbols_per_tti*PHY_vars_eNB_g[0]->lte_frame_parms.ofdm_symbol_size,1,1);

      write_output("UErxsig0.m","rxs0", PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      write_output("eNBrxsig0.m","rxs0", PHY_vars_eNB_g[0]->lte_eNB_common_vars.rxdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    } 
  
#ifdef XFORMS
    eNB_id = 0;
    for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {
        phy_scope_UE(form_ue[UE_id], 
                     PHY_vars_UE_g[UE_id],
                     eNB_id,
                     UE_id,
                     7);

        phy_scope_eNB(form_enb[UE_id], 
                      PHY_vars_eNB_g[eNB_id],
                      UE_id);

    }
#endif  

    // calibrate at the end of each frame if there is some time  left
    if((sleep_time_us > 0)&& (ethernet_flag ==0)){
      LOG_I(EMU,"[TIMING] Adjust average frame duration, sleep for %d us\n",sleep_time_us);
      usleep(sleep_time_us);
      sleep_time_us=0; // reset the timer, could be done per n SF 
    }
  }     //end of frame
  //  fclose(SINRpost);
  LOG_I(EMU,">>>>>>>>>>>>>>>>>>>>>>>>>>> OAIEMU Ending <<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");

  //Perform KPI measurements
   if (oai_emulation.info.otg_enabled==1)
     kpi_gen();

  // relase all rx state
  if (ethernet_flag == 1) {
    emu_transport_release ();
  }

#ifdef PROC
  if (abstraction_flag == 0 && Channel_Flag==0 && Process_Flag==0) 
#else
  if (abstraction_flag == 0) 
#endif
    {
    /*
       #ifdef IFFT_FPGA
       free(txdataF2[0]);
       free(txdataF2[1]);
       free(txdataF2);
       free(txdata[0]);
       free(txdata[1]);
       free(txdata);
       #endif
     */

    for (i = 0; i < 2; i++) {
      free (s_re[i]);
      free (s_im[i]);
      free (r_re[i]);
      free (r_im[i]);
    }
    free (s_re);
    free (s_im);
    free (r_re);
    free (r_im);

    lte_sync_time_free ();
  }
  //  pthread_join(sigth, NULL);

  // added for PHY abstraction
  if (oai_emulation.info.ocm_enabled == 1) {
    for (eNB_id = 0; eNB_id < NUMBER_OF_eNB_MAX; eNB_id++) 
      free(enb_data[eNB_id]); 
    
    for (UE_id = 0; UE_id < NUMBER_OF_UE_MAX; UE_id++)
      free(ue_data[UE_id]); 
  } //End of PHY abstraction changes

#ifdef OPENAIR2
  mac_top_cleanup();
#endif 
  
#ifdef PRINT_STATS
  for(UE_id=0;UE_id<NB_UE_INST;UE_id++) 
    {

    if (UE_stats[UE_id]) 
      fclose (UE_stats[UE_id]);
    if(UE_stats_th[UE_id])
  	fclose (UE_stats_th[UE_id]);
    }
	if (eNB_stats)
    fclose (eNB_stats);
	if (eNB_avg_thr)
        fclose (eNB_avg_thr);
#endif

  // stop OMG
  stop_mobility_generator(oai_emulation.info.omg_model_ue);//omg_param_list.mobility_type
#ifdef OPENAIR2
  if (oai_emulation.info.omv_enabled == 1)
    omv_end(pfd[1],omv_data);
#endif
  if ((oai_emulation.info.ocm_enabled == 1) && (ethernet_flag == 0) && (ShaF != NULL)) 
    destroyMat(ShaF,map1, map2);

  if ((oai_emulation.info.opt_enabled == 1) )
    terminate_opt();
  
  if (oai_emulation.info.cli_enabled)
    cli_server_cleanup();

  //bring oai if down
  terminate();
  logClean();
  
  return(0);
}

/*
static void *sigh(void *arg) {
   
  int signum;
  sigset_t sigcatch;
  sigemptyset(&sigcatch);
  sigaddset(&sigcatch, SIGHUP);
  sigaddset(&sigcatch, SIGINT);
  sigaddset(&sigcatch, SIGTERM);
  
  for (;;) {
    sigwait(&sigcatch, &signum);
    switch (signum) {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
      terminate();
    default:
      break;
    }
  }
  pthread_exit(NULL);
}
*/

void terminate(void) {
  int i;
  char interfaceName[8];
  for (i=0; i < NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX; i++)
    if (oai_emulation.info.oai_ifup[i]==1){
      sprintf(interfaceName, "oai%d", i);
      bringInterfaceUp(interfaceName,0);
    }
}

void exit_fun(const char* s)
{
  void *array[10];
  size_t size;

  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, 2);

  fprintf(stderr, "Error: %s. Exiting!\n",s);
  exit (-1);
}
