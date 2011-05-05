#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/vars.h"
#include "UTIL/LOG/log_if.h"
#include "RRC/LITE/vars.h"
#include "PHY_INTERFACE/vars.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OPT/opt.h" // to test OPT
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"

#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "SCHED/defs.h"
#include "SCHED/vars.h"

#ifdef XFORMS
#include "forms.h"
#include "phy_procedures_sim_form.h"
#endif


#define RF

//#define DEBUG_SIM 

#define BW 7.68

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

#ifdef OPENAIR2
u16 NODE_ID[1];
u8 NB_INST=2;

char stats_buffer[16384];


extern void init_channel_vars(LTE_DL_FRAME_PARMS *frame_parms, double ***s_re,double ***s_im,double ***r_re,double ***r_im,double ***r_re0,double ***r_im0);

extern void do_UL_sig(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX],u16 next_slot,double *nf,double snr_dB,double sinr_dB,u8 abstraction_flag,LTE_DL_FRAME_PARMS *frame_parms);

extern void do_DL_sig(double **r_re0,double **r_im0,double **r_re,double **r_im,double **s_re,double **s_im,channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX],u16 next_slot,double *nf,double snr_dB,double sinr_dB,u8 abstraction_flag,LTE_DL_FRAME_PARMS *frame_parms);

extern void init_lte_vars(LTE_DL_FRAME_PARMS **frame_parms, u8 extended_prefix_flag, u8 cooperation_flag,u8 transmission_mode,u8 abstraction_flag);

#ifndef CYGWIN
void init_bypass() {

  msg("[PHYSIM] INIT BYPASS\n");      
  pthread_mutex_init(&Tx_mutex,NULL);
  pthread_cond_init(&Tx_cond,NULL);
  Tx_mutex_var=1; 
  pthread_mutex_init(&emul_low_mutex,NULL);
  pthread_cond_init(&emul_low_cond,NULL);
  emul_low_mutex_var=1; 
  bypass_init(emul_tx_handler,emul_rx_handler);
}
#endif

mapping log_level_names[] =
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


#endif

void help(void) {
  printf("Usage: physim -h -a -e -x transmission_mode -m target_dl_mcs -r(ate_adaptation) -n n_frames -s snr_dB -k ricean_factor -t max_delay -f forgetting factor -z cooperation_flag\n");
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
  printf("-c Activate the config generator (OCG) to porcess the scenario- 0: remote web server 1: local web server \n");
  printf("-x Set the transmission mode (1,2,6 supported for now)\n");
  printf("-z Set the cooperation flag (0 for no cooperation, 1 for delay diversity and 2 for distributed alamouti\n");
}

#ifdef XFORMS
void do_forms(FD_phy_procedures_sim *form, LTE_UE_DLSCH **lte_ue_dlsch_vars,LTE_eNB_ULSCH **lte_eNB_ulsch_vars, struct complex **ch,u32 ch_len) {

  s32 j,s,i;
  float I[3600],Q[3600],I2[3600],Q2[3600],I3[300],Q3[300];

  j=0;
  //  printf("rxdataF_comp %p, lte_ue_dlsch_vars[0] %p\n",lte_ue_dlsch_vars[0]->rxdataF_comp[0],lte_ue_dlsch_vars[0]);
  for (s=4;s<12;s++) {
    for(i=0;i<12*12;i++) {
      I[j] = (float)((short*)lte_ue_dlsch_vars[0]->rxdataF_comp[0])[(2*25*12*s)+2*i];
      Q[j] = (float)((short*)lte_ue_dlsch_vars[0]->rxdataF_comp[0])[(2*25*12*s)+2*i+1];
      //      printf("%d (%d): %f,%f : %d,%d\n",j,(25*12*s)+i,I[j],Q[j],lte_ue_dlsch_vars[0]->rxdataF_comp[0][(2*25*12*s)+2*i],lte_ue_dlsch_vars[0]->rxdataF_comp[0][(2*25*12*s)+2*i+1]);
      j++;
    }
    if (s==5)
      s=6;
    else if (s==8)
      s=9;
  }
  if (j>0)
    fl_set_xyplot_data(form->pdsch_constellation,I,Q,j,"","","");


  //fl_set_xyplot_xbounds(form->pdsch_constellation,-800,800);
  //fl_set_xyplot_ybounds(form->pdsch_constellation,-800,800);


  j=0;
  //  printf("rxdataF_comp %p, lte_ue_dlsch_vars[0] %p\n",lte_ue_dlsch_vars[0]->rxdataF_comp[0],lte_ue_dlsch_vars[0]);
  for (s=0;s<12;s++) {
    for(i=0;i<6*12;i++) {
      I2[j] = (float)((short*)lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0])[(2*25*12*s)+2*i];
      Q2[j] = (float)((short*)lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0])[(2*25*12*s)+2*i+1];
      //      printf("%d (%d): %f,%f : %d,%d\n",j,(25*12*s)+i,I[j],Q[j],lte_ue_dlsch_vars[0]->rxdataF_comp[0][(2*25*12*s)+2*i],lte_ue_dlsch_vars[0]->rxdataF_comp[0][(2*25*12*s)+2*i+1]);
      j++;
    }
    if (s==1)
      s=2;
    else if (s==7)
      s=8;
  }
  if (j>0)
    fl_set_xyplot_data(form->pusch_constellation,I2,Q2,j,"","","");

  fl_set_xyplot_xbounds(form->pusch_constellation,-800,800);
  fl_set_xyplot_ybounds(form->pusch_constellation,-800,800);

  for (j=0;j<ch_len;j++) {

    I3[j] = j;
    Q3[j] = 10*log10(ch[0][j].r*ch[0][j].r + ch[0][j].i*ch[0][j].i);
  }

  fl_set_xyplot_data(form->ch00,I3,Q3,ch_len,"","","");
  //fl_set_xyplot_ybounds(form->ch00,-20,20);
}
#endif


channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];


void set_envi(OAI_Emulation * emulation_scen) {
	/*LOG_T(OCG,"the area is x %f y %f option %s\n",
	emulation_scen->envi_config.area.x,
	emulation_scen->envi_config.area.y, 
	emulation_scen->topo_config.eNB_topology.selected_option);*/
	LOG_I(OCG, "environment is set\n");
}

void set_topo(OAI_Emulation * emulation_scen, emu_info_t * emu_info) {
	emu_info->nb_ue_local  = emulation_scen->topo_config.number_of_UE; // configure the number of UE
	emu_info->nb_enb_local = emulation_scen->topo_config.number_of_eNB; // configure the number of eNB


/*
// inputs for OMG : TODO
	emulation_scen->envi_config.area.x;
	emulation_scen->envi_config.area.y;
	emulation_scen->topo_config.mobility.moving_dynamics.min_speed;
	emulation_scen->topo_config.mobility.moving_dynamics.max_speed;
	emulation_scen->topo_config.mobility.moving_dynamics.min_pause_time;
	emulation_scen->topo_config.mobility.moving_dynamics.max_pause_time;

	emulation_scen->topo_config.number_of_UE;
	if (!strcmp(emulation_scen->topo_config.UE_distribution.selected_option, "random")) {
	} else if (!strcmp(emulation_scen->topo_config.UE_distribution.selected_option, "concentrated")) {
	} else if (!strcmp(emulation_scen->topo_config.UE_distribution.selected_option, "grid_map")) {
	}

	emulation_scen->topo_config.number_of_eNB;
	if (!strcmp(emulation_scen->topo_config.eNB_topology.selected_option, "random")) {
	} else if (!strcmp(emulation_scen->topo_config.eNB_topology.selected_option, "hexagonal")) {
	} else if (!strcmp(emulation_scen->topo_config.eNB_topology.selected_option, "grid")) {
	}

	if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "fixed")) {
	} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "random_waypoint")) {
	} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "random_walk")) {
	} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "grid_walk")) {
	}
	
	if (!strcmp(emulation_scen->topo_config.mobility.random_seed.selected_option, "oaiseed")) {
	} else if (!strcmp(emulation_scen->topo_config.mobility.random_seed.selected_option, "userseed")) {
		emulation_scen->topo_config.mobility.random_seed.user_seed.seed_value;
	}

// outputs from OMG : the positions of eNBs and UEs
	emulation_scen->topo_config.positions = OMG();
	
*/
	LOG_I(OCG, "topology is set\n");
}

void set_app(OAI_Emulation * emulation_scen) {

	LOG_I(OCG, "application is set\n");
}

void set_emu(OAI_Emulation * emulation_scen, u16 * n_frames) {
	*n_frames  =  (int) emulation_scen->emu_config.emu_time / 10; // configure the number of frame

	set_comp_log(OCG,  LOG_ERR, 1);
	set_comp_log(OCG,  LOG_INFO, 1);
	set_comp_log(OCG,  LOG_TRACE, 1);

	LOG_I(OCG, "emulation is set\n");
}


int main(int argc, char **argv) {

 
  char c;
  s32 i,j;
  double **s_re,**s_im,**r_re,**r_im,**r_re0,**r_im0;
  double amps[1] = {1};//{0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=.0000000000001,Td=.8,forgetting_factor=.999,maxDoppler=0;
  u8 channel_length,nb_taps=1;


  s32 n_errors;
  u16 n_frames, n_frames_flag;
  s32 slot,last_slot, next_slot;

  double nf[2] = {3.0,3.0}; //currently unused
  double snr_dB, sinr_dB,snr_dB2,sinr_dB2;
  u8 set_sinr = 0;
  u8 cooperation_flag; // for cooperative communication

  u8 target_dl_mcs=4;
  u8 target_ul_mcs=2;
  u8 rate_adaptation_flag;
  u8 transmission_mode;
  u8 abstraction_flag=0,ethernet_flag=0;
  u16 ethernet_id;
  u8 extended_prefix_flag=0;
  s32 UE_id,eNB_id,ret; 
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif
  //u8 nb_ue_local=1,nb_ue_remote=0;
  //u8 nb_eNB_local=1,nb_eNB_remote=0;
  //u8 first_eNB_local=0,first_UE_local=0, nb_machine=0;
 
  char * g_log_level="trace"; // by default global log level is set to trace 
  lte_subframe_t direction;

  OAI_Emulation * emulation_scen;

#ifdef XFORMS
  FD_phy_procedures_sim *form[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
  char title[255];
#endif
  LTE_DL_FRAME_PARMS *frame_parms;

  FILE *UE_stats, *eNB_stats; 
  int len; 
  int mod_path_loss=0;
  
  //time_t t0,t1;
  clock_t start, stop;
  
  //default parameters
  emu_info.is_primary_master=0;
  emu_info.master_list=0;
  emu_info.nb_ue_remote=0;
  emu_info.nb_enb_remote=0;
  emu_info.first_ue_local=0;
  emu_info.offset_ue_inst=0;
  emu_info.first_enb_local=0;
  emu_info.master_id=0;
  emu_info.nb_master =0;
  emu_info.nb_ue_local= 1;
  emu_info.nb_enb_local= 1;
  emu_info.ethernet_flag=0;
  strcpy(emu_info.local_server, ""); // this is the web portal version, ie. the httpd server is remote 
  emu_info.multicast_group=0;
  emu_info.ocg_enabled=0;// flag c
  emu_info.opt_enabled=0; // P flag
  emu_info.omg_enabled=0; //O flag 
  emu_info.otg_enabled=0;// T flag
  
  transmission_mode = 2;
  target_dl_mcs = 0;
  rate_adaptation_flag = 0;
  n_frames =  0xffff; //100; 
  n_frames_flag = 0;
  snr_dB = 30;

  cooperation_flag = 0; // default value 0 for no cooperation, 1 for Delay diversity, 2 for Distributed Alamouti

  while ((c = getopt (argc, argv, "haeOPTot:k:x:m:rn:s:S:f:z:u:b:c:M:p:g:l:d")) != -1)

    {
       switch (c)
	{
	case 'h':
	  help();
	  exit(1);
	case 'x':
	  transmission_mode = atoi(optarg);
	  break;
	case 'm':
	  target_dl_mcs = atoi(optarg);
	  break;
	case 'r':
	  rate_adaptation_flag = 1;
	  break;
	case 'n':
	  n_frames = atoi(optarg); 
	  n_frames_flag=1;
	  break;
	case 's':
	  snr_dB = atoi(optarg);
	  break;
	case 'S':
	  sinr_dB = atoi(optarg);
	  set_sinr = 1;
	  break;
	case 'k': 
	  ricean_factor = atof(optarg);
	  break;
	case 't':
    	  Td = atof(optarg);
	  break;
	case 'f':
	  forgetting_factor = atof(optarg);
	  break;
	case 'z':
	  cooperation_flag=atoi(optarg);
	  break;
	case 'u':
	  emu_info.nb_ue_local = atoi(optarg);
	  break;
	  //	case 'U':
	  //nb_ue_remote = atoi(optarg);
	  //break;
	case 'b':
	  emu_info.nb_enb_local = atoi(optarg);
	  break;
	  //	case 'B':
	  // nb_eNB_remote = atoi(optarg);
	  //break;
	case 'a':
	  abstraction_flag=1;
	  break;
	case 'p':
	  emu_info.nb_master = atoi(optarg);
	  break;
	case 'M':
	  abstraction_flag=1;
	  ethernet_flag=1;
	  ethernet_id = atoi(optarg);
	  emu_info.master_id=ethernet_id;
	  emu_info.ethernet_flag=1;
	  break;
	case 'e':
	  extended_prefix_flag=1;
	  break;
	case 'l':
	  g_log_level=optarg;
	  break;
	case 'c':
          strcpy(emu_info.local_server, optarg);
	  emu_info.ocg_enabled=1;
	  abstraction_flag=1;
	  extended_prefix_flag=1;
	  n_frames_flag=1; 
	  transmission_mode = 1;
	  break;
	case 'g':
	  emu_info.multicast_group=atoi(optarg);
	  break;	
	case 'O':
	  emu_info.omg_enabled=1;
	  break; 
	case 'T':
	  emu_info.otg_enabled=1;
	  break;
	case 'P':
	  emu_info.opt_enabled=1;
	  break;
	case 'o':
	  mod_path_loss = 1;
	  break;
	default:
	  help ();
	  exit (-1);
	  break;
	}
    }
  if (set_sinr==0)
    sinr_dB = snr_dB-20;

 //initialize the log generator 
  logInit(map_str_to_int(log_level_names, g_log_level));
  LOG_T(LOG,"global log level is set to %s \n",g_log_level );

#ifdef OCG_FLAG
  if (emu_info.ocg_enabled == 1){ // activate OCG: xml-based scenario parser 
    emulation_scen= OCG_main(emu_info.local_server);// eurecom or portable
    // here is to check if OCG is successful, otherwise, we might not run the emulation
    if (emulation_scen->useful_info.OCG_OK != 1) { 
      LOG_E(OCG, "Error found by OCG; emulation not launched. Please find more information in the OCG_report.xml. \nRemind: please check the name of the XML configuration file and its content if you use a self-specified file.\n");
      exit(EXIT_FAILURE);
      }

      set_envi(emulation_scen);
      set_topo(emulation_scen, &emu_info);
      set_app(emulation_scen);
      set_emu(emulation_scen, &n_frames);

      LOG_T(OCG," ue local %d enb local %d frame %d\n",   emu_info.nb_ue_local,   emu_info.nb_enb_local, n_frames );

     /* : TODO
      LOG_I(OCG, "OPT output file directory = %s\n", emulation_scen->useful_info.output_path);
      Init_OPT(2, emulation_scen->useful_info.output_path, NULL, 0);*/

   }
#endif

#ifndef CYGWIN 
  ret=netlink_init();
#endif
  
  if (ethernet_flag==1){
    emu_info.master[emu_info.master_id].nb_ue=emu_info.nb_ue_local;
    emu_info.master[emu_info.master_id].nb_enb=emu_info.nb_enb_local;

    if(!emu_info.master_id) 
      emu_info.is_primary_master=1;
    j=1;
    for(i=0;i<emu_info.nb_master;i++){
      if(i!=emu_info.master_id)
	emu_info.master_list=emu_info.master_list+j;
      LOG_T(EMU, "Index of master id i=%d  MASTER_LIST %d\n",i,emu_info.master_list);
      j*=2;
    }
    LOG_T(EMU," Total number of master %d my master id %d\n", 
	  emu_info.nb_master,
	  emu_info.master_id);
#ifndef CYGWIN    
    init_bypass();
#endif
    
    while (emu_tx_status != SYNCED_TRANSPORT ) {
      LOG_T(EMU, " Waiting for EMU Transport to be synced\n"); 
      emu_transport_sync();//emulation_tx_rx();
    }
  }// ethernet flag

  UE_stats = fopen("UE_stats.txt", "w");
  eNB_stats = fopen("eNB_stats.txt", "w");
  printf("UE_stats=%d, eNB_stats=%d\n",UE_stats,eNB_stats);

  NB_UE_INST = emu_info.nb_ue_local + emu_info.nb_ue_remote;
  NB_eNB_INST = emu_info.nb_enb_local + emu_info.nb_enb_remote;
      
  LOG_I(EMU, "total number of UE %d (local %d, remote %d) \n", NB_UE_INST,emu_info.nb_ue_local,emu_info.nb_ue_remote);
  LOG_I(EMU, "Total number of eNB %d (local %d, remote %d) \n", NB_eNB_INST,emu_info.nb_enb_local,emu_info.nb_enb_remote);
   
  printf("Running with mode %d, target dl_mcs %d, rate adaptation %d, nframes %d\n",
  	 transmission_mode,target_dl_mcs,rate_adaptation_flag,n_frames);

  channel_length = (u8) (11+2*BW*Td);


  init_lte_vars(&frame_parms, extended_prefix_flag, cooperation_flag, transmission_mode, abstraction_flag);


  if (abstraction_flag==0)
    init_channel_vars(frame_parms,&s_re,&s_im,&r_re,&r_im,&r_re0,&r_im0);

  // initialize channel descriptors
  for (eNB_id=0;eNB_id<NB_eNB_INST;eNB_id++) {
    for (UE_id=0;UE_id<NB_UE_INST;UE_id++) {
#ifdef DEBUG_SIM
      printf("[SIM] Initializing channel from eNB %d to UE %d\n",eNB_id,UE_id);
#endif
      eNB2UE[eNB_id][UE_id] = new_channel_desc(PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_tx,
					       PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_rx,
					       nb_taps,
					       channel_length,
					       amps,
					       NULL,
					       NULL,
					       Td,
					       BW,
					       ricean_factor,
					       aoa,
					       forgetting_factor,
					       maxDoppler,
					       0,
					       0);
      
      eNB2UE[eNB_id][UE_id]->path_loss_dB = -105 + snr_dB;

      UE2eNB[UE_id][eNB_id] = new_channel_desc(PHY_vars_UE_g[UE_id]->lte_frame_parms.nb_antennas_tx,
					       PHY_vars_eNB_g[eNB_id]->lte_frame_parms.nb_antennas_rx,
					       nb_taps,
					       channel_length,
					       amps,
					       NULL,
					       NULL,
					       Td,
					       BW,
					       ricean_factor,
					       aoa,
					       forgetting_factor,
					       maxDoppler,
					       0,
					       0);
      
      UE2eNB[UE_id][eNB_id]->path_loss_dB = -105 + snr_dB;// - 20;
#ifdef DEBUG_SIM
      printf("[SIM] Path loss from eNB %d to UE %d => %f dB\n",eNB_id,UE_id,eNB2UE[eNB_id][UE_id]->path_loss_dB);
      printf("[SIM] Path loss from UE %d to eNB %d => %f dB\n",UE_id,eNB_id,UE2eNB[UE_id][eNB_id]->path_loss_dB);
#endif
    }
  }

  randominit(0);
  set_taus_seed(0);

  number_of_cards = 1;

  openair_daq_vars.rx_rf_mode = 1;
  openair_daq_vars.tdd = 1;
  openair_daq_vars.rx_gain_mode = DAQ_AGC_ON;
  if ((transmission_mode != 1) && (transmission_mode != 6))
    openair_daq_vars.dlsch_transmission_mode = 2;
  else
    openair_daq_vars.dlsch_transmission_mode = transmission_mode;
  openair_daq_vars.target_ue_dl_mcs = target_dl_mcs;
  openair_daq_vars.target_ue_ul_mcs = target_ul_mcs;
  openair_daq_vars.dlsch_rate_adaptation = rate_adaptation_flag;
  openair_daq_vars.ue_ul_nb_rb = 2;

  for (UE_id=0; UE_id<NB_UE_INST;UE_id++){ // begin navid
    PHY_vars_UE_g[UE_id]->rx_total_gain_dB=140;
    PHY_vars_UE_g[UE_id]->UE_mode[0] = PRACH;
    PHY_vars_UE_g[UE_id]->lte_ue_pdcch_vars[0]->crnti = 0xBEEF;
    PHY_vars_UE_g[UE_id]->current_dlsch_cqi[0]=4;
  }// end navid 
 

#ifdef XFORMS
  fl_initialize(&argc, argv, NULL, 0, 0);
  for (UE_id=0; UE_id<NB_UE_INST;UE_id++)
    for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++) {
      form[eNB_id][UE_id] = create_form_phy_procedures_sim();                 
      sprintf(title,"LTE SIM UE %d eNB %d",UE_id,eNB_id);   
      fl_show_form(form[eNB_id][UE_id]->phy_procedures_sim,FL_PLACE_HOTSPOT,FL_FULLBORDER,title);
    }
#endif


#ifdef OPENAIR2
  l2_init(&PHY_vars_eNB_g[0]->lte_frame_parms);



  for (i=0;i<NB_eNB_INST;i++)
    mac_xface->mrbch_phy_sync_failure(i,i);
#ifdef DEBUG_SIM
  printf("[SIM] Synching to eNB\n");
#endif
  for (UE_id=0;UE_id<NB_UE_INST;UE_id++)
    mac_xface->chbch_phy_sync_success(UE_id,0);
#endif 
 

  for (mac_xface->frame=0; mac_xface->frame<n_frames; mac_xface->frame++) {
    if (n_frames_flag == 0) // if n_frames not set bu the user then let the emulation run to infinity
      mac_xface->frame %=(n_frames-1);
    
    for (slot=0 ; slot<20 ; slot++) {
      last_slot = (slot - 1)%20;
      if (last_slot <0)
	last_slot+=20;
      next_slot = (slot + 1)%20;

      if((next_slot %2) ==0)
	clear_eNB_transport_info(emu_info.nb_enb_local);
      
      for (eNB_id=emu_info.first_enb_local;eNB_id<(emu_info.first_enb_local+emu_info.nb_enb_local);eNB_id++) {
#ifdef DEBUG_SIM
	printf("[SIM] EMU PHY procedures eNB %d for frame %d, slot %d (subframe %d) (rxdataF_ext %p)\n",eNB_id,mac_xface->frame,slot,next_slot>>1,PHY_vars_eNB_g[0]->lte_eNB_ulsch_vars[0]->rxdataF_ext);
#endif
	phy_procedures_eNB_lte(last_slot,next_slot,PHY_vars_eNB_g[eNB_id],abstraction_flag);
	//if ((mac_xface->frame % 10) == 0) {
	len = dump_eNB_stats(PHY_vars_eNB_g[eNB_id],stats_buffer,0);
	rewind(eNB_stats);
	fwrite(stats_buffer,1,len,eNB_stats);
	//}
      }
      direction = subframe_select(frame_parms,next_slot>>1);
      
      if (ethernet_flag ==1) { // include PBCH
	if (( (direction == SF_DL) || (direction == SF_S) ) && (((next_slot%2)== 0) || (next_slot==1))){ 
	  //LOG_T(EMU, "DL frame %d subframe %d slot %d \n", mac_xface->frame, next_slot>>1, slot);
	  //assert((start = clock())!=-1);// t0= time(NULL);
	  emu_transport_DL(mac_xface->frame, last_slot,next_slot);
	  //stop = clock(); //t1= time(NULL);
	  //LOG_T(PERF,"emu_transport_DL diff time %f (ms)\n",	(double) (stop-start)/1000);
	}
      }
      // Call ETHERNET emulation here
      if((next_slot %2) == 0) 
	clear_UE_transport_info(emu_info.nb_ue_local);


      for (UE_id=emu_info.first_ue_local; UE_id<(emu_info.first_ue_local+emu_info.nb_ue_local);UE_id++)
	if (mac_xface->frame >= (UE_id*10)) { // activate UE only after 10*UE_id frames so that different UEs turn on separately
	  printf("[SIM] EMU PHY procedures UE %d for frame %d, slot %d (subframe %d)\n", UE_id,mac_xface->frame,slot, next_slot>>1);
	  //printf("[SIM] txdataF[0] %p\n",PHY_vars_UE_g[UE_id]->lte_ue_common_vars.txdataF[0]);
	  phy_procedures_UE_lte(last_slot,next_slot,PHY_vars_UE_g[UE_id],0,abstraction_flag);
	  //if ((mac_xface->frame % 10) == 0) {
	  len=dump_ue_stats(PHY_vars_UE_g[UE_id],stats_buffer,0);
	  rewind(UE_stats);
	  fwrite(stats_buffer,1,len,UE_stats);
	  //}
	}
      
      if (ethernet_flag == 1){
	if (((direction == SF_UL) && ((next_slot%2)==0)) || ((direction == SF_S) && ((last_slot%2)==1))){
	  //  LOG_T(EMU, "UL frame %d subframe %d slot %d \n", mac_xface->frame, next_slot>>1, slot);
	  //assert((start = clock())!=-1);//t0= time(NULL);
	  emu_transport_UL(mac_xface->frame, last_slot , next_slot);
	  // stop = clock(); // t1= time(NULL);
	  //LOG_T(PERF,"emu_transport_UL diff time %f (ms)\n",	(double) (stop-start)/1000);
	   
	}
      }

      if (mod_path_loss && ((mac_xface->frame % 150) >= 100)){
	snr_dB2 = -20;
	sinr_dB2 = -40;
      }
      else {
	snr_dB2 = snr_dB;
	sinr_dB2 = sinr_dB;
      }
      if (direction  == SF_DL) {
	/*
	  u8 aarx,aatx,k;	  for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	  for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	  for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	  printf("DL A(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	*/
	do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,next_slot,nf,snr_dB2,sinr_dB2,abstraction_flag,frame_parms);
	/*
	  for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	  for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	  for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	  printf("DL B(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	*/
      }
      else if (direction  == SF_UL) {
	/*
	  for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	  for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	  for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	  printf("UL A(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	*/
	do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,next_slot,nf,snr_dB2,sinr_dB2,abstraction_flag,frame_parms);
	/*
	  for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	  for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	  for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	  printf("UL B(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	*/
      }
      else {//it must be a special subframe
	if (next_slot%2==0) {//DL part
	  /*
	    for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	    for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	    for (k=0;k<UE2eNB[1][0]->channel_length;k++)
		
	    printf("SA(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	  */
	  do_DL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,eNB2UE,next_slot,nf,snr_dB2,sinr_dB2,abstraction_flag,frame_parms);
	  /*
	    for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	    for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	    for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	    printf("SB(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	  */
	}
	else {// UL part
	  /*
	    for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	    for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	    for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	    printf("SC(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	  */
	  do_UL_sig(r_re0,r_im0,r_re,r_im,s_re,s_im,UE2eNB,next_slot,nf,snr_dB2,sinr_dB2,abstraction_flag,frame_parms);
	  /*
	    for (aarx=0;aarx<UE2eNB[1][0]->nb_rx;aarx++)
	    for (aatx=0;aatx<UE2eNB[1][0]->nb_tx;aatx++)
	    for (k=0;k<UE2eNB[1][0]->channel_length;k++)
	    printf("SD(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].r,UE2eNB[1][0]->ch[aarx+(aatx*UE2eNB[1][0]->nb_rx)][k].i);
	  */
	}
      }
      if ((last_slot==1) && (mac_xface->frame==1) && (abstraction_flag==0)) {
	/*
	write_output("UErxsigF0.m","rxsF0", PHY_vars_UE_g[0]->lte_ue_common_vars.rxdataF[0],frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti,2,1);
	write_output("eNBrxsigF0.m","rxsF0", PHY_vars_eNB_g[0]->lte_eNB_common_vars.rxdataF[0][0],frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti,2,1);
	write_output("dlchan0.m","dlch0",&(PHY_vars_UE_g[0]->lte_ue_common_vars.dl_ch_estimates[0][0][0]),(6*(PHY_vars_UE_g[0]->lte_frame_parms.ofdm_symbol_size)),1,1);
	write_output("pbch_rxF_comp0.m","pbch_comp0",PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->rxdataF_comp[0],6*12*4,1,1);
	write_output("pbch_rxF_llr.m","pbch_llr",PHY_vars_UE_g[0]->lte_ue_pbch_vars[0]->llr,(frame_parms->Ncp==0) ? 1920 : 1728,1,4);
	*/
      }
      /*
      if ((last_slot==1) && (mac_xface->frame==1)) {
	write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",PHY_vars_UE->lte_ue_dlsch_vars[eNB_id]->rxdataF_comp[0],300*(-(PHY_vars_UE->lte_frame_parms.Ncp*2)+14),1,1);
	write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0],4*300,1,1);
      }
      */ 
    }

    if ((mac_xface->frame==1)&&(abstraction_flag==0)) {
      write_output("UErxsig0.m","rxs0", PHY_vars_UE_g[0]->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      write_output("eNBrxsig0.m","rxs0", PHY_vars_eNB_g[0]->lte_eNB_common_vars.rxdata[0][0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    }

#ifdef XFORMS
    for (UE_id=0; UE_id<NB_UE_INST;UE_id++)
      for (eNB_id=0; eNB_id<NB_eNB_INST;eNB_id++) {
	do_forms(form[eNB_id][UE_id],PHY_vars_UE_g[UE_id]->lte_ue_dlsch_vars,PHY_vars_eNB_g[eNB_id]->lte_eNB_ulsch_vars,eNB2UE[eNB_id][UE_id]->ch,eNB2UE[eNB_id][UE_id]->channel_length);
      }
#endif
      
  }
  // relase all rx state
  if (ethernet_flag==1){ emu_transport_release();}
  
  if (abstraction_flag==0) {
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

    for (i=0;i<2;i++) {
      free(s_re[i]);
      free(s_im[i]);
      free(r_re[i]);
      free(r_im[i]);
    }
    free(s_re);
    free(s_im);
    free(r_re);
    free(r_im);
    
    lte_sync_time_free();
  }

  fclose(UE_stats);
  fclose(eNB_stats);

  return(n_errors);
}
   

