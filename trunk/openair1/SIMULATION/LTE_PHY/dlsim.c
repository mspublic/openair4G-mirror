#include <string.h>
#include <math.h>
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>

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
#include "UTIL/LOG/log.h"

#ifdef XFORMS
#include "forms.h"
#include "../../USERSPACE_TOOLS/SCOPE/lte_scope.h"
#endif

//#define AWGN
//#define NO_DCI

#define BW 7.68

//#define ABSTRACTION
//#define PERFECT_CE

/*
  #define RBmask0 0x00fc00fc
  #define RBmask1 0x0
  #define RBmask2 0x0
  #define RBmask3 0x0
*/

PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}



#ifdef XFORMS
void do_forms(FD_lte_scope *form, LTE_DL_FRAME_PARMS *frame_parms, short **channel, short **channel_f, short **rx_sig, short **rx_sig_f, short *dlsch_comp, short* dlsch_comp_i, short* dlsch_rho, short *dlsch_llr, int coded_bits_per_codeword)
{

  int i,j,ind,k,s;

  float Re,Im;
  float mag_sig[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig_time[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig2[FRAME_LENGTH_COMPLEX_SAMPLES],
    time2[FRAME_LENGTH_COMPLEX_SAMPLES],
    I[25*12*11*4], Q[25*12*11*4],
    *llr,*llr_time;

  float avg, cum_avg;
  
  llr = malloc(coded_bits_per_codeword*sizeof(float));
  llr_time = malloc(coded_bits_per_codeword*sizeof(float));

  // Channel frequency response
  cum_avg = 0;
  ind = 0;
  for (j=0; j<4; j++) { 
    for (i=0;i<frame_parms->nb_antennas_rx;i++) {
      for (k=0;k<NUMBER_OF_OFDM_CARRIERS*7;k++){
	sig_time[ind] = (float)ind;
	Re = (float)(channel_f[(j<<1)+i][2*k]);
	Im = (float)(channel_f[(j<<1)+i][2*k+1]);
	//mag_sig[ind] = (short) rand(); 
	mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	cum_avg += (short)sqrt((double)Re*Re + (double)Im*Im) ;
	ind++;
      }
      //      ind+=NUMBER_OF_OFDM_CARRIERS/4; // spacing for visualization
    }
  }

  avg = cum_avg/NUMBER_OF_USEFUL_CARRIERS;

  //fl_set_xyplot_ybounds(form->channel_f,30,70);
  fl_set_xyplot_data(form->channel_f,sig_time,mag_sig,ind,"","","");

  /*
  // channel time resonse
  cum_avg = 0;
  ind = 0;
  for (k=0;k<1;k++){
    for (j=0;j<1;j++) {
      
      for (i=0;i<frame_parms->ofdm_symbol_size;i++){
	sig_time[ind] = (float)ind;
	Re = (float)(channel[k+2*j][2*i]);
	Im = (float)(channel[k+2*j][2*i+1]);
	//mag_sig[ind] = (short) rand(); 
	mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	cum_avg += (short)sqrt((double)Re*Re + (double)Im*Im) ;
	ind++;
      }
    }
  }
  
  //fl_set_xyplot_ybounds(form->channel_t_im,10,90);
  fl_set_xyplot_data(form->channel_t_im,sig_time,mag_sig,ind,"","","");
  */

  // channel_t_re = rx_sig_f[0]
  //for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX; i++)  {
  for (i=0; i<NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti/2; i++)  {
    sig2[i] = 10*log10(1.0+(double) ((rx_sig_f[0][4*i])*(rx_sig_f[0][4*i])+(rx_sig_f[0][4*i+1])*(rx_sig_f[0][4*i+1])));
    time2[i] = (float) i;
  } 

  //fl_set_xyplot_ybounds(form->channel_t_re,10,90);
  fl_set_xyplot_data(form->channel_t_re,time2,sig2,NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti,"","","");
  //fl_set_xyplot_data(form->channel_t_re,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,"","","");
  

  // channel_t_im = rx_sig[0]
  //if (frame_parms->nb_antennas_rx>1) {
    for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)  {
      //for (i=0; i<NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti/2; i++)  {
      sig2[i] = 10*log10(1.0+(double) ((rx_sig[0][2*i])*(rx_sig[0][2*i])+(rx_sig[0][2*i+1])*(rx_sig[0][2*i+1])));
      time2[i] = (float) i;
    }

    //fl_set_xyplot_ybounds(form->channel_t_im,0,100);
    //fl_set_xyplot_data(form->channel_t_im,&time2[640*12*6],&sig2[640*12*6],640*12,"","","");
    fl_set_xyplot_data(form->channel_t_im,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");
    //}

  /*
  // PBCH LLR
  j=0;
  for(i=0;i<1920;i++) {
    llr[j] = (float) pbch_llr[i];
    llr_time[j] = (float) j;
    //if (i==63)
    //  i=127;
    //else if (i==191)
    //  i=319;
    j++;
  }

  fl_set_xyplot_data(form->decoder_input,llr_time,llr,1920,"","","");
  //fl_set_xyplot_ybounds(form->decoder_input,-100,100);

  // PBCH I/Q
  j=0;
  for(i=0;i<12*12;i++) {
    I[j] = pbch_comp[2*i];
    Q[j] = pbch_comp[2*i+1];
    j++;
    //if (i==47)
    //  i=96;
    //else if (i==191)
    //  i=239;
  }

  fl_set_xyplot_data(form->scatter_plot,I,Q,12*12,"","","");
  //fl_set_xyplot_xbounds(form->scatter_plot,-100,100);
  //fl_set_xyplot_ybounds(form->scatter_plot,-100,100);

  // PDCCH I/Q
  j=0;
  for(i=0;i<12*25*3;i++) {
    I[j] = pdcch_comp[2*i];
    Q[j] = pdcch_comp[2*i+1];
    j++;
    //if (i==47)
    //  i=96;
    //else if (i==191)
    //  i=239;
  }

  fl_set_xyplot_data(form->scatter_plot1,I,Q,12*25*3,"","","");
  //fl_set_xyplot_xbounds(form->scatter_plot,-100,100);
  //fl_set_xyplot_ybounds(form->scatter_plot,-100,100);
  */

  // DLSCH LLR
  for(i=0;i<coded_bits_per_codeword;i++) {
    llr[i] = (float) dlsch_llr[i];
    llr_time[i] = (float) i;
  }

  fl_set_xyplot_data(form->demod_out,llr_time,llr,coded_bits_per_codeword,"","","");
  //fl_set_xyplot_ybounds(form->demod_out,-256,256);

  // DLSCH I/Q
  j=0;
  for (s=0;s<frame_parms->symbols_per_tti;s++) {
    for(i=0;i<12*25;i++) {
      I[j] = dlsch_comp[(2*25*12*s)+2*i];
      Q[j] = dlsch_comp[(2*25*12*s)+2*i+1];
      j++;
    }
    //if (s==2)
    //  s=3;
    //else if (s==5)
    //  s=6;
    //else if (s==8)
    //  s=9;
  }

  fl_set_xyplot_data(form->scatter_plot,I,Q,j,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot,-10000,10000);
  fl_set_xyplot_ybounds(form->scatter_plot,-10000,10000);

  // DLSCH I/Q
  j=0;
  for (s=0;s<frame_parms->symbols_per_tti;s++) {
    for(i=0;i<12*25;i++) {
      I[j] = dlsch_comp_i[(2*25*12*s)+2*i];
      Q[j] = dlsch_comp_i[(2*25*12*s)+2*i+1];
      j++;
    }
    //if (s==2)
    //  s=3;
    //else if (s==5)
    //  s=6;
    //else if (s==8)
    //  s=9;
  }

  fl_set_xyplot_data(form->scatter_plot1,I,Q,j,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot1,-10000,10000);
  fl_set_xyplot_ybounds(form->scatter_plot1,-10000,10000);

  // DLSCH I/Q
  j=0;
  for (s=0;s<frame_parms->symbols_per_tti;s++) {
    for(i=0;i<12*25;i++) {
      I[j] = dlsch_rho[(2*25*12*s)+2*i];
      Q[j] = dlsch_rho[(2*25*12*s)+2*i+1];
      j++;
    }
    //if (s==2)
    //  s=3;
    //else if (s==5)
    //  s=6;
    //else if (s==8)
    //  s=9;
  }

  fl_set_xyplot_data(form->scatter_plot2,I,Q,j,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot2,-10000,10000);
  fl_set_xyplot_ybounds(form->scatter_plot2,-10000,10000);


  free(llr);
  free(llr_time);

}
#endif

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,u8 extended_prefix_flag,u8 fdd_flag, u16 Nid_cell,u8 tdd_config,u8 N_RB_DL,u8 osf) {

  LTE_DL_FRAME_PARMS *lte_frame_parms;
  int i;


  printf("Start lte_param_init\n");
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  srand(1);
  randominit(1);
  set_taus_seed(1);
  
  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = N_RB_DL;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = N_RB_DL;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = Nid_cell;
  lte_frame_parms->nushift            = Nid_cell%6;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->nb_antennas_tx_eNB = N_tx;
  lte_frame_parms->phich_config_common.phich_resource         = oneSixth;
  lte_frame_parms->tdd_config         = tdd_config;
  lte_frame_parms->frame_type         = (fdd_flag==1)?0 : 1;
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;44
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms,osf);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  PHY_vars_UE->is_secondary_ue = 0;
  PHY_vars_UE->lte_frame_parms = *lte_frame_parms;
  PHY_vars_eNB->lte_frame_parms = *lte_frame_parms;

  phy_init_lte_top(lte_frame_parms);
  dump_frame_parms(lte_frame_parms);

  PHY_vars_UE->PHY_measurements.n_adj_cells=2;
  PHY_vars_UE->PHY_measurements.adj_cell_id[0] = Nid_cell+1;
  PHY_vars_UE->PHY_measurements.adj_cell_id[1] = Nid_cell+2;

  for (i=0;i<3;i++)
    lte_gold(lte_frame_parms,PHY_vars_UE->lte_gold_table[i],Nid_cell+i);    

  phy_init_lte_ue(PHY_vars_UE,1,0);
  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);

  
  // DL power control init
  PHY_vars_eNB->pdsch_config_dedicated->p_a  = 4; // 4 = 0dB
  ((PHY_vars_eNB->lte_frame_parms).pdsch_config_common).p_b = (lte_frame_parms->nb_antennas_tx_eNB>1) ? 1 : 0; // rho_a = rhob

  PHY_vars_UE->pdsch_config_dedicated->p_a  = 4; // 4 = 0dB
  ((PHY_vars_UE->lte_frame_parms).pdsch_config_common).p_b = (lte_frame_parms->nb_antennas_tx_eNB>1) ? 1 : 0; // rho_a = rhob

  printf("Done lte_param_init\n");


}


//DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2_2A[2];

DCI1E_5MHz_2A_M10PRB_TDD_t  DLSCH_alloc_pdu2_1E[2];


#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(PHY_vars_eNB->lte_frame_parms.N_RB_UL,0,2)
//#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13
//#define DLSCH_RB_ALLOC 0x0001
void do_OFDM_mod(mod_sym_t **txdataF, s32 **txdata, u16 next_slot, LTE_DL_FRAME_PARMS *frame_parms) {

  int aa, slot_offset, slot_offset_F;

#ifdef IFFT_FPGA
  s32 **txdataF2;
  int i, l;

  txdataF2    = (s32 **)malloc(2*sizeof(s32*));
  txdataF2[0] = (s32 *)malloc(NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  txdataF2[1] = (s32 *)malloc(NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  
  bzero(txdataF2[0],NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  bzero(txdataF2[1],NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7)*sizeof(s32));
  
  slot_offset_F = (next_slot)*(frame_parms->N_RB_DL*12)*((frame_parms->Ncp==1) ? 6 : 7);
  slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
  
  //write_output("eNB_txsigF0.m","eNB_txsF0", lte_eNB_common_vars->txdataF[eNB_id][0],300*120,1,4);
  //write_output("eNB_txsigF1.m","eNB_txsF1", lte_eNB_common_vars->txdataF[eNB_id][1],300*120,1,4);
  
  
  // do talbe lookup and write results to txdataF2
  for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
    
    l = slot_offset_F;	
    for (i=0;i<NUMBER_OF_OFDM_CARRIERS*((frame_parms->Ncp==1) ? 6 : 7);i++) 
      if ((i%512>=1) && (i%512<=150))
	txdataF2[aa][i] = ((s32*)mod_table)[txdataF[aa][l++]];
      else if (i%512>=362)
	txdataF2[aa][i] = ((s32*)mod_table)[txdataF[aa][l++]];
      else 
	txdataF2[aa][i] = 0;
    
  }
  
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1)
      PHY_ofdm_mod(txdataF2[aa],        // input
		   &txdata[aa][slot_offset],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   6,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(txdataF2[aa],&txdata[aa][slot_offset],7,frame_parms);
    }
  }
  
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  
#else //IFFT_FPGA
  
  slot_offset_F = (next_slot)*(frame_parms->ofdm_symbol_size)*((frame_parms->Ncp==1) ? 6 : 7);
  slot_offset = (next_slot)*(frame_parms->samples_per_tti>>1);
  
  for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
    if (frame_parms->Ncp == 1)
      PHY_ofdm_mod(&txdataF[aa][slot_offset_F],        // input
		   &txdata[aa][slot_offset],         // output
		   frame_parms->log2_symbol_size,                // log2_fft_size
		   6,                 // number of symbols
		   frame_parms->nb_prefix_samples,               // number of prefix samples
		   frame_parms->twiddle_ifft,  // IFFT twiddle factors
		   frame_parms->rev,           // bit-reversal permutation
		   CYCLIC_PREFIX);
    else {
      normal_prefix_mod(&txdataF[aa][slot_offset_F],
			&txdata[aa][slot_offset],
			7,
			frame_parms);
    }
  }  
#endif //IFFT_FPGA
  
}

int main(int argc, char **argv) {

  char c;
  int k,i,aa,aarx,aatx;

  int s,Kr,Kr_bytes;

  double sigma2, sigma2_dB=10,SNR,snr0=-2.0,snr1,rate,saving_bler=0;
  double snr_step=1,input_snr_step=1, snr_int=30;

  LTE_DL_FRAME_PARMS *frame_parms;
  double **s_re,**s_im,**r_re,**r_im;
  double forgetting_factor=0.0; //in [0,1] 0 means a new channel every time, 1 means keep the same channel
  double iqim=0.0;

  u8 extended_prefix_flag=0,transmission_mode=1,n_tx=1,n_rx=1;
  u16 Nid_cell=0;

  int eNB_id = 0, eNB_id_i = 1;
  unsigned char mcs,dual_stream_UE = 0,awgn_flag=0,round,dci_flag=0;
  unsigned char i_mod = 2;
  unsigned short NB_RB;
  unsigned char Ns,l,m;
  u16 tdd_config=3;
  u16 n_rnti=0x1234;
  int n_users = 1;

  SCM_t channel_model=Rayleigh1;
  //  unsigned char *input_data,*decoded_output;

  unsigned char *input_buffer[2];
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb,dci_cnt,tbs;
 
  unsigned int tx_lev,tx_lev_dB,trials,errs[4]={0,0,0,0},round_trials[4]={0,0,0,0},dci_errors=0,dlsch_active=0,num_layers;
  int re_allocated;
  FILE *bler_fd;
  char bler_fname[256];
  FILE *tikz_fd;
  char tikz_fname[256];

  FILE *input_trch_fd;
  unsigned char input_trch_file=0;
  FILE *input_fd=NULL;
  unsigned char input_file=0;
  char input_val_str[50],input_val_str2[50];

  char input_trch_val[16];
  double channelx,channely;

  //  unsigned char pbch_pdu[6];

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];
  int num_common_dci=0,num_ue_spec_dci=0,num_dci=0;

  //  FILE *rx_frame_file;

  int n_frames;
  int n_ch_rlz = 1;
  channel_desc_t *eNB2UE[4];
  u8 num_pdcch_symbols=3,num_pdcch_symbols_2=0;
  u8 pilot1,pilot2,pilot3;
  u8 rx_sample_offset = 0;
  //char stats_buffer[4096];
  //int len;
  u8 num_rounds = 4,fix_rounds=0;
  u8 subframe=6;
  int u;
  int n=0;
  int abstx=0;
  int iii;
  FILE *csv_fd;
  char csv_fname[512];
  int ch_realization;
  int pmi_feedback=0;
  int hold_channel=0;
  int hold_channel1=0; 
  // void *data;
  // int ii;
  // int bler;
  double blerr[4],uncoded_ber,avg_ber;
  short *uncoded_ber_bit;
  u8 N_RB_DL=25,osf=1;
  u8 fdd_flag = 0;
#ifdef XFORMS
  FD_lte_scope *form;
  char title[255];
#endif
  u32 DLSCH_RB_ALLOC = 0x1fff;
  
  signal(SIGSEGV, handler); 

  logInit();

  // default parameters
  mcs = 0;
  n_frames = 1000;
  snr0 = 0;
  num_layers = 1;

  while ((c = getopt (argc, argv, "hadpDm:n:o:s:f:t:c:g:r:F:x:y:z:M:N:I:i:R:S:C:T:b:u:w:")) != -1) {
    switch (c)
      {
      case 'a':
	awgn_flag = 1;
	break;
      case 'b':
	tdd_config=atoi(optarg);
	break;
      case 'd':
	dci_flag = 1;
	break;
      case 'm':
	mcs = atoi(optarg);
	break;
      case 'n':
	n_frames = atoi(optarg);
	break;
      case 'C':
	Nid_cell = atoi(optarg);
	break;
      case 'o':
	rx_sample_offset = atoi(optarg);
	break;
      case 'D':
	fdd_flag = 1;
	break;
      case 'r':
	DLSCH_RB_ALLOC = atoi(optarg);
	break;
      case 'F':
	forgetting_factor = atof(optarg);
	break;
      case 's':
	snr0 = atoi(optarg);
	break;
      case 'w':
	snr_int = atoi(optarg);
	break;
      case 't':
	//Td= atof(optarg);
	printf("Please use the -G option to select the channel model\n");
	exit(-1);
	break;
      case 'f':
	input_snr_step= atof(optarg);
	break;
      case 'M':
	abstx= atof(optarg);
	break;
      case 'N':
	n_ch_rlz= atof(optarg);
	break;
      case 'p':
	extended_prefix_flag=1;
	break;
      case 'c':
	num_pdcch_symbols=atoi(optarg);
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
	}
	break;
      case 'x':
	transmission_mode=atoi(optarg);
	if ((transmission_mode!=1) &&
	    (transmission_mode!=2) &&
	    (transmission_mode!=5) &&
	    (transmission_mode!=6)) {
	  msg("Unsupported transmission mode %d\n",transmission_mode);
	  exit(-1);
	}
	break;
      case 'y':
	n_tx=atoi(optarg);
	if ((n_tx==0) || (n_tx>2)) {
	  msg("Unsupported number of tx antennas %d\n",n_tx);
	  exit(-1);
	}
	break;
      case 'z':
	n_rx=atoi(optarg);
	if ((n_rx==0) || (n_rx>2)) {
	  msg("Unsupported number of rx antennas %d\n",n_rx);
	  exit(-1);
	}
	break;
      case 'I':
	input_trch_fd = fopen(optarg,"r");
	input_trch_file=1;
	break;
      case 'i':
	input_fd = fopen(optarg,"r");
	input_file=1;
	dci_flag = 1;	
	break;
      case 'R':
	num_rounds=atoi(optarg);
	fix_rounds=1;
	break;
      case 'S':
	subframe=atoi(optarg);
	break;
      case 'T':
	n_rnti=atoi(optarg);
	break;	
      case 'u':
	dual_stream_UE=atoi(optarg);
	if ((n_tx!=2) || (transmission_mode!=5)) {
	  msg("Unsupported nb of decoded users: %d user(s), %d user(s) to decode\n", n_tx, dual_stream_UE);
	  exit(-1);
	}
	break;
      case 'h':
      default:
	printf("%s -h(elp) -a(wgn on) -d(ci decoding on) -p(extended prefix on) -m mcs -n n_frames -s snr0 -t Delayspread -x transmission mode (1,2,5,6) -y TXant -z RXant -I trch_file\n",argv[0]);
      printf("-h This message\n");
      printf("-a Use AWGN channel and not multipath\n");
      printf("-c Number of PDCCH symbols\n");
      printf("-m MCS\n");
      printf("-d Transmit the DCI and compute its error statistics and the overall throughput\n");
      printf("-p Use extended prefix mode\n");
      printf("-n Number of frames to simulate\n");
      printf("-o Sample offset for receiver\n");
      printf("-s Starting SNR, runs from SNR to SNR+%.1fdB in steps of %.1fdB. If n_frames is 1 then just SNR is simulated and MATLAB/OCTAVE output is generated\n", snr_int, snr_step);
      printf("-f step size of SNR, default value is 1.\n");
      printf("-r ressource block allocation (see  section 7.1.6.3 in 36.213\n");
      printf("-g [A:M] Use 3GPP 25.814 SCM-A/B/C/D('A','B','C','D') or 36-101 EPA('E'), EVA ('F'),ETU('G') models (ignores delay spread and Ricean factor), Rayghleigh8 ('H'), Rayleigh1('I'), Rayleigh1_corr('J'), Rayleigh1_anticorr ('K'), Rice8('L'), Rice1('M')\n");
      printf("-F forgetting factor (0 new channel every trial, 1 channel constant\n");
      printf("-x Transmission mode (1,2,6 for the moment)\n");
      printf("-y Number of TX antennas used in eNB\n");
      printf("-z Number of RX antennas used in UE\n");
      printf("-R Number of HARQ rounds (fixed)\n");
      printf("-M Determines whether the Absraction flag is on or Off. 1-->On and 0-->Off. Default status is Off. \n");
      printf("-N Determines the number of Channel Realizations in Absraction mode. Default value is 1. \n");
      printf("-I Input filename for TrCH data (binary)\n");
      printf("-u Determines if the 2 streams at the UE are decoded or not. 0-->U2 is interference only and 1-->U2 is detected\n");
      exit(1);
      break;
      }
  }

  NB_RB=conv_nprb(0,DLSCH_RB_ALLOC);
#ifdef XFORMS
  fl_initialize (&argc, argv, NULL, 0, 0);
  form = create_form_lte_scope();
  sprintf (title, "LTE DLSIM SCOPE");
  fl_show_form (form->lte_scope, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
#endif

  if (transmission_mode==5) {
    n_users = 2;
    printf("dual_stream_UE=%d\n", dual_stream_UE);
  }

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag,fdd_flag,Nid_cell,tdd_config,N_RB_DL,osf);  

  eNB_id_i = PHY_vars_UE->n_connected_eNB;
  
  printf("Setting mcs = %d\n",mcs);
  printf("NPRB = %d\n",NB_RB);
  printf("n_frames = %d\n",n_frames);
  printf("Transmission mode %d with %dx%d antenna configuration, Extended Prefix %d\n",transmission_mode,n_tx,n_rx,extended_prefix_flag);

  snr1 = snr0+snr_int;
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

  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  //  r_re0 = malloc(2*sizeof(double*));
  //  r_im0 = malloc(2*sizeof(double*));

  nsymb = (PHY_vars_eNB->lte_frame_parms.Ncp == 0) ? 14 : 12;

  printf("Channel Model=%d\n",channel_model);
  printf("SCM-A=%d, SCM-B=%d, SCM-C=%d, SCM-D=%d, EPA=%d, EVA=%d, ETU=%d, Rayleigh8=%d, Rayleigh1=%d, Rayleigh1_corr=%d, Rayleigh1_anticorr=%d, Rice1=%d, Rice8=%d\n",
	 SCM_A, SCM_B, SCM_C, SCM_D, EPA, EVA, ETU, Rayleigh8, Rayleigh1, Rayleigh1_corr, Rayleigh1_anticorr, Rice1, Rice8);
  
  if(awgn_flag==0)
    sprintf(bler_fname,"second_bler_tx%d_mcs%d_chan%d.csv",transmission_mode,mcs,channel_model);
  else 
    if(transmission_mode==5)
      sprintf(bler_fname,"awgn_bler_tx%d_mcs%d_u%d.csv",transmission_mode,mcs,dual_stream_UE);
    else
       sprintf(bler_fname,"awgn_bler_tx%d_mcs%d.csv",transmission_mode,mcs);
  
  bler_fd = fopen(bler_fname,"w");
  fprintf(bler_fd,"SNR; MCS; TBS; rate; err0; trials0; err1; trials1; err2; trials2; err3; trials3; dci_err\n");
  
  
   if(abstx){
   // CSV file 
     sprintf(csv_fname,"dataout_tx%d_u2%d_mcs%d_chan%d_nsimus%d_R%d.m",transmission_mode,dual_stream_UE,mcs,channel_model,n_frames,num_rounds);
   csv_fd = fopen(csv_fname,"w");
   fprintf(csv_fd,"data_all%d=[",mcs);
    }
 
 //sprintf(tikz_fname, "second_bler_tx%d_u2=%d_mcs%d_chan%d_nsimus%d.tex",transmission_mode,dual_stream_UE,mcs,channel_model,n_frames);
 sprintf(tikz_fname, "second_bler_tx%d_u2%d_mcs%d_chan%d_nsimus%d",transmission_mode,dual_stream_UE,mcs,channel_model,n_frames);
 tikz_fd = fopen(tikz_fname,"w");
 //fprintf(tikz_fd,"\\addplot[color=red, mark=o] plot coordinates {");
 switch (mcs)
   {
   case 0:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=star] plot coordinates {");
     break;
   case 1:
     fprintf(tikz_fd,"\\addplot[color=red, mark=star] plot coordinates {");
     break;
   case 2:
     fprintf(tikz_fd,"\\addplot[color=green, mark=star] plot coordinates {");
     break;
   case 3:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=star] plot coordinates {");
     break;
   case 4:
     fprintf(tikz_fd,"\\addplot[color=black, mark=star] plot coordinates {");
     break;
   case 5:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=o] plot coordinates {");
     break;
   case 6:
     fprintf(tikz_fd,"\\addplot[color=red, mark=o] plot coordinates {");
     break;
   case 7:
     fprintf(tikz_fd,"\\addplot[color=green, mark=o] plot coordinates {");
     break;
   case 8:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=o] plot coordinates {");
     break;
   case 9:
     fprintf(tikz_fd,"\\addplot[color=black, mark=o] plot coordinates {");
     break;
   case 10:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=square] plot coordinates {");
     break;
   case 11:
     fprintf(tikz_fd,"\\addplot[color=red, mark=square] plot coordinates {");
     break;
   case 12:
     fprintf(tikz_fd,"\\addplot[color=green, mark=square] plot coordinates {");
     break;
   case 13:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=square] plot coordinates {");
     break;
   case 14:
     fprintf(tikz_fd,"\\addplot[color=black, mark=square] plot coordinates {");
     break;
   case 15:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=diamond] plot coordinates {");
     break;
   case 16:
     fprintf(tikz_fd,"\\addplot[color=red, mark=diamond] plot coordinates {");
     break;
   case 17:
     fprintf(tikz_fd,"\\addplot[color=green, mark=diamond] plot coordinates {");
     break;
   case 18:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=diamond] plot coordinates {");
     break;
   case 19:
     fprintf(tikz_fd,"\\addplot[color=black, mark=diamond] plot coordinates {");
     break;
   case 20:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=x] plot coordinates {");
     break;
   case 21:
     fprintf(tikz_fd,"\\addplot[color=red, mark=x] plot coordinates {");
     break;
   case 22:
     fprintf(tikz_fd,"\\addplot[color=green, mark=x] plot coordinates {");
     break;
   case 23:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=x] plot coordinates {");
     break;
   case 24:
     fprintf(tikz_fd,"\\addplot[color=black, mark=x] plot coordinates {");
     break;
   case 25:
     fprintf(tikz_fd,"\\addplot[color=blue, mark=x] plot coordinates {");
     break;
   case 26:
     fprintf(tikz_fd,"\\addplot[color=red, mark=+] plot coordinates {");
     break;
   case 27:
     fprintf(tikz_fd,"\\addplot[color=green, mark=+] plot coordinates {");
     break;
   case 28:
     fprintf(tikz_fd,"\\addplot[color=yellow, mark=+] plot coordinates {");
     break;
   }
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


  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;

  // Fill in UL_alloc
  UL_alloc_pdu.type    = 0;
  UL_alloc_pdu.hopping = 0;
  UL_alloc_pdu.rballoc = UL_RB_ALLOC;
  UL_alloc_pdu.mcs     = 1;
  UL_alloc_pdu.ndi     = 1;
  UL_alloc_pdu.TPC     = 0;
  UL_alloc_pdu.cqi_req = 1;

  CCCH_alloc_pdu.type               = 0;
  CCCH_alloc_pdu.vrb_type           = 0;
  CCCH_alloc_pdu.rballoc            = CCCH_RB_ALLOC;
  CCCH_alloc_pdu.ndi      = 1;
  CCCH_alloc_pdu.mcs      = 1;
  CCCH_alloc_pdu.harq_pid = 0;

  DLSCH_alloc_pdu2_1E[0].rah              = 0;
  DLSCH_alloc_pdu2_1E[0].rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2_1E[0].TPC              = 0;
  DLSCH_alloc_pdu2_1E[0].dai              = 0;
  DLSCH_alloc_pdu2_1E[0].harq_pid         = 0;
  //DLSCH_alloc_pdu2_1E[0].tb_swap          = 0;
  DLSCH_alloc_pdu2_1E[0].mcs             = mcs;  
  DLSCH_alloc_pdu2_1E[0].ndi             = 1;
  DLSCH_alloc_pdu2_1E[0].rv              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2_1E[0].tpmi             = (transmission_mode>=5 ? 5 : 0);  // precoding
  DLSCH_alloc_pdu2_1E[0].dl_power_off     = (transmission_mode==5 ? 0 : 1);

  DLSCH_alloc_pdu2_1E[1].rah              = 0;
  DLSCH_alloc_pdu2_1E[1].rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2_1E[1].TPC              = 0;
  DLSCH_alloc_pdu2_1E[1].dai              = 0;
  DLSCH_alloc_pdu2_1E[1].harq_pid         = 0;
  //DLSCH_alloc_pdu2_1E[1].tb_swap          = 0;
  DLSCH_alloc_pdu2_1E[1].mcs             = mcs;  
  DLSCH_alloc_pdu2_1E[1].ndi             = 1;
  DLSCH_alloc_pdu2_1E[1].rv              = 0;
  // Forget second codeword
  DLSCH_alloc_pdu2_1E[1].tpmi             = (transmission_mode>=5 ? 5 : 0) ;  // precoding
  DLSCH_alloc_pdu2_1E[1].dl_power_off     = (transmission_mode==5 ? 0 : 1);

  eNB2UE[0] = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				forgetting_factor,
				rx_sample_offset,
				0);
 if(abstx==1){
    for(n=1;n<num_rounds;n++)
  eNB2UE[n] = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				forgetting_factor,
				rx_sample_offset,
				0);
   }
  if (eNB2UE[0]==NULL) {
    msg("Problem generating channel model. Exiting.\n");
    exit(-1);
  }

  for (k=0;k<n_users;k++) {
    // Create transport channel structures for 2 transport blocks (MIMO)
    for (i=0;i<2;i++) {
      PHY_vars_eNB->dlsch_eNB[k][i] = new_eNB_dlsch(1,8,0);
      
      if (!PHY_vars_eNB->dlsch_eNB[k][i]) {
	printf("Can't get eNB dlsch structures\n");
	exit(-1);
      }
      
      PHY_vars_eNB->dlsch_eNB[k][i]->rnti = n_rnti+k;
    }
  }

  for (i=0;i<2;i++) {
    PHY_vars_UE->dlsch_ue[0][i]  = new_ue_dlsch(1,8,0);
    if (!PHY_vars_UE->dlsch_ue[0][i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }    
    PHY_vars_UE->dlsch_ue[0][i]->rnti   = n_rnti;
  }
  
  if (DLSCH_alloc_pdu2_1E[0].tpmi == 5) {

    PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single = (unsigned short)(taus()&0xffff);
    if (n_users>1)
      PHY_vars_eNB->eNB_UE_stats[1].DL_pmi_single = (PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single ^ 0x1555); //opposite PMI 
  }
  else {
    PHY_vars_eNB->eNB_UE_stats[0].DL_pmi_single = 0;
    if (n_users>1)
      PHY_vars_eNB->eNB_UE_stats[1].DL_pmi_single = 0;
  }


  if (input_fd==NULL) {

    for(k=0;k<n_users;k++) {
      printf("Generating dlsch params for user %d\n",k);
      generate_eNB_dlsch_params_from_dci(0,
					 &DLSCH_alloc_pdu2_1E[k],
					 n_rnti+k,
					 format1E_2A_M10PRB,
					 PHY_vars_eNB->dlsch_eNB[k],
					 &PHY_vars_eNB->lte_frame_parms,
					 PHY_vars_eNB->pdsch_config_dedicated,
					 SI_RNTI,
					 0,
					 P_RNTI,
					 PHY_vars_eNB->eNB_UE_stats[k].DL_pmi_single);
    }

    num_dci = 0;
    num_ue_spec_dci = 0;
    num_common_dci = 0;

    /*
    // common DCI 
    memcpy(&dci_alloc[num_dci].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_alloc[num_dci].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    dci_alloc[num_dci].L          = 2;
    dci_alloc[num_dci].rnti       = SI_RNTI;
    num_dci++;
    num_common_dci++;
    */

    // UE specific DCI
    for(k=0;k<n_users;k++) {
      memcpy(&dci_alloc[num_dci].dci_pdu[0],&DLSCH_alloc_pdu2_1E[k],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
      dci_alloc[num_dci].dci_length = sizeof_DCI1E_5MHz_2A_M10PRB_TDD_t;
      dci_alloc[num_dci].L          = 2;
      dci_alloc[num_dci].rnti       = n_rnti+k;
      dci_alloc[num_dci].format     = format1E_2A_M10PRB;
      dci_alloc[num_dci].nCCE       = 4*k;
      dump_dci(&PHY_vars_eNB->lte_frame_parms,&dci_alloc[num_dci]);

      num_dci++;
      num_ue_spec_dci++;

      /*
	memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
	dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
	dci_alloc[1].L          = 2;
	dci_alloc[1].rnti       = n_rnti;
      */
    }

    for (k=0;k<n_users;k++) {

      input_buffer_length = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->TBS/8;
      input_buffer[k] = (unsigned char *)malloc(input_buffer_length+4);
      memset(input_buffer[k],0,input_buffer_length+4);
    
      if (input_trch_file==0) {
	for (i=0;i<input_buffer_length;i++) {
	  input_buffer[k][i]= (unsigned char)(taus()&0xff);
	}
      }
      
      else {
	i=0;
	while ((!feof(input_trch_fd)) && (i<input_buffer_length<<3)) {
	  fscanf(input_trch_fd,"%s",input_trch_val);
	  if (input_trch_val[0] == '1')
	    input_buffer[k][i>>3]+=(1<<(7-(i&7)));
	  if (i<16)
	    printf("input_trch_val %d : %c\n",i,input_trch_val[0]);
	  i++;
	  if (((i%8) == 0) && (i<17))
	    printf("%x\n",input_buffer[k][(i-1)>>3]);
	}
	printf("Read in %d bits\n",i);
      }
    }
  }
  snr_step = input_snr_step;
  for (ch_realization=0;ch_realization<n_ch_rlz;ch_realization++){
    if(abstx){
      printf("**********************Channel Realization Index = %d **************************\n", ch_realization);
       saving_bler=0;
       hold_channel1=0;
    }
    
    for (SNR=snr0;SNR<snr1;SNR+=snr_step) {
      errs[0]=0;
      errs[1]=0;
      errs[2]=0;
      errs[3]=0;
      round_trials[0] = 0;
      round_trials[1] = 0;
      round_trials[2] = 0;
      round_trials[3] = 0;

      dci_errors=0;
      avg_ber = 0;

      round=0;

      
      for (trials = 0;trials<n_frames;trials++) {
	//  printf("Trial %d\n",trials);
	fflush(stdout);
	round=0;

	//if (trials%100==0)
	eNB2UE[0]->first_run = 1;

	while (round < num_rounds) {
	  round_trials[round]++;

	  if(transmission_mode>=5)
	    pmi_feedback=1;
	  else 
	    pmi_feedback=0;
	  
	  if (abstx) {
	    if (trials==0 && round==0 && SNR==snr0)  //generate a new channel
	      hold_channel = 0;
	    else
	      hold_channel = 1;
	  }
	  else
	    hold_channel = 0;

	PMI_FEEDBACK:
	
	  //  printf("Trial %d : Round %d, pmi_feedback %d \n",trials,round,pmi_feedback);
	  for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
#ifdef IFFT_FPGA
	    memset(&PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][aa][0],0,NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME*sizeof(mod_sym_t));
#else
	    memset(&PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][aa][0],0,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#endif
	  }
	
	  if (input_fd==NULL) {

	    // Simulate HARQ procedures!!!
	    if (round == 0) {   // First round, set Ndi to 1 and rv to floor(round/2)
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->Ndi = 1;
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->rvidx = round&3;
	      DLSCH_alloc_pdu2_1E[0].ndi             = 1;
	      DLSCH_alloc_pdu2_1E[0].rv              = 0;
	      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2_1E[0],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
	    }
	    else { // set Ndi to 0
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->Ndi = 0;
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->rvidx = round&3;
	      DLSCH_alloc_pdu2_1E[0].ndi             = 0;
	      DLSCH_alloc_pdu2_1E[0].rv              = round&3;
	      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2_1E[0],sizeof(DCI1E_5MHz_2A_M10PRB_TDD_t));
	    }
	    
	    num_pdcch_symbols_2 = generate_dci_top(num_ue_spec_dci,
						   num_common_dci,
						   dci_alloc,
						   0,
						   1024,
						   &PHY_vars_eNB->lte_frame_parms,
						   PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
						   subframe);

	    if (num_pdcch_symbols_2 > num_pdcch_symbols) {
	      msg("Error: given num_pdcch_symbols not big enough\n");
	      exit(-1);
	    }

	    for (k=0;k<n_users;k++) {
	      coded_bits_per_codeword = get_G(&PHY_vars_eNB->lte_frame_parms,
					      PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb,
					      PHY_vars_eNB->dlsch_eNB[k][0]->rb_alloc,
					      get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
					      num_pdcch_symbols,
					      subframe);
	      
#ifdef TBS_FIX   // This is for MESH operation!!!
	      tbs = (double)3*dlsch_tbs25[get_I_TBS(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs)][PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb-1]/4;
#else
	      tbs = (double)dlsch_tbs25[get_I_TBS(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs)][PHY_vars_eNB->dlsch_eNB[k][0]->nb_rb-1];
#endif
	      
	      rate = (double)tbs/(double)coded_bits_per_codeword;

	      uncoded_ber_bit = (short*) malloc(2*coded_bits_per_codeword);
	      
	      if (trials==0 && round==0) 
		printf("Rate = %f (%f bits/dim) (G %d, TBS %d, mod %d, pdcch_sym %d)\n",
		       rate,rate*get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
		       coded_bits_per_codeword,
		       tbs,
		       get_Qm(PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->mcs),
		       num_pdcch_symbols);
	      
	      /*
	      // generate channel here
	      random_channel(eNB2UE);
	      // generate frequency response
	      freq_channel(eNB2UE,NB_RB);
	      // generate PMI from channel
	      */
	      
		
	      // use the PMI from previous trial
	      if (DLSCH_alloc_pdu2_1E[0].tpmi == 5) {
		PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc = quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0);
		PHY_vars_UE->dlsch_ue[0][0]->pmi_alloc = quantize_subband_pmi(&PHY_vars_UE->PHY_measurements,0);
		if (n_users>1) 
		  PHY_vars_eNB->dlsch_eNB[1][0]->pmi_alloc = (PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc ^ 0x1555); 
		/*
		if ((trials<10) && (round==0)) {
		  printf("tx PMI UE0 %x (pmi_feedback %d)\n",pmi2hex_2Ar1(PHY_vars_eNB->dlsch_eNB[0][0]->pmi_alloc),pmi_feedback);
		  if (transmission_mode ==5)
		    printf("tx PMI UE1 %x\n",pmi2hex_2Ar1(PHY_vars_eNB->dlsch_eNB[1][0]->pmi_alloc));
		}
		*/		
	      }
	      
	      if (dlsch_encoding(input_buffer[k],
				 &PHY_vars_eNB->lte_frame_parms,
				 num_pdcch_symbols,
				 PHY_vars_eNB->dlsch_eNB[k][0],
				 subframe)<0)
		exit(-1);
	      
	      // printf("Did not Crash here 1\n");
	      PHY_vars_eNB->dlsch_eNB[k][0]->rnti = n_rnti+k;	  
	      dlsch_scrambling(&PHY_vars_eNB->lte_frame_parms,
			       num_pdcch_symbols,
			       PHY_vars_eNB->dlsch_eNB[k][0],
			       coded_bits_per_codeword,
			       0,
			       subframe<<1);
	      if (n_frames==1) {
		for (s=0;s<PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->C;s++) {
		  if (s<PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Cminus)
		    Kr = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Kminus;
		  else
		    Kr = PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->Kplus;
	      
		  Kr_bytes = Kr>>3;
	      
		  for (i=0;i<Kr_bytes;i++)
		    printf("%d : (%x)\n",i,PHY_vars_eNB->dlsch_eNB[k][0]->harq_processes[0]->c[s][i]);
		}
	      }
	      // printf("Did not Crash here 2\n");
	  
	      //	      if (k==1)

	      re_allocated = dlsch_modulation(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
					      AMP,
					      subframe,
					      &PHY_vars_eNB->lte_frame_parms,
					      num_pdcch_symbols,
					      PHY_vars_eNB->dlsch_eNB[k][0]);

	      // printf("Did not Crash here 3\n");
	      if (trials==0 && round==0)
		printf("RE count %d\n",re_allocated);
	  
	      if (num_layers>1)
		re_allocated = dlsch_modulation(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
						1024,
						subframe,
						&PHY_vars_eNB->lte_frame_parms,
						num_pdcch_symbols,
						PHY_vars_eNB->dlsch_eNB[k][1]);
	    } //n_users

	    
	    generate_pilots(PHY_vars_eNB,
			    PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
			    1024,
			    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
	  

	    do_OFDM_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
			PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id],
			(subframe*2),
			&PHY_vars_eNB->lte_frame_parms);
	    do_OFDM_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
			PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id],
			(subframe*2)+1,
			&PHY_vars_eNB->lte_frame_parms);
	    do_OFDM_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id],
			PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id],
			(subframe*2)+2,
			&PHY_vars_eNB->lte_frame_parms);

#ifdef IFFT_FPGA
	    if (n_frames==1) {
	      write_output("txsigF0.m","txsF0", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][0][subframe*nsymb*300],300*nsymb,1,4);
	      if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		write_output("txsigF1.m","txsF1", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][1][subframe*nsymb*300],300*nsymb,1,4);
	      write_output("txsigF20.m","txsF20", txdataF2[0], FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	      if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		write_output("txsigF21.m","txsF21", txdataF2[1], FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
	    }
#else //IFFT_FPGA
	    if (n_frames==1) {
	      write_output("txsigF0.m","txsF0", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][0][subframe*nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);
	      if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		write_output("txsigF1.m","txsF1", &PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNB_id][1][subframe*nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size],nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);
	    }
#endif

	    tx_lev = 0;
	    for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
	      tx_lev += signal_energy(&PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]
				      [subframe*PHY_vars_eNB->lte_frame_parms.samples_per_tti],
				      PHY_vars_eNB->lte_frame_parms.samples_per_tti);
	    }
	    tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
	  
	    if (n_frames==1) {
	      printf("tx_lev = %d (%d dB)\n",tx_lev,tx_lev_dB);
	      write_output("txsig0.m","txs0", &PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][0][subframe* PHY_vars_eNB->lte_frame_parms.samples_per_tti],
			   
			   PHY_vars_eNB->lte_frame_parms.samples_per_tti,1,1);
	    }
	  }
	  /*
	  else {  // Read signal from file
	    i=0;
	    while (!feof(input_fd)) {
	      fscanf(input_fd,"%s %s",input_val_str,input_val_str2);
	    
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
	  */
 
	  //	  printf("Copying tx ..., nsymb %d (n_tx %d), awgn %d\n",nsymb,PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,awgn_flag);
	  for (i=0;i<2*frame_parms->samples_per_tti;i++) {
	    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
	      if (awgn_flag == 0) {
		s_re[aa][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) + (i<<1)]);
		s_im[aa][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
	      }
	      else {
		for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
		  if (aa==0) {
		    r_re[aarx][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		    r_im[aarx][i] = ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		  }
		  else {
		    r_re[aarx][i] += ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)]);
		    r_im[aarx][i] += ((double)(((short *)PHY_vars_eNB->lte_eNB_common_vars.txdata[eNB_id][aa]))[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti) +(i<<1)+1]);
		  }
 
		}
	      }
	    }
	  }

	  //Multipath channel
	  if (awgn_flag == 0) {	
	    multipath_channel(eNB2UE[0],s_re,s_im,r_re,r_im,
			      2*frame_parms->samples_per_tti,hold_channel);
	    if(abstx==1){
	      if(hold_channel1==0){
	       	hold_channel1 = multipath_channel_nosigconv(eNB2UE[1]);
	        hold_channel1 = multipath_channel_nosigconv(eNB2UE[2]);
	        hold_channel1 = multipath_channel_nosigconv(eNB2UE[3]);
	      }
	      switch (round)
		{
		case 1: 
		  multipath_channel(eNB2UE[1],s_re,s_im,r_re,r_im,
				    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,hold_channel1);
		  break;
		case 2:
		  multipath_channel(eNB2UE[2],s_re,s_im,r_re,r_im,
				    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,hold_channel1);
		  break;
		case 3:
		  multipath_channel(eNB2UE[3],s_re,s_im,r_re,r_im,
				    2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,hold_channel1);
		  break;
		}
	    }
	  }
	  
	  if(abstx){
	    if(saving_bler==0)
	    if (trials==0 && round==0) {
	      // calculate freq domain representation to compute SINR
	      freq_channel(eNB2UE[0], NB_RB,2*NB_RB + 1);
	      // snr=pow(10.0,.1*SNR);
	      fprintf(csv_fd,"%f,",SNR);
	      
	      for (u=0;u<2*NB_RB;u++){
		for (aarx=0;aarx<eNB2UE[0]->nb_rx;aarx++) {
		  for (aatx=0;aatx<eNB2UE[0]->nb_tx;aatx++) {
		    channelx = eNB2UE[0]->chF[aarx+(aatx*eNB2UE[0]->nb_rx)][u].x;
		    channely = eNB2UE[0]->chF[aarx+(aatx*eNB2UE[0]->nb_rx)][u].y;
		    fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
		  }
		}
	      }
	      
	      freq_channel(eNB2UE[1], NB_RB,2*NB_RB + 1);
	      
	      for (u=0;u<2*NB_RB;u++){
		for (aarx=0;aarx<eNB2UE[1]->nb_rx;aarx++) {
		  for (aatx=0;aatx<eNB2UE[1]->nb_tx;aatx++) {
		    channelx = eNB2UE[1]->chF[aarx+(aatx*eNB2UE[1]->nb_rx)][u].x;
		    channely = eNB2UE[1]->chF[aarx+(aatx*eNB2UE[1]->nb_rx)][u].y;
		    fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
		  }
		}
	      }
	       freq_channel(eNB2UE[2], NB_RB,2*NB_RB + 1);
	      
	      for (u=0;u<2*NB_RB;u++){
		for (aarx=0;aarx<eNB2UE[2]->nb_rx;aarx++) {
		  for (aatx=0;aatx<eNB2UE[2]->nb_tx;aatx++) {
		    channelx = eNB2UE[2]->chF[aarx+(aatx*eNB2UE[2]->nb_rx)][u].x;
		    channely = eNB2UE[2]->chF[aarx+(aatx*eNB2UE[2]->nb_rx)][u].y;
		    fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
		  }
		}
	      }
	      
	       freq_channel(eNB2UE[3], NB_RB,2*NB_RB + 1);
	       
	       for (u=0;u<2*NB_RB;u++){
		for (aarx=0;aarx<eNB2UE[3]->nb_rx;aarx++) {
		  for (aatx=0;aatx<eNB2UE[3]->nb_tx;aatx++) {
		    channelx = eNB2UE[3]->chF[aarx+(aatx*eNB2UE[3]->nb_rx)][u].x;
		    channely = eNB2UE[3]->chF[aarx+(aatx*eNB2UE[3]->nb_rx)][u].y;
		    fprintf(csv_fd,"%e+i*(%e),",channelx,channely);
		  }
		}
	      }
	    }
	  }

	 
	  
	  //AWGN
      // This is the SNR on the PDSCH for OFDM symbols without pilots -> rho_A
	  sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size/(NB_RB*12)) - SNR - get_pa_dB(PHY_vars_eNB->pdsch_config_dedicated);
	  sigma2 = pow(10,sigma2_dB/10);
	  if (n_frames==1)
	    printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);

	  for (i=0; i<2*frame_parms->samples_per_tti; i++) {
	    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
	      //printf("s_re[0][%d]=> %f , r_re[0][%d]=> %f\n",i,s_re[aa][i],i,r_re[aa][i]);
	      ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i] = 
		(short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	      ((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti)+2*i+1] = 
		(short) (r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    }
	  }   

	  //    lte_sync_time_init(PHY_vars_eNB->lte_frame_parms,lte_ue_common_vars);
	  //    lte_sync_time(lte_ue_common_vars->rxdata, PHY_vars_eNB->lte_frame_parms);
	  //    lte_sync_time_free();
	  
	  /*
	  // optional: read rx_frame from file
	  if ((rx_frame_file = fopen("rx_frame.dat","r")) == NULL)
	  {
	  printf("Cannot open rx_frame.m data file\n");
	  exit(0);
	  }
	    
	  result = fread((void *)PHY_vars->rx_vars[0].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	  printf("Read %d bytes\n",result);
	  result = fread((void *)PHY_vars->rx_vars[1].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
	  printf("Read %d bytes\n",result);
	    
	  fclose(rx_frame_file);
	  */
	  
	  if (n_frames==1) {
	    printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&PHY_vars_UE->lte_ue_common_vars.rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	    printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	    printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	  }
	  
	  if (PHY_vars_eNB->lte_frame_parms.Ncp == 0) {  // normal prefix
	    pilot1 = 4;
	    pilot2 = 7;
	    pilot3 = 11;
	  }
	  else {  // extended prefix
	    pilot1 = 3;
	    pilot2 = 6;
	    pilot3 = 9;
	  }
	  
	  i_mod = get_Qm(mcs);
	  
	  // Inner receiver scheduling for 3 slots
	  for (Ns=(2*subframe);Ns<((2*subframe)+3);Ns++) {
	    for (l=0;l<pilot2;l++) {
	      if (n_frames==1)
		printf("Ns %d, l %d\n",Ns,l);
	      /*
		This function implements the OFDM front end processor (FEP).
	      
		Parameters:
		frame_parms 	LTE DL Frame Parameters
		ue_common_vars 	LTE UE Common Vars
		l 	symbol within slot (0..6/7)
		Ns 	Slot number (0..19)
		sample_offset 	offset within rxdata (points to beginning of subframe)
		no_prefix 	if 1 prefix is removed by HW 
	      
	      */
	      slot_fep(PHY_vars_UE,
		       l,
		       Ns%20,
		       0,
		       0);

#ifdef PERFECT_CE
	      if (awgn_flag==0) {
		// fill in perfect channel estimates
		if (abstx==1)
		freq_channel(eNB2UE[round],PHY_vars_UE->lte_frame_parms.N_RB_DL,12*PHY_vars_UE->lte_frame_parms.N_RB_DL + 1);
		else
		    freq_channel(eNB2UE[0],PHY_vars_UE->lte_frame_parms.N_RB_DL,12*PHY_vars_UE->lte_frame_parms.N_RB_DL + 1);
		//write_output("channel.m","ch",desc1->ch[0],desc1->channel_length,1,8);
		//write_output("channelF.m","chF",desc1->chF[0],nb_samples,1,8);
		for(k=0;k<NUMBER_OF_eNB_MAX;k++) {
		  for(aa=0;aa<frame_parms->nb_antennas_tx;aa++) 
		    { 
		      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
			{
			  for (i=0;i<frame_parms->N_RB_DL*12;i++)
			    { 
			      if (abstx==1){
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[round]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].x*AMP/2);
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[round]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].y*AMP/2) ;
			      }
			      else {
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[0]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].x*AMP/2);
				((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[k][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(s16)(eNB2UE[0]->chF[aarx+(aa*frame_parms->nb_antennas_rx)][i].y*AMP/2) ;
			      }
			    }
			}
		    }
		}
	      }
	      else {
		for(aa=0;aa<frame_parms->nb_antennas_tx;aa++) 
		  { 
		    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
		      {
			for (i=0;i<frame_parms->N_RB_DL*12;i++)
			  { 
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=(short)(AMP/2);
			    ((s16 *) PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[2*i+1+(l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH)*2]=0/2;
			  }
		      }
		  }
	      }
#endif
 

	      if ((Ns==((2*subframe))) && (l==0)) {
		lte_ue_measurements(PHY_vars_UE,
				    subframe*PHY_vars_UE->lte_frame_parms.samples_per_tti,
				    1,
				    0);
		/*
	debug_msg("RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), avg rx power %d dB (%d lin), RX gain %d dB\n",
		  PHY_vars_UE->PHY_measurements.rx_rssi_dBm[0] - ((PHY_vars_UE->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0), 
		  PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi_dB[0][1],
		  PHY_vars_UE->PHY_measurements.wideband_cqi[0][0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi[0][1],		  
		  PHY_vars_UE->PHY_measurements.rx_power_avg_dB[0],
		  PHY_vars_UE->PHY_measurements.rx_power_avg[0],
		  PHY_vars_UE->rx_total_gain_dB);
	debug_msg("N0 %d dBm digital (%d, %d) dB, linear (%d, %d), avg noise power %d dB (%d lin)\n",
		  PHY_vars_UE->PHY_measurements.n0_power_tot_dBm,
		  PHY_vars_UE->PHY_measurements.n0_power_dB[0],
		  PHY_vars_UE->PHY_measurements.n0_power_dB[1],
		  PHY_vars_UE->PHY_measurements.n0_power[0],
		  PHY_vars_UE->PHY_measurements.n0_power[1],
		  PHY_vars_UE->PHY_measurements.n0_power_avg_dB,
		  PHY_vars_UE->PHY_measurements.n0_power_avg);
	debug_msg("Wideband CQI tot %d dB, wideband cqi avg %d dB\n",
		  PHY_vars_UE->PHY_measurements.wideband_cqi_tot[0],
		  PHY_vars_UE->PHY_measurements.wideband_cqi_avg[0]);
		*/
		    
		if (transmission_mode==5 || transmission_mode==6) {
		  if (pmi_feedback == 1) {
		    pmi_feedback = 0;
		    hold_channel = 1;
		    goto PMI_FEEDBACK;
		  }
		}

	      }


	      if ((Ns==(2*subframe)) && (l==pilot1)) {// process symbols 0,1,2

		if (dci_flag == 1) {
		  rx_pdcch(&PHY_vars_UE->lte_ue_common_vars,
			   PHY_vars_UE->lte_ue_pdcch_vars,
			   &PHY_vars_UE->lte_frame_parms,
			   subframe,
			   0,
			   (PHY_vars_UE->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
			   0);

		  // overwrite number of pdcch symbols
		  PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols = num_pdcch_symbols;

		  dci_cnt = dci_decoding_procedure(PHY_vars_UE,
						   dci_alloc_rx,
						   eNB_id,
						   subframe);
		  //printf("dci_cnt %d\n",dci_cnt);
		
		  if (dci_cnt==0) {
		    dlsch_active = 0;
		    if (round==0) {
		      dci_errors++;
		      round=5;
		      errs[0]++;
		      round_trials[0]++;
		      //		  printf("DCI error trial %d errs[0] %d\n",trials,errs[0]);
		    }
		    //		for (i=1;i<=round;i++)
		    //		  round_trials[i]--;
		    //		round=5;
		  }
		
		  for (i=0;i<dci_cnt;i++) {
		    //printf("Generating dlsch parameters for RNTI %x\n",dci_alloc_rx[i].rnti);
		    if ((dci_alloc_rx[i].rnti == n_rnti) && 
			(generate_ue_dlsch_params_from_dci(0,
							   dci_alloc_rx[i].dci_pdu,
							   dci_alloc_rx[i].rnti,
							   dci_alloc_rx[i].format,
							   PHY_vars_UE->dlsch_ue[0],
							   &PHY_vars_UE->lte_frame_parms,
                               PHY_vars_UE->pdsch_config_dedicated,
							   SI_RNTI,
							   0,
							   P_RNTI)==0)) {
		      //dump_dci(&PHY_vars_UE->lte_frame_parms,&dci_alloc_rx[i]);
		      coded_bits_per_codeword = get_G(&PHY_vars_eNB->lte_frame_parms,
						      PHY_vars_UE->dlsch_ue[0][0]->nb_rb,
						      PHY_vars_UE->dlsch_ue[0][0]->rb_alloc,
						      get_Qm(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs),
						      PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
						      subframe);
		      /*
			rate = (double)dlsch_tbs25[get_I_TBS(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs)][PHY_vars_UE->dlsch_ue[0][0]->nb_rb-1]/(coded_bits_per_codeword);
			rate*=get_Qm(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->mcs);

			printf("num_pdcch_symbols %d, G %d, TBS %d\n",PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,coded_bits_per_codeword,PHY_vars_UE->dlsch_ue[0][0]->harq_processes[PHY_vars_UE->dlsch_ue[0][0]->current_harq_pid]->TBS);
		      */
		      dlsch_active = 1;
		    }
		    else {
		      dlsch_active = 0;
		      if (round==0) {
			dci_errors++;
			errs[0]++;
			round_trials[0]++;

			if (n_frames==1) {
			  printf("DCI misdetection trial %d\n",trials);
			  round=5;
			}
		      }
		      //		  for (i=1;i<=round;i++)
		      //		    round_trials[i]--;
		      //		  round=5;
		    }
		  }
		}  // if dci_flag==1
		else { //dci_flag == 0

		  PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;
		  PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols = num_pdcch_symbols;

		  generate_ue_dlsch_params_from_dci(0,
						    &DLSCH_alloc_pdu2_1E[0],
						    C_RNTI,
						    format1E_2A_M10PRB,
						    PHY_vars_UE->dlsch_ue[0],
						    &PHY_vars_UE->lte_frame_parms,
                            PHY_vars_UE->pdsch_config_dedicated,
						    SI_RNTI,
						    0,
						    P_RNTI);
		  dlsch_active = 1;
		} // if dci_flag == 1
	      }

	      if (dlsch_active == 1) {
		if ((Ns==(1+(2*subframe))) && (l==0)) {// process PDSCH symbols 1,2,3,4,5,(6 Normal Prefix)

		  for (m=PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols;
		       m<pilot2;
		       m++) 
		    {
		      if (rx_pdsch(PHY_vars_UE,
				   PDSCH,
				   eNB_id,
				   eNB_id_i,
				   subframe,
				   m,
				   (m==PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols)?1:0,
				   dual_stream_UE,
				   i_mod)==-1)
			{
			  dlsch_active = 0;
			  break;
			}
		    }
		}
		  
		if ((Ns==(1+(2*subframe))) && (l==pilot1))
		  {// process symbols (6 Extended Prefix),7,8,9 
		    for (m=pilot2;
			 m<pilot3;
			 m++)
		      {
			if (rx_pdsch(PHY_vars_UE,
				     PDSCH,
				     eNB_id,
				     eNB_id_i,
				     subframe,
				     m,
				     0,
				     dual_stream_UE,
				     i_mod)==-1)
			  {
			    dlsch_active=0;
			    break;
			  }
		      }
		  }
		
		if ((Ns==(2+(2*subframe))) && (l==0))  // process symbols 10,11,(12,13 Normal Prefix) do deinterleaving for TTI
		  {
		    for (m=pilot3;
			 m<PHY_vars_UE->lte_frame_parms.symbols_per_tti;
			 m++)
		      {
			if (rx_pdsch(PHY_vars_UE,
				     PDSCH,
				     eNB_id,
				     eNB_id_i,
				     subframe,
				     m,
				     0,
				     dual_stream_UE,
				     i_mod)==-1)
			  {
			    dlsch_active=0;
			    break;
			  }
		      }
		  }
		
		if ((n_frames==1) && (Ns==(2+(2*subframe))) && (l==0))  {
		  write_output("ch0.m","ch0",eNB2UE[0]->ch[0],eNB2UE[0]->channel_length,1,8);
		  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		    write_output("ch1.m","ch1",eNB2UE[0]->ch[PHY_vars_eNB->lte_frame_parms.nb_antennas_rx],eNB2UE[0]->channel_length,1,8);

		  //common vars
		  write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		  write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
		  if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
		    write_output("rxsig1.m","rxs1", PHY_vars_UE->lte_ue_common_vars.rxdata[1],PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		    write_output("rxsigF1.m","rxsF1", PHY_vars_UE->lte_ue_common_vars.rxdataF[1],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
		  }

		  write_output("dlsch00_ch0.m","dl00_ch0",
			       &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0]),
			       PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
		  if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1)
		    write_output("dlsch01_ch0.m","dl01_ch0",
				 &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][1][0]),
				 PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
		  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		    write_output("dlsch10_ch0.m","dl10_ch0",
				 &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][2][0]),
				 PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
		  if ((PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) && (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1))
		    write_output("dlsch11_ch0.m","dl11_ch0",
				 &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][3][0]),
				 PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
		    
		  //pdsch_vars
		  dump_dlsch2(PHY_vars_UE,eNB_id,coded_bits_per_codeword);
          dump_dlsch2(PHY_vars_UE,eNB_id_i,coded_bits_per_codeword);
		  write_output("dlsch_e.m","e",PHY_vars_eNB->dlsch_eNB[0][0]->e,coded_bits_per_codeword,1,4);

		  //pdcch_vars
		  write_output("pdcchF0_ext.m","pdcchF_ext", PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_ext[0],2*3*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size,1,1);
		  write_output("pdcch00_ch0_ext.m","pdcch00_ch0_ext",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext[0],300*3,1,1);

		  write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->rxdataF_comp[0],4*300,1,1);
		  write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNB_id]->llr,2400,1,4);
		    
		}
	      
	      }
	    }
	  }

	  //saving PMI incase of Transmission Mode > 5

	  if(abstx){
	    if(saving_bler==0)
	    if (trials==0 && round==0 && transmission_mode>=5){
	      for (iii=0; iii<NB_RB; iii++){
		//fprintf(csv_fd, "%d, %d", (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]),(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id_i]->pmi_ext[iii]));
		fprintf(csv_fd,"%x,%x,",(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]),(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]));
		msg(" %x",(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->pmi_ext[iii]));
	      }
	    }
	  }
	
	  // calculate uncoded BLER
	  /* uncoded_ber=0;
	  for (i=0;i<coded_bits_per_codeword;i++) 
	    if (PHY_vars_eNB->dlsch_eNB[0][0]->e[i] != (PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i]<0)) {
	      uncoded_ber_bit[i] = 1;
	      uncoded_ber++;
	    }
	    else
	      uncoded_ber_bit[i] = 0;

	  uncoded_ber/=coded_bits_per_codeword;
	  avg_ber += uncoded_ber;
	  */
	  //write_output("uncoded_ber_bit.m","uncoded_ber_bit",uncoded_ber_bit,coded_bits_per_codeword,1,0);
	 
	  /*
	    printf("precoded CQI %d dB, opposite precoded CQI %d dB\n",
	    PHY_vars_UE->PHY_measurements.precoded_cqi_dB[eNB_id][0],
	    PHY_vars_UE->PHY_measurements.precoded_cqi_dB[eNB_id_i][0]);
	  */

	  // clip the llrs
	  /*	   for (i=0; i<coded_bits_per_codeword; i++) {
	    if (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i]>127)
	      PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i] = 127;
	    else if (PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i]<-128)
	      PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0][i] = -128;
	      }
	  */
	  PHY_vars_UE->dlsch_ue[0][0]->rnti = n_rnti;
	  dlsch_unscrambling(&PHY_vars_UE->lte_frame_parms,
			     PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
			     PHY_vars_UE->dlsch_ue[0][0],
			     coded_bits_per_codeword,
			     PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0],
			     0,
			     subframe<<1);

	  /*
	  for (i=0;i<coded_bits_per_codeword;i++) 
	    PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i] = (short)quantize(100,PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0][i],4);
	  */

	  ret = dlsch_decoding(PHY_vars_UE->lte_ue_pdsch_vars[eNB_id]->llr[0],		 
			       &PHY_vars_UE->lte_frame_parms,
			       PHY_vars_UE->dlsch_ue[0][0],
			       subframe,
			       PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols);
 
#ifdef XFORMS
	  do_forms(form,
		   &PHY_vars_UE->lte_frame_parms,  
		   PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates_time[0],
		   PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0],
		   PHY_vars_UE->lte_ue_common_vars.rxdata,
		   PHY_vars_UE->lte_ue_common_vars.rxdataF,
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->rxdataF_comp[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[1]->rxdataF_comp[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->dl_ch_rho_ext[0],
		   PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0],coded_bits_per_codeword);
	  //PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->w[0],3*(tbs+64)); 
	  //uncoded_ber_bit,coded_bits_per_codeword);


	  /*
	  printf("Hit a key to continue\n");
	  c = getchar();
	  */
 
#endif

	  if (ret <= MAX_TURBO_ITERATIONS) {
		
	    if (n_frames==1) 
	      printf("No DLSCH errors found\n");
	    //	    exit(-1);
	    if (fix_rounds==0)
	      round=5;
	    else
	      round++;
	  }	
	  else {
	    errs[round]++;
		
	    if (n_frames==1) {
	      //if ((n_frames==1) || (SNR>=30)) {
	      printf("DLSCH errors found, uncoded ber %f\n",uncoded_ber);
	      for (s=0;s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->C;s++) {
		if (s<PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Cminus)
		  Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kminus;
		else
		  Kr = PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Kplus;
		    
		Kr_bytes = Kr>>3;
		    
		printf("Decoded_output (Segment %d):\n",s);
		for (i=0;i<Kr_bytes;i++)
		  printf("%d : %x (%x)\n",i,PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i],PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->c[s][i]^PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->c[s][i]);
	      }
	      write_output("rxsig0.m","rxs0", &PHY_vars_UE->lte_ue_common_vars.rxdata[0][0],10*PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
	      write_output("rxsigF0.m","rxsF0", &PHY_vars_UE->lte_ue_common_vars.rxdataF[0][0],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
	      if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) {
		write_output("rxsig1.m","rxs1", PHY_vars_UE->lte_ue_common_vars.rxdata[1],PHY_vars_UE->lte_frame_parms.samples_per_tti,1,1);
		write_output("rxsigF1.m","rxsF1", PHY_vars_UE->lte_ue_common_vars.rxdataF[1],2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb,2,1);
	      }
	      
	      write_output("dlsch00_ch0.m","dl00_ch0",
			   &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][0][0]),
			   PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
	      if (PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1)
		write_output("dlsch01_ch0.m","dl01_ch0",
			     &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][1][0]),
			     PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
	      if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
		write_output("dlsch10_ch0.m","dl10_ch0",
			     &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][2][0]),
				 PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
	      if ((PHY_vars_UE->lte_frame_parms.nb_antennas_rx>1) && (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1))
		write_output("dlsch11_ch0.m","dl11_ch0",
			     &(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[eNB_id][3][0]),
			     PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb/2,1,1);
	      
	      //pdsch_vars
	      dump_dlsch2(PHY_vars_UE,eNB_id,coded_bits_per_codeword);
	      write_output("dlsch_e.m","e",PHY_vars_eNB->dlsch_eNB[0][0]->e,coded_bits_per_codeword,1,4);
	      write_output("dlsch_ber_bit.m","ber_bit",uncoded_ber_bit,coded_bits_per_codeword,1,0);
	      write_output("dlsch_eNB_w.m","w",PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->w[0],3*(tbs+64),1,4);
	      write_output("dlsch_UE_w.m","w",PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->w[0],3*(tbs+64),1,0);

	      
	      exit(-1);
	    }
	    //	    printf("round %d errors %d/%d\n",round,errs[round],trials);

	
	    round++;
		
	    if (n_frames==1)
	      printf("DLSCH in error in round %d\n",round);
		
	  }
	  //free(uncoded_ber_bit);
	  //uncoded_ber_bit = NULL;
	  
	}  //round
	//      printf("\n");

	if ((errs[0]>=n_frames/10) && (trials>(n_frames/2)))
	  break;
      
	//len = chbch_stats_read(stats_buffer,NULL,0,4096);
	//printf("%s\n\n",stats_buffer);
      
      }   //trials
      printf("\n**********************SNR = %f dB (tx_lev %f, sigma2_dB %f)**************************\n",
	     SNR,
	     (double)tx_lev_dB+10*log10(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size/(NB_RB*12)),
	     sigma2_dB);
    
      printf("Errors (%d/%d %d/%d %d/%d %d/%d), Pe = (%e,%e,%e,%e), dci_errors %d/%d, Pe = %e => effective rate %f (%f), normalized delay %f (%f)\n",
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
	     dci_errors,
	     round_trials[0],
	     (double)dci_errors/(round_trials[0]),
	     rate*((double)(round_trials[0]-dci_errors)/((double)round_trials[0] + round_trials[1] + round_trials[2] + round_trials[3])),
	     rate,
	     (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0])/(double)PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->TBS,
	     (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0]));
    
      
      fprintf(bler_fd,"%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
	      SNR,
	      mcs,
	      PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->TBS,
	      rate,
	      errs[0],
	      round_trials[0],
	      errs[1],
	      round_trials[1],
	      errs[2],
	      round_trials[2],
	      errs[3],
	      round_trials[3],
	      dci_errors);

      fprintf(tikz_fd,"(%f,%f)", SNR, (float)errs[0]/round_trials[0]);
    
      if(abstx){ //ABSTRACTION         
	blerr[0] = (double)errs[0]/(round_trials[0]);
	blerr[1] = (double)errs[1]/(round_trials[1]);
	blerr[2] = (double)errs[2]/(round_trials[2]);
	blerr[3] = (double)errs[3]/(round_trials[3]);
	fprintf(csv_fd,"%e,%e,%e,%e;\n",blerr[0],blerr[1],blerr[2],blerr[3]);
	/*	
	blerr = (double)errs[0]/(round_trials[0]);
	
	if(saving_bler==0)
	   fprintf(csv_fd,"%e;\n",blerr);

	if(blerr<1)
	  saving_bler = 0;
	else saving_bler =1;
	*/
	 
      } //ABStraction
      /* if(num_rounds==1){
      blerr= (double)errs[1]/(round_trials[1]);
	if (blerr>.1)
	  snr_step = 1.5;
	else snr_step = input_snr_step;
	}*/
      if (((double)errs[0]/(round_trials[0]))<1e-2) 
	break;
    }// SNR
  
  
  } //ch_realization
  
  
  fclose(bler_fd);
  fprintf(tikz_fd,"};\n");
  fclose(tikz_fd);

  if (input_trch_file==1)
    fclose(input_trch_fd);
  if (input_file==1)
    fclose(input_fd);
  if(abstx){// ABSTRACTION
    fprintf(csv_fd,"];");
    fclose(csv_fd);
  }
  
  
  
  
  printf("Freeing dlsch structures\n");
  for (i=0;i<2;i++) {
    printf("eNB %d\n",i);
    free_eNB_dlsch(PHY_vars_eNB->dlsch_eNB[0][i]);
    printf("UE %d\n",i);
    free_ue_dlsch(PHY_vars_UE->dlsch_ue[0][i]);
  }
  
  
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
  

