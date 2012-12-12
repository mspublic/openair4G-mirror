#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"

#include "PHY/CODING/defs.h"
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#include "LAYER2/MAC/vars.h"
#include "OCG_vars.h"

#include "UTIL/LOG/log.h" 

//#include "PHY/CODING/lte_interleaver.h"
//#include "PHY/CODING/lte_interleaver_inline.h"


#include "SIMULATION/TOOLS/defs.h"

//#include "decoder.h"

#define INPUT_LENGTH 5
#define F1 3
#define F2 10

#include "emmintrin.h"

#define sgn(a) (((a)<0) ? 0 : 1)

//#define DEBUG_CODER 1

int current_dlsch_cqi;

PHY_VARS_eNB *PHY_vars_eNB;
PHY_VARS_UE *PHY_vars_UE;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
channel_desc_t *UE2eNB[NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX];

void lte_param_init(unsigned char N_tx, unsigned char N_rx,unsigned char transmission_mode,unsigned char extended_prefix_flag,u16 Nid_cell,u8 tdd_config) {

  unsigned int ind;
  LTE_DL_FRAME_PARMS *lte_frame_parms;

  printf("Start lte_param_init (Nid_cell %d, extended_prefix %d, transmission_mode %d, N_tx %d, N_rx %d)\n",
	 Nid_cell, extended_prefix_flag,transmission_mode,N_tx,N_rx);
  PHY_vars_eNB = malloc(sizeof(PHY_VARS_eNB));

  PHY_vars_UE = malloc(sizeof(PHY_VARS_UE));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  lte_frame_parms = &(PHY_vars_eNB->lte_frame_parms);

  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->N_RB_UL            = 25;   
  lte_frame_parms->Ncp                = extended_prefix_flag;
  lte_frame_parms->Nid_cell           = Nid_cell;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->tdd_config         = tdd_config;

  //  lte_frame_parms->Csrs = 2;
  //  lte_frame_parms->Bsrs = 0;
  //  lte_frame_parms->kTC = 0;
  //  lte_frame_parms->n_RRC = 0;
  lte_frame_parms->mode1_flag = (transmission_mode == 1)? 1 : 0;

  init_frame_parms(lte_frame_parms,1);
  
  //copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(lte_frame_parms); //allocation

   
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;

  memcpy(&PHY_vars_UE->lte_frame_parms,lte_frame_parms,sizeof(LTE_DL_FRAME_PARMS));

  
  phy_init_lte_top(lte_frame_parms);
  phy_init_lte_ue(PHY_vars_UE,1,0);
  phy_init_lte_eNB(PHY_vars_eNB,0,0,0);

  printf("Done lte_param_init\n");


}

/*
void print_shorts(char *s,__m128i *x) {

  short *tempb = (short *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
         );

}
*/

// 4-bit quantizer
char quantize4bit(double D,double x) {

  double qxd;

  qxd = floor(x/D);
  //  printf("x=%f,qxd=%f\n",x,qxd);

  if (qxd <= -8)
    qxd = -8;
  else if (qxd > 7)
    qxd = 7;

  return((char)qxd);
}

char quantize(double D,double x,unsigned char B) {

  double qxd;
  char maxlev;

  qxd = floor(x/D);
  //  printf("x=%f,qxd=%f\n",x,qxd);

  maxlev = 1<<(B-1);

  if (qxd <= -maxlev)
    qxd = -maxlev;
  else if (qxd >= maxlev)
    qxd = maxlev-1;

  return((char)qxd);
}

#define MAX_BLOCK_LENGTH 6000
static char channel_output[2*MAX_BLOCK_LENGTH]__attribute__ ((aligned(16)));
static unsigned char decoded_output[MAX_BLOCK_LENGTH/8];

int test_logmap8(LTE_eNB_DLSCH_t *dlsch_eNB,
		 LTE_UE_DLSCH_t *dlsch_ue,
		 unsigned int coded_bits,
		 unsigned char NB_RB,
		 double sigma,
		 unsigned char qbits,
		 unsigned int block_length,
		 unsigned int ntrials,
		 unsigned int *errors,
		 unsigned int *trials,
		 unsigned int *uerrors,
		 unsigned int *crc_misses,
		 unsigned int *iterations,
		 unsigned int num_pdcch_symbols,
		 unsigned int subframe) {

  unsigned char test_input[block_length+1];
  //_declspec(align(16))  char channel_output[512];
  //_declspec(align(16))  unsigned char output[512],decoded_output[16], *inPtr, *outPtr;

  short *channel_output;


  unsigned char decoded_output[block_length];
  unsigned int i,trial=0;
  unsigned int crc=0;
  unsigned char ret;
  unsigned char uerr;
  unsigned char crc_type;


  channel_output = (short *)malloc(coded_bits*sizeof(short));

  *iterations=0;
  *errors=0;
  *crc_misses=0;
  *uerrors=0;



  //  printf("dlsch_eNB->TBS= %d\n",dlsch_eNB->harq_processes[0]->TBS);

  while (trial++ < ntrials) {

    //    printf("encoding\n");
    //    test_input[0] = 0x80;
    for (i=0;i<block_length;i++) {
      
      test_input[i] = i&0xff;//(unsigned char)(taus()&0xff);
    }

    dlsch_encoding(test_input,
		   &PHY_vars_eNB->lte_frame_parms,
		   num_pdcch_symbols,
		   PHY_vars_eNB->dlsch_eNB[0][0],
		   subframe);

    uerr=0;


    for (i = 0; i < coded_bits; i++){
#ifdef DEBUG_CODER
      if ((i&0xf)==0) 
	printf("\ne %d..%d:    ",i,i+15);
      printf("%d.",PHY_vars_eNB->dlsch_eNB[0][0]->e[i]);
#endif
      channel_output[i] = (short)quantize(sigma/4.0,(2.0*PHY_vars_eNB->dlsch_eNB[0][0]->e[i]) - 1.0 + sigma*gaussdouble(0.0,1.0),qbits);
    }
#ifdef DEBUG_CODER
    printf("\n");
    exit(-1);
#endif
  
    
    //    memset(decoded_output,0,16);
    //    printf("decoding %d\n",trial);
    ret = dlsch_decoding(channel_output,
			 &PHY_vars_UE->lte_frame_parms,
			 PHY_vars_UE->dlsch_ue[0][0],
			 subframe,
			 num_pdcch_symbols);

    /*    int diffs = 0,puncts=0;
    for (i=0;i<dlsch_ue->harq_processes[0]->Kplus*3;i++) {
      if (dlsch_ue->harq_processes[0]->d[0][96+i] == 0) {
	printf("%d punct (%d,%d)\n",i,dlsch_ue->harq_processes[0]->d[0][96+i],dlsch_eNb->harq_processes[0]->d[0][96+i]);
	puncts++;
      }
      else if (sgn(dlsch_ue->harq_processes[0]->d[0][96+i]) != dlsch_eNb->harq_processes[0]->d[0][96+i]) {
	printf("%d differs (%d,%d)\n",i,dlsch_ue->harq_processes[0]->d[0][96+i],dlsch_eNb->harq_processes[0]->d[0][96+i]);
	diffs++;
      }
      else
	printf("%d same (%d,%d)\n",i,dlsch_ue->harq_processes[0]->d[0][96+i],dlsch_eNb->harq_processes[0]->d[0][96+i]);
    }
    printf("diffs %d puncts %d(%d,%d,%d,%d,%d)\n",diffs,puncts,dlsch_ue->harq_processes[0]->F,coded_bits,3*(block_length<<3),3*dlsch_ue->harq_processes[0]->Kplus,3*dlsch_ue->harq_processes[0]->F+3*(block_length<<3)-coded_bits);
    */
    
    //    printf("ret %d\n",ret);
    //    printf("trial %d : i %d/%d : Input %x, Output %x (%x, F %d)\n",trial,0,block_length,test_input[0],
    //	   dlsch_ue->harq_processes[0]->b[0],
    //	   dlsch_ue->harq_processes[0]->c[0][0],
    //	   (dlsch_ue->harq_processes[0]->F>>3));
    
    if (ret < MAX_TURBO_ITERATIONS+1) {
      *iterations = (*iterations) + ret;
      //      if (ret>1)
      //	printf("ret %d\n",ret);
    }
    else
      *iterations = (*iterations) + (ret-1);
    
    if (uerr==1)
      *uerrors = (*uerrors) + 1;
    
    for (i=0;i<block_length;i++) {
            
      if (dlsch_ue->harq_processes[0]->b[i] != test_input[i]) {
	/*		
		printf("i %d/%d : Input %x, Output %x (%x, F %d)\n",i,block_length,test_input[i],
		       dlsch_ue->harq_processes[0]->b[i],
		       dlsch_ue->harq_processes[0]->c[0][i],
		       (dlsch_ue->harq_processes[0]->F>>3));
	*/
	*errors = (*errors) + 1;
	//	printf("*%d, ret %d\n",*errors,ret);	

	
	
	if (ret < MAX_TURBO_ITERATIONS+1)
	  *crc_misses = (*crc_misses)+1;
	break;

      }

    
    }
    
    if (ret == MAX_TURBO_ITERATIONS+1) {
      //      exit(-1);
    }
      /*
      for (i=0;i<block_length;i++) {
	
	if (dlsch_ue->harq_processes[0]->b[i] != test_input[i]) {
	  printf("i %d/%d : Input %x, Output %x (%x, F %d)\n",i,block_length,test_input[i],
		 dlsch_ue->harq_processes[0]->b[i],
		 dlsch_ue->harq_processes[0]->c[0][i],
		 (dlsch_ue->harq_processes[0]->F>>3));
	  
	}
	
      }
    }
    */
    if (*errors == 100) {
      //printf("\n");
      break;
    }
  }

  *trials = trial;
  //  printf("lte: trials %d, errors %d\n",trial,*errors);
  return(0);
}

#define NTRIALS 1000
#define DLSCH_RB_ALLOC 0x1fff//0x1fbf // igore DC component,RB13

int main(int argc, char *argv[]) {

  int ret,ret2;
  unsigned int errors,uerrors,errors2,crc_misses,iterations,trials,trials2,block_length,errors3,trials3;
  double SNR,sigma,rate=.5;
  unsigned char qbits,mcs;
  
  char done0=0;
  char done1=1;
  char done2=1;

  unsigned int coded_bits;
  unsigned char NB_RB=25;

  int num_pdcch_symbols = 3;
  int subframe = 6;

  randominit(0);
  logInit();
  lte_param_init(1,1,1,0,0,3);

  PHY_vars_eNB->dlsch_eNB[0][0] = new_eNB_dlsch(1,8,0);
  PHY_vars_UE->dlsch_ue[0][0]  = new_ue_dlsch(1,8,0);
  PHY_vars_eNB->dlsch_eNB[0][1] = new_eNB_dlsch(1,8,0);
  PHY_vars_UE->dlsch_ue[0][1]  = new_ue_dlsch(1,8,0);

  if (argc>1)
    mcs = atoi(argv[1]);
  else
    mcs = 0;

  printf("NB_RB %d\n",NB_RB);
  DLSCH_alloc_pdu2.rah              = 0;
  DLSCH_alloc_pdu2.rballoc          = DLSCH_RB_ALLOC;
  DLSCH_alloc_pdu2.TPC              = 0;
  DLSCH_alloc_pdu2.dai              = 0;
  DLSCH_alloc_pdu2.harq_pid         = 0;
  DLSCH_alloc_pdu2.tb_swap          = 0;
  DLSCH_alloc_pdu2.mcs1             = mcs;  
  DLSCH_alloc_pdu2.ndi1             = 1;
  DLSCH_alloc_pdu2.rv1              = 0;

  if (argc>2)
    qbits = atoi(argv[2]);
  else
    qbits = 4;

  printf("Quantization bits %d\n",qbits);

  generate_eNB_dlsch_params_from_dci(subframe,
                                     &DLSCH_alloc_pdu2,
				     0x1234,
				     format2_2A_M10PRB,
				     PHY_vars_eNB->dlsch_eNB[0],
				     &PHY_vars_eNB->lte_frame_parms,
				     PHY_vars_eNB->pdsch_config_dedicated,
				     SI_RNTI,
				     0,
				     P_RNTI,
				     0); //change this later
  generate_ue_dlsch_params_from_dci(subframe,
				    &DLSCH_alloc_pdu2,
				    C_RNTI,
				    format2_2A_M10PRB,
				    PHY_vars_UE->dlsch_ue[0],
				    &PHY_vars_UE->lte_frame_parms,
				    PHY_vars_UE->pdsch_config_dedicated,
				    SI_RNTI,
				    0,
				    P_RNTI);
  
  coded_bits = 	get_G(&PHY_vars_eNB->lte_frame_parms,NB_RB,PHY_vars_eNB->dlsch_eNB[0][0]->rb_alloc,
		      get_Qm(mcs),num_pdcch_symbols,subframe);

  printf("Coded_bits (G) = %d\n",coded_bits);

  block_length =  dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]>>3;
  printf("Block_length = %d bytes (%d bits, rate %f), mcs %d, I_TBS %d, F %d, NB_RB %d\n",block_length,
	 dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1],(double)dlsch_tbs25[get_I_TBS(mcs)][NB_RB-1]/coded_bits,
	 mcs,get_I_TBS(mcs),PHY_vars_eNB->dlsch_eNB[0][0]->harq_processes[0]->F,NB_RB);

  // Test Openair0 3GPP encoder
/*
  test_encoder(block_length,
	       f1f2mat[(block_length-5)*2],   // f1 (see 36121-820, page 14)
	       f1f2mat[((block_length-5)*2)+1],  // f2 (see 36121-820, page 14)
	       3);
 */ //  exit(0);



  for (SNR=-5;SNR<5;SNR+=.1) {


    //    printf("\n\nSNR %f dB\n",SNR);

    sigma = pow(10.0,-.05*SNR);

    errors=0;
    crc_misses=0;
    errors2=0;
    errors3=0;

    iterations=0;

    if (done0 == 0) {    
    

    
    ret = test_logmap8(PHY_vars_eNB->dlsch_eNB[0][0],
		       PHY_vars_UE->dlsch_ue[0][0],
		       coded_bits,
		       NB_RB,
		       sigma,   // noise standard deviation
		       qbits,
		       block_length,   // block length bytes
		       NTRIALS,
		       &errors,
		       &trials,
		       &uerrors,
		       &crc_misses,
		       &iterations,
		       num_pdcch_symbols,
		       subframe);

    if (ret>=0)
      //      printf("ref: Errors %d (%f), Uerrors %d (%f), CRC Misses %d (%f), Avg iterations %f\n",errors,(double)errors/trials,uerrors,(double)uerrors/trials,crc_misses,(double)crc_misses/trials,(double)iterations/trials);
      printf("%f,%f,%f,%f\n",SNR,(double)errors/trials,(double)crc_misses/trials,(double)iterations/trials);
    if (((double)errors/trials) < 1e-2)
      done0=1;
    } 
    /*    
    if (done1 == 0) { 

      printf("exmimo\n");
      ret = test_logmapexmimo(rate,    // code rate
			      sigma,   // noise standard deviation
			      qbits,
			      block_length,   // block length bytes
			      f1f2mat[iind*2],   // f1 (see 36121-820, page 14)
			      f1f2mat[(iind*2)+1],  // f2 (see 36121-820, page 14)
			      3,
			      NTRIALS,
			      &errors3,
			      &trials3);

      if (ret>=0)
	printf("exmimo : Errors %d (%f)\n",errors3,(double)errors3/trials3);
      if (((double)errors3/trials3) < 1e-3)
	done1=1;
    }
    

    if (done2 == 0) {  
    
      printf("Viterbi ...\n");
      ret2 = test_viterbi(sigma,
			  8*block_length,
			  NTRIALS,
			  &errors2,
			  &trials2,
			  rate);
      
      if (ret2>=0)
	printf("viterbi : Errors %d (%f)\n",errors2,(double)errors2/trials2);
      if (((double)errors2/trials2) < 1e-3)
	done2=1;
    } 
    */
    if ((done0==1) && (done1==1) && (done2==1)) {
      printf("done\n");
      break;
    }
  }
  return(0);
}

 
