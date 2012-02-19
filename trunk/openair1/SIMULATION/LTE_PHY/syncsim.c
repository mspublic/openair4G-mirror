#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "SIMULATION/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/COMMON/defs.h"
#include "LAYER2/MAC/vars.h"

#ifdef XFORMS
#include <forms.h>
#include "../../USERSPACE_TOOLS/SCOPE/lte_scope.h"
#endif //XFORMS


#include "OCG_vars.h"

#define BW 5.0


PHY_VARS_eNB *PHY_vars_eNB,*PHY_vars_eNB1,*PHY_vars_eNB2;
PHY_VARS_UE *PHY_vars_UE;

#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13

extern int setup_oai_hw(LTE_DL_FRAME_PARMS *frame_parms,
			PHY_VARS_UE  *phy_vars_ue,
			PHY_VARS_eNB *phy_vars_eNB);

#ifdef XFORMS
void do_forms2(FD_lte_scope *form, 
	       LTE_DL_FRAME_PARMS *frame_parms, 
	       short **channel, 
	       short **channel_f, 
	       short **rx_sig, 
	       short **rx_sig_f, 
	       short *pdcch_comp, 
	       short *dlsch_comp, 
	       short* dlsch_comp_i, 
	       short* dlsch_llr, 
	       short* pbch_comp, 
	       char *pbch_llr, 
	       int coded_bits_per_codeword)
{

  int i,j,k,s;

  float Re,Im;
  float mag_sig[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig_time[NB_ANTENNAS_RX*4*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig2[FRAME_LENGTH_COMPLEX_SAMPLES],
    time2[FRAME_LENGTH_COMPLEX_SAMPLES],
    I[25*12*11*4], Q[25*12*11*4],
    *llr,*llr_time;

  float avg, cum_avg;

  extern int* sync_corr_ue0;
  
  u16 nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  llr = malloc(coded_bits_per_codeword*sizeof(float));
  llr_time = malloc(coded_bits_per_codeword*sizeof(float));


  // Channel frequency response
  if (channel_f != NULL) {
    cum_avg = 0;
    ind = 0;
    for (j=0; j<4; j++) { 
      for (i=0;i<frame_parms->nb_antennas_rx;i++) {
	for (k=0;k<NUMBER_OF_OFDM_CARRIERS*(nsymb>>1);k++){
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
  }
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

  // sync_corr
  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++){
    time2[i] = (float) i;
    sig2[i] = (float) sync_corr_ue0[i];
  }

  fl_set_xyplot_data(form->channel_t_re,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");
  //fl_set_xyplot_ybounds(form->channel_t_re,0,1e7);

  /*
  // channel_t_re = rx_sig_f[0]
  //for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX; i++)  {
  for (i=0; i<NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti/2; i++)  {
    sig2[i] = 10*log10(1.0+(double) ((rx_sig_f[0][4*i])*(rx_sig_f[0][4*i])+(rx_sig_f[0][4*i+1])*(rx_sig_f[0][4*i+1])));
    time2[i] = (float) i;
  } 

  //fl_set_xyplot_ybounds(form->channel_t_re,10,90);
  fl_set_xyplot_data(form->channel_t_re,time2,sig2,NUMBER_OF_OFDM_CARRIERS*frame_parms->symbols_per_tti,"","","");
  //fl_set_xyplot_data(form->channel_t_re,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,"","","");
  */

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

  // PBCH LLR
  if (pbch_llr!=NULL) {
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
  }

  // PBCH I/Q
  if (pbch_comp!=NULL) {
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
  }

  
  // PDCCH I/Q
  j=0;
  for(i=0;i<12*25*3;i++) {
    I[j] = pdcch_comp[2*i];
    Q[j] = pdcch_comp[2*i+1];
    j++;
  }

  fl_set_xyplot_data(form->scatter_plot1,I,Q,12*25*3,"","","");
  //fl_set_xyplot_xbounds(form->scatter_plot,-100,100);
  //fl_set_xyplot_ybounds(form->scatter_plot,-100,100);
  

  // DLSCH LLR
  if (dlsch_llr != NULL) {
    for(i=0;i<coded_bits_per_codeword;i++) {
      llr[i] = (float) dlsch_llr[i];
      llr_time[i] = (float) i;
    }

    fl_set_xyplot_data(form->demod_out,llr_time,llr,coded_bits_per_codeword,"","","");
    fl_set_xyplot_ybounds(form->demod_out,-1000,1000);
  }

  // DLSCH I/Q
  if (dlsch_comp!=NULL) {
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
    
    fl_set_xyplot_data(form->scatter_plot2,I,Q,j,"","","");
    //fl_set_xyplot_xbounds(form->scatter_plot,-2000,2000);
    //fl_set_xyplot_ybounds(form->scatter_plot,-2000,2000);
  }

  // DLSCH I/Q
  if (dlsch_comp_i!=NULL) {
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
    

    fl_set_xyplot_data(form->scatter_plot2,I,Q,j,"","","");
    //fl_set_xyplot_xbounds(form->scatter_plot1,-2000,2000);
    //fl_set_xyplot_ybounds(form->scatter_plot1,-2000,2000);
  }
  /*
  // DLSCH rho
  if (dlsch_rho!=NULL) {
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
  //fl_set_xyplot_xbounds(form->scatter_plot2,-1000,1000);
  //fl_set_xyplot_ybounds(form->scatter_plot2,-1000,1000);
  }
  */

  free(llr);
  free(llr_time);

}  
#endif

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u8 frame_type,u16 Nid_cell,u8 N_RB_DL,u8 osf) {

  LTE_DL_FRAME_PARMS *lte_frame_parms;
  int i;

  printf("Start lte_param_init, frame_type %d, extended_prefix %d\n",frame_type,extended_prefix_flag);
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNB1 = malloc(sizeof(PHY_VARS_eNB));
  PHY_vars_eNB2 = malloc(sizeof(PHY_VARS_eNB));
  
  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  //PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));
  
  randominit(0);
  set_taus_seed(0);
  

  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = N_RB_DL;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = N_RB_DL;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = Nid_cell;
  lte_frame_parms->nushift            = Nid_cell%6;
  lte_frame_parms->phich_config_common.phich_resource            = oneSixth;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;
  lte_frame_parms->tdd_config = 3;
  lte_frame_parms->frame_type = frame_type;
  init_frame_parms(lte_frame_parms,osf);
  
  
  

  phy_init_top(lte_frame_parms); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  phy_init_lte_top(lte_frame_parms);
  
  memcpy((void*)&PHY_vars_UE->lte_frame_parms,(void*)&PHY_vars_eNB->lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));
  
  phy_init_lte_ue(PHY_vars_UE,0);
  for (i=0;i<3;i++)
    lte_gold(lte_frame_parms,PHY_vars_UE->lte_gold_table[i],i);    
  
  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);
  
  memcpy((void*)&PHY_vars_eNB1->lte_frame_parms,(void*)&PHY_vars_eNB->lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNB1->lte_frame_parms.nushift=1;
  PHY_vars_eNB1->lte_frame_parms.Nid_cell=2;
  
  memcpy((void*)&PHY_vars_eNB2->lte_frame_parms,(void*)&PHY_vars_eNB->lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));
  PHY_vars_eNB2->lte_frame_parms.nushift=2;
  PHY_vars_eNB2->lte_frame_parms.Nid_cell=3;
  
  phy_init_lte_eNB(PHY_vars_eNB1,0,0,0);
  
  phy_init_lte_eNB(PHY_vars_eNB2,0,0,0);
  
  phy_init_lte_top(lte_frame_parms);
  
  printf("Done lte_param_init\n");


}



int main(int argc, char **argv) {

  char c;

  int i,iout,l,aa,aarx;
  double sigma2, sigma2_dB=0,SNR,snr0=-2.0,snr1;
  u8 snr1set=0;
  //mod_sym_t **txdataF;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  int **txdata,**txdata1,**txdata2;
  double **s_re,**s_im,**s_re1,**s_im1,**s_re2,**s_im2,**r_re,**r_im,**r_re1,**r_im1,**r_re2,**r_im2;
  double iqim = 0.0;
  unsigned char pbch_pdu[6];
  FILE *output_fd;
  u8 write_output_file=0;
  int trial, n_trials, ntrials=1, n_errors,n_errors2,n_alamouti;
  u8 transmission_mode = 1,n_tx=1,n_rx=1;
  unsigned char eNb_id = 0;
  u16 Nid_cell=0;
  u8 awgn_flag=0;
  int n_frames=1;
  channel_desc_t *eNB2UE,*eNB2UE1,*eNB2UE2;
  u32 nsymb,tx_lev;
  u8 extended_prefix_flag=0,frame_type=1;
  s8 interf1=-21,interf2=-21;
  LTE_DL_FRAME_PARMS *frame_parms;
#ifdef EMOS
  fifo_dump_emos emos_dump;
#endif

  FILE *input_fd=NULL,*pbch_file_fd=NULL;
  char input_val_str[50],input_val_str2[50];
  u8 num_pdcch_symbols;
  u16 NB_RB=25;

  SCM_t channel_model=Rayleigh1_anticorr;

  u8 abstraction_flag=0;
  double pbch_sinr; 
  u8 N_RB_DL=25,osf=1;

  int openair_fd;
  int frequency=0,tcxo=74,fc=0;
  unsigned char temp[4];

  int oai_hw_input=0;
  int oai_hw_output=0;


  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];
  u16 n_rnti,dci_cnt;
  u32 DLSCH_alloc_pdu;
  u16 coded_bits_per_codeword;
  double tmp_re,tmp_im,foff,deltaF=0.0,cs,sn;

#ifdef XFORMS
  FD_lte_scope *form_dl;
  char title[255];


  fl_initialize (&argc, argv, NULL, 0, 0);
  form_dl = create_form_lte_scope();
  sprintf (title, "LTE DL SCOPE UE");
  fl_show_form (form_dl->lte_scope, FL_PLACE_HOTSPOT, FL_FULLBORDER, title);
#endif

  number_of_cards = 1;
  openair_daq_vars.rx_rf_mode = 1;
  
  /*
    rxdataF    = (int **)malloc16(2*sizeof(int*));
    rxdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
    
    rxdata    = (int **)malloc16(2*sizeof(int*));
    rxdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    rxdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */
  while ((c = getopt (argc, argv, "aehc:f:g:i:j:n:r:s:t:x:y:z:A:F:N:O:R:S:ZYDT:")) != -1)
    {
      switch (c)
	{
	case 'a':
	  printf("Running AWGN simulation\n");
	  awgn_flag = 1;
	  ntrials=1;
	  break;
	case 'f':
	  output_fd = fopen(optarg,"w");
	  write_output_file=1;
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
	  default:
	    msg("Unsupported channel model!\n");
	    exit(-1);
	  }
	break;
	case 'i':
	  interf1=atoi(optarg);
	  break;
	case 'c':
	  deltaF=atof(optarg);
	  break;
	case 'j':
	  interf2=atoi(optarg);
	  break;
	case 'n':
	  n_frames = atoi(optarg);
	  break;
	case 's':
	  snr0 = atof(optarg);
	  msg("Setting SNR0 to %f\n",snr0);
	  break;
	case 'S':
	  snr1 = atof(optarg);
	  snr1set=1;
	  msg("Setting SNR1 to %f\n",snr1);
	  break;
	  /*
	case 't':
	  Td= atof(optarg);
	  break;
	  */
	case 'e':
	  extended_prefix_flag=1;
	  break;
	  
	case 'r':
	  n_rnti=atoi(optarg);
	  break;
	  
	case 'x':
	  transmission_mode=atoi(optarg);
	  if ((transmission_mode!=1) &&
	      (transmission_mode!=2) &&
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
	case 'A':
	  abstraction_flag=1;
	  ntrials=10000;
	  msg("Running Abstraction test\n");
	  pbch_file_fd=fopen(optarg,"r");
	  if (pbch_file_fd==NULL) {
	    printf("Problem with filename %s\n",optarg);
	    exit(-1);
	  }
	  break;
	case 'D':
	  frame_type=0;
	  msg("Running in FDD\n");
	  break;
	case 'N':
	  Nid_cell = atoi(optarg);
	  break;
	case 'R':
	  N_RB_DL = atoi(optarg);
	  break;
	case 'O':
	  osf = atoi(optarg);
	  break;
	case 'Z':
	  oai_hw_input = 1;
	  break;
	case 'Y':
	  oai_hw_output = 1;
	  break;
	case 'F':
	  input_fd = fopen(optarg,"r");
	  if (input_fd==NULL) {
	    printf("Problem with filename %s\n",optarg);
	    exit(-1);
	  }
	  break;
	case 'T':
	  tcxo = atoi(optarg);
	  break;
	default:
	case 'h':
	  printf("-h This message\n");
	  printf("-a Use AWGN channel and not multipath\n");
	  printf("-e Use extended prefix mode\n");
	  printf("-D Use FDD frame\n");
	  printf("-n Number of frames to simulate\n");
	  printf("-r RNTI for DCI detection in SF 0/5\n");
	  printf("-s Starting SNR, runs from SNR0 to SNR0 + 5 dB.  If n_frames is 1 then just SNR is simulated\n");
	  printf("-S Ending SNR, runs from SNR0 to SNR1\n");
	  printf("-t Delay spread for multipath channel\n");
	  printf("-g [A,B,C,D,E,F,G] Use 3GPP SCM (A,B,C,D) or 36-101 (E-EPA,F-EVA,G-ETU) models (ignores delay spread and Ricean factor)\n");
	  printf("-x Transmission mode (1,2,6 for the moment)\n");
	  printf("-y Number of TX antennas used in eNB\n");
	  printf("-z Number of RX antennas used in UE\n");
	  printf("-i Relative strength of first intefering eNB (in dB) - cell_id mod 3 = 1\n");
	  printf("-j Relative strength of second intefering eNB (in dB) - cell_id mod 3 = 2\n");
	  printf("-N Nid_cell\n");
	  printf("-R N_RB_DL\n");
	  printf("-O oversampling factor (1,2,4,8,16)\n");
	  printf("-A Interpolation_filname Run with Abstraction to generate Scatter plot using interpolation polynomial in file\n");
	  printf("-f Output filename (.txt format) for Pe/SNR results\n");
	  printf("-F Input filename (.txt format) for RX conformance testing\n");
	  printf("-Y just generate tx frame and send it to hardware\n");
	  printf("-Z grab frame from hardware and do rx processing\n");
	  printf("-T set TCXO parameter on hardware\n");
	  exit (-1);
	  break;
	}
    }

  if (transmission_mode==2)
    n_tx=2;

  lte_param_init(n_tx,n_rx,transmission_mode,extended_prefix_flag,
		 frame_type,Nid_cell,N_RB_DL,osf);

  if (snr1set==0) {
    if (n_frames==1)
      snr1 = snr0+.1;
    else
      snr1 = snr0+5.0;
  }

  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  frame_parms = &PHY_vars_eNB->lte_frame_parms;

  if (oai_hw_input == 1)
    openair_fd=setup_oai_hw(frame_parms,PHY_vars_UE,NULL);

  if (oai_hw_output == 1)
    openair_fd=setup_oai_hw(frame_parms,NULL,PHY_vars_eNB);

  if ((oai_hw_input==1) ||
      (oai_hw_output==1)) {
    msg("setting TCXO to %d\n",tcxo);

    ioctl(openair_fd,openair_SET_TCXO_DAC,(void *)&tcxo);
  }

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
  txdata = PHY_vars_eNB->lte_eNB_common_vars.txdata[eNb_id];
  txdata1 = PHY_vars_eNB1->lte_eNB_common_vars.txdata[eNb_id];
  txdata2 = PHY_vars_eNB2->lte_eNB_common_vars.txdata[eNb_id];
#endif
  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  s_re1 = malloc(2*sizeof(double*));
  s_im1 = malloc(2*sizeof(double*));
  s_re2 = malloc(2*sizeof(double*));
  s_im2 = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));
  r_re1 = malloc(2*sizeof(double*));
  r_im1 = malloc(2*sizeof(double*));
  r_re2 = malloc(2*sizeof(double*));
  r_im2 = malloc(2*sizeof(double*));

  nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

  printf("FFT Size %d, Extended Prefix %d, Samples per subframe %d, Symbols per subframe %d\n",NUMBER_OF_OFDM_CARRIERS,
	 frame_parms->Ncp,frame_parms->samples_per_tti,nsymb);


  eNB2UE = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				channel_model,
				BW,
				0,
				0,
				0);

  if (interf1>-20)
    eNB2UE1 = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				   PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				   channel_model,
				   BW,
				   0,
				   0,
				   0);
  
  if (interf2>-20)
    eNB2UE2 = new_channel_desc_scm(PHY_vars_eNB->lte_frame_parms.nb_antennas_tx,
				   PHY_vars_UE->lte_frame_parms.nb_antennas_rx,
				   channel_model,
				   BW,
				   0,
				   0,
				   0);
  

  if (eNB2UE==NULL) {
    msg("Problem generating channel model. Exiting.\n");
    exit(-1);
  }

  for (i=0;i<2;i++) {

    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_re1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));    
    s_re2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_re2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(s_im2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));

    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im1[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im1[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_re2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im2[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    bzero(r_im2[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }

  for (i=0;i<2;i++) {
    PHY_vars_UE->dlsch_ue[0][i]  = new_ue_dlsch(1,8,0);
    if (!PHY_vars_UE->dlsch_ue[0][i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }    
  }

  //  if (pbch_file_fd!=NULL) {
  //    load_pbch_desc(pbch_file_fd);
  //  }

  
  if ((input_fd==NULL)&&(oai_hw_input==0)) {

    //    for (i=0;i<6;i++)
    //      pbch_pdu[i] = i;
    //pbch_pdu[0]=100;
    //pbch_pdu[1]=1;
    //pbch_pdu[2]=0;
    ((u8*) pbch_pdu)[0] = 0;
    switch (PHY_vars_eNB->lte_frame_parms.N_RB_DL) {
    case 6:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (0<<5);
      break;
    case 15:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (1<<5);
      break;
    case 25:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (2<<5);
      break;
    case 50:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (3<<5);
      break;
    case 100:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (4<<5);
      break;
    default:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0x1f) | (2<<5);
      break;
    }
    ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xef) | 
      ((PHY_vars_eNB->lte_frame_parms.phich_config_common.phich_duration << 4)&0x10);
    
    switch (PHY_vars_eNB->lte_frame_parms.phich_config_common.phich_resource) {
    case oneSixth:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (0<<3);
      break;
    case half:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (1<<3);
      break;
    case one:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (2<<3);
      break;
    case two:
      ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xf3) | (3<<3);
      break;
    default:
      break;
    }
    
    ((u8*) pbch_pdu)[0] = (((u8*) pbch_pdu)[0]&0xfc) | ((PHY_vars_eNB->frame>>8)&0x3);
    ((u8*) pbch_pdu)[1] = PHY_vars_eNB->frame&0xfc;
    ((u8*) pbch_pdu)[2] = 0;
    
    if (PHY_vars_eNB->lte_frame_parms.frame_type == 1) {
      generate_pss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   2,
		   2);
      generate_pss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   2,
		   12);
      generate_sss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		   1);
      generate_sss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		   11);
    }
    else {
      generate_pss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		   0);
      generate_sss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 5 : 4,
		   0);
      generate_pss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 6 : 5,
		   10);
      generate_sss(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB->lte_frame_parms,
		   (PHY_vars_eNB->lte_frame_parms.Ncp==0) ? 5 : 4,
		   10);
    }


    
    printf("Generating PBCH for mode1_flag = %d, frame_type %d\n", PHY_vars_eNB->lte_frame_parms.mode1_flag,PHY_vars_eNB->lte_frame_parms.frame_type);
    
    
    generate_pilots(PHY_vars_eNB,
		    PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		    AMP,
		    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);


        
   

 
    generate_pbch(&PHY_vars_eNB->lte_eNB_pbch,
		  PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		  AMP,
		  &PHY_vars_eNB->lte_frame_parms,
		  pbch_pdu,
		  0);
    /*
    generate_pbch(PHY_vars_eNB->lte_eNB_common_vars.txdataF[0],
		  AMP,
		  &PHY_vars_eNB->lte_frame_parms,
		  pbch_pdu,
		  3);
    */

    if (interf1>-20) {
      generate_pss(PHY_vars_eNB1->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB1->lte_frame_parms,
		   (PHY_vars_eNB1->lte_frame_parms.Ncp==0)?6:5,
		   0);
      
      
      
      
      
      generate_pilots(PHY_vars_eNB1,
		      PHY_vars_eNB1->lte_eNB_common_vars.txdataF[0],
		      AMP,
		      2);//LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
      
      
      generate_pbch(&PHY_vars_eNB1->lte_eNB_pbch,
		    PHY_vars_eNB1->lte_eNB_common_vars.txdataF[0],
		    AMP,
		    &PHY_vars_eNB1->lte_frame_parms,
		    pbch_pdu,
		    0);
      
    }
    
    if (interf2>-20) {
      generate_pss(PHY_vars_eNB2->lte_eNB_common_vars.txdataF[0],
		   AMP,
		   &PHY_vars_eNB2->lte_frame_parms,
		   (PHY_vars_eNB2->lte_frame_parms.Ncp==0)?6:5,
		   0);
      
      
      
      
      
      generate_pilots(PHY_vars_eNB2,
		      PHY_vars_eNB2->lte_eNB_common_vars.txdataF[0],
		      AMP,
		      LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
      
      
      generate_pbch(&PHY_vars_eNB2->lte_eNB_pbch,
		    PHY_vars_eNB2->lte_eNB_common_vars.txdataF[0],
		    AMP,
		    &PHY_vars_eNB2->lte_frame_parms,
		    pbch_pdu,
		    0);
      
    }
    
    // Generate two PDCCH

    if (frame_type == 0) {
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->rah             = 0;
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->rballoc         = DLSCH_RB_ALLOC;
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->TPC             = 0;
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->harq_pid        = 0;
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->mcs             = 0;  
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->ndi             = 1;
      ((DCI1_5MHz_FDD_t*)&DLSCH_alloc_pdu)->rv              = 0;
      dci_alloc[0].dci_length = sizeof_DCI1_5MHz_FDD_t;
      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu,sizeof(DCI1_5MHz_FDD_t));
    }
    else {
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->rah             = 0;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->rballoc         = DLSCH_RB_ALLOC;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->TPC             = 0;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->dai             = 0;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->harq_pid        = 0;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->mcs             = 0;  
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->ndi             = 1;
      ((DCI1_5MHz_TDD_t*)&DLSCH_alloc_pdu)->rv              = 0;
      dci_alloc[0].dci_length = sizeof_DCI1_5MHz_TDD_t;
      memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu,sizeof(DCI1_5MHz_TDD_t));
    }
    dci_alloc[0].L          = 2;
    dci_alloc[0].rnti       = n_rnti;
    dci_alloc[0].format     = format1;

    num_pdcch_symbols = generate_dci_top(1,
					 0,
					 dci_alloc,
					 0,
					 AMP,
					 &PHY_vars_eNB->lte_frame_parms,
					 PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id],
					 0);

    num_pdcch_symbols = generate_dci_top(1,
					 0,
					 dci_alloc,
					 0,
					 AMP,
					 &PHY_vars_eNB->lte_frame_parms,
					 PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id],
					 5);
    
    //  write_output("pilotsF.m","rsF",txdataF[0],lte_frame_parms->ofdm_symbol_size,1,1);
#ifdef IFFT_FPGA
    write_output("txsigF0.m","txsF0", PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][0],300*120,1,4);
    if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF1.m","txsF1", PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][1],300*120,1,4);
    
    for (i=0;i<10;i++) 
      debug_msg("%08x\n",((unsigned int*)&PHY_vars_eNB->lte_eNB_common_vars.txdataF[0][0][1*(PHY_vars_eNB->lte_frame_parms.N_RB_DL*12)*(PHY_vars_eNB->lte_frame_parms.symbols_per_tti>>1)])[i]);
    
    
    // do talbe lookup and write results to txdataF2
    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
      l = 0;
      for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	if ((i%512>=1) && (i%512<=150))
	  txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][aa][l++]];
	else if (i%512>=362)
	  txdataF2[aa][i] = ((int*)mod_table)[PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][aa][l++]];
	else 
	  txdataF2[aa][i] = 0;
      //printf("l=%d\n",l);
    }
    
    write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF21.m","txsF21", txdataF2[1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    
    
    tx_lev=0;
    
    for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
      
      if (frame_parms->Ncp == 1) 
	PHY_ofdm_mod(txdataF2[aa],        // input
		     txdata[aa],         // output
		     PHY_vars_eNB->lte_frame_parms.log2_symbol_size,                // log2_fft_size
		     LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		     PHY_vars_eNB->lte_frame_parms.nb_prefix_samples,               // number of prefix samples
		     PHY_vars_eNB->lte_frame_parms.twiddle_ifft,  // IFFT twiddle factors
		     PHY_vars_eNB->lte_frame_parms.rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      else {
	normal_prefix_mod(txdataF2[aa],txdata[aa],LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,frame_parms);
      }
      tx_lev += signal_energy(&txdata[aa][0],
			      OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    }
#else
    write_output("txsigF0.m","txsF0", PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
      write_output("txsigF1.m","txsF1", PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
    
    tx_lev = 0;
    
    
    
    
    for (aa=0; aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx; aa++) {
      if (frame_parms->Ncp == 1) 
	PHY_ofdm_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		     txdata[aa],         // output
		     frame_parms->log2_symbol_size,                // log2_fft_size
		     LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		     frame_parms->nb_prefix_samples,               // number of prefix samples
		     frame_parms->twiddle_ifft,  // IFFT twiddle factors
		     frame_parms->rev,           // bit-reversal permutation
		     CYCLIC_PREFIX);
      else {
	normal_prefix_mod(PHY_vars_eNB->lte_eNB_common_vars.txdataF[eNb_id][aa],
			  txdata[aa],
			  LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,
			  frame_parms);
      }
      
      tx_lev += signal_energy(&txdata[aa][0],
			      OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    }
    
    if (interf1>-20) {
      for (aa=0; aa<PHY_vars_eNB1->lte_frame_parms.nb_antennas_tx; aa++) {
	if (frame_parms->Ncp == 1) 
	  PHY_ofdm_mod(PHY_vars_eNB1->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		       PHY_vars_eNB1->lte_eNB_common_vars.txdata[eNb_id][aa],         // output
		       frame_parms->log2_symbol_size,                // log2_fft_size
		       LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		       frame_parms->nb_prefix_samples,               // number of prefix samples
		       frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	else {
	  normal_prefix_mod(PHY_vars_eNB1->lte_eNB_common_vars.txdataF[eNb_id][aa],
			    PHY_vars_eNB1->lte_eNB_common_vars.txdata[eNb_id][aa],
			    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,
			    frame_parms);
	}
      }
    }
    
    if (interf2>-20) {
      for (aa=0; aa<PHY_vars_eNB2->lte_frame_parms.nb_antennas_tx; aa++) {
	if (frame_parms->Ncp == 1) 
	  PHY_ofdm_mod(PHY_vars_eNB2->lte_eNB_common_vars.txdataF[eNb_id][aa],        // input,
		       PHY_vars_eNB2->lte_eNB_common_vars.txdata[eNb_id][aa],         // output
		       frame_parms->log2_symbol_size,                // log2_fft_size
		       LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,                 // number of symbols
		       frame_parms->nb_prefix_samples,               // number of prefix samples
		       frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	else {
	  normal_prefix_mod(PHY_vars_eNB2->lte_eNB_common_vars.txdataF[eNb_id][aa],
			    PHY_vars_eNB2->lte_eNB_common_vars.txdata[eNb_id][aa],
			    LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,
			    frame_parms);
	}
      }
    }
#endif
    
    
    
    write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    if (frame_parms->nb_antennas_tx>1)
      write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
  }
  else if ((oai_hw_input==0)&&(oai_hw_output==0)){  //read in from file
    i=0;
    while (!feof(input_fd)) {
      fscanf(input_fd,"%s %s",input_val_str,input_val_str2);//&input_val1,&input_val2);
      /*      
      if ((i%4)==0) {
	((short*)txdata[0])[i/2] = (short)((1<<15)*strtod(input_val_str,NULL));
	((short*)txdata[0])[(i/2)+1] = (short)((1<<15)*strtod(input_val_str2,NULL));
	if ((i/4)<100)
	  printf("sample %d => %e + j%e (%d +j%d)\n",i/4,strtod(input_val_str,NULL),strtod(input_val_str2,NULL),((short*)txdata[0])[i/4],((short*)txdata[0])[(i/4)+1]);//1,input_val2,);
      }
      */
      ((short*)txdata[0])[i<<1] = 16*(short)(strtod(input_val_str,NULL));
      ((short*)txdata[0])[(i<<1)+1] = 16*(short)(strtod(input_val_str2,NULL));

      i++;
      if (i==(FRAME_LENGTH_COMPLEX_SAMPLES))
	break;
    }
    printf("Read in %d samples (%d)\n",i,FRAME_LENGTH_COMPLEX_SAMPLES);
    write_output("txsig0.m","txs0", txdata[0],10*frame_parms->samples_per_tti,1,1);
    //    write_output("txsig1.m","txs1", txdata[1],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
    tx_lev = signal_energy(&txdata[0][0],
			   OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
  }
  else { // get from OAI HW
    // set check if we have to set up a signal generator here


    printf("Doing Acquisition from OAI HW\n");
    snr0=snr1-.1; 
  }
  foff = deltaF/(PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*15e3);
  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
    cs = cos(2*M_PI*foff*i);
    sn = sin(2*M_PI*foff*i);
    for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;aa++) {
      
      tmp_re =  (double)((short*)txdata[aa])[(i<<1)]*cs - (double)((short*)txdata[aa])[1+(i<<1)]*sn;
      
      tmp_im =  (double)((short*)txdata[aa])[1+(i<<1)]*cs + (double)((short*)txdata[aa])[(i<<1)]*sn;
      
      if (awgn_flag == 0) {
	
	s_re[aa][i] = tmp_re;
	s_im[aa][i] = tmp_im;
	if (interf1>-20) {
	  s_re1[aa][i] = ((double)(((short *)txdata1[aa]))[(i<<1)]);
	  s_im1[aa][i] = ((double)(((short *)txdata1[aa]))[(i<<1)+1]);
	}
	if (interf2>-20) {
	  s_re2[aa][i] = ((double)(((short *)txdata2[aa]))[(i<<1)]);
	  s_im2[aa][i] = ((double)(((short *)txdata2[aa]))[(i<<1)+1]);
	}
	
     }
     else {
       for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
	 if (aa==0) {
	   r_re[aarx][i] = tmp_re;//((double)(((short *)txdata[aa]))[(i<<1)]);
	   r_im[aarx][i] = tmp_im;//((double)(((short *)txdata[aa]))[(i<<1)+1]);
	 }
	 else {
	   r_re[aarx][i] += tmp_re;//((double)(((short *)txdata[aa]))[(i<<1)]);
	   r_im[aarx][i] += tmp_im;//((double)(((short *)txdata[aa]))[(i<<1)+1]);
	 }
	 /*	  	  
			  if (interf1>=-20) {
			  r_re[aarx][i]+= pow(10.0,.05*interf1)*((double)(((short *)PHY_vars_eNB1->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)]);
			  r_im[aarx][i]+= pow(10.0,.05*interf1)*((double)(((short *)PHY_vars_eNB1->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)+1]);
			  
			  }
			  if (interf2>=-20) {
			  r_re[aarx][i]+=pow(10.0,.05*interf2)*((double)(((short *)PHY_vars_eNB2->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)]);
			  r_im[aarx][i]+=pow(10.0,.05*interf2)*((double)(((short *)PHY_vars_eNB2->lte_eNB_common_vars.txdata[eNb_id][aa]))[(i<<1)+1]);
			  }
	 */
       }
     }
   }
  }

  if (oai_hw_output==0) {
    for (SNR=snr0;SNR<snr1;SNR+=.2) {
      
      
      n_errors = 0;
      n_errors2 = 0;
      n_alamouti = 0;
      for (trial=0; trial<n_frames; trial++) {
	pbch_sinr=0.0;
	if (abstraction_flag==1)
	  printf("*********************** trial %d ***************************\n",trial);
	
	while (pbch_sinr>-2.0) {
	  
	  if (awgn_flag == 0) {	
	    
	    
	    multipath_channel(eNB2UE,s_re,s_im,r_re,r_im,
			      FRAME_LENGTH_COMPLEX_SAMPLES,0);//LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	    
	    if (interf1>-20) 
	      multipath_channel(eNB2UE1,s_re1,s_im1,r_re1,r_im1,
				LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	    
	    if (interf2>-20) 
	      multipath_channel(eNB2UE2,s_re2,s_im2,r_re2,r_im2,
				LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0);
	    
	    if (abstraction_flag == 1) {
	      freq_channel(eNB2UE,25,51);
	      if (interf1>-20) 
		freq_channel(eNB2UE1,25,51);
	      if (interf2>-20) 
		freq_channel(eNB2UE2,25,51);
	      pbch_sinr = compute_pbch_sinr(eNB2UE,eNB2UE1,eNB2UE2,SNR,SNR+interf1,SNR+interf2,25);
	      printf("total_sinr %f\n",compute_sinr(eNB2UE,eNB2UE1,eNB2UE2,SNR,SNR+interf1,SNR+interf2,25));
	      printf("pbch_sinr %f => BLER %f\n",pbch_sinr,pbch_bler(pbch_sinr));
	    }
	    else {
	      pbch_sinr = -3.0;
	    }
	    //	  exit(-1);
	  } // awgn_flag
	  else {
	    pbch_sinr = -3.0;
	  }
	}
	
	sigma2_dB = 10*log10((double)tx_lev) +10*log10(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size/(12*NB_RB)) - SNR;
	if (n_frames==1)
	  printf("sigma2_dB %f (SNR %f dB) tx_lev_dB %f\n",sigma2_dB,SNR,10*log10((double)tx_lev));
	//AWGN
	sigma2 = pow(10,sigma2_dB/10);
        
	/*    
	      if (n_frames==1) {
	      printf("rx_level data symbol %f, tx_lev %f\n",
	      10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,0)),
	      10*log10(tx_lev));
	      }
	*/
	
	for (n_trials=0;n_trials<ntrials;n_trials++) {
	  
	  if (oai_hw_input==0) {
	    iout = 0;//taus()%(FRAME_LENGTH_COMPLEX_SAMPLES>>2);
	    for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) { //nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	      for (aa=0;aa<PHY_vars_eNB->lte_frame_parms.nb_antennas_rx;aa++) {
		if (n_trials==0) {
		  r_re[aa][i] += (pow(10.0,.05*interf1)*r_re1[aa][i] + pow(10.0,.05*interf2)*r_re2[aa][i]);
		  r_im[aa][i] += (pow(10.0,.05*interf1)*r_im1[aa][i] + pow(10.0,.05*interf2)*r_im2[aa][i]);
		}
		
		((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[2*i] = (short) (.167*(r_re[aa][i] +sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
		((short*) PHY_vars_UE->lte_ue_common_vars.rxdata[aa])[(2*i)+1] = (short) (.167*(r_im[aa][i] + (iqim*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0)));
	      }
	      iout++;
	      if (iout==FRAME_LENGTH_COMPLEX_SAMPLES)
		iout=0;
	    }    
	  }
	  else {
	    fc=0;
	    ioctl(openair_fd,openair_GET_BUFFER,(void *)&fc);
	    sleep(1);   
	  }
	  /*
	    if (n_trials==0) {
	    printf("rx_level data symbol %f\n",
	    10*log10(signal_energy(PHY_vars_UE->lte_ue_common_vars.rxdata[0],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)));
	    }
	  */
	  //	printf("Calling initial_sync\n");
	  
	  if (initial_sync(PHY_vars_UE)==0) {

	    // Do DCI
	    PHY_vars_UE->lte_frame_parms.N_RB_DL=N_RB_DL;
	    PHY_vars_UE->lte_frame_parms.phich_config_common.phich_duration=0;
	    PHY_vars_UE->lte_frame_parms.phich_config_common.phich_resource = oneSixth;
	    generate_pcfich_reg_mapping(&PHY_vars_UE->lte_frame_parms);
	    generate_phich_reg_mapping(&PHY_vars_UE->lte_frame_parms);
	    
	    for (l=0;l<(1+((PHY_vars_UE->lte_frame_parms.Ncp==0)?4:3));l++) {
	      printf("pdcch %d\n",l);
	      slot_fep(PHY_vars_UE,
		       l,
		       0,
		       PHY_vars_UE->rx_offset,
		       0);
	    }

	    
	    PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti = n_rnti;
	    
	    printf("Fine Frequency offset %d\n",PHY_vars_UE->lte_ue_common_vars.freq_offset);
	    printf("Doing PDCCH RX : num_pdcch_symbols at TX %d\n",num_pdcch_symbols);
	    rx_pdcch(&PHY_vars_UE->lte_ue_common_vars,
		     PHY_vars_UE->lte_ue_pdcch_vars,
		     &PHY_vars_UE->lte_frame_parms,
		     0,
		     0,
		     (PHY_vars_UE->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
		     PHY_vars_UE->is_secondary_ue); 
	    printf("Got PCFICH for %d pdcch symbols\n",PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols);

	    dci_cnt = dci_decoding_procedure(PHY_vars_UE,
					     dci_alloc_rx,
					     0,0);
	    printf("Found %d DCIs\n",dci_cnt);

	    generate_ue_dlsch_params_from_dci(0,
					      (void *)&dci_alloc_rx[0].dci_pdu,
					      PHY_vars_UE->lte_ue_pdcch_vars[0]->crnti,
					      dci_alloc_rx[0].format,
					      PHY_vars_UE->dlsch_ue[0],
					      &PHY_vars_UE->lte_frame_parms,
					      SI_RNTI,
					      0,
					      P_RNTI);
	    // overwrite some values until source is sure
	    PHY_vars_UE->dlsch_ue[0][0]->nb_rb = 25;
	    PHY_vars_UE->dlsch_ue[0][0]->rb_alloc[0] = 0x1ffffff;
	    PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->Ndi = 1;
	    PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->mcs = 20;

	    dump_dci(&PHY_vars_UE->lte_frame_parms, &dci_alloc_rx[0]);
	    for (l=PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols;
		 l<(((PHY_vars_UE->lte_frame_parms.Ncp==0)?4:3));
		 l++) {
	      rx_pdsch(PHY_vars_UE,
		       PDSCH,
		       0,
		       3,
		       0,  // subframe,
		       l,  // symbol
		       (l==PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols)?1:0,   // first_symbol_flag
		       0,  // dual stream
		       0);
	    }
	    for (l=1+(PHY_vars_UE->lte_frame_parms.Ncp==0)?4:3 ; 
		 l<((PHY_vars_UE->lte_frame_parms.Ncp==0)?7:6);
		 l++) {
	      slot_fep(PHY_vars_UE,
		       l,
		       0,
		       PHY_vars_UE->rx_offset,
		       0);
	    }
	    for (l=(PHY_vars_UE->lte_frame_parms.Ncp==0)?4:3 ;
		 l<((PHY_vars_UE->lte_frame_parms.Ncp==0)?7:6);
		 l++) {
	      rx_pdsch(PHY_vars_UE,
		       PDSCH,
		       0,
		       3,
		       0,  // subframe,
		       l,  // symbol
		       0,   // first_symbol_flag
		       0,  // dual stream
		       0);
	    }
	    for (l=0 ; 
		 l<((PHY_vars_UE->lte_frame_parms.Ncp==0)?7:6);
		 l++) {
	      slot_fep(PHY_vars_UE,
		       l,
		       1,   //slot 1
		       PHY_vars_UE->rx_offset,
		       0);
	    }
	    slot_fep(PHY_vars_UE,
		     0,
		     2,   //slot 1
		     PHY_vars_UE->rx_offset,
		     0);

	    for (l=(PHY_vars_UE->lte_frame_parms.Ncp==0)?7:6 ;
		 l<((PHY_vars_UE->lte_frame_parms.Ncp==0)?14:12);
		 l++) {
	      rx_pdsch(PHY_vars_UE,
		       PDSCH,
		       0,
		       3,
		       0,  // subframe,
		       l,  // symbol
		       0,   // first_symbol_flag
		       0,  // dual stream
		       0);
	    }
	  }
	  else {
	    if (PHY_vars_UE->lte_frame_parms.Nid_cell !=  Nid_cell)
	      n_errors2++;
	    else
	      n_errors++;

	  // avoid ra_RNTI=0

	    //	  msg("pbch error\n");
	  }
	  
 	  
#ifdef XFORMS
	  do_forms2(form_dl,
		    frame_parms,  
		    PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates_time,
		    PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0],
		    PHY_vars_UE->lte_ue_common_vars.rxdata,
		    PHY_vars_UE->lte_ue_common_vars.rxdataF,
		    PHY_vars_UE->lte_ue_pdcch_vars[0]->rxdataF_comp[0],
		    PHY_vars_UE->lte_ue_pdsch_vars[0]->rxdataF_comp[0],
		    PHY_vars_UE->lte_ue_pdsch_vars[3]->rxdataF_comp[0],
		    PHY_vars_UE->lte_ue_pdsch_vars[0]->llr[0],
		    PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_comp[0],
		    PHY_vars_UE->lte_ue_pbch_vars[0]->llr,
		    1920);
#endif
	  
	  
	  
	} //noise trials
	if (abstraction_flag==1) {
	  printf("SNR %f : n_errors = %d/%d, n_alamouti %d\n", SNR,n_errors,ntrials,n_alamouti);
	  if (write_output_file==1)
	    fprintf(output_fd,"%f %f %e %e\n",SNR,pbch_sinr,(double)n_errors/ntrials,pbch_bler(pbch_sinr));
	}
	n_errors=0;
	if ((abstraction_flag==0) && (n_errors2>1000) && (trial>5000))
	  break;
      } // trials
      if (abstraction_flag==0) {
	printf("SNR %f : PSS/SSS errors %d/%d (Perror %e) PBCH errors = %d/%d (BLER %e), n_alamouti %d\n", 
	       SNR,
	       n_errors2,ntrials*(1+trial),(double)n_errors2/(ntrials*(1+trial)),
	       n_errors,(ntrials*(1+trial)-n_errors2),(double)n_errors/(ntrials*(1+trial)-n_errors2),
	       n_alamouti);
	if (write_output_file==1)
	  fprintf(output_fd,"%f %e\n",SNR,(double)n_errors2/(ntrials*(1+trial)));
      }
    } // NSR
    
    if (n_frames==1) {
      
      write_output("H00.m","h00",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size),1,1);
      if (n_tx==2)
	write_output("H10.m","h10",&(PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][2][0]),((frame_parms->Ncp==0)?7:6)*(PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size),1,1);
      write_output("rxsig0.m","rxs0", PHY_vars_UE->lte_ue_common_vars.rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
      write_output("rxsigF0.m","rxsF0", PHY_vars_UE->lte_ue_common_vars.rxdataF[0],NUMBER_OF_OFDM_CARRIERS*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*nsymb,2,1);    
      write_output("PBCH_rxF0_ext.m","pbch0_ext",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_ext[0],12*4*6,1,1);
      write_output("PBCH_rxF0_comp.m","pbch0_comp",PHY_vars_UE->lte_ue_pbch_vars[0]->rxdataF_comp[0],12*4*6,1,1);
      write_output("PBCH_rxF_llr.m","pbch_llr",PHY_vars_UE->lte_ue_pbch_vars[0]->llr,(frame_parms->Ncp==0) ? 1920 : 1728,1,4);
      write_output("pdcch_rxF_ext0.m","pdcch_rxF_ext0",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_ext[0],3*300,1,1); 
      write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->rxdataF_comp[0],4*300,1,1);
      write_output("pdcch_rxF_llr.m","pdcch_llr",PHY_vars_UE->lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);    

      coded_bits_per_codeword = get_G(&PHY_vars_UE->lte_frame_parms,
				      PHY_vars_UE->dlsch_ue[0][0]->nb_rb,
				      PHY_vars_UE->dlsch_ue[0][0]->rb_alloc,
				      get_Qm(PHY_vars_UE->dlsch_ue[0][0]->harq_processes[0]->mcs),
				      PHY_vars_UE->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
				      0);

      dump_dlsch2(PHY_vars_UE,0,coded_bits_per_codeword);
      
    }
  }
  else {
    printf("Sending frame to OAI HW\n");
    temp[0] = 110;
    temp[1] = 110;
    temp[2] = 110;
    temp[3] = 110;
    ioctl(openair_fd,openair_SET_TX_GAIN,(void *)&temp[0]);
    ioctl(openair_fd,openair_START_TX_SIG,(void *)NULL);
  }

#ifdef XFORMS
  fl_hide_form(form_dl);
  //fl_free_form(form_dl);
#endif


#ifdef IFFT_FPGA
  free(txdataF2[0]);
  free(txdataF2[1]);
  free(txdataF2);
  free(txdata[0]);
  free(txdata[1]);
  free(txdata);
#endif 

  for (i=0;i<2;i++) {
    printf("Freeing s_re[%d]\n",i);
    free(s_re[i]);
    printf("Freeing s_im[%d]\n",i);
    free(s_im[i]);
    printf("Freeing r_re[%d]\n",i);
    free(r_re[i]);
    printf("Freeing r_im[%d]\n",i);
    free(r_im[i]);
  }
  printf("Freeing s_re\n");
  free(s_re);
  printf("Freeing s_im\n");
  free(s_im);
  printf("Freeing r_re\n");
  free(r_re);
  printf("Freeing r_im\n");
  free(r_im);


  lte_sync_time_free();

  if (write_output_file)
    fclose(output_fd);

  if ((oai_hw_input==1)||
      (oai_hw_output==1)){
    close(openair_fd);
  }

  return(n_errors);

}
   


/*  
  for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
	   (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
	   12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/

