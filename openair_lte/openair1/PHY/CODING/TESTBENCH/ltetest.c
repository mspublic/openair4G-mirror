#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "PHY/CODING/defs.h"
//#include "PHY/CODING/lte_interleaver.h"
//#include "PHY/CODING/lte_interleaver_inline.h"


#include "SIMULATION/TOOLS/defs.h"

//#include "decoder.h"

#define INPUT_LENGTH 5
#define F1 3
#define F2 10

#include "emmintrin.h"

#define sgn(a) (((a)<0) ? 0 : 1)

void lte_param_init(unsigned char N_tx, unsigned char N_rx) {

  printf("Start lte_param_init\n");
  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));

  randominit(0);
  set_taus_seed(0);
  
  crcTableInit();

  lte_frame_parms = &(PHY_config->lte_frame_parms);   //openair1/PHY/impl_defs_lte.h
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars);
  lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars);
  
  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 1;
  lte_frame_parms->nb_antennas_tx     = N_tx;
  lte_frame_parms->nb_antennas_rx     = N_rx;
  lte_frame_parms->first_dlsch_symbol = 2;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  
  phy_init_top(N_tx); //allocation
  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  generate_64qam_table();
  generate_16qam_table();
  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars,lte_ue_pbch_vars);//allocation
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

int test_viterbi(double sigma,
		 unsigned int BLOCK_LENGTH,
		 unsigned int ntrials,
		 unsigned int *errors,
		 unsigned int *trials,
		 double rate)
{

  unsigned int i,n,coded_bits;



  for (n=0;n<ntrials;n++) {

    coded_bits = (unsigned int)((BLOCK_LENGTH+8.0)/rate);

    for (i=0;i<2*(BLOCK_LENGTH+8);i++)
      channel_output[i] = 0;

    rate_matching(coded_bits,2*(BLOCK_LENGTH+8),channel_output,1,n);

    for (i = 0; i < 2*(BLOCK_LENGTH+8); i++){

      
      if ((channel_output[i]&0x80) == 0) 
	channel_output[i]=0;
      else if ((channel_output[i]&0x40) != 0)
	channel_output[i] = quantize4bit(sigma/4.0,(sqrt(2.0)*sigma*gaussdouble(0.0,1.0))- 2);
      else
	channel_output[i] = quantize4bit(sigma/4.0,(sigma*gaussdouble(0.0,1.0))- 1);
    }
 
    memset(decoded_output,0,1+(BLOCK_LENGTH/8));


    phy_viterbi_dot11_sse2(channel_output,decoded_output,8+BLOCK_LENGTH);

    // check for errors
    for (i=0;i<BLOCK_LENGTH/8;i++)
      if (decoded_output[i]!=0) {
	*errors = *errors+1;
	break;
      }
    if (*errors == 100)
      break;
  }

  *trials = n;


  return(0);

}

int test_logmap8(LTE_DL_eNb_DLSCH_t *dlsch_eNb,
		 LTE_DL_UE_DLSCH_t *dlsch_ue,
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
		 unsigned int *iterations) {

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




  while (trial++ < ntrials) {

    for (i=0;i<block_length;i++) {
      
      test_input[i] = (unsigned char)(taus()&0xff);
    }

    dlsch_encoding(test_input,
		   (block_length<<3),
		   lte_frame_parms,
		   dlsch_eNb,    //encoded subframe taking just even subframes corresponding to one spatial stream
		   0,                // harq_pid
		   NB_RB);           // number of allocated RB

    uerr=0;


    for (i = 0; i < coded_bits; i++){
      channel_output[i] = (short)quantize(sigma/4.0,(2.0*dlsch_eNb->e[i]) - 1.0 + sigma*gaussdouble(0.0,1.0),qbits);
    }


  
    
    memset(decoded_output,0,16);

    ret = dlsch_decoding(block_length<<3,
			 channel_output,
			 lte_frame_parms,
			 dlsch_ue,
			 0,               //harq_pid
			 NB_RB);
    /*        
    int diffs = 0,puncts=0;
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

    if (ret < MAX_TURBO_ITERATIONS+1)
      *iterations = (*iterations) + ret;
    else
      *iterations = (*iterations) + (ret-1);

    if (uerr==1)
      *uerrors = (*uerrors) + 1;
    
    for (i=0;i<block_length;i++) {
    /*        
      if (test_input[i] != dlsch_ue->harq_processes[0]->b[i])
	printf("i %d/%d : Input %x, Output %x\n",i,block_length,test_input[i],dlsch_ue->harq_processes[0]->b[i]);
     */ 
      if (dlsch_ue->harq_processes[0]->b[i] != test_input[i]) {

	*errors = (*errors) + 1;
//	printf("*%d\n",*errors);	


	
	if (ret < MAX_TURBO_ITERATIONS+1)
	  *crc_misses = (*crc_misses)+1;
	break;
	
      }
    }
    if (*errors == 100) {
      //printf("\n");
      break;
    }
  }

  *trials = trial;
  //  printf("lte: trials %d, errors %d\n",trial,*errors);
  return(0);
}

/*
int test_logmapexmimo(double rate,
		      double sigma,
		      unsigned char qbits,
		      unsigned int block_length,
		      unsigned short f1,
		      unsigned short f2,
		      unsigned char crc_len,
		      unsigned int ntrials,
		      unsigned int *errors,
		      unsigned int *trials) {


  unsigned char test_input[block_length+1];
  //_declspec(align(16))  char channel_output[512];
  //_declspec(align(16))  unsigned char output[512],decoded_output[16], *inPtr, *outPtr;

  char channel_output[(3*8*block_length)+12];
  unsigned int coded_bits;
  unsigned char output[(3*8*block_length)+12],decoded_output[block_length];
  short decoded_output16[(8*block_length)+12];
  unsigned int i,trial=0;
  unsigned int crc;
  unsigned char decoded_byte=0,err=0;

  *errors=0;

  while (trial++ < ntrials) {

    for (i=0;i<block_length-crc_len;i++) {
      
      test_input[i] = (unsigned char)(taus()&0xff);
    }
    
    switch (crc_len) {
      
    case 1:
      crc = crc8(test_input,
		 (block_length-1)<<3)>>24;
      break;
    case 2:
      crc = crc16(test_input,
		  (block_length-2)<<3)>>16;
      break;
    case 3:
      crc = crc24a(test_input,
		  (block_length-3)<<3)>>8;
      break;
    default:
      break;
      
    }
    
    if (crc_len > 0)
      *(unsigned int*)(&test_input[block_length-crc_len]) = crc;
    
    
    
    threegpplte_turbo_encoder(test_input,
			      block_length, 
			      output,
			      0,
			      f1,
			      f2);

    coded_bits = (unsigned int)((12.0+(3.0*8.0*block_length))/(3.0*rate));
    
    rate_matching(coded_bits,(3*8*block_length)+12,output,1,trial);

    for (i = 0; i < (3*8*block_length)+12; i++){
      if ((output[i]&0x80) != 0) {
	output[i]&=0x7f;
	channel_output[i] = quantize(sigma/4.0,(2.0*output[i]) - 1.0 + sigma*gaussdouble(0.0,1.0),qbits);
      }
      else
	channel_output[i]=0;

      //    printf("Position %d : %d\n",i,channel_output[i]);
    }

    // Puncture termination bits
    for (i=(3*8*block_length);i<(3*8*block_length)+12;i++)
      channel_output[i]=0;

    memset(decoded_output16,0,sizeof(short)*(8*6144));
    lte_turbo_decoding((8*block_length),8,channel_output,decoded_output16);

    //convert decoded output to bytes

    decoded_byte = 0;
    for (i=0;i<block_length*8;i++) {

      //      printf("decoded_output %d:%d\n",i,decoded_output16[i]);
      if (decoded_output16[i]>0) {
	//	printf("*\n");
	decoded_byte |= (1<<(i%8));
      }
      err=0;

      if ((i%8)==7) {
	//	printf("Position %d (%x,%x)\n",i>>3,test_input[i>>3],decoded_byte);
	if (decoded_byte != test_input[i>>3]) {
	  *errors = (*errors)+ 1;
	  err=1;
//	  	  printf("Trials %d, Errors %d --> position %d (byte %x, decoded %x)\n",trial,*errors,i>>3,test_input[i>>3],decoded_byte);

	}

	decoded_byte = 0;
      }


      if (err==1) {
	break;
      }
    }

    if (*errors == 100)
      break;
  }

  *trials = trial;
  //  printf("lte: trials %d, errors %d\n",trial,*errors);
  return(0);
}


void test_encoder(unsigned int block_length,
		  unsigned short f1,
		  unsigned short f2,
		  unsigned char crc_len) {
  
  unsigned char test_input[block_length+1];
  unsigned char output[(3*8*block_length)+12];
  unsigned int i;
  unsigned int crc;

  int size = block_length*8;

  char * encoder_input = (char*) malloc(sizeof(char) * size);
  char * encoder_output = (char*) malloc(sizeof(char) * (3*size+TAIL));
  char * parity0 = (char*) malloc(sizeof(char) * size);
  char * parity1 = (char*) malloc(sizeof(char) * size);
  char * tail0 = (char*) malloc(sizeof(char) * (TAIL/2));
  char * tail1 = (char*) malloc(sizeof(char) * (TAIL/2));
  char * intlv_output = (char*) malloc(sizeof(char) * size);
  short * interleaver_buffer = (short*) malloc(sizeof(short) * size);
  short * deinterleaver_buffer = (short*) malloc(sizeof(short) * size);

  unsigned short pi=0;

  generate_permutation_table_lte(((size <= MAX_BLOCK_LENGTH_UMTS) ?
				  size : MAX_BLOCK_LENGTH_UMTS),
				 (unsigned short*)interleaver_buffer,
				 (unsigned short*)deinterleaver_buffer);

  threegpplte_interleaver_reset();
  pi = 0;

  for (i=0;i<size;i++) {
    printf("Interleaver i %d : %d\n", interleaver_buffer[i],pi);
    pi = threegpplte_interleaver(f1,f2,size);
  }
  printf("Generating data ...\n");
  for (i=0;i<block_length-crc_len;i++) {
    

    test_input[i] = i;//(unsigned char)(taus()&0xff);

    printf("i %d : %x\n",i,test_input[i]);
  }
  printf("Generating CRC %d\n",crc_len);
  switch (crc_len) {
    
  case 1:
    crc = crc8(test_input,
	       (block_length-1)<<3)>>24;
    break;
  case 2:
    crc = crc16(test_input,
		(block_length-2)<<3)>>16;
    break;
  case 3:
    crc = crc24a(test_input,
		(block_length-3)<<3)>>8;
    break;
  default:
    break;
    
  }
  
  if (crc_len > 0)
    *(unsigned int*)(&test_input[block_length-crc_len]) = crc;
  
  printf("Encoding\n");
  
  threegpplte_turbo_encoder(test_input,
			    block_length, 
			    output,
			    0,
			    f1,
			    f2);
  



    
    for (i=0;i<size;i++) {
      if ((test_input[i>>3]&(1<<(i%8)))>0)
	encoder_input[i]=1;
      else
	encoder_input[i]=0;
    }
    // RSC turbo encoding #0
    rsc_encoder(size, encoder_input, parity0, tail0);
    
    // interleaving
    permute(size, encoder_input, intlv_output, (unsigned short*)interleaver_buffer, 1, "char");
    //    for (i=0;i<size;i++)
    //      printf("intelv_output %d (%d): %d (%d)\n",i,interleaver_buffer[i],intlv_output[i],encoder_input[interleaver_buffer[i]]);

    // RSC turbo encoding #1
    rsc_encoder(size, intlv_output, parity1, tail1);
    
    // multiplexing encoder output
    for (i=0; i<size; i++) {
      encoder_output[3*i] = encoder_input[i];
      encoder_output[3*i+1] = parity0[i];
      encoder_output[3*i+2] = parity1[i];
      printf("i %d : %d,%d,%d (%d,%d,%d)\n",i,encoder_input[i],parity0[i],parity1[i],output[3*i],output[(3*i)+1],output[(3*i)+2]);
    }
    for (i=0; i<TAIL; i++) {
      if (i < TAIL/2)
	encoder_output[i+3*size] = tail0[i];
      else
	encoder_output[i+3*size] = tail1[i-(TAIL/2)];
      printf("i %d(TAIL) : %d,%d\n",i,encoder_output[i+(3*size)],output[i+(3*size)]);
    }

}
*/

#define NTRIALS 1000000

int main(int argc, char *argv[]) {

  int ret,ret2;
  unsigned int errors,uerrors,errors2,crc_misses,iterations,trials,trials2,block_length,errors3,trials3;
  double SNR,sigma,rate;
  unsigned char qbits;
  
  char done0=0;
  char done1=1;
  char done2=0;

  unsigned short iind;
  LTE_DL_eNb_DLSCH_t *dlsch_eNb = new_eNb_dlsch(1,4);
  LTE_DL_UE_DLSCH_t *dlsch_ue = new_ue_dlsch(1,4);
  unsigned int coded_bits;
  unsigned char NB_RB=25;
  unsigned char mod_order[2];
  MIMO_mode_t mimo_mode;


  mod_order[0]=2;
  mod_order[1]=2;
  mimo_mode = SISO;//ALAMOUTI;//ANTCYCLING;

  PHY_config = malloc(sizeof(PHY_CONFIG));
  PHY_vars = malloc(sizeof(PHY_VARS));
  mac_xface = malloc(sizeof(MAC_xface));


  randominit(0);
  set_taus_seed(0);
  
  crcTableInit();
  ccodedot11_init();
  ccodedot11_init_inv();
   
  phy_generate_viterbi_tables();
  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_frame_parms->N_RB_DL            = 25;   //50 for 10MHz and 25 for 5 MHz
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 1;
  lte_frame_parms->nb_antennas_tx     = 2;
  lte_frame_parms->nb_antennas_rx     = 2;
  lte_frame_parms->first_dlsch_symbol = 2;
  init_frame_parms(lte_frame_parms);
  
  copy_lte_parms_to_phy_framing(lte_frame_parms, &(PHY_config->PHY_framing));
  lte_param_init(2,2);
  
  phy_init_top(2); //allocation

  if (argc>1)
    rate = atof(argv[1]);
  else
    rate = .33333;
  printf("Rate %f\n",rate);

  if (argc>2)
    NB_RB = atoi(argv[2]);
  else
    NB_RB = 25;

  printf("NB_RB %d bytes\n",NB_RB);

  if (argc>3)
    qbits = atoi(argv[3]);
  else
    qbits = 4;

  printf("Quantization bits %d\n",qbits);
  coded_bits = (lte_frame_parms->Ncp == 0) ?
    ( NB_RB * (12 * mod_order[0]) * (14-lte_frame_parms->first_dlsch_symbol-3)) :
    ( NB_RB * (12 * mod_order[0]) * (12-lte_frame_parms->first_dlsch_symbol-3)) ;     
  printf("Coded_bits (G) = %d\n",coded_bits);

  block_length = ((int)(coded_bits * rate))>>3;
  printf("Block_length = %d\n",block_length);

  // Test Openair0 3GPP encoder
/*
  test_encoder(block_length,
	       f1f2mat[(block_length-5)*2],   // f1 (see 36121-820, page 14)
	       f1f2mat[((block_length-5)*2)+1],  // f2 (see 36121-820, page 14)
	       3);
 */ //  exit(0);

  if (!dlsch_eNb) {
    printf("Can't get eNb dlsch structures\n");
    exit(-1);
  }

  if (!dlsch_ue) {
    printf("Can't get ue dlsch structures\n");
    exit(-1);
  }
  dlsch_eNb->harq_processes[0]->mimo_mode          = mimo_mode;
  dlsch_eNb->layer_index        = 0;
  dlsch_eNb->harq_processes[0]->mod_order          = mod_order[0];
  dlsch_eNb->harq_processes[0]->active             = 0;
  dlsch_eNb->harq_processes[0]->Nl                 = 1;
  dlsch_eNb->rvidx                                 = 0;
  
  dlsch_ue->harq_processes[0]->mimo_mode           = mimo_mode;
  dlsch_ue->layer_index        = 0;
  dlsch_ue->harq_processes[0]->mod_order           = mod_order[0];
  dlsch_ue->harq_processes[0]->active              = 0;
  dlsch_ue->harq_processes[0]->Nl                  = 1;
  dlsch_ue->rvidx                                  = 0;
  

  for (SNR=-1;SNR<8;SNR+=.2) {


    printf("\n\nSNR %f dB\n",SNR);

    sigma = pow(10.0,-.05*SNR);

    errors=0;
    crc_misses=0;
    errors2=0;
    errors3=0;

    iterations=0;

    if (done0 == 0) {    
    

    printf("Turbo %d (%d bits)\n",block_length,8*block_length);
    
    
    ret = test_logmap8(dlsch_eNb,
		       dlsch_ue,
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
		       &iterations);

    if (ret>=0)
      printf("ref: Errors %d (%f), Uerrors %d (%f), CRC Misses %d (%f), Avg iterations %f\n",errors,(double)errors/trials,uerrors,(double)uerrors/trials,crc_misses,(double)crc_misses/trials,(double)iterations/trials);
    
    if (((double)errors/trials) < 1e-3)
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
    */

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

    if ((done0==1) && (done1==1) && (done2==1)) {
      printf("done\n");
      break;
    }
  }
  return(0);
}

 
