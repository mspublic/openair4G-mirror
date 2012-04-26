#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#include "LAYER2/MAC/vars.h"
#include "OCG_vars.h"

extern unsigned short dftsizes[33];
extern short *ul_ref_sigs[30][2][33];

//#define AWGN
//#define NO_DCI

#define BW 7.68

PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;

#define MCS_COUNT 23//added for PHY abstraction

channel_desc_t *eNB2UE[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX];
channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];
//Added for PHY abstraction
node_desc_t *enb_data[NUMBER_OF_eNB_MAX]; 
node_desc_t *ue_data[NUMBER_OF_UE_MAX];
double sinr_bler_map[MCS_COUNT][2][9];

extern u16 beta_ack[16],beta_ri[16],beta_cqi[16];

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,u8 extended_prefix_flag,u8 N_RB_DL,u8 frame_type,u8 tdd_config,u8 osf) {

  LTE_DL_FRAME_PARMS *lte_frame_parms;

  printf("Start lte_param_init\n");
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

  lte_frame_parms->frame_type         = frame_type;
  lte_frame_parms->tdd_config         = tdd_config;
  lte_frame_parms->N_RB_DL            = N_RB_DL;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = N_RB_DL;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Ncp_UL             = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = 10;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;
  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift = 0;//n_DMRS1 set to 0

  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;
  
  phy_init_lte_top(lte_frame_parms);

  phy_init_lte_ue(PHY_vars_UE,0);

  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);

  printf("Done lte_param_init\n");


}



/*
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
*/

//#define UL_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,18);
#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(PHY_vars_eNB->lte_frame_parms.N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fff // igore DC component,RB13
//#define DLSCH_RB_ALLOC 0x1f0f // igore DC component,RB13



int main(int argc, char **argv) {

  char c;
  int i,j,aa,b,u,Msc_RS_idx;

 
  double sigma2, sigma2_dB=10,SNR,snr0=-2.0,snr1,SNRmeas,rate;
  //int **txdataF, **txdata;
  int **txdata;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  LTE_DL_FRAME_PARMS *frame_parms;
  double **s_re,**s_im,**r_re,**r_im;
  double forgetting_factor=0.0; //in [0,1] 0 means a new channel every time, 1 means keep the same channel
  double iqim=0.0;
  u8 extended_prefix_flag=0;

  int eNB_id = 0;
  int UE_id = 0;
  unsigned char nb_rb=2,first_rb=0,mcs=4,round=0,bundling_flag=1;
  unsigned char l;
  SCM_t channel_model=Rayleigh1_corr;

  unsigned char *input_buffer,harq_pid;
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb;
  int subframe=3;

  unsigned int tx_lev,tx_lev_dB=0,trials,errs[4]={0,0,0,0},round_trials[4]={0,0,0,0};

  FILE *bler_fd;
  char bler_fname[20];

  char fname[20],vname[20];

  FILE *input_fd=NULL,*trch_out_fd=NULL;
  unsigned char input_file=0;
  char input_val_str[50],input_val_str2[50];

  //  FILE *rx_frame_file;

  int n_frames=100;

  channel_desc_t *UE2eNB;

  u8 control_only_flag = 0;

  u8 srs_flag = 0;

  u8 N_RB_DL=25,osf=1;

  u8 cyclic_shift = 0;
  u8 cooperation_flag = 0; //0 no cooperation, 1 delay diversity, 2 Alamouti
  u8 beta_ACK=0,beta_RI=0,beta_CQI=2;
  u8 tdd_config=3,frame_type=0;

  u8 N0=40;
  double tx_gain=1.0;

  logInit();

  while ((c = getopt (argc, argv, "hapbm:n:s:c:r:i:f:c:oA:C:R:g:N:S:T:")) != -1) {
    switch (c) {
    case 'a':
      channel_model = AWGN;
      break;
    case 'b':
      bundling_flag = 0;
      break;
    case 'm':
      mcs = atoi(optarg);
      break;
    case 'n':
      n_frames = atoi(optarg);
      break;
    case 'g':
      switch((char)*optarg) {
      case 'A': 
	channel_model=SCM_A;
	break;
      case 'B': 
	channel_model=SCM_B;
	break;
      case 'C': 
	channel_model=SCM_C;
	break;
      case 'D': 
	channel_model=SCM_D;
	break;
      case 'E': 
	channel_model=EPA;
	break;
      case 'F': 
	channel_model=EVA;
	break;
      case 'G': 
	channel_model=ETU;
	break;
      case 'H':
	channel_model=Rayleigh8;
	break;
      case 'I':
	channel_model=Rayleigh1;
	break;
      case 'J':
	channel_model=Rayleigh1_corr;
	break;
      case 'K':
	channel_model=Rayleigh1_anticorr;
	break;
      case 'L':
	channel_model=Rice8;
	break;
      case 'M':
	channel_model=Rice1;
	break;
      default:
	msg("Unsupported channel model!\n");
	exit(-1);
	break;
      }
      break;
    case 's':
      snr0 = atoi(optarg);
      break;
    case 'S':
      subframe = atoi(optarg);
      break;
    case 'T':
      tdd_config=atoi(optarg);
      frame_type=1;
      break;
    case 'p':
      extended_prefix_flag=1;
      break;
    case 'r':
      nb_rb = atoi(optarg);
      break;
    case 'f':
      first_rb = atoi(optarg);
      break;
    case 'c':
      cyclic_shift = atoi(optarg);
      break;
    case 'N':
      N0 = atoi(optarg);
      break;
    case 'o':
      srs_flag = 1;
      break;
    case 'i':
      input_fd = fopen(optarg,"r");
      msg("Reading in %s (%p)\n",optarg,input_fd);
      if (input_fd == (FILE*)NULL) {
	msg("Unknown file %s\n",optarg);
	exit(-1);
      }
      input_file=1;
      break;
    case 'A':
      beta_ACK = atoi(optarg);
      if (beta_ACK>15) {
	printf("beta_ack must be in (0..15)\n");
	exit(-1);
      }
      break;
    case 'C':
      beta_CQI = atoi(optarg);
      if ((beta_CQI>15)||(beta_CQI<2)) {
	printf("beta_cqi must be in (2..15)\n");
	exit(-1);
      }
      break;
      
    case 'R':
      beta_RI = atoi(optarg);
      if ((beta_RI>15)||(beta_RI<2)) {
	printf("beta_ri must be in (0..13)\n");
	exit(-1);
      }
      break;
    case 'h':
    default:
      printf("%s -h(elp) -a(wgn on) -m mcs -n n_frames -s snr0 -t delay_spread -p (extended prefix on) -r nb_rb -f first_rb -c cyclic_shift -o (srs on) -g channel_model [A:M] Use 3GPP 25.814 SCM-A/B/C/D('A','B','C','D') or 36-101 EPA('E'), EVA ('F'),ETU('G') models (ignores delay spread and Ricean factor), Rayghleigh8 ('H'), Rayleigh1('I'), Rayleigh1_corr('J'), Rayleigh1_anticorr ('K'), Rice8('L'), Rice1('M') \n",argv[0]);
      exit(1);
      break;
    }
  }
  
  lte_param_init(1,1,1,extended_prefix_flag,N_RB_DL,frame_type,tdd_config,osf);  
  printf("Setting mcs = %d\n",mcs);
  printf("n_frames = %d\n",n_frames);

  snr1 = snr0+25.0;
  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  /*
    txdataF    = (int **)malloc16(2*sizeof(int*));
    txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
    txdata    = (int **)malloc16(2*sizeof(int*));
    txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */

  frame_parms = &PHY_vars_eNB->lte_frame_parms;

#ifdef IFFT_FPGA
  txdata    = (int **)malloc16(2*sizeof(int*));
  txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
  txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);

  bzero(txdata[0],FRAME_LENGTH_BYTES);
  bzero(txdata[1],FRAME_LENGTH_BYTES);

  txdataF2    = (int **)malloc16(2*sizeof(int*));
  txdataF2[0] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);
  txdataF2[1] = (int *)malloc16(FRAME_LENGTH_BYTES_NO_PREFIX);

  bzero(txdataF2[0],FRAME_LENGTH_BYTES_NO_PREFIX);
  bzero(txdataF2[1],FRAME_LENGTH_BYTES_NO_PREFIX);
#else
  txdata = PHY_vars_UE->lte_ue_common_vars.txdata;
#endif

  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  //  r_re0 = malloc(2*sizeof(double*));
  //  r_im0 = malloc(2*sizeof(double*));

  nsymb = (PHY_vars_eNB->lte_frame_parms.Ncp == 0) ? 14 : 12;
  
  coded_bits_per_codeword = nb_rb * (12 * get_Qm(mcs)) * nsymb;

  rate = (double)dlsch_tbs25[get_I_TBS(mcs)][nb_rb-1]/(coded_bits_per_codeword);

  printf("Rate = %f (mod %d)\n",rate,get_Qm(mcs));
  

  sprintf(bler_fname,"bler_%d.m",mcs);
  bler_fd = fopen(bler_fname,"w");
  //fprintf(bler_fd,"bler = [");

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    r_re0[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    bzero(r_re0[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    r_im0[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    //    bzero(r_im0[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }


  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = 14;

  // Fill in UL_alloc
  CCCH_alloc_pdu.type               = 0;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 0;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = mcs;  
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2.tpmi             = 5 ;  // precoding

  // Create transport channel structures for SI pdus
  PHY_vars_eNB->dlsch_eNB_SI   = new_eNB_dlsch(1,1,0);
  PHY_vars_UE->dlsch_ue_SI[0]  = new_ue_dlsch(1,1,0);
  PHY_vars_eNB->dlsch_eNB_SI->rnti  = SI_RNTI;
  PHY_vars_UE->dlsch_ue_SI[0]->rnti = SI_RNTI;


  PHY_vars_UE->lte_frame_parms.soundingrs_ul_config_common.srs_BandwidthConfig = 2;
  PHY_vars_UE->lte_frame_parms.soundingrs_ul_config_common.srs_SubframeConfig = 7;
  PHY_vars_UE->soundingrs_ul_config_dedicated[eNB_id].srs_Bandwidth = 0;
  PHY_vars_UE->soundingrs_ul_config_dedicated[eNB_id].transmissionComb = 0;
  PHY_vars_UE->soundingrs_ul_config_dedicated[eNB_id].freqDomainPosition = 0;

  PHY_vars_eNB->lte_frame_parms.soundingrs_ul_config_common.srs_BandwidthConfig = 2;
  PHY_vars_eNB->lte_frame_parms.soundingrs_ul_config_common.srs_SubframeConfig = 7;

  PHY_vars_eNB->soundingrs_ul_config_dedicated[UE_id].srs_ConfigIndex = 1;
  PHY_vars_eNB->soundingrs_ul_config_dedicated[UE_id].srs_Bandwidth = 0;
  PHY_vars_eNB->soundingrs_ul_config_dedicated[UE_id].transmissionComb = 0;
  PHY_vars_eNB->soundingrs_ul_config_dedicated[UE_id].freqDomainPosition = 0;
  PHY_vars_eNB->cooperation_flag = cooperation_flag;
  //  PHY_vars_eNB->eNB_UE_stats[0].SRS_parameters = PHY_vars_UE->SRS_parameters;

  PHY_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_ACK_Index = beta_ACK;
  PHY_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_RI_Index  = beta_RI;
  PHY_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_CQI_Index = beta_CQI;
  PHY_vars_UE->pusch_config_dedicated[eNB_id].betaOffset_ACK_Index = beta_ACK;
  PHY_vars_UE->pusch_config_dedicated[eNB_id].betaOffset_RI_Index  = beta_RI;
  PHY_vars_UE->pusch_config_dedicated[eNB_id].betaOffset_CQI_Index = beta_CQI;
  
  printf("PUSCH Beta : ACK %f, RI %f, CQI %f\n",(double)beta_ack[beta_ACK]/8,(double)beta_ri[beta_RI]/8,(double)beta_cqi[beta_CQI]/8);

  UE2eNB = new_channel_desc_scm(1,//PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				1,//PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				forgetting_factor,
				0,
				0);
  PHY_vars_eNB->ulsch_eNB[0] = new_eNB_ulsch(8,0);
  PHY_vars_UE->ulsch_ue[0]   = new_ue_ulsch(8,0);

  // Create transport channel structures for 2 transport blocks (MIMO)
  for (i=0;i<2;i++) {
    PHY_vars_eNB->dlsch_eNB[0][i] = new_eNB_dlsch(1,8,0);
    PHY_vars_UE->dlsch_ue[0][i]  = new_ue_dlsch(1,8,0);
  
    if (!PHY_vars_eNB->dlsch_eNB[0][i]) {
      printf("Can't get eNB dlsch structures\n");
      exit(-1);
    }
    
    if (!PHY_vars_UE->dlsch_ue[0][i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }
    
    PHY_vars_eNB->dlsch_eNB[0][i]->rnti = 14;
    PHY_vars_UE->dlsch_ue[0][i]->rnti   = 14;

  }
  
  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.rballoc = computeRIV(PHY_vars_eNB->lte_frame_parms.N_RB_UL,first_rb,nb_rb);// 12 RBs from position 8
  printf("rballoc %d (dci %x)\n",UL_alloc_pdu.rballoc,*(u32 *)&UL_alloc_pdu);
  UL_alloc_pdu.mcs     = mcs;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 0;
  UL_alloc_pdu.cshift  = 0;
  UL_alloc_pdu.dai     = 1;

  PHY_vars_UE->PHY_measurements.rank[0] = 0;
  PHY_vars_UE->transmission_mode[0] = 2;
  PHY_vars_UE->pucch_config_dedicated[0].tdd_AckNackFeedbackMode = bundling_flag == 1 ? bundling : multiplexing;
  PHY_vars_eNB->transmission_mode[0] = 2;
  PHY_vars_eNB->pucch_config_dedicated[0].tdd_AckNackFeedbackMode = bundling_flag == 1 ? bundling : multiplexing;
  PHY_vars_UE->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  PHY_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  PHY_vars_UE->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  PHY_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  PHY_vars_UE->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  PHY_vars_eNB->lte_frame_parms.pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  msg("Init UL hopping UE\n");
  init_ul_hopping(&PHY_vars_UE->lte_frame_parms);
  msg("Init UL hopping eNB\n");
  init_ul_hopping(&PHY_vars_eNB->lte_frame_parms);
  /*
  if (n_frames==1) {
    for (b=0;b<33;b++) {
      printf("dftsizes[%d] %d\n",b,dftsizes[b]);
      if ((nb_rb*12)==dftsizes[b])
	Msc_RS_idx = b;
    }
    printf("nb_rb %d => Msc_RS_idx %d\n",nb_rb,Msc_RS_idx);
    for (u=0;u<30;u++) {
      printf("Writing u %d\n",u);
      sprintf(fname,"ul_zc%d_%d.m",nb_rb,u);
      sprintf(vname,"ulzc%d_%d",nb_rb,u);
      write_output(fname,vname,(void*)&ul_ref_sigs[u][0][Msc_RS_idx][0],2*nb_rb*12,1,1);
    }
  }
  */

  generate_ue_ulsch_params_from_dci((void *)&UL_alloc_pdu,
				    14,
				    ul_subframe2pdcch_alloc_subframe(&PHY_vars_UE->lte_frame_parms,subframe),
				    format0,
				    PHY_vars_UE,
				    SI_RNTI,
				    0,
				    P_RNTI,
				    0,
				    srs_flag);

  //  printf("RIV %d\n",UL_alloc_pdu.rballoc);

  generate_eNB_ulsch_params_from_dci((DCI0_5MHz_TDD_1_6_t *)&UL_alloc_pdu,
				     14,
				     ul_subframe2pdcch_alloc_subframe(&PHY_vars_eNB->lte_frame_parms,subframe),
				     format0,
				     0,
				     PHY_vars_eNB,
				     SI_RNTI,
				     0,
				     P_RNTI,
				     srs_flag);

  PHY_vars_UE->ulsch_ue[0]->o_ACK[0] = 1;

  for (SNR=snr0;SNR<snr1;SNR+=.2) {
    errs[0]=0;
    errs[1]=0;
    errs[2]=0;
    errs[3]=0;
    round_trials[0] = 0;
    round_trials[1] = 0;
    round_trials[2] = 0;
    round_trials[3] = 0;

    round=0;

    randominit(0);
      
    PHY_vars_UE->frame=1;
    PHY_vars_eNB->frame=1;
    harq_pid = subframe2harq_pid(&PHY_vars_UE->lte_frame_parms,PHY_vars_UE->frame,subframe);
    //    printf("harq_pid %d\n",harq_pid);
    if (input_fd == NULL) {
      input_buffer_length = PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->TBS/8;
      input_buffer = (unsigned char *)malloc(input_buffer_length+4);

      if (n_frames == 1) {
	trch_out_fd = fopen("ulsch_trch.txt","w");
	for (i=0;i<input_buffer_length;i++) {
	  input_buffer[i] = taus()&0xff;
	  for (j=0;j<8;j++)
	    fprintf(trch_out_fd,"%d\n",(input_buffer[i]>>(7-j))&1);
	}
	fclose(trch_out_fd);
      }
      else {
	for (i=0;i<input_buffer_length;i++)
	  input_buffer[i] = taus()&0xff;
      }
    }
    else {
      n_frames=1;
      i=0;
      while (!feof(input_fd)) {
	fscanf(input_fd,"%s %s",input_val_str,input_val_str2);//&input_val1,&input_val2);
	
	if ((i%4)==0) {
	  ((short*)txdata[0])[i/2] = (short)((1<<15)*strtod(input_val_str,NULL));
	  ((short*)txdata[0])[(i/2)+1] = (short)((1<<15)*strtod(input_val_str2,NULL));
	  if ((i/4)<100)
	    printf("sample %d => %e + j%e (%d +j%d)\n",i/4,strtod(input_val_str,NULL),strtod(input_val_str2,NULL),((short*)txdata[0])[i/4],((short*)txdata[0])[(i/4)+1]);//1,input_val2,);
	}
	i++;
	if (i>(FRAME_LENGTH_SAMPLES))
	  break;
      }
      printf("Read in %d samples\n",i/4);
      write_output("txsig0.m","txs0", txdata[0],2*frame_parms->samples_per_tti,1,1);
      //    write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      tx_lev = signal_energy(&txdata[0][0],
			     OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
      tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
      
    }

    for (trials = 0;trials<n_frames;trials++) {
      //      printf("*");
      fflush(stdout);
      round=0;
      while (round < 4) {
	//	printf("Trial %d : Round %d ",trials,round);
	round_trials[round]++;
	if (round == 0) {
	  PHY_vars_eNB->ulsch_eNB[0]->harq_processes[harq_pid]->Ndi = 1;
	  PHY_vars_eNB->ulsch_eNB[0]->harq_processes[harq_pid]->rvidx = round>>1;
	  PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->Ndi = 1;
	  PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->rvidx = round>>1;
	}
	else {
	  PHY_vars_eNB->ulsch_eNB[0]->harq_processes[harq_pid]->Ndi = 0;
	  PHY_vars_eNB->ulsch_eNB[0]->harq_processes[harq_pid]->rvidx = round>>1;
	  PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->Ndi = 0;
	  PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->rvidx = round>>1;
	}

	if (input_fd == NULL) {
#ifdef OFDMA_ULSCH
	  if (srs_flag)
	    generate_srs_tx(PHY_vars_UE,0,AMP,subframe);
	  generate_drs_pusch(PHY_vars_UE,0,AMP,subframe,first_rb,nb_rb);
	  
#else
	  if (srs_flag)
	    generate_srs_tx(PHY_vars_UE,0,scfdma_amps[PHY_vars_UE->lte_frame_parms.N_RB_UL],subframe);
	  generate_drs_pusch(PHY_vars_UE,0,
			     scfdma_amps[PHY_vars_UE->lte_frame_parms.N_RB_UL],subframe,
			     PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->first_rb,
			     PHY_vars_UE->ulsch_ue[0]->harq_processes[harq_pid]->nb_rb);
#endif	
	  
	  if (ulsch_encoding(input_buffer,
			     &PHY_vars_UE->lte_frame_parms,
			     PHY_vars_UE->ulsch_ue[0],
			     harq_pid,
			     2, // transmission mode
			     control_only_flag,
			     1// Nbundled
			     )==-1) {
	    printf("ulsim.c Problem with ulsch_encoding\n");
	    exit(-1);
	  }
	  
#ifdef OFDMA_ULSCH
	  ulsch_modulation(PHY_vars_UE->lte_ue_common_vars.txdataF,AMP,
			   PHY_vars_UE->frame,subframe,&PHY_vars_UE->lte_frame_parms,PHY_vars_UE->ulsch_ue[0]);
#else  
	  //	  printf("Generating PUSCH in subframe %d with amp %d, nb_rb %d\n",subframe,scfdma_amps[nb_rb],nb_rb);
	  ulsch_modulation(PHY_vars_UE->lte_ue_common_vars.txdataF,scfdma_amps[PHY_vars_UE->lte_frame_parms.N_RB_UL],
			   PHY_vars_UE->frame,subframe,&PHY_vars_UE->lte_frame_parms,
			   PHY_vars_UE->ulsch_ue[0]);
#endif
	  
#ifdef IFFT_FPGA_UE
	  if (n_frames==1)
	    write_output("txsigF0.m","txsF0", &PHY_vars_UE->lte_ue_common_vars.txdataF[0][frame_parms->ofdm_symbol_size*nsymb*subframe],frame_parms->ofdm_symbol_size*nsymb,1,4);
	  
	  //write_output("txsigF1.m","txsF1", lte_ue_common_vars->txdataF[1],300*120,1,4);
	  
	  // do talbe lookup and write results to txdataF2
	  for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
	    l = 0;
	    for (i=0;i<nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	      if ((i%512>=1) && (i%512<=150))
		txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_UE->lte_ue_common_vars.txdataF[aa][l++]];
	      else if (i%512>=362)
		txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_UE->lte_ue_common_vars.txdataF[aa][l++]];
	      else 
		txdataF2[aa][i] = 0;
	    //printf("l=%d\n",l);
	  }
	  if (n_frames==1) {
	    write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	    //write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	  }
	  for (aa=0; aa<1; aa++)  {
	    if (frame_parms->Ncp == 1) 
	      PHY_ofdm_mod(txdataF2[aa],        // input
			   txdata[aa],         // output
			   PHY_vars_UE->lte_frame_parms.log2_symbol_size,                // log2_fft_size
			   nsymb,                 // number of symbols
			   PHY_vars_UE->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
			   PHY_vars_UE->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
			   PHY_vars_UE->lte_frame_parms.rev,           // bit-reversal permutation
			   CYCLIC_PREFIX);
	    else 
	      normal_prefix_mod(txdataF2[aa],txdata[aa],nsymb,frame_parms);
	    
	    
	  }
	  //  tx_lev += signal_energy(&txdata[aa][4*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
	  //		  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	  
#else
	  if (n_frames==1) {
	    write_output("txsigF0.m","txsF0", &PHY_vars_UE->lte_ue_common_vars.txdataF[0][512*nsymb*subframe],512*nsymb,1,1);
	    //write_output("txsigF1.m","txsF1", PHY_vars_UE->lte_ue_common_vars.txdataF[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	  }
	  tx_lev=0;
	  for (aa=0; aa<1; aa++) {
	    if (frame_parms->Ncp == 1) 
	      PHY_ofdm_mod(&PHY_vars_UE->lte_ue_common_vars.txdataF[aa][subframe*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX],        // input
			   &txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],         // output
			   PHY_vars_UE->lte_frame_parms.log2_symbol_size,                // log2_fft_size
			   nsymb,                 // number of symbols
			   PHY_vars_UE->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
			   PHY_vars_UE->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
			   PHY_vars_UE->lte_frame_parms.rev,           // bit-reversal permutation
			   CYCLIC_PREFIX);
	    else
	      normal_prefix_mod(&PHY_vars_UE->lte_ue_common_vars.txdataF[aa][subframe*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX],
				&txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],
				nsymb,
				frame_parms);
	    
#ifndef OFDMA_ULSCH
	    apply_7_5_kHz(PHY_vars_UE,subframe<<1);
	    apply_7_5_kHz(PHY_vars_UE,1+(subframe<<1));
#endif
	    
	    tx_lev += signal_energy(&txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],
				  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	
	  }
#endif
	}  // input_fd == NULL 

	tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
	//(double)tx_lev_dB - (SNR+sigma2_dB));
	sigma2_dB = N0;//10*log10((double)tx_lev)  +10*log10(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size/(PHY_vars_UE->lte_frame_parms.N_RB_DL*12)) - SNR;
	tx_gain = sqrt(pow(10.0,.1*(N0+SNR))*nb_rb/((double)tx_lev*PHY_vars_eNB->lte_frame_parms.N_RB_UL));
 
	//AWGN

	sigma2 = pow(10,sigma2_dB/10);

	if (n_frames==1) {
	  printf("tx_lev_dB = %d dB\n",tx_lev_dB);
	  write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
	  //write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
	}

	// fill measurement symbol (19) with noise
      
	for (i=0;i<OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	  for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	    
	    ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i] = (short) ((sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	    ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i+1] = (short) ((sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	  }
	}

	// multipath channel
      
	for (i=0;i<PHY_vars_eNB->lte_frame_parms.samples_per_tti;i++) {
	  for (aa=0;aa<1;aa++) {
	    s_re[aa][i] = ((double)(((short *)&txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe]))[(i<<1)]);
	    s_im[aa][i] = ((double)(((short *)&txdata[aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe]))[(i<<1)+1]);
	  }
	}
      
	multipath_channel(UE2eNB,s_re,s_im,r_re,r_im,
			  PHY_vars_eNB->lte_frame_parms.samples_per_tti,0);
	
	if (n_frames==1)
	  printf("Sigma2 %f (sigma2_dB %f), tx_gain %f (%f dB)\n",sigma2,sigma2_dB,tx_gain,20*log10(tx_gain));
	for (i=0; i<PHY_vars_eNB->lte_frame_parms.samples_per_tti; i++) {
	  for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	    ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe])[2*i] = (short) ((tx_gain*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    ((short*) &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][aa][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe])[2*i+1] = (short) ((tx_gain*r_im[aa][i]) + (iqim*tx_gain*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	  }
	}    
      
	if (n_frames==1) {
	  printf("rx_level Null symbol %f\n",10*log10((double)signal_energy((int*)&PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][(PHY_vars_eNB->lte_frame_parms.samples_per_tti<<1) -PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	  printf("rx_level data symbol %f\n",10*log10(signal_energy((int*)&PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][160+(PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	}

	SNRmeas = 10*log10(((double)signal_energy((int*)&PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][160+(PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2))/((double)signal_energy((int*)&PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][(PHY_vars_eNB->lte_frame_parms.samples_per_tti<<1) -PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)) - 1)+10*log10(PHY_vars_eNB->lte_frame_parms.N_RB_UL/nb_rb);
      
	if (n_frames==1) {
	  printf("SNRmeas %f\n",SNRmeas);
      
	  write_output("rxsig0.m","rxs0", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);
	  write_output("rxsig1.m","rxs1", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);
	}
#ifndef OFDMA_ULSCH
	remove_7_5_kHz(PHY_vars_eNB,subframe<<1);
	remove_7_5_kHz(PHY_vars_eNB,1+(subframe<<1));
	//	write_output("rxsig0_75.m","rxs0_75", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);
	//	write_output("rxsig1_75.m","rxs1_75", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][PHY_vars_eNB->lte_frame_parms.samples_per_tti*subframe],PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);

#endif      
	lte_eNB_I0_measurements(PHY_vars_eNB,
				0,
				1);

	for (l=subframe*PHY_vars_UE->lte_frame_parms.symbols_per_tti;l<((1+subframe)*PHY_vars_UE->lte_frame_parms.symbols_per_tti);l++) {
	
	  slot_fep_ul(&PHY_vars_eNB->lte_frame_parms,
		      &PHY_vars_eNB->lte_eNB_common_vars,
		      l%(PHY_vars_eNB->lte_frame_parms.symbols_per_tti/2),
		      l/(PHY_vars_eNB->lte_frame_parms.symbols_per_tti/2),
		      0,
		      0);
	}

	PHY_vars_eNB->ulsch_eNB[0]->cyclicShift = cyclic_shift;// cyclic shift for DMRS
	rx_ulsch(PHY_vars_eNB,
		 subframe,
		 0,  // this is the effective sector id
		 0,  // this is the UE_id
		 PHY_vars_eNB->ulsch_eNB,
		 cooperation_flag);
	
	
	ret= ulsch_decoding(PHY_vars_eNB,
			    0, // UE_id
			    subframe,
			    control_only_flag,
			    1  // Nbundled 
			    );
      
	if (ret <= MAX_TURBO_ITERATIONS) {
	  if (n_frames==1) {
	    printf("No ULSCH errors found, o_ACK[0]= %d\n",PHY_vars_eNB->ulsch_eNB[0]->o_ACK[0]);
	    dump_ulsch(PHY_vars_eNB);
	    exit(-1);
	  }
	  round=5;
	}	
	else {
	  errs[round]++;
	  if (n_frames==1) {
	    printf("ULSCH errors found o_ACK[0]= %d\n",PHY_vars_eNB->ulsch_eNB[0]->o_ACK[0]);
	    dump_ulsch(PHY_vars_eNB);
	    exit(-1);
	  }
	  //	    printf("round %d errors %d/%d\n",round,errs[round],trials);
	  round++;
	
	  if (n_frames==1) {
	    printf("ULSCH in error in round %d\n",round);
	  }
	}  // ulsch error
      } // round
          
      //      printf("\n");
      if ((errs[0]>=100) && (trials>(n_frames/2)))
	break;
      
    }   //trials
    printf("\n**********************SNR = %f dB : TX %f dB (gain %f dB), N0W %f dB, I0 %d dB [ (%d,%d) dB / (%d,%d) dB ]**************************\n",
	   SNR,
	   tx_lev_dB+20*log10(tx_gain),
	   20*log10(tx_gain),
	   (double)N0,
	   PHY_vars_eNB->PHY_measurements_eNB[0].n0_power_tot_dB,
	   dB_fixed(PHY_vars_eNB->lte_eNB_pusch_vars[0]->ulsch_power[0]),
	   dB_fixed(PHY_vars_eNB->lte_eNB_pusch_vars[0]->ulsch_power[1]),
	   PHY_vars_eNB->PHY_measurements_eNB->n0_power_dB[0],
	   PHY_vars_eNB->PHY_measurements_eNB->n0_power_dB[1]);
    
    printf("Errors (%d/%d %d/%d %d/%d %d/%d), Pe = (%e,%e,%e,%e) => effective rate %f (%f), normalized delay %f (%f)\n",
	   errs[0],
	   round_trials[0],
	   errs[1],
	   round_trials[1],
	   errs[2],
	   round_trials[2],
	   errs[3],
	   round_trials[3],
	   (double)errs[0]/(round_trials[0]),
	   (double)errs[1]/(round_trials[1]),
	   (double)errs[2]/(round_trials[2]),
	   (double)errs[3]/(round_trials[3]),
	   rate*((double)(round_trials[0])/((double)round_trials[0] + round_trials[1] + round_trials[2] + round_trials[3])),
	   rate,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0])/(double)PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[harq_pid]->TBS,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0]));
    
    fprintf(bler_fd,"%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d\n",
	    SNR,
	    mcs,
	    PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[harq_pid]->TBS,
	    rate,
	    errs[0],
	    round_trials[0],
	    errs[1],
	    round_trials[1],
	    errs[2],
	    round_trials[2],
	    errs[3],
	    round_trials[3]);
    
    if (((double)errs[0]/(round_trials[0]))<1e-2) 
      break;
  } // SNR
  
  fclose(bler_fd);
  

  
  
#ifdef IFFT_FPGA
  printf("Freeing transmit signals\n");
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
#endif
  
  printf("Freeing channel I/O\n");
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
  
  //  lte_sync_time_free();
  
  return(0);
}
   

