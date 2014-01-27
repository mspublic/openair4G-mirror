#include <string.h>
#include <math.h>
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

#define BW 7.68
#define AWGN
#define NO_DCI

//#define OUTPUT_DEBUG 1
#define N_TRIALS 10000

#define RBmask0 0x00fc00fc
#define RBmask1 0x0
#define RBmask2 0x0
#define RBmask3 0x0

unsigned char dlsch_cqi;
unsigned char current_dlsch_cqi=7;

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode) {

  unsigned int ind;

  printf("Start lte_param_init\n");
  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  

  lte_frame_parms = &(PHY_config->lte_frame_parms);   //openair1/PHY/impl_defs_lte.h
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars[0]);
  lte_ue_pdcch_vars = &(PHY_vars->lte_ue_pdcch_vars[0]);
  lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars[0]);
  lte_eNB_common_vars = &(PHY_vars->lte_eNB_common_vars);
  lte_eNB_ulsch_vars = &(PHY_vars->lte_eNB_ulsch_vars[0]);
  lte_ue_dlsch_vars_cntl = &PHY_vars->lte_ue_dlsch_vars_cntl[0];
  lte_ue_dlsch_vars_ra   = &PHY_vars->lte_ue_dlsch_vars_ra[0];
  lte_ue_dlsch_vars_1A   = &PHY_vars->lte_ue_dlsch_vars_1A[0];

  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = 25;   
  lte_frame_parms->Ng_times6          = 1;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 4;
  lte_frame_parms->num_dlsch_symbols  = 6;
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(N_tx); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  lte_gold(lte_frame_parms);
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();

  generate_pcfich_reg_mapping(lte_frame_parms);
  generate_phich_reg_mapping_ext(lte_frame_parms);

  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars,lte_ue_dlsch_vars_cntl,lte_ue_dlsch_vars_ra,lte_ue_dlsch_vars_1A,lte_ue_pbch_vars,lte_ue_pdcch_vars);
  phy_init_lte_eNB(lte_frame_parms,lte_eNB_common_vars,lte_eNB_ulsch_vars);

  
  printf("Done lte_param_init\n");


}

DCI0_5MHz_TDD0_t          UL_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

#define UL_RB_ALLOC 0x1ff;
#define CCCH_RB_ALLOC computeRIV(lte_frame_parms->N_RB_UL,0,2)
#define DLSCH_RB_ALLOC 0x1fbf // igore DC component,RB13


int main(int argc, char **argv) {

  int i,aa,s,ind,Kr,Kr_bytes;;
  double sigma2, sigma2_dB=10,SNR,snr0=-2.0,snr1,SNRmeas,rate;
  //int **txdataF, **txdata;
  int **txdata;
#ifdef IFFT_FPGA
  int **txdataF2;
#endif
  //LTE_DL_FRAME_PARMS *frame_parms = (LTE_DL_FRAME_PARMS *)malloc(sizeof(LTE_DL_FRAME_PARMS));
  //LTE_UE_COMMON      *lte_ue_common_vars = (LTE_UE_COMMON *)malloc(sizeof(LTE_UE_COMMON));
  double **s_re,**s_im,**r_re,**r_im;
  double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  double aoa=.03,ricean_factor=0.5,Td=1.0,iqimb = 0;
  int channel_length;
  struct complex **ch;

  int eNb_id = 0, eNb_id_i = 1;
  unsigned char mcs,dual_stream_UE = 0;
  unsigned short NB_RB=conv_nprb(0,DLSCH_RB_ALLOC);
  unsigned char Ns,l,m;


  unsigned char *input_data,*decoded_output;

  unsigned char *input_buffer;
  unsigned short input_buffer_length;
  unsigned int ret;
  unsigned int coded_bits_per_codeword,nsymb,dci_cnt;

  unsigned int tx_lev,tx_lev_dB,trials,errs[4]={0,0,0,0},round_trials[4]={0,0,0,0},dci_errors=0,dlsch_active=0,num_layers;
  int re_allocated;
  FILE *bler_fd;
  char bler_fname[256];

  unsigned char pbch_pdu[6];

  DCI_ALLOC_t dci_alloc[8],dci_alloc_rx[8];

  FILE *rx_frame_file;
  int result;
  int round;

  dci_alloc[0].dci_length = sizeof_DCI0_5MHz_TDD_0_t;

  channel_length = (int) 11+2*BW*Td;

  lte_param_init(1,1,1);

  num_layers = 1;

  printf("argc %d\n",argc);
  if (argc<2) {
    mcs = 0;
    printf("Setting mcs = %d\n",mcs);
  }
  else if (argc>=2){
    mcs = atoi(argv[1]);
    printf("Setting mcs to %d\n",mcs);
  }

  printf("NPRB = %d\n",NB_RB);

  /*
    txdataF    = (int **)malloc16(2*sizeof(int*));
    txdataF[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    txdataF[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  
    txdata    = (int **)malloc16(2*sizeof(int*));
    txdata[0] = (int *)malloc16(FRAME_LENGTH_BYTES);
    txdata[1] = (int *)malloc16(FRAME_LENGTH_BYTES);
  */


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
  txdata = lte_eNB_common_vars->txdata[eNb_id];
#endif

  
  s_re = malloc(2*sizeof(double*));
  s_im = malloc(2*sizeof(double*));
  r_re = malloc(2*sizeof(double*));
  r_im = malloc(2*sizeof(double*));

  if (argc>=3) {
    snr0 = atof(argv[2]);
  }
  snr1 = snr0+10.0;


  if (argc>=4) {
    ricean_factor = atof(argv[3]);
  }

  if (argc>5) {
    Td = atof(argv[4]);
  }

  printf("Target mcs %d\n",mcs);

  printf("SNR0 %f, SNR1 %f\n",snr0,snr1);

  nsymb = (lte_frame_parms->Ncp == 0) ? 14 : 12;

  coded_bits_per_codeword = NB_RB * (12 * get_Qm(mcs)) * (lte_frame_parms->num_dlsch_symbols);

#ifdef TBS_FIX
  rate = (double)3*dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]/(4*coded_bits_per_codeword);
#else
  rate = (double)dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]/(coded_bits_per_codeword);
#endif

  printf("Rate = %f (mod %d,code %f)\n",rate*get_Qm(mcs),get_Qm(mcs),rate);
  rate *= get_Qm(mcs);

  sprintf(bler_fname,"BLER_SIMULATIONS/AWGN_SISO_HARQ/bler_%d.csv",mcs);
  bler_fd = fopen(bler_fname,"w");
  if (bler_fd == NULL) {
    printf("Error opening file %s\n",bler_fname);
    exit(-1);
  }
  fprintf(bler_fd,"SNR; MCS; TBS; rate; err0; trials0; err1; trial1; err2; trial2; err3; trial3; dci_err\n");

  for (i=0;i<2;i++) {
    s_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    s_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_re[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
    r_im[i] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  }


  lte_ue_pdcch_vars[0]->crnti = C_RNTI;

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
  DLSCH_alloc_pdu2.tpmi             = 1;//5 ;  // precoding

  // Create transport channel structures for SI pdus
  dlsch_eNb_cntl = new_eNb_dlsch(1,1);
  dlsch_ue_cntl  = new_ue_dlsch(1,1);

  
  // Create transport channel structures for 2 transport blocks (MIMO)
  dlsch_eNb = (LTE_eNb_DLSCH_t**) malloc16(2*sizeof(LTE_eNb_DLSCH_t*));
  dlsch_ue = (LTE_UE_DLSCH_t**) malloc16(2*sizeof(LTE_UE_DLSCH_t*));
  for (i=0;i<2;i++) {
    dlsch_eNb[i] = new_eNb_dlsch(1,8);
    dlsch_ue[i]  = new_ue_dlsch(1,8);
  
    if (!dlsch_eNb[i]) {
      printf("Can't get eNb dlsch structures\n");
      exit(-1);
    }
    
    if (!dlsch_ue[i]) {
      printf("Can't get ue dlsch structures\n");
      exit(-1);
    }
  }
  

  if (DLSCH_alloc_pdu2.tpmi == 5) {
    dlsch_eNb[0]->pmi_alloc = (unsigned short)(taus()&0xffff);
    dlsch_ue[0]->pmi_alloc = dlsch_eNb[0]->pmi_alloc;
    eNB_UE_stats[0].DL_pmi_single[0] = dlsch_eNb[0]->pmi_alloc;
  }
  
  generate_eNb_dlsch_params_from_dci(0,
                                     &DLSCH_alloc_pdu2,
				     C_RNTI,
				     format2_2A_M10PRB,
				     dlsch_eNb,
				     lte_frame_parms,
				     SI_RNTI,
				     RA_RNTI,
				     P_RNTI);

				     
  /*
    generate_eNb_dlsch_params_from_dci(0,
    &CCCH_alloc_pdu,
    SI_RNTI,
    format1A,
    &dlsch_eNb_cntl,
    lte_frame_parms,
    SI_RNTI,
    RA_RNTI,
    P_RNTI);
  */
  
  
  //  input_data     = (unsigned char*) malloc(block_length/8);
  //  decoded_output = (unsigned char*) malloc(block_length/8);

  // DCI
  
  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
  dci_alloc[0].dci_length = sizeof_DCI2_5MHz_2A_M10PRB_TDD_t;
  dci_alloc[0].L          = 3;
  dci_alloc[0].rnti       = C_RNTI;
  /*
    memcpy(&dci_alloc[0].dci_pdu[0],&CCCH_alloc_pdu,sizeof(DCI1A_5MHz_TDD_1_6_t));
    dci_alloc[0].dci_length = sizeof_DCI1A_5MHz_TDD_1_6_t;
    dci_alloc[0].L          = 3;
    dci_alloc[0].rnti       = SI_RNTI;
  */

  memcpy(&dci_alloc[1].dci_pdu[0],&UL_alloc_pdu,sizeof(DCI0_5MHz_TDD0_t));
  dci_alloc[1].dci_length = sizeof_DCI0_5MHz_TDD_0_t;
  dci_alloc[1].L          = 3;
  dci_alloc[1].rnti       = C_RNTI;




  input_buffer_length = dlsch_eNb[0]->harq_processes[0]->TBS/8;
  
  printf("dlsch0: TBS      %d\n",dlsch_eNb[0]->harq_processes[0]->TBS);
  
  printf("Input buffer size %d bytes\n",input_buffer_length);
  
  
  input_buffer = (unsigned char *)malloc(input_buffer_length+4);
  
  for (i=0;i<input_buffer_length;i++)
    input_buffer[i]= (unsigned char)(taus()&0xff);
  
  
    
  ch = (struct complex**) malloc(4 * sizeof(struct complex*));
  for (i = 0; i<4; i++)
    ch[i] = (struct complex*) malloc(channel_length * sizeof(struct complex));



  for (SNR=snr0;SNR<snr1;SNR+=.2) {
    errs[0]=0;
    errs[1]=0;
    errs[2]=0;
    errs[3]=0;
    round_trials[0] = 0;
    round_trials[1] = 0;
    round_trials[2] = 0;
    round_trials[3] = 0;

    dci_errors=0;

    round=0;
    for (trials = 0;trials<N_TRIALS;trials++) {
      //      printf("Trial %d\n",trials);
      round=0;
      while (round < 4) {
	//	printf("Trial %d : Round %d",trials,round);
	round_trials[round]++;
	if (round == 0) {
	  dlsch_eNb[0]->harq_processes[0]->Ndi = 1;
	  dlsch_eNb[0]->harq_processes[0]->rvidx = round>>1;
	  DLSCH_alloc_pdu2.ndi1             = 1;
	  DLSCH_alloc_pdu2.rv1              = 0;
	  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	}
	else {
	  dlsch_eNb[0]->harq_processes[0]->Ndi = 0;
	  dlsch_eNb[0]->harq_processes[0]->rvidx = round>>1;
	  DLSCH_alloc_pdu2.ndi1             = 0;
	  DLSCH_alloc_pdu2.rv1              = round>>1;
	  memcpy(&dci_alloc[0].dci_pdu[0],&DLSCH_alloc_pdu2,sizeof(DCI2_5MHz_2A_M10PRB_TDD_t));
	}

	dlsch_encoding(input_buffer,
		       lte_frame_parms,
		       dlsch_eNb[0]);
	
#ifdef OUTPUT_DEBUG
	for (s=0;s<dlsch_eNb[0]->harq_processes[0]->C;s++) {
	  if (s<dlsch_eNb[0]->harq_processes[0]->Cminus)
	    Kr = dlsch_eNb[0]->harq_processes[0]->Kminus;
	  else
	    Kr = dlsch_eNb[0]->harq_processes[0]->Kplus;
	  
	  Kr_bytes = Kr>>3;
	  
	  for (i=0;i<Kr_bytes;i++)
	    printf("%d : (%x)\n",i,dlsch_eNb[0]->harq_processes[0]->c[s][i]);
	}
	
#endif 
	
	re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
					1024,
					0,
					lte_frame_parms,
					dlsch_eNb[0]);
	
#ifdef OUTPUT_DEBUG    
	printf("RB count %d (%d,%d)\n",re_allocated,re_allocated/lte_frame_parms->num_dlsch_symbols/12,lte_frame_parms->num_dlsch_symbols);
#endif    
	
	
	if (num_layers>1)
	  re_allocated = dlsch_modulation(lte_eNB_common_vars->txdataF[eNb_id],
					  1024,
					  i,
					  lte_frame_parms,
					  dlsch_eNb[1]);
	
	generate_dci_top(1,
			 0,
			 dci_alloc,
			 0,
			 1024,
			 lte_frame_parms,
			 lte_eNB_common_vars->txdataF[eNb_id],
			 0);
	
	generate_pilots(lte_eNB_common_vars->txdataF[eNb_id],
			1024,
			lte_frame_parms,
			eNb_id,
			LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);
	
	
#ifdef IFFT_FPGA
	
#ifdef OUTPUT_DEBUG  
	write_output("txsigF0.m","txsF0", lte_eNB_common_vars->txdataF[0][0],300*120,1,4);
	write_output("txsigF1.m","txsF1", lte_eNB_common_vars->txdataF[0][1],300*120,1,4);
#endif
	
	// do talbe lookup and write results to txdataF2
	for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	  ind = 0;
	  //	  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	  for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX;i++) 
	    if (((i%512)>=1) && ((i%512)<=150))
	      txdataF2[aa][i] = ((int*)mod_table)[lte_eNB_common_vars->txdataF[eNb_id][aa][ind++]];
	    else if ((i%512)>=362)
	      txdataF2[aa][i] = ((int*)mod_table)[lte_eNB_common_vars->txdataF[eNb_id][aa][ind++]];
	    else 
	      txdataF2[aa][i] = 0;
	  //    printf("ind=%d\n",ind);
	}
	
#ifdef OUTPUT_DEBUG  
	write_output("txsigF20.m","txsF20", txdataF2[0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,1,1);
#endif
	
	tx_lev = 0;
	for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	  PHY_ofdm_mod(txdataF2[aa],        // input
		       txdata[aa],         // output
		       lte_frame_parms->log2_symbol_size,                // log2_fft_size
		       2*nsymb,//NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
		       lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		       lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       lte_frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	  
	  tx_lev += signal_energy(&txdata[aa][4*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
				  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	}
	
#else //IFFT_FPGA
	
#ifdef OUTPUT_DEBUG  
	write_output("txsigF0.m","txsF0", lte_eNB_common_vars->txdataF[eNb_id][0],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5,1,1);
	write_output("txsigF1.m","txsF1", lte_eNB_common_vars->txdataF[eNb_id][1],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX/5,1,1);
#endif
	
	tx_lev = 0;
	for (aa=0; aa<lte_frame_parms->nb_antennas_tx; aa++) {
	  PHY_ofdm_mod(lte_eNB_common_vars->txdataF[eNb_id][aa],        // input
		       txdata[aa],         // output
		       lte_frame_parms->log2_symbol_size,                // log2_fft_size
		       2*nsymb,//NUMBER_OF_SYMBOLS_PER_FRAME,                 // number of symbols
		       lte_frame_parms->nb_prefix_samples,               // number of prefix samples
		       lte_frame_parms->twiddle_ifft,  // IFFT twiddle factors
		       lte_frame_parms->rev,           // bit-reversal permutation
		       CYCLIC_PREFIX);
	  
	  tx_lev += signal_energy(&txdata[aa][OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES*2],
				  OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
	  
	}  
#endif //IFFT_FPGA
	
	
	//printf("tx_lev = %d\n",tx_lev);
	tx_lev_dB = (unsigned int) dB_fixed(tx_lev);
	
	
#ifdef OUTPUT_DEBUG  
	write_output("txsig0.m","txs0", txdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
#endif
	
	for (i=0;i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	  for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
	    s_re[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)]);
	    s_im[aa][i] = ((double)(((short *)txdata[aa]))[(i<<1)+1]);
	  }
	}
	
	// multipath channel
#ifdef AWGN // copy s_re and s_im to r_re and r_im
  for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
    for (aa=0;aa<lte_frame_parms->nb_antennas_tx;aa++) {
      r_re[aa][i] = s_re[aa][i];
      r_im[aa][i] = s_im[aa][i];
    }
  }
#else
	multipath_channel(ch,s_re,s_im,r_re,r_im,
			  amps,Td,BW,ricean_factor,aoa,
			  lte_frame_parms->nb_antennas_tx,
			  lte_frame_parms->nb_antennas_rx,
			  2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,//FRAME_LENGTH_COMPLEX_SAMPLES,
			  channel_length,0,
			  1,1);
#endif

#ifdef OUTPUT_DEBUG
	write_output("channel0.m","chan0",ch[0],channel_length,1,8);
#endif

	sigma2_dB = tx_lev_dB +10*log10(lte_frame_parms->ofdm_symbol_size/(NB_RB*12)) - SNR;

	//AWGN
	sigma2 = pow(10,sigma2_dB/10);
	//	printf("Sigma2 %f (sigma2_dB %f)\n",sigma2,sigma2_dB);
	for (i=0; i<2*nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES; i++) {
	  for (aa=0;aa<lte_frame_parms->nb_antennas_rx;aa++) {
	    ((short*) lte_ue_common_vars->rxdata[aa])[2*i] = (short) (r_re[aa][i] + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	    ((short*) lte_ue_common_vars->rxdata[aa])[2*i+1] = (short) (r_im[aa][i] + (iqimb*r_re[aa][i]) + sqrt(sigma2/2)*gaussdouble(0.0,1.0));
	  }
	}    
	//    lte_sync_time_init(lte_frame_parms,lte_ue_common_vars);
	//    lte_sync_time(lte_ue_common_vars->rxdata, lte_frame_parms);
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

#ifdef OUTPUT_DEBUG
	printf("RX level in null symbol %d\n",dB_fixed(signal_energy(&lte_ue_common_vars->rxdata[0][160+OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	printf("RX level in data symbol %d\n",dB_fixed(signal_energy(&lte_ue_common_vars->rxdata[0][160+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES)],OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2)));
	printf("rx_level Null symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
	printf("rx_level data symbol %f\n",10*log10(signal_energy_fp(r_re,r_im,1,OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES/2,256+(2*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES))));
#endif
	
	// Inner receiver scheduling for 3 slots
	for (Ns=0;Ns<3;Ns++) {
	  for (l=0;l<6;l++) {
	    //	    printf("Ns %d, l %d\n",Ns,l);
	    slot_fep(lte_frame_parms,
		     lte_ue_common_vars,
		     l,
		     Ns%20,
		     0,
		     0);
	    
	    lte_ue_measurements(lte_ue_common_vars,
				lte_frame_parms,
				&PHY_vars->PHY_measurements,
				0,
				1,
				0);
	    //	printf("rx_avg_power_dB %d\n",PHY_vars->PHY_measurements.rx_avg_power_dB[0]);
	    //	printf("n0_power_dB %d\n",PHY_vars->PHY_measurements.n0_power_dB[0]);

	    if ((Ns==0) && (l==3)) {// process symbols 0,1,2
#ifndef NO_DCI	
	      rx_pdcch(lte_ue_common_vars,
		       lte_ue_pdcch_vars,
		       lte_frame_parms,
		       eNb_id,
		       2,
		       (lte_frame_parms->mode1_flag == 1) ? SISO : ALAMOUTI);

	      dci_cnt = dci_decoding_procedure(lte_ue_pdcch_vars,dci_alloc_rx,eNb_id,lte_frame_parms,SI_RNTI,RA_RNTI);
	      //	      printf("dci_cnt %d\n",dci_cnt);
	      //	      write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	      //	      exit(-1);


	      if (dci_cnt==0) {
		dlsch_active = 0;
		if (round==0) {
		  dci_errors++;
#ifdef OUTPUT_DEBUG
		  printf("DCI error trial %d errs[0] %d\n",trials,errs[0]);
#endif
                }
		for (i=1;i<=round;i++)
		  round_trials[i]--;
		round=0;
	      }
		
	      for (i=0;i<dci_cnt;i++) {
		if ((dci_alloc_rx[i].rnti == C_RNTI) && (dci_alloc_rx[i].format == format2_2A_M10PRB) &&
		    (generate_ue_dlsch_params_from_dci(0,
						       (DCI2_5MHz_2A_M10PRB_TDD_t *)&dci_alloc_rx[i].dci_pdu,
						       C_RNTI,
						       format2_2A_M10PRB,
						       dlsch_ue,
						       lte_frame_parms,
						       SI_RNTI,
						       RA_RNTI,
						       P_RNTI)==0)) {
		  dlsch_active = 1;
		}
		else {
		  dlsch_active = 0;
		  if (round==0) {
		     dci_errors++;
#ifdef OUTPUT_DEBUG
	 	     printf("DCI misdetection trial %d\n",trials);
#endif
                  }
		  for (i=1;i<=round;i++)
		    round_trials[i]--;
		  round=0;
		}
	      }

	      /*
		else if ((dci_alloc_rx[i].rnti == SI_RNTI) && (dci_alloc_rx[i].format == format1A))
		generate_ue_dlsch_params_from_dci(0,
		(DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
		SI_RNTI,
		format1A,
		&dlsch_ue_cntl, 
		lte_frame_parms,
		SI_RNTI,
		RA_RNTI,
		P_RNTI);
	      */
	      //	      msg("dci_cnt = %d\n",dci_cnt);

#else
	      generate_ue_dlsch_params_from_dci(0,
						&DLSCH_alloc_pdu2,
						C_RNTI,
						format2_2A_M10PRB,
						dlsch_ue,
						lte_frame_parms,
						SI_RNTI,
						RA_RNTI,
						P_RNTI);
	      dlsch_active = 1;
#endif
	    }

	    /*
	      for (m=lte_frame_parms->first_dlsch_symbol;m<3;m++)
	      rx_dlsch(lte_ue_common_vars,
	      lte_ue_dlsch_vars,
	      lte_frame_parms,
	      eNb_id,
	      eNb_id_i,
	      m,
	      rb_alloc,
	      mod_order,
	      mimo_mode,
	      dual_stream_UE);
	    */

	    if (dlsch_active == 1) {
	      if ((Ns==1) && (l==0)) // process symbols 3,4,5
		for (m=4;m<6;m++)
		  if (rx_dlsch(lte_ue_common_vars,
			       lte_ue_dlsch_vars,
			       lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       dlsch_ue,
			       m,
			       dual_stream_UE)==-1) {
		    dlsch_active = 0;
		    break;
		  }	      
	      if ((Ns==1) && (l==3)) {// process symbols 6,7,8
		/*
		  if (rx_pbch(lte_ue_common_vars,
		  lte_ue_pbch_vars[0],
		  lte_frame_parms,
		  0,
		  SISO)) {
		  msg("pbch decoded sucessfully!\n");
		  }
		  else {
		  msg("pbch not decoded!\n");
		  }
		*/
		for (m=7;m<9;m++)
		  if (rx_dlsch(lte_ue_common_vars,
			       lte_ue_dlsch_vars,
			       lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       dlsch_ue,
			       m,
			       dual_stream_UE)==-1) {
		    dlsch_active=0;
		    break;
		  }
	      }
	      
	      if ((Ns==2) && (l==0))  // process symbols 10,11, do deinterleaving for TTI
		for (m=10;m<12;m++)
		  if (rx_dlsch(lte_ue_common_vars,
			       lte_ue_dlsch_vars,
			       lte_frame_parms,
			       eNb_id,
			       eNb_id_i,
			       dlsch_ue,
			       m,
			       dual_stream_UE)==-1) {
		    dlsch_active=0;
		    break;
		  }
	    }
	  }
	}

	if (dlsch_active == 1) {
#ifdef OUTPUT_DEBUG      
	  write_output("rxsig0.m","rxs0", lte_ue_common_vars->rxdata[0],FRAME_LENGTH_COMPLEX_SAMPLES,1,1);
	  write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	  
	  /*
	    write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][1][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	    write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][2][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	    write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[eNb_id][3][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
	  */
	  write_output("rxsigF0.m","rxsF0", lte_ue_common_vars->rxdataF[0],2*12*lte_frame_parms->ofdm_symbol_size,2,1);
	  write_output("rxsigF0_ext.m","rxsF0_ext", lte_ue_dlsch_vars[eNb_id]->rxdataF_ext[0],2*12*lte_frame_parms->ofdm_symbol_size,1,1);
	  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[0],300*12,1,1);
	  write_output("pdcchF0_ext.m","pdcchF_ext", lte_ue_pdcch_vars[eNb_id]->rxdataF_ext[0],2*3*lte_frame_parms->ofdm_symbol_size,1,1);
	  write_output("pdcch00_ch0_ext.m","pdcch00_ch0_ext",lte_ue_pdcch_vars[eNb_id]->dl_ch_estimates_ext[0],300*3,1,1);
	  /*
	    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[1],300*12,1,1);
	    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[2],300*12,1,1);
	    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext[3],300*12,1,1);
	    write_output("dlsch_rho.m","dl_rho",lte_ue_dlsch_vars[eNb_id]->rho[0],300*12,1,1);
	  */
	  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0",lte_ue_dlsch_vars[eNb_id]->rxdataF_comp[0],300*12,1,1);
	  write_output("pdcch_rxF_comp0.m","pdcch0_rxF_comp0",lte_ue_pdcch_vars[eNb_id]->rxdataF_comp[0],4*300,1,1);
	  write_output("dlsch_rxF_llr.m","dlsch_llr",lte_ue_dlsch_vars[eNb_id]->llr[0],coded_bits_per_codeword,1,0);
	  write_output("pdcch_rxF_llr.m","pdcch_llr",lte_ue_pdcch_vars[eNb_id]->llr,2400,1,4);
	  
	  write_output("dlsch_mag1.m","dlschmag1",lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,300*12,1,1);
	  write_output("dlsch_mag2.m","dlschmag2",lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,300*12,1,1);
#endif //OUTPUT_DEBUG
	  
	  //	printf("Calling decoding (Ndi %d, harq_pid %d)\n",
	  //       dlsch_ue[0]->harq_processes[0]->Ndi,
	  //       dlsch_ue[0]->current_harq_pid);
	  
	  ret = dlsch_decoding(lte_ue_dlsch_vars[eNb_id]->llr[0],		 
			       lte_frame_parms,
			       dlsch_ue[0],
			       0);
	  
	  if (ret <= MAX_TURBO_ITERATIONS) {
#ifdef OUTPUT_DEBUG  
	    printf("No DLSCH errors found\n");
	    exit(-1);
#endif
	    round=5;
	  }	
	  else {
	    errs[round]++;
#ifdef OUTPUT_DEBUG  
	    printf("DLSCH errors found\n");
	    exit(-1);
#endif
	    //	    printf("round %d errors %d/%d\n",round,errs[round],trials);
	    round++;

#ifdef OUTPUT_DEBUG  
	    printf("DLSCH in error in round %d\n",round);
#endif
	  }
	}
	  
#ifdef OUTPUT_DEBUG  
	for (s=0;s<dlsch_ue[0]->harq_processes[0]->C;s++) {
	  if (s<dlsch_ue[0]->harq_processes[0]->Cminus)
	    Kr = dlsch_ue[0]->harq_processes[0]->Kminus;
	  else
	    Kr = dlsch_ue[0]->harq_processes[0]->Kplus;
	
	  Kr_bytes = Kr>>3;
	
	  printf("Decoded_output (Segment %d):\n",s);
	  for (i=0;i<Kr_bytes;i++)
	    printf("%d : %x (%x)\n",i,dlsch_ue[0]->harq_processes[0]->c[s][i],dlsch_ue[0]->harq_processes[0]->c[s][i]^dlsch_eNb[0]->harq_processes[0]->c[s][i]);
	}

	
#endif
      }  //round
      //      printf("\n");
      if ((errs[0]>=100) && (trials>(N_TRIALS/2)))
	break;

    }   //trials
    printf("**********************SNR = %f dB (tx_lev %f, sigma2_dB %f)**************************\n",
	   SNR,
	   (double)tx_lev_dB+10*log10(lte_frame_parms->ofdm_symbol_size/(NB_RB*12)),
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
	   rate*((double)round_trials[0]/((double)round_trials[0] + round_trials[1] + round_trials[2] + round_trials[3])),
	   rate,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0])/(double)dlsch_eNb[0]->harq_processes[0]->TBS,
	   (1.0*(round_trials[0]-errs[0])+2.0*(round_trials[1]-errs[1])+3.0*(round_trials[2]-errs[2])+4.0*(round_trials[3]-errs[3]))/((double)round_trials[0]));

    fprintf(bler_fd,"%f;%d;%d;%f;%d;%d;%d;%d;%d;%d;%d;%d;%d\n",
	    SNR,
	    mcs,
	    dlsch_eNb[0]->harq_processes[0]->TBS,
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
    
    if (((double)errs[0]/(round_trials[0]))<1e-2) 
      break;
  } // SNR
  
  fclose(bler_fd);
  
  printf("Freeing dlsch structures\n");
  for (i=0;i<2;i++) {
    printf("eNb %d\n",i);
    free_eNb_dlsch(dlsch_eNb[i]);
    printf("UE %d\n",i);
    free_ue_dlsch(dlsch_ue[i]);
  }


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
   


/*  
    for (i=1;i<4;i++)
    memcpy((void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[i*12*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*2],
    (void *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0],
    12*OFDM_SYMBOL_SIZE_SAMPLES_NO_PREFIX*2);
*/
