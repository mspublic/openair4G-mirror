/* 
File:   sync_sim.c
Author: florian.kaltenberger@eurecom.fr
Date:   7.1.2008
Description: 
        This test bench tests the distributed synchronization. 
        It generates the signal from a primary CH and a MR, which 
	is assumed to be in sync with CH1.     
	This function is based on chbch_sim.c
*/

#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "PHY/CONFIG/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SIMULATION/TOOLS/defs.h"


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

extern double pow(double,double);

int main(int argc, char** argv) {
  //void main() {

  int i,ii,j,jj,delay,l;
  //short seed[3];

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
  double offset = 0.05;
  int channel_length;

  struct complex rx_tmp,tx,n;

  char tx_power,gain_db[2];
  int chbch_size;
  int extension;
  unsigned char *chbch_pdu_tx[2],*chbch_pdu_rx[2];

  double gain,gain_N0[2],sqrt_gain;
  double SNR=0.0;
  int errors[2]={0,0};

  unsigned char index;

  short *rx_vec[NB_ANTENNAS_RX]; //received signal vector
  char ret[2] = {0, 0};

  FILE *rx_frame_file;
  int result;

#ifdef USER_MODE
  char fname[40],vname[40];
#endif //USER_MODE

  if (argc < 9) {
    printf("Not enough arguments: usage sync_sim sener nener nframes nant_tx nant_rx errors rfact aoa\n");
    printf("sener  : Signal strength in dB (<65 dB)\n");
    printf("nener  : Noise strength in dB (<65 dB)\n");
    printf("nframes: Number of Frames to run (1 is minimum)\n");
    printf("nant_tx: Number of Transmit Antennas\n");
    printf("nant_rx: Number of Receive Antennas\n");
    printf("errors : Number of Error Events+1 until simulation stops (by putting 1, it stops after one try!)\n");
    printf("rfact:   Ricean K Factor in Channel Simulation (in dB, 60 is AWGN channel, 0 is Rayleigh channel\n");
    printf("aoa  :   Phase difference between the two streams in fractions of 2*pi, i.e., phase=2*pi/aoa (aoa=2 channels orthogonal)\n");
    printf("s2ener : Signal strength of second stream (optional)\n");
  

    exit(-1);
  
  }  

  nb_antennas_tx = atoi(argv[4]);
  nb_antennas_rx = atoi(argv[5]);

  gain_db[0] = atoi(argv[1]);
  if (argc==10)
    gain_db[1] = atoi(argv[9]);
  else
    gain_db[1] = atoi(argv[1]);

  if (nb_antennas_tx > NB_ANTENNAS_TX){
    printf("[OPENAIR][SIM][CHBCH] Num TX antennas %d > %d\n",nb_antennas_tx,NB_ANTENNAS_RX);
    exit(-1);
  }
  if (nb_antennas_rx > NB_ANTENNAS_RX) {
    printf("[OPENAIR][SIM][CHBCH] Num RX antennas %d > %d\n",nb_antennas_rx,NB_ANTENNAS_RX);
    exit(-1);
  }
  
  ricean_factor[0] = pow(10.0,-.1*atof(argv[7]));
  ricean_factor[1] = pow(10.0,-.1*atof(argv[7]));
  aoa = atof(argv[8]);
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

  srand(1);
  randominit();

  chbch_size = (NUMBER_OF_CARRIERS_PER_GROUP*(NUMBER_OF_CHBCH_SYMBOLS)*16)>>3;
  //printf("[OPENAIR][SIM][CHBCH] chbch_size = %d\n",chbch_size);
  chbch_pdu_tx[0]  = malloc(chbch_size);
  chbch_pdu_tx[1]  = malloc(chbch_size);
  chbch_pdu_rx[0]  = malloc(chbch_size);
  chbch_pdu_rx[1]  = malloc(chbch_size);

  for (i=0;i<chbch_size-4;i++) {
    chbch_pdu_tx[0][i] = i;
    chbch_pdu_tx[1][i] = chbch_size-i;
  }
  //printf("[OPENAIR][SIM][CHBCH] Filled CHBCH PDU (%d bytes) with random data\n",chbch_size);

  delay = 0;

  mac_xface->frame = 0;

  while ((errors[0] < atoi(argv[6])) && (mac_xface->frame < atoi(argv[3]))) {

    Zero_Buffer(PHY_vars->rx_vars[0].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);
    Zero_Buffer(PHY_vars->rx_vars[1].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);

    for (index = 0; index<2; index++) {

      //clear the tx buffer
      for (j=0;j<nb_antennas_tx;j++)      
	Zero_Buffer(PHY_vars->tx_vars[j].TX_DMA_BUFFER,FRAME_LENGTH_SAMPLES*2);

      if (index == 0) {
	mac_xface->is_cluster_head = 1;
	mac_xface->is_primary_cluster_head = 1;
	mac_xface->is_secondary_cluster_head = 0;

	tx_power = phy_generate_chbch(1,
				      1,
				      nb_antennas_tx,
				      chbch_pdu_tx[index]);
      }
      else if (index == 1) {
	mac_xface->is_cluster_head = 0;
	mac_xface->is_primary_cluster_head = 0;
	mac_xface->is_secondary_cluster_head = 0;

	phy_generate_sch(0,
			 MRSCH_INDEX,
			 SYMBOL_OFFSET_MRSCH,
			 0xffff,
			 1,
			 nb_antennas_tx);

	tx_power = phy_generate_mrbch(MRSCH_INDEX,
				      1,
				      nb_antennas_tx,
				      chbch_pdu_tx[index]);
      }
      else {
	tx_power = 0;
      }

    
      gain = pow(10.0,.1*((double)gain_db[index] - (double)tx_power));
      gain_N0[0] = pow(10.0,.1*(atof(argv[2])));
      gain_N0[1] = pow(10.0,.1*(atof(argv[2])));
      sqrt_gain = sqrt(gain);

#ifdef DEBUG_PHY    
      printf("[OPENAIR][SIM][STREAM %d] tx_power = %d dB,gain = %f (%f dB),gain_N0 = %f (%f dB)\n",
	     index, 
	     tx_power,
	     gain,
	     (double)gain_db[index]-(double)tx_power,
	     gain_N0[0],
	     atof(argv[2]));

#endif

      /*
      //apply phase rotation at the 2nd transmitter
      if (chsch_index==2) {
	alpha[0] = (short) (cos(2*M_PI*i*offset)*32767.0);
	alpha[1] = (short) (sin(2*M_PI*i*offset)*32767.0);
	//printf("alpha = %d +1j %d\n",alpha[0],alpha[1]);

	for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
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
	sprintf(fname,"txsig%d_ch%d.m",ii,index);
	sprintf(vname,"txs%d_ch%d",ii,index);
      
	write_output(fname,vname,
		     (s16 *)&PHY_vars->tx_vars[ii].TX_DMA_BUFFER[0],
		     FRAME_LENGTH_COMPLEX_SAMPLES,
		     1,
		     1);
      }

      // generate channels
      printf("[OPENAIR][SIM][CHBCH] Generating MIMO Channels\n");
#endif //DEBUG_PHY	       



      for (i=0;i<nb_antennas_rx;i++)      // RX Antenna loop
	for (j=0;j<nb_antennas_tx;j++) {  // TX Antenna loop
	
	  if (index==0) {
	    phase.r = 1;
	    phase.i = 0;
	  }
	  else {
	    phase.r = cos(2*M_PI*(i-j)/aoa);
	    phase.i = sin(2*M_PI*(i-j)/aoa);
	  }

	  random_channel(amps[index],Td, 8,BW,ch[i + (j*nb_antennas_rx)],ricean_factor[index],&phase);
	  //ch[j + (i*nb_antennas_rx)][0].r = gaussdouble(0.0,1.0)/sqrt(2);
	  //ch[j + (i*nb_antennas_rx)][0].i = gaussdouble(0.0,1.0)/sqrt(2);
	  
	  

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
      for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
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
	    
	    }
	  }  // j
	  ((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay])[2*i] += (short)rx_tmp.r ; 
	  ((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay])[1+(2*i)] += (short)rx_tmp.i; 

	  
	} // ii
      } // i
#ifdef DEBUG_PHY    
    for (ii=0; ii<nb_antennas_rx; ii++) {
      sprintf(fname,"rxsig%d_ch%d.m",ii,index);
      sprintf(vname,"rxs%d_ch%d",ii,index);
      write_output(fname,vname,
		   (s16 *)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay],
		   FRAME_LENGTH_COMPLEX_SAMPLES,
		   1,
		   1);
    }
#endif //DEBUG_PHY	       
    } // chsch_index

    // AWGN
    for (i=0;i<FRAME_LENGTH_COMPLEX_SAMPLES;i++) {
      for (ii=0;ii<nb_antennas_rx;ii++) {
	rx_tmp.r = 0;
	rx_tmp.i = 0;
	n.r = sqrt(.5*gain_N0[ii])*gaussdouble(0.0,1.0);
	n.i = sqrt(.5*gain_N0[ii])*gaussdouble(0.0,1.0);
	
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay])[2*i] += (short)n.r ; 
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay])[1+(2*i)] += (short)n.i; 
      }
    }



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


#ifdef DEBUG_PHY    
    for (ii=0; ii<nb_antennas_rx; ii++) {
      sprintf(fname,"rxsig%d.m",ii);
      sprintf(vname,"rxs%d",ii);
      write_output(fname,vname,
		   (s16 *)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[delay],
		   FRAME_LENGTH_COMPLEX_SAMPLES,
		   1,
		   1);
    }
    printf("[OPENAIR][SIM][CHBCH] Starting RX\n");	       
#endif //DEBUG_PHY

    mac_xface->is_cluster_head = 1;
    mac_xface->is_primary_cluster_head = 0;
    mac_xface->is_secondary_cluster_head = 1;
	       
    
    phy_synch_time((short*) PHY_vars->rx_vars[0].RX_DMA_BUFFER,
		   &sync_pos,
		   FRAME_LENGTH_COMPLEX_SAMPLES,
		   768,
		   SCH,
		   MRSCH_INDEX);

    //#ifdef DEBUG_PHY      
    msg("[OPENAIR][SIM][CHBCH] sync_pos = %d\n",sync_pos);
    //#endif

    PHY_vars->rx_vars[0].offset = sync_pos;

    phy_channel_estimation_top(PHY_vars->rx_vars[0].offset,
			       SYMBOL_OFFSET_MRSCH,
			       0,
			       MRSCH_INDEX,
			       nb_antennas_rx,
			       SCH);
    
    
    ret[0] = phy_decode_mrbch(MRSCH_INDEX,
			   nb_antennas_rx,
			   nb_antennas_tx,
			   chbch_pdu_rx[1],
			   MRBCH_PDU_SIZE);

    if (ret[0] == -1) errors[0]++;
    if (ret[1] == -1) errors[1]++;

    if ((ret[0] == -1) || (ret[1] == -1))
      msg("[OPENAIR][SIM][CHBCH] frame =%d, errors[0]= %d, errors[1] = %d\n",mac_xface->frame,errors[0],errors[1]);
    mac_xface->frame++;
  }

  printf("[OPENAIR][SIM][CHBCH] Clearing memory for channel\n");
  for (ii = 0; ii<nb_antennas_tx * nb_antennas_rx; ii++)
    free(ch[ii]);
  free(ch);

  printf("[OPENAIR][SIM][CHBCH] Clearing dummy_mac_pdu\n");
  //free(chbch_pdu_rx[0]);
  //  free(chbch_pdu_rx[1]);
  //free(chbch_pdu_tx[0]);
  //  free(chbch_pdu_tx[1]);
  
  printf("[OPENAIR][SIM][CHBCH] Clearing PHY\n");
  phy_cleanup();

  printf("[OPENAIR][SIM][CHBCH] Exiting, Error Rate = %e,%e\n",(double)errors[0]/(mac_xface->frame), (double)errors[1]/(mac_xface->frame));

  return 0;  
}
