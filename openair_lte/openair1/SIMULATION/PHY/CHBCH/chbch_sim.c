#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "PHY/CONFIG/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/TOOLS/defs.h"
#include "SIMULATION/RF/defs.h"

#ifdef XFORMS
#include "forms.h"
#include "chbch_scope.h"
#endif

#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#define BW 5.0
#define Td 1.0

#undef RF
#define PERFECT_CE

#ifdef XFORMS
extern short *channel[4],*channel_f[4],*rx_sig[4],*rx_sig_f2[4],*rx_sig_f3[4],*sach_data,*magh[4];
extern char *demod_data,*rx_sig_f4;

FD_chbch_scope *form;


void init_forms(PHY_VARS *PHY_vars) {

  int i;

  for (i=0;i<NB_ANTENNAS_RX;i++) {
    channel[i]    = (short*)(PHY_vars->chsch_data[0].channel[i]);
    channel_f[i]  = (short*)(PHY_vars->chsch_data[0].channel_f[i]);
    rx_sig[i] = (short *)(PHY_vars->rx_vars[i].RX_DMA_BUFFER);
    rx_sig_f2[i] = (short *)(PHY_vars->chbch_data[0].rx_sig_f2[i]);
    printf("channel[%d] = %p,channel_f[%d]=%p,rx_sig[%d]=%p,rx_sig_f2[%d]=%p\n",i,channel[i],i,channel_f[i],i,rx_sig[i],i,rx_sig_f2[i]);
  }
  rx_sig_f4  = (char*)(PHY_vars->chbch_data[0].rx_sig_f4);
   
  demod_data  = (char *)(PHY_vars->chbch_data[0].demod_data);
 
  printf("Xforms data done ...\n");
}
#endif //XFORMS

int main(int argc, char *argv[]) {
  //void main() {

  int i,ii,j,jj,delay[2],l;
  //short seed[3];

  int N_frames,N_errors;

  unsigned char nb_antennas_rx,nb_antennas_tx;
  double aoa,ricean_factor[2];
  int nb_taps = 8;
  // if the first or the last tap is 1, something goes wrong
  //double amps[2][8] = {{0,1.0,0,0,0,0,0,0},
  //		       {0,0,0,0,0,0,1.0,0}};
  double amps[2][8] = {{0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685},
  		       {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685}};
  struct complex **ch;
  struct complex phase;
  short alpha[2]; 
  double phase_offset = 0;
  double freq_offset = 0;
  int channel_length;

  struct complex rx_tmp,tx,n;
  double **rx_sig_re, **rx_sig_im;

  char chbch_tx_power,chbch_gain_db[2],gain_N0_db[2];
  int chbch_size;
  int extension;
  unsigned char *chbch_pdu_tx[2],*chbch_pdu_rx[2];

  double gain,gain_N0[2],sqrt_gain;
  double SNR=0.0;
  int errors[2]={0,0};

#ifdef DUALSTREAM
#define NUMBER_OF_PARALLEL_CHBCH 2
#else
#define NUMBER_OF_PARALLEL_CHBCH 1
#endif

  unsigned char chsch_indices[2] = {1, 2};
  unsigned char chsch_index;

  short *rx_vec[NB_ANTENNAS_RX]; //received signal vector
  int ret[2];
  int tx_energy;

  FILE *rx_frame_file;
#ifdef FILE_OUTPUT
  FILE *out_file;
  char out_filename[256];
#endif

  int result;

#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE
  unsigned char icflag;
  char print_buf[4096];

  double nf[2] = {3.0,3.0}; //currently unused
  double ip =0.0;
  double N0W, path_loss, path_loss_dB;


  if (argc < 11) {
    printf("Not enough arguments: usage chbch_sim sener nener nframes nant_tx nant_rx errors rfact aoa\n");
    printf("sener  : Signal strength in dB (45<x<65 dB)\n");
    printf("nener  : Noise strength in dB (<65 dB)\n");
    printf("nframes: Number of Frames to run (1 is minimum)\n");
    printf("nant_tx: Number of Transmit Antennas\n");
    printf("nant_rx: Number of Receive Antennas\n");
    printf("errors : Number of Error Events+1 until simulation stops (by putting 1, it stops after one try!)\n");
    printf("rfact:   Ricean K Factor in Channel Simulation (in dB, 60 is AWGN channel, 0 is Rayleigh channel, any negative value results in a frequency flat channel\n");
    printf("aoa  :   Phase difference between the two streams in fractions of 2*pi, i.e., phase=2*pi/aoa (aoa=2 channels orthogonal)\n");
    printf("freq :   RF frequency offset (Hz)\n");
    printf("icflag : Type of IC receiver (0-MMSE, 1-ML)\n");
    printf("s2ener : Signal strength of second stream (optional)\n");
  

    exit(-1);
  
  }  

  chbch_gain_db[0] = atoi(argv[1]);
  if (argc==12)
    chbch_gain_db[1] = atoi(argv[11]);
  else
    chbch_gain_db[1] = atoi(argv[1]);

  gain_N0_db[0] = atoi(argv[2]);
  gain_N0_db[1] = atoi(argv[2]);
  gain_N0[0] = pow(10.0,.1*(atof(argv[2])));
  gain_N0[1] = pow(10.0,.1*(atof(argv[2])));

  N_frames = atoi(argv[3]);

  nb_antennas_tx = atoi(argv[4]);
  nb_antennas_rx = atoi(argv[5]);

  N_errors = atoi(argv[6]);

  ricean_factor[0] = pow(10.0,-.1*atof(argv[7]));
  ricean_factor[1] = pow(10.0,-.1*atof(argv[7]));
  aoa = atof(argv[8]);

  freq_offset = atof(argv[9]);

  icflag = atoi(argv[10]);

  if (nb_antennas_tx > NB_ANTENNAS_TX){
    printf("[OPENAIR][SIM][CHBCH] Num TX antennas %d > %d\n",nb_antennas_tx,NB_ANTENNAS_RX);
    exit(-1);
  }
  if (nb_antennas_rx > NB_ANTENNAS_RX) {
    printf("[OPENAIR][SIM][CHBCH] Num RX antennas %d > %d\n",nb_antennas_rx,NB_ANTENNAS_RX);
    exit(-1);
  }
  
  channel_length = (int) 11+2*BW*Td;

  printf("[OPENAIR][SIM][CHBCH] Allocating memory for channel\n");
  ch = (struct complex**) malloc(nb_antennas_tx * nb_antennas_rx * sizeof(struct complex*));
  for (ii = 0; ii<nb_antennas_tx * nb_antennas_rx; ii++)
    ch[ii] = (struct complex*) malloc(channel_length * sizeof(struct complex));

  printf("[OPENAIR][SIM][CHBCH] Allocating memory for PHY_VARS\n");

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));
  
  if((config = fopen("config.cfg","r")) == NULL) // this can be configured
    {
      printf("[OPENAIR][SIM][CHBCH] The openair configuration file <config.cfg> could not be found!");
      exit(0);
    }		
  
  if ((scenario= fopen("scenario.scn","r")) ==NULL)
    {
      printf("[OPENAIR][SIM][CHBCH] The openair scenario file <scenario.scn> could not be found!");
      exit(0);
    }
  
  printf("[OPENAIR][SIM][CHBCH] Opened configuration files\n");

  reconfigure_MACPHY(scenario);

  dump_config();

  mac_xface->is_cluster_head = 1;

  phy_init(nb_antennas_tx);
  printf("[OPENAIR][SIM][CHBCH] Initialized PHY variables\n");

  rx_sig_re = malloc(nb_antennas_rx*sizeof (double *));
  rx_sig_im = malloc(nb_antennas_rx*sizeof (double *));

  for (i=0;i<nb_antennas_rx;i++) {
    rx_sig_re[i] = (double *)malloc(FRAME_LENGTH_COMPLEX_SAMPLES * sizeof (double ));
    rx_sig_im[i] = (double *)malloc(FRAME_LENGTH_COMPLEX_SAMPLES * sizeof (double ));
  }


#ifdef XFORMS
  init_forms(PHY_vars);
  /*
    printf("Rev: ");
    for (i=0;i<16;i++)
    printf("%d ",rev[i]);
    printf("\n");
  */
#endif 

  srand(1);
  randominit();

  chbch_size = (NUMBER_OF_CARRIERS_PER_GROUP*(NUMBER_OF_CHBCH_SYMBOLS)*16)>>3;
  //printf("[OPENAIR][SIM][CHBCH] chbch_size = %d\n",chbch_size);
  chbch_pdu_tx[0]  = malloc(chbch_size);
  chbch_pdu_tx[1]  = malloc(chbch_size);

  for (i=0;i<chbch_size-4;i++) {
    chbch_pdu_tx[0][i] = i;
    chbch_pdu_tx[1][i] = chbch_size-i;
  }

  //  printf("[OPENAIR][SIM][CHBCH] Filled CHBCH PDU (%d bytes) with random data\n",chbch_size);

  delay[0] = 0;
  delay[1] = 0;

  mac_xface->frame = 0;

  /*
    printf("Rev: ");
    for (i=0;i<16;i++)
    printf("%d ",rev[i]);
    printf("\n");
  */

  msg("[OPENAIR][SIM][CHBCH] CHBCH_FREQUENCY_REUSE_FACTOR=%d, NUMBER_OF_PARALLEL_CHBCH=%d\n",
      CHBCH_FREQUENCY_REUSE_FACTOR,
      NUMBER_OF_PARALLEL_CHBCH);

#ifdef XFORMS
  fl_initialize(&argc, argv, 0, 0, 0);    /* SIGSCOPE */


  form = create_form_chbch_scope();                 /* SIGSCOPE */


  fl_show_form(form->chbch_scope,FL_PLACE_HOTSPOT,FL_FULLBORDER,NULL);   /* SIGSCOPE */
#endif


  while ((errors[0] < N_errors) && (mac_xface->frame < N_frames)) {

    Zero_Buffer(PHY_vars->rx_vars[0].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);
    Zero_Buffer(PHY_vars->rx_vars[1].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);
    for (i=0;i<nb_antennas_rx;i++) {
      Zero_Buffer( rx_sig_re[i], FRAME_LENGTH_COMPLEX_SAMPLES * sizeof (double ));
      Zero_Buffer( rx_sig_im[i], FRAME_LENGTH_COMPLEX_SAMPLES * sizeof (double ));
    }

    for (chsch_index = 1; chsch_index<=NUMBER_OF_PARALLEL_CHBCH; chsch_index++) {

      //clear the tx buffer
      for (j=0;j<nb_antennas_tx;j++)      
	Zero_Buffer(PHY_vars->tx_vars[j].TX_DMA_BUFFER,FRAME_LENGTH_SAMPLES*2);

      chbch_tx_power = phy_generate_chbch(chsch_index,
					  1,
					  nb_antennas_tx,
					  chbch_pdu_tx[chsch_index-1]);
    
      tx_energy=0;
      for (j=0;j<nb_antennas_tx;j++) {
	tx_energy += signal_energy(&PHY_vars->tx_vars[j].TX_DMA_BUFFER[SAMPLE_OFFSET_CHBCH_NO_PREFIX],
				   (NUMBER_OF_CHBCH_SYMBOLS)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX);
      }

      gain = pow(10.0,.1*((double)chbch_gain_db[chsch_index-1] - (double)chbch_tx_power));
      //gain = pow(10.0,.1*((double)chbch_gain_db[chsch_index-1]))/((double)tx_energy);
      sqrt_gain = sqrt(gain);

#ifdef DEBUG_PHY    
      printf("[OPENAIR][SIM][CHBCH %d] tx_power = %d dB,gain = %f (%f dB),gain_N0 = %f (%f dB)\n",
	     chsch_index, 
	     chbch_tx_power,
	     gain,
	     (double)chbch_gain_db[chsch_index-1]-(double)chbch_tx_power,
	     gain_N0[0],
	     gain_N0_db[0]);

#endif

      /*
      //apply phase rotation at the 2nd transmitter
      if (chsch_index==2) {
	alpha[0] = (short) (cos(2*M_PI*i*offset)*32767.0);
	alpha[1] = (short) (sin(2*M_PI*i*offset)*32767.0);
	//printf("alpha = %d +1j %d\n",alpha[0],alpha[1]);

	for (i=0;i<TX_RX_SWITCH_SYMBOL;i++) {
	  for (j=0;j<nb_antennas_tx;j++) {
	    rotate_cpx_vector_norep((short*) &PHY_vars->tx_vars[j].TX_DMA_BUFFER[i*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
				    alpha,
				    (short*) &PHY_vars->tx_vars[j].TX_DMA_BUFFER[i*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES],
				    OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
				    15);
	  }
	}
      }
      */
    
#ifdef DEBUG_PHY    
      for (ii=0; ii<nb_antennas_tx; ii++) {
	sprintf(fname,"txsig%d_ch%d.m",ii,chsch_index);
	sprintf(vname,"txs%d_ch%d",ii,chsch_index);
      
	write_output(fname,vname,
		     (s16 *)&PHY_vars->tx_vars[ii].TX_DMA_BUFFER[0],
		     TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		     1,
		     1);
      }

      // generate channels
      printf("[OPENAIR][SIM][CHBCH] Generating MIMO Channels\n");
#endif //DEBUG_PHY	       



      for (i=0;i<nb_antennas_rx;i++)      // RX Antenna loop
	for (j=0;j<nb_antennas_tx;j++) {  // TX Antenna loop
	
	  if (chsch_index==1) {
	    phase.r = 1;
	    phase.i = 0;
	  }
	  else {
	    phase.r = cos(2*M_PI*(i-j)/aoa);
	    phase.i = sin(2*M_PI*(i-j)/aoa);
	  }

	  Zero_Buffer(ch[i + (j*nb_antennas_rx)], channel_length * sizeof(struct complex));

	  if (atof(argv[7])<0) {

	      if (chsch_index==1) {
		ch[i + (j*nb_antennas_rx)][0].r = gaussdouble(0.0,1.0)/sqrt(2);
		ch[i + (j*nb_antennas_rx)][0].i = gaussdouble(0.0,1.0)/sqrt(2);
	      }
	      else {
		ch[i + (j*nb_antennas_rx)][0].r = gaussdouble(0.0,1.0)/sqrt(2);
		ch[i + (j*nb_antennas_rx)][0].i = gaussdouble(0.0,1.0)/sqrt(2);
	      }
	      
	      /*
		if (i==j) {
		ch[i + (j*nb_antennas_rx)][0].r = 1;
		ch[i + (j*nb_antennas_rx)][0].i = 0;
		}
	      */
	    }
	  else {
	    random_channel(amps[chsch_index-1],Td, 8,BW,ch[i + (j*nb_antennas_rx)],ricean_factor[chsch_index-1],&phase);
	  }

#ifdef DEBUG_PHY
	  sprintf(fname,"channel_ch%d_%d%d.m",chsch_index,i,j);
	  sprintf(vname,"chan_ch%d_%d%d",chsch_index,i,j);
	  write_output(fname,vname,
		       ch[i + (j*nb_antennas_rx)],
		       channel_length,
		       1,
		       7);
#endif

	}
  
      /*    
      for (i=0;i<nb_antennas_rx;i++)      // RX Antenna loop
	for (j=0;j<nb_antennas_tx;j++) {  // TX Antenna loop
	  printf("H%d%d = ",i,j);
	  for (l=0;l<16;l++)
	    printf("%f+j*(%f),",ch[i+(j*nb_antennas_rx)][l]);
	  printf("\n\n");
	}
      */

      // MIMO channel
      for (i=0;i<(TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
	for (ii=0;ii<nb_antennas_rx;ii++) {
	  rx_tmp.r = 0;
	  rx_tmp.i = 0;
	  for (j=0;j<nb_antennas_tx;j++) {
	    for (l = 0;l<channel_length;l++) {
	      if (i-l>=0) {
#ifndef BIT8_TXMUX	    
		tx.r = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[2*(i-l)])*sqrt(gain);
		tx.i = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[1+(2*(i-l))])*sqrt(gain);
#else
		tx.r = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+(4*(i-l))])*sqrt(gain);
		tx.i = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+1+(4*(i-l))])*sqrt(gain);
#endif	    
	      }
	      else {
		tx.r =0;
		tx.i =0;
	      }
	      rx_tmp.r += (tx.r * ch[ii+(j*nb_antennas_rx)][l].r) - (tx.i * ch[ii+(j*nb_antennas_rx)][l].i);
	      rx_tmp.i += (tx.i * ch[ii+(j*nb_antennas_rx)][l].r) + (tx.r * ch[ii+(j*nb_antennas_rx)][l].i);
	    
	    } //l
	  }  // j
	  rx_sig_re[ii][i+delay[chsch_index-1]] += rx_tmp.r;
	  rx_sig_im[ii][i+delay[chsch_index-1]] += rx_tmp.i;
	  
	} // ii
      } // i
#ifdef DEBUG_PHY    
    for (ii=0; ii<nb_antennas_rx; ii++) {
      sprintf(fname,"rxsig%d_ch%d.m",ii,chsch_index);
      sprintf(vname,"rxs%d_ch%d",ii,chsch_index);
      write_output(fname,vname,
		   (s16 *)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0],
		   TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		   1,
		   1);
    }
#endif //DEBUG_PHY	       
    } // chsch_index

#ifdef RF
#ifdef PERFECT_CE
#error "Perfect channel estimation not possible with RF"
#endif

    // scale by path_loss = NOW - P_noise
    N0W          = -95.87;
    path_loss_dB = N0W - gain_N0_db[0];
    path_loss    = pow(10,path_loss_dB/10);

    for (i=0;i<(TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
      for (ii=0;ii<nb_antennas_rx;ii++) {
	rx_sig_re[ii][i]*=sqrt(path_loss); 
	rx_sig_im[ii][i]*=sqrt(path_loss); 
      }
    }
    PHY_vars->rx_vars[0].rx_total_gain_dB=-path_loss_dB;

    // RF model
    rf_rx(rx_sig_re,
	  rx_sig_im,
	  nb_antennas_rx,
	  (TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
	  1.0/6.5e6 * 1e9,      // sampling time (ns)
	  freq_offset,            // freq offset (Hz) (-20kHz..20kHz)
	  0.0,            // drift (Hz) NOT YET IMPLEMENTED
	  nf,             // noise_figure NOT YET IMPLEMENTED
	  PHY_vars->rx_vars[0].rx_total_gain_dB,            // rx_gain (dB)
	  200,            // IP3_dBm (dBm)
	  &ip,            // initial phase
	  30.0e3,         // pn_cutoff (kHz)
	  -500.0,          // pn_amp (dBc) default: 50
	  0.0,           // IQ imbalance (dB),
	  0.0);           // IQ phase imbalance (rad)

#ifdef DEBUG_PHY    
    for (ii=0; ii<nb_antennas_rx; ii++) {
      sprintf(fname,"rx_rf_re%d.m",ii);
      sprintf(vname,"rx_rf_re%d",ii);
      write_output(fname,vname,
		   rx_sig_re,
		   TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		   1,
		   7);
      sprintf(fname,"rx_rf_im%d.m",ii);
      sprintf(vname,"rx_rf_im%d",ii);
      write_output(fname,vname,
		   rx_sig_im,
		   TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		   1,
		   7);
    }
#endif //DEBUG_PHY
#endif //RF
    

    for (i=0;i<(TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
      for (ii=0;ii<nb_antennas_rx;ii++) {
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[2*i] = (short) (rx_sig_re[ii][i]); 
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[1+2*i] = (short) (rx_sig_im[ii][i]); 
      }
    }

#ifndef RF
    // AWGN
#ifdef PERFECT_CE
    for (i=0;i<(TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
#else
    for (i=(NUMBER_OF_CHSCH_SYMBOLS_MAX-1)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i<(TX_RX_SWITCH_SYMBOL)*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
#endif
      for (ii=0;ii<nb_antennas_rx;ii++) {
	rx_tmp.r = 0;
	rx_tmp.i = 0;
	n.r = sqrt(.5*gain_N0[ii])*gaussdouble(0.0,1.0);
	n.i = sqrt(.5*gain_N0[ii])*gaussdouble(0.0,1.0);
	
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[2*i] += (short)n.r ; 
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[1+(2*i)] += (short)n.i; 
      }
    }
#endif //RF

    /*
    // optional: read rx_frame from file
#ifdef DUALSTREAM
    if ((rx_frame_file = fopen("rx_frame_2streams.dat","r")) == NULL)
#else
    if ((rx_frame_file = fopen("rx_frame_1stream.dat","r")) == NULL)
#endif
      {
	printf("[openair][CHBCH_TEST][INFO] Cannot open rx_frame.m data file\n");
	exit(0);
      }

    result = fread((void *)PHY_vars->rx_vars[0].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
    printf("Read %d bytes\n",result);
    result = fread((void *)PHY_vars->rx_vars[1].RX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
    printf("Read %d bytes\n",result);

    fclose(rx_frame_file);

    */

    mac_xface->is_cluster_head = 0;
	       
    /*
    phy_synch_time((short*) PHY_vars->rx_vars[0].RX_DMA_BUFFER,
		   &sync_pos,
		   TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		   768,
		   CHSCH,
		   1);

    PHY_vars->rx_vars[0].offset = 0; //sync_pos;

#ifdef DEBUG_PHY      
    msg("[OPENAIR][SIM][CHBCH] sync_pos = %d\n",sync_pos);
#endif
    */


    for (chsch_index=0;chsch_index<4;chsch_index++) {
      phy_channel_estimation_top(PHY_vars->rx_vars[0].offset,
				 chsch_index+SYMBOL_OFFSET_CHSCH,
				 0,
				 chsch_index,
				 nb_antennas_rx,
				 CHSCH);
    }

    /*
    phy_adjust_synch_multi_CH(1,16384,CHSCH);

    phy_adjust_sync_CH2(1,0,16384,CHSCH);
    */

    /*
    for (ii=0;ii<NB_ANTENNAS_TX;ii++)
      for (jj=0;jj<NB_ANTENNAS;jj++) {
	phy_channel_interpolation(&PHY_vars->chsch_data[chsch_index].channel_f[ii][0],
				  &PHY_vars->chsch_data[chsch_index].channel_f_interp[ii][jj][0],
				  jj);
	
#ifdef DEBUG_PHY    
	sprintf(fname,"chsch_channelF_interp_%d%d.m",ii,jj);
	sprintf(vname,"h%d%d",ii,jj);
	write_output(fname,vname,
		     (s16 *)&PHY_vars->chsch_data[chsch_index].channel_f_interp[ii][jj][0],
		     NUMBER_OF_OFDM_CARRIERS*2,
		     2,
		     1);
#endif //DEBUG_PHY	       
      }
    */

#ifdef DEBUG_PHY    
    for (ii=0; ii<nb_antennas_rx; ii++) {
      sprintf(fname,"rxsig%d.m",ii);
      sprintf(vname,"rxs%d",ii);
      write_output(fname,vname,
		   (s16 *)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0],
		   TX_RX_SWITCH_SYMBOL*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		   1,
		   1);
    }
    printf("[OPENAIR][SIM][CHBCH] Starting RX\n");	       
#endif //DEBUG_PHY

    
#ifdef DUALSTREAM  
    if (icflag == 0) {
      
      phy_calc_mmse_filter(PHY_vars->chsch_data[1].channel_f,
			   PHY_vars->chsch_data[2].channel_f,
			   PHY_vars->chsch_data[1].channel_mmse_filter_f,
			   PHY_vars->chsch_data[1].det,
			   PHY_vars->chsch_data[1].idet,
			   PHY_vars->PHY_measurements.n0_power[0]);
    }
#endif //DUALSTREAM
    /*
#ifdef DEBUG_PHY    
    for (ii=0;ii<NB_ANTENNAS_TX;ii++)
      for (jj=0;jj<NB_ANTENNAS_RX;jj++) {


	sprintf(fname,"chsch1_mmse_%d%d.m",ii,jj);
	sprintf(vname,"h%d%d",ii,jj);
	write_output(fname,vname,
		     (s16 *)&PHY_vars->chsch_data[1].channel_mmse_filter_f[ii][jj][0],
		     NUMBER_OF_OFDM_CARRIERS*2,
		     2,
		     1);
      }
#endif //DEBUG_PHY
    */

    rx_vec[0] = (short*) PHY_vars->rx_vars[0].RX_DMA_BUFFER;
    rx_vec[1] = (short*) PHY_vars->rx_vars[1].RX_DMA_BUFFER;

    chbch_pdu_rx[0] = malloc(CHBCH_PDU_SIZE);
    chbch_pdu_rx[1] = malloc(CHBCH_PDU_SIZE);

#ifndef DUALSTREAM
    ret[0] = phy_decode_chbch(1,nb_antennas_rx,nb_antennas_tx,chbch_pdu_rx[0],CHBCH_PDU_SIZE);
    ret[1] = -1;
#else

    //msg("chsch_indices = %d, %d\n",chsch_indices[0],chsch_indices[1]);

    if (icflag == 0) 
      phy_decode_chbch_2streams(chsch_indices,
				MMSE,
				nb_antennas_rx,
				nb_antennas_tx,
				chbch_pdu_rx,
				ret,
				CHBCH_PDU_SIZE);
    else
      phy_decode_chbch_2streams_ml(chsch_indices,
				   0,
				   nb_antennas_rx,
				   nb_antennas_tx,
				   chbch_pdu_rx,
				   ret,
				   CHBCH_PDU_SIZE);
  
#endif


    if (ret[0] == -1) errors[0]++;
    if (ret[1] == -1) errors[1]++;
    if ((mac_xface->frame % 100) == 0) { 
      chbch_stats_read(print_buf,NULL,0,4096);
      printf("%s",print_buf);
    }

#ifdef DEBUG_PHY
    msg("[OPENAIR][SIM][CHBCH] frame = %d, Dual stream return code = (%d,%d), errors = (%d,%d)\n",
	mac_xface->frame,ret[0],ret[1],errors[0],errors[1]);
#endif
    if ((ret[0] == -1) || (ret[1] == -1))
      msg("[OPENAIR][SIM][CHBCH] frame =%d, errors[0]= %d, errors[1] = %d\n",mac_xface->frame,errors[0],errors[1]);
    mac_xface->frame++;
#ifdef XFORMS
    chbch_scope_idle_callback();
#endif
  }


  printf("[OPENAIR][SIM][CHBCH] Exiting, Error Rate = %e,%e\n",(double)errors[0]/(mac_xface->frame), (double)errors[1]/(mac_xface->frame));

#ifdef FILE_OUTPUT
  sprintf(out_filename,"results_rx%d_rice%d_aoa%d_freq%d_ntx%d_nrx%d.csv", icflag, (int) atoi(argv[7]), (int) aoa, (int) freq_offset, nb_antennas_tx, nb_antennas_rx);
  out_file = fopen(out_filename,"a");
  // Gain S0, Gain S1, N0, Frames, RX type, Ant Tx, Ant Rx, Rice, AoA, FER S0, FER S1
  fprintf(out_file,"%d; %d; %f; %u; %d; %d; %d; %f; %f; %f; %f\n", chbch_gain_db[0], chbch_gain_db[1], gain_N0_db[0], mac_xface->frame, icflag, nb_antennas_tx, nb_antennas_rx, ricean_factor[0], aoa, (double)errors[0]/(mac_xface->frame),(double)errors[1]/(mac_xface->frame));
  fclose(out_file);
#endif //FILE_OUTPUT

  printf("[OPENAIR][SIM][CHBCH] Clearing memory for channel\n");
  for (ii = 0; ii<nb_antennas_tx * nb_antennas_rx; ii++)
    free(ch[ii]);
  free(ch);

  for (i=0;i<nb_antennas_rx;i++) {
    free(rx_sig_re[i]);
    free(rx_sig_im[i]);
  }
  free(rx_sig_re);
  free(rx_sig_im);


  printf("[OPENAIR][SIM][CHBCH] Clearing dummy_mac_pdu\n");
  //free(chbch_pdu_rx[0]);
  //  free(chbch_pdu_rx[1]);
  //free(chbch_pdu_tx[0]);
  //  free(chbch_pdu_tx[1]);
  
  printf("[OPENAIR][SIM][CHBCH] Clearing PHY\n");
  phy_cleanup();

  return 0;
  
}
