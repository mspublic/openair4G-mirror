#include <execinfo.h>

#include "oaisim_functions.h"

#include "PHY/extern.h"
#include "MAC_INTERFACE/extern.h"
#include "LAYER2/MAC/extern.h"
#include "LAYER2/PDCP_v10.1.0/pdcp.h"
#include "LAYER2/PDCP_v10.1.0/pdcp_primitives.h"
#include "RRC/LITE/extern.h"
#include "PHY_INTERFACE/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "SCHED/extern.h"
#include "UTIL/OCG/OCG_extern.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"

#ifdef SMBV
extern u8 config_smbv;
extern char smbv_ip[16];
#endif

//constant for OAISIM soft realtime calibration
#define SF_DEVIATION_OFFSET_NS 100000 //= 0.1ms : should be as a number of UE
#define SLEEP_STEP_US       100 //  = 0.01ms could be adaptive, should be as a number of UE
#define K 2                  // averaging coefficient
#define TARGET_SF_TIME_NS 1000000       // 1ms = 1000000 ns

int otg_times = 0;
int if_times = 0;
int for_times = 0;


u16 Nid_cell = 0; //needed by init_lte_vars
int nb_antennas_rx=2; // //
u8 target_dl_mcs = 0;
u8 rate_adaptation_flag = 0;
u8 set_sinr=0;
double snr_dB, sinr_dB;
u8 set_seed=0;
u8 cooperation_flag;          // for cooperative communication
u8 abstraction_flag = 0, ethernet_flag = 0;
double snr_step=1.0;
u8 ue_connection_test=0;
double forgetting_factor=0.0;
u8 beta_ACK=0,beta_RI=0,beta_CQI=2;
u8 target_ul_mcs = 2;
LTE_DL_FRAME_PARMS *frame_parms;
int map1,map2;
double **ShaF= NULL;
// pointers signal buffers (s = transmit, r,r0 = receive)
double **s_re, **s_im, **r_re, **r_im, **r_re0, **r_im0;
Node_list ue_node_list = NULL;
Node_list enb_node_list = NULL;
int pdcp_period, omg_period;

// time calibration for soft realtime mode
struct timespec time_spec;
unsigned long time_last, time_now;
int td, td_avg, sleep_time_us;

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

Packet_OTG_List *otg_pdcp_buffer;

extern node_desc_t *enb_data[NUMBER_OF_eNB_MAX];
extern node_desc_t *ue_data[NUMBER_OF_UE_MAX];
extern channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
extern channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];

extern mapping small_scale_names[];
extern pdcp_mbms_t pdcp_mbms_array[MAX_MODULES][16*29];
extern int eMBMS_active;

extern void help (void);

void get_simulation_options(int argc, char *argv[]) {
  char c;
  int option_index;
  static struct option long_options[] = {
    {"pdcp_period", 1, 0, 0},
    {"omg_period", 1, 0, 0},
    {NULL, 0, NULL, 0}
  };

  while ((c = getopt_long (argc, argv, "aA:b:B:c:C:D:d:eE:f:FGg:hi:IJ:k:l:m:M:n:N:O:p:P:rR:s:S:t:T:L:u:U:vVx:y:w:W:X:z:Z:", long_options, &option_index)) != -1) {

    switch (c) {
    case 0:
      if (! strcmp(long_options[option_index].name, "pdcp_period")) {
        if (optarg) {
          pdcp_period = atoi(optarg);
          printf("PDCP period is %d\n", pdcp_period);
        }
      } else if (! strcmp(long_options[option_index].name, "omg_period")) {
        if (optarg) {
          omg_period = atoi(optarg);
          printf("OMG period is %d\n", omg_period);
        }
      }
      break;
    case 'L':                   // set FDD
      flag_LA = atoi(optarg);
      break;/*
      case 'L':                   //TVT: flag=1 --> use lolasched
      flag_lolasched = atoi(optarg);
      break;*/
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
   case 'Y':
      oai_emulation.info.g_log_verbosity = optarg;
      break;
    case 'c':
      strcpy(oai_emulation.info.local_server, optarg);
      oai_emulation.info.ocg_enabled=1;
      break;
    case 'g':
      oai_emulation.info.multicast_group = atoi (optarg);
      break;
    case 'D':
      oai_emulation.info.multicast_ifname = strdup(optarg);
      break;
    case 'B':
      oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option = optarg;
      //oai_emulation.info.omg_model_enb = atoi (optarg);
      break;
    case 'U':
      oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = optarg;
      break;
    case 'T':
      oai_emulation.info.otg_enabled = 1;
      oai_emulation.info.otg_traffic = optarg;
      break;
    case 'P':
      oai_emulation.info.opt_enabled = 1;

      if (strcmp(optarg, "wireshark") == 0) {
          opt_type = OPT_WIRESHARK;
          printf("Enabling OPT for wireshark\n");
      } else if (strcmp(optarg, "pcap") == 0) {
          opt_type = OPT_PCAP;
          printf("Enabling OPT for pcap\n");
      } else {
          printf("Unrecognized option for OPT module. -> Disabling it\n");
          printf("Possible values are either wireshark or pcap\n");
          opt_type = OPT_NONE;
          oai_emulation.info.opt_enabled = 0;
      }
      oai_emulation.info.opt_mode = opt_type;
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
    case 'w':
      oai_emulation.info.cba_group_active = atoi (optarg);
      break;
    case 'W':
#ifdef SMBV
      config_smbv = 1;
      if(atoi(optarg)!=0)
	strcpy(smbv_ip,optarg);
#endif
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
      printf("You enabled MME mode without MME support...\n");
#endif
      break;
    default:
      help ();
      exit (-1);
      break;
    }
    }
}

void check_and_adjust_params() {

  s32 ret;
  int i,j;

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

  // setup netdevice interface (netlink socket)
  LOG_I(EMU,"[INIT] Starting NAS netlink interface\n");
  ret = netlink_init ();
  if (ret < 0)
    LOG_E(EMU,"[INIT] Netlink not available, careful ...\n");


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
  } // ethernet flag


  NB_UE_INST = oai_emulation.info.nb_ue_local + oai_emulation.info.nb_ue_remote;
  NB_eNB_INST = oai_emulation.info.nb_enb_local + oai_emulation.info.nb_enb_remote;
}

void init_omv() {
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
}

void init_seed(u8 set_seed) {

  if(set_seed) {

    randominit (oai_emulation.info.seed);
    set_taus_seed (oai_emulation.info.seed);

  } else {
    randominit (0);
    set_taus_seed (0);
  }
}

void init_openair1() {
  s32 UE_id, eNB_id;

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

  // init_ue_status();
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++) {
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
}

void init_openair2() {
#ifdef OPENAIR2
  s32 i;
  s32 UE_id;
  l2_init (&PHY_vars_eNB_g[0]->lte_frame_parms,eMBMS_active, oai_emulation.info.cba_group_active);
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

  mac_xface->macphy_exit = exit_fun;
#endif
}

void init_ocm() {
  s32 UE_id, eNB_id;
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

  if (abstraction_flag == 0)
    init_channel_vars (frame_parms, &s_re, &s_im, &r_re, &r_im, &r_re0, &r_im0);

  // initialize channel descriptors
  for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++) {
    for (UE_id = 0; UE_id < NB_UE_INST; UE_id++) {

      LOG_D(OCM,"Initializing channel (%s, %d) from eNB %d to UE %d\n", oai_emulation.environment_system_config.fading.small_scale.selected_option,
            map_str_to_int(small_scale_names,oai_emulation.environment_system_config.fading.small_scale.selected_option), eNB_id, UE_id);

      /* if (oai_emulation.info.transmission_mode == 5)
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
                                                   oai_emulation.environment_system_config.system_bandwidth_MB,
                                                   forgetting_factor,
                                                   0,
                                                   0);

      random_channel(UE2eNB[UE_id][eNB_id]);
    }
  }

  // Not needed anymore, done automatically in init_freq_channel upon first call to the function

  // if (abstraction_flag==1)
  //    init_freq_channel(eNB2UE[0][0],PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL,PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL*12+1);
  freq_channel(eNB2UE[0][0],PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL,PHY_vars_UE_g[0]->lte_frame_parms.N_RB_DL*12+1);
}

void init_otg_pdcp_buffer() {
  s32 i;
  otg_pdcp_buffer = malloc((NB_UE_INST + NB_eNB_INST) * sizeof(Packet_OTG_List));

  for (i = 0; i < NB_UE_INST + NB_eNB_INST; i++) {
    pkt_list_init(&(otg_pdcp_buffer[i]));
    //LOG_I(EMU,"HEAD of otg_pdcp_buffer[%d] is %p\n", i, pkt_list_get_head(&(otg_pdcp_buffer[i])));
  }
}

void update_omg () {
  s32 UE_id, eNB_id;
  int new_omg_model;

  if ((frame % omg_period) == 0 ) { // call OMG every 10ms
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
}

void update_omg_ocm() {
  enb_node_list = get_current_positions(oai_emulation.info.omg_model_enb, eNB, oai_emulation.info.time_s);
  ue_node_list = get_current_positions(oai_emulation.info.omg_model_ue, UE, oai_emulation.info.time_s);
}

void update_ocm() {
  s32 UE_id, eNB_id;
  for (eNB_id = 0; eNB_id < NB_eNB_INST; eNB_id++)
    enb_data[eNB_id]->tx_power_dBm = PHY_vars_eNB_g[eNB_id]->lte_frame_parms.pdsch_config_common.referenceSignalPower;
  for (UE_id = 0; UE_id < NB_UE_INST; UE_id++)
    ue_data[UE_id]->tx_power_dBm = PHY_vars_UE_g[UE_id]->tx_power_dBm;

  /* check if the openair channel model is activated used for PHY abstraction : path loss*/
  if ((oai_emulation.info.ocm_enabled == 1)&& (ethernet_flag == 0 )) {
    //LOG_D(OMG," extracting position of eNb...\n");
    extract_position(enb_node_list, enb_data, NB_eNB_INST);
    //LOG_D(OMG," extracting position of UE...\n");
    //      if (oai_emulation.info.omg_model_ue == TRACE)
    extract_position(ue_node_list, ue_data, NB_UE_INST);

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
}

void update_otg_eNB(int module_id, unsigned int ctime) {
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.otg_enabled ==1 ) {

    int dst_id;
    Packet_otg_elt *otg_pkt;
    

    for (dst_id = 0; dst_id < NUMBER_OF_UE_MAX; dst_id++) {
      for_times += 1;
      // generate traffic if the ue is rrc reconfigured state
      if (mac_get_rrc_status(module_id, 1/*eNB_flag*/, dst_id) > 2 /*RRC_CONNECTED*/ ) {
        otg_pkt = malloc (sizeof(Packet_otg_elt));
        if_times += 1;

        (otg_pkt->otg_pkt).sdu_buffer = (u8*) packet_gen(module_id, dst_id + NB_eNB_INST, ctime, &((otg_pkt->otg_pkt).sdu_buffer_size));

        if ((otg_pkt->otg_pkt).sdu_buffer != NULL) {
          otg_times += 1;
          (otg_pkt->otg_pkt).rb_id = dst_id * NB_RB_MAX + DTCH;
          (otg_pkt->otg_pkt).module_id = module_id;
          (otg_pkt->otg_pkt).dst_id = dst_id;
          (otg_pkt->otg_pkt).mode = PDCP_DATA_PDU;
          //Adding the packet to the OTG-PDCP buffer
          pkt_list_add_tail_eurecom(otg_pkt, &(otg_pdcp_buffer[module_id]));
          LOG_I(EMU, "[eNB %d] ADD pkt to OTG buffer for dst %d on rb_id %d\n", (otg_pkt->otg_pkt).module_id, (otg_pkt->otg_pkt).dst_id,(otg_pkt->otg_pkt).rb_id);
        } else {
          //LOG_I(EMU, "OTG returns null \n");
          free(otg_pkt);
          otg_pkt=NULL;
        }
      }
      LOG_T(EMU,"[eNB %d] UE mod id %d is not connected\n", module_id, dst_id);
      //LOG_I(EMU,"HEAD of otg_pdcp_buffer[%d] is %p\n", module_id, pkt_list_get_head(&(otg_pdcp_buffer[module_id])));
    }

#ifdef Rel10
    int service_id, session_id, rb_id;
    // MBSM multicast traffic
    for (service_id = 0; service_id < 2 ; service_id++) { //maxServiceCount
      for (session_id = 0; session_id < 2; session_id++) { // maxSessionPerPMCH
        if (pdcp_mbms_array[module_id][service_id*maxSessionPerPMCH + session_id].instanciated_instance== module_id + 1){ // this service/session is configured
          otg_pkt = malloc (sizeof(Packet_otg_elt));
          // LOG_T(OTG,"multicast packet gen for (service/mch %d, session/lcid %d)\n", service_id, session_id);
          rb_id = pdcp_mbms_array[module_id][service_id*maxSessionPerPMCH + session_id].rb_id;
          (otg_pkt->otg_pkt).sdu_buffer = (u8*) packet_gen_multicast(module_id, session_id, ctime, &((otg_pkt->otg_pkt).sdu_buffer_size));
          if ((otg_pkt->otg_pkt).sdu_buffer != NULL) {
            (otg_pkt->otg_pkt).rb_id = rb_id;
            (otg_pkt->otg_pkt).module_id = module_id;
            //(otg_pkt->otg_pkt).dst_id = session_id;
            //Adding the packet to the OTG-PDCP buffer
            (otg_pkt->otg_pkt).mode = PDCP_TM;
            pkt_list_add_tail_eurecom(otg_pkt, &(otg_pdcp_buffer[module_id]));
            LOG_I(EMU, "[eNB %d] ADD pkt to OTG buffer for dst %d on rb_id %d\n", (otg_pkt->otg_pkt).module_id, (otg_pkt->otg_pkt).dst_id,(otg_pkt->otg_pkt).rb_id);
          } else {
            //LOG_I(EMU, "OTG returns null \n");
            free(otg_pkt);
            otg_pkt=NULL;
          }
        }
      }
    }
#endif

    //LOG_I(EMU, "[eNB %d] update OTG nb_elts = %d \n", module_id, otg_pdcp_buffer[module_id].nb_elements);

    //free(otg_pkt);
    //otg_pkt = NULL;
  }
#else
  if (otg_enabled==1) {
    ctime = frame * 100;
    for (dst_id = 0; dst_id < NUMBER_OF_UE_MAX; dst_id++) {
      if (mac_get_rrc_status(eNB_index, eNB_flag, dst_id ) > 2) {
        otg_pkt = malloc (sizeof(Packet_otg_elt));
        (otg_pkt->otg_pkt).sdu_buffer = packet_gen(module_id, dst_id, ctime, &pkt_size);
        if (otg_pkt != NULL) {
          rb_id = dst_id * NB_RB_MAX + DTCH;
          (otg_pkt->otg_pkt).rb_id = rb_id;
          (otg_pkt->otg_pkt).module_id = module_id;
          (otg_pkt->otg_pkt).mode = PDCP_DATA_PDU;
          //Adding the packet to the OTG-PDCP buffer
          pkt_list_add_tail_eurecom(otg_pkt, &(otg_pdcp_buffer[module_id]));
          LOG_I(EMU, "[eNB %d] ADD pkt to OTG buffer for dst %d on rb_id %d\n", (otg_pkt->otg_pkt).module_id, (otg_pkt->otg_pkt).dst_id,(otg_pkt->otg_pkt).rb_id);
        } else {
          //LOG_I(EMU, "OTG returns null \n");
          free(otg_pkt);
          otg_pkt=NULL;
        }
        /*else {
    LOG_I(OTG,"nothing generated (src %d, dst %d)\n",src_id, dst_id);
    }*/
      }
      /*else {
  LOG_I(OTG,"rrc_status (src %d, dst %d) = %d\n",src_id, dst_id, mac_get_rrc_status(src_id, eNB_flag, dst_id ));
  }*/
    }
  }
#endif
}

void update_otg_UE(int module_id, unsigned int ctime) {
  #if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.otg_enabled ==1 ) {
    int dst_id, src_id;
    int eNB_index = 0; //See how phy_procedures_UE_lte is called: 3rd parameter from the right = 0

    src_id = module_id;
    dst_id = eNB_index;

    if (mac_get_rrc_status(module_id, 0/*eNB_flag*/, eNB_index ) > 2 /*RRC_CONNECTED*/) {
      Packet_otg_elt *otg_pkt = malloc (sizeof(Packet_otg_elt));
      // Manage to add this packet to the tail of your list
      (otg_pkt->otg_pkt).sdu_buffer = (u8*) packet_gen(src_id, dst_id, ctime, &((otg_pkt->otg_pkt).sdu_buffer_size));

      if ((otg_pkt->otg_pkt).sdu_buffer != NULL) {
        (otg_pkt->otg_pkt).rb_id = eNB_index * NB_RB_MAX + DTCH;
        (otg_pkt->otg_pkt).module_id = module_id;
        //(otg_pkt->otg_pkt).dst_id = dst_id;
        //Adding the packet to the OTG-PDCP buffer
        (otg_pkt->otg_pkt).mode = PDCP_DATA_PDU;
        pkt_list_add_tail_eurecom(otg_pkt, &(otg_pdcp_buffer[module_id]));
      } else {
        free(otg_pkt);
        otg_pkt=NULL;
      }
    }
  }
#endif
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

void init_time() {
  clock_gettime (CLOCK_REALTIME, &time_spec);
  time_now = (unsigned long) time_spec.tv_nsec;
  td_avg = 0;
  sleep_time_us = SLEEP_STEP_US;
  td_avg = TARGET_SF_TIME_NS;
}

