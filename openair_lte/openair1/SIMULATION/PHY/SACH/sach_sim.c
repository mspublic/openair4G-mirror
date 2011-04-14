#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "PHY/CONFIG/vars.h"
#include "MAC_INTERFACE/vars.h"
#include "SIMULATION/TOOLS/defs.h"
#include "PHY/TOOLS/defs.h"
#include "SIMULATION/TOOLS/defs.h"

//#include "dlc_engine.h"
//#include "MAC_defs.h"

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
#define Td 2.0

#define NUMBER_OF_SYMBOLS 20
#define NUMBER_OF_SACH_FREQUENCY_GROUPS 16 

#define TIME_ALLOC_UE (0x20+(NUMBER_OF_SYMBOLS>>2))
#define TIME_ALLOC_NODEB (0x30+(NUMBER_OF_SYMBOLS>>2))

#define NUMBER_OF_PILOTS_PER_GROUP 2

int main(int argc, char** argv) {
//void main() {

  int i,ii,j,ret,delay,l;
  short seed[3];


      double amps[8] = {1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  //double amps[8] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
  struct complex ch[NB_ANTENNAS_RX*NB_ANTENNAS_TX][11+(int)(1+2*BW*Td)];
  struct complex rx_tmp,tx,n;

  char *sach_pdu,*sacch_pdu,*sach_pdu_rx,*sacch_pdu_rx,*dummy_chbch_pdu,*dummy_mrbch_pdu;
  char *sach_pdu2,*sacch_pdu2,*sach_pdu_rx2,*sacch_pdu_rx2,*dummy_chbch_pdu2,*dummy_mrbch_pdu2;
  int sach_size,sach_size2;
  int extension;

  unsigned char *chbch_pdu_tx[2],*chbch_pdu_rx[2];

  double gain,gain_N0,gain2;
  double aoa,ricean_factor;

  int sacch_errors=0,sach_errors=0;
  unsigned char nb_antennas_rx,nb_antennas_tx;

  struct complex phase;
  char sach_tx_power,sach_tx_power2,mrbch_tx_power;

  char *dummy;
  unsigned int crc_status[16];
  unsigned int crc_status2[16];
  unsigned int number_of_rbs;
  unsigned int number_of_rbs2;

  unsigned short freq_alloc,total_alloc;
  unsigned int tb_alloc;

  unsigned short freq_alloc2;
  unsigned int tb_alloc2;

  PHY_RESOURCES SACH_resources,SACH_resources2;

  int TB_SIZE_BYTES;
  int MODULATION_ORDER;
  unsigned char NUM_TBS,CODING_FMT,SPEC_EFF;

  int TB_SIZE_BYTES2;
  int MODULATION_ORDER2;
  unsigned char NUM_TBS2,CODING_FMT2,SPEC_EFF2;

  unsigned char total_groups,total_groups2;
  int mrbch_crc;

  unsigned char ch_index,ch_index2;

  unsigned char sach_dual_stream=0,two_user_flag=0;

  unsigned char chsch_indices[2] = {1, 2};

  if (argc < 14) {
    printf("Not enough arguments (%d)\nusage: sach_sim Es N0 num_frames Ntx Nrx num_errors Ricef AoA CHflag TBsize CodingFMT Freq_alloc Cluster_ID Es2 TBsize2 CodingFMT2 Freq_alloc2 Cluster_ID2\n",argc);
    exit(-1);
  }

  if ((argc > 15) && (argc < 20)) {
    
    sach_dual_stream=1;
    
  }
  else if (argc > 15) {

    printf("Not enough arguments (%d)\nusage: sach_sim Es N0 num_frames Ntx Nrx num_errors Ricef AoA CHflag TBsize CodingFMT Freq_alloc Cluster_ID TBsize2 CodingFMT2 Freq_alloc2 Cluster_ID2\n",argc);
    exit(-1);

  }

  nb_antennas_tx = atoi(argv[4]);
  nb_antennas_rx = atoi(argv[5]);

  if (nb_antennas_tx > NB_ANTENNAS_TX){
    printf("Num TX antennas %d > %d\n",nb_antennas_tx,NB_ANTENNAS_TX);
    exit(-1);
  }
  if (nb_antennas_rx > NB_ANTENNAS_RX) {
    printf("Num RX antennas %d > %d\n",nb_antennas_rx,NB_ANTENNAS_RX);
    exit(-1);
  }


  ricean_factor = pow(10.0,-.1*atof(argv[7]));
  aoa = atof(argv[8]);


  printf("Allocating memory for PHY_VARS\n");

  PHY_vars = malloc(sizeof(PHY_VARS));
  PHY_config = malloc(sizeof(PHY_CONFIG));
  mac_xface = malloc(sizeof(MAC_xface));
  
  if((config = fopen("config.cfg","r")) == NULL) // this can be configured
	{
	  printf("[Main USER] The openair configuration file <openair_config.cfg> could not be found!");
	  exit(0);
	}		
  
  if ((scenario= fopen("scenario.scn","r")) ==NULL)
    {
      printf("[Main USER] The openair scenario file <openair_scenario.scn> could not be found!");
      exit(0);
    }
  
  printf("Opened configuration files\n");

  reconfigure_MACPHY(scenario);

  dump_config();

  mac_xface->is_cluster_head = atoi(argv[9]);
  if (mac_xface->is_cluster_head == 2) {
    two_user_flag == 1;
    mac_xface->is_cluster_head=0;
  }
  else
    two_user_flag = 0;

  if (mac_xface->is_cluster_head == 1)
    printf("Running as Clusterhead (%s)\n",argv[9]);
  else
    printf("Running as %d UE\n",1+two_user_flag);
  
  TB_SIZE_BYTES = atoi(argv[10]);

  SPEC_EFF = atoi(argv[11]);
  freq_alloc = (unsigned short)strtol (argv[12], &dummy, 0 );
  conv_alloc_to_tb2(1-mac_xface->is_cluster_head,
		    TIME_ALLOC_NODEB,
		    freq_alloc,
		    SPEC_EFF,
		    0,
		    99,
		    &CODING_FMT,
		    &NUM_TBS,
		    TB_SIZE_BYTES);

  printf("Target spectral Efficiency %d (%f bps/Hz), resulting CODING format %d, number of TBs %d\n",SPEC_EFF,1.0+(SPEC_EFF/16.0),CODING_FMT,NUM_TBS);

  ch_index =atoi(argv[13]);

  switch (CODING_FMT) {
  case 0: 
    MODULATION_ORDER = 1; 
    break;
  case 1: 
    MODULATION_ORDER = 2; 
    break;
  case 2: 
    MODULATION_ORDER = 3; 
    break;
  default:
    printf("unsupported coding format %d\n",CODING_FMT);
    exit(-1);
    break;
  }

  if (sach_dual_stream == 1) {
    
    TB_SIZE_BYTES2 = atoi(argv[15]);
    
    SPEC_EFF2 = atoi(argv[16]);
    freq_alloc2 = (unsigned short)strtol (argv[17], &dummy, 0 );
    conv_alloc_to_tb2(1-mac_xface->is_cluster_head,
		      TIME_ALLOC_NODEB,
		      freq_alloc2,
		      SPEC_EFF2,
		      0,
		      99,
		      &CODING_FMT2,
		      &NUM_TBS2,
		      TB_SIZE_BYTES2);
    
    printf("Target spectral Efficiency %d (%f bps/Hz), resulting CODING format %d, number of TBs %d\n",SPEC_EFF2,1.0+(SPEC_EFF2/16.0),CODING_FMT2,NUM_TBS2);
    
    ch_index2 =atoi(argv[18]);
    
    switch (CODING_FMT2) {
    case 0: 
      MODULATION_ORDER2 = 1; 
      break;
    case 1: 
      MODULATION_ORDER2 = 2; 
      break;
    case 2: 
      MODULATION_ORDER2 = 3; 
      break;
    default:
      printf("unsupported coding format %d\n",CODING_FMT2);
      exit(-1);
      break;
    }
    
  }


  phy_init(nb_antennas_tx);
  printf("Initialized PHY variables\n");
  printf("twiddle256[254*4] = %d, twiddle256[1+(254*4)]= %d\n",twiddle_ifft256[254*4],twiddle_ifft256[1+(254*4)]);

  // Fill MAC PDU buffer for CHBCH
  seed[0] = (short)time(NULL);
  seed[1] = (short)time(NULL);
  seed[2] = (short)time(NULL);
  seed48(&seed[0]);

  randominit();

  /*
  for (i=0;i<mac_xface->mac_tch->bch_tx[0].size-4;i++) {
    mac_xface->mac_tch->bch_tx[0].data[i] = i;//(u8)lrand48();
  }
  

  printf("Filled CHBCH PDU with random data\n");

  // Generate one CHBCH
  phy_generate_chbch(0);

  */




  number_of_rbs=0;
  for (i=0;i<16;i++)
    if ((freq_alloc>>i & 1) == 1)
      number_of_rbs++;

  sach_size = NUM_TBS*(TB_SIZE_BYTES+4);
  tb_alloc = 0;
  for (i=0;i<NUM_TBS;i++)
    tb_alloc |= (1<<i);

  PHY_vars->rx_vars[0].rx_total_gain_dB=115;

  dummy_chbch_pdu = (char *)malloc(256);
  dummy_mrbch_pdu = (char *)malloc(256);


  sach_pdu  = (char *)malloc(sach_size+8);

  sacch_pdu = (char *)malloc(SACCH_SIZE_BYTES);

  sach_pdu_rx = (char *)malloc(sach_size);
  sacch_pdu_rx = (char *)malloc(SACCH_SIZE_BYTES);

  for (i=0;i<256;i++) {
    dummy_chbch_pdu[i] = 256-i;
  }
  
  for (i=0;i<sach_size;i++) {
    sach_pdu[i] = i;
  }
  printf("Filled SACH PDU (%d bytes) with random data\n",sach_size);

  for (i=0;i<SACCH_SIZE_BYTES-4;i++) {
    sacch_pdu[i] = i;
  }

  mac_xface->frame = 0;
  ((UL_SACCH_PDU *)sacch_pdu)->Coding_fmt = CODING_FMT;

  chbch_pdu_rx[0] = malloc(CHBCH_PDU_SIZE);


  if (sach_dual_stream == 1) {
    number_of_rbs2=0;
    for (i=0;i<16;i++)
      if ((freq_alloc2>>i & 1) == 1)
	number_of_rbs2++;
    
    sach_size2 = NUM_TBS2*(TB_SIZE_BYTES2+4);
    tb_alloc2 = 0;
    for (i=0;i<NUM_TBS2;i++)
      tb_alloc2 |= (1<<i);

    dummy_chbch_pdu2 = (char *)malloc(256);
    dummy_mrbch_pdu2 = (char *)malloc(256);
    
    
    sach_pdu2  = (char *)malloc(sach_size2+8);
    
    sacch_pdu2 = (char *)malloc(SACCH_SIZE_BYTES);
    
    sach_pdu_rx2 = (char *)malloc(sach_size);
    sacch_pdu_rx2 = (char *)malloc(SACCH_SIZE_BYTES);
    

    for (i=0;i<256;i++) {
      dummy_chbch_pdu2[i] = i;
    }

    for (i=0;i<sach_size2;i++) {
      sach_pdu2[i] = sach_size2-i;
    }
    printf("Filled SACH PDU (%d bytes) with random data\n",sach_size2);

    for (i=0;i<SACCH_SIZE_BYTES-4;i++) {
      sacch_pdu2[i] = i;
    }
    
    mac_xface->frame = 0;
    ((UL_SACCH_PDU *)sacch_pdu2)->Coding_fmt = CODING_FMT2;
    
    chbch_pdu_rx[1] = malloc(CHBCH_PDU_SIZE);
    

  }


  //

  while ((sach_errors < atoi(argv[6])) && (mac_xface->frame < atoi(argv[3]))) {

    mac_xface->is_cluster_head = atoi(argv[9]);

    Zero_Buffer(PHY_vars->rx_vars[0].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);
    Zero_Buffer(PHY_vars->rx_vars[1].RX_DMA_BUFFER,FRAME_LENGTH_BYTES);

    if (mac_xface->is_cluster_head == 0) {

      
      total_groups = phy_generate_sch(0,
				      1,
				      NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index,
				      freq_alloc,
				      1,
				      nb_antennas_tx);

            
      if ((sach_dual_stream == 1) && (two_user_flag == 0) )
	total_groups = phy_generate_sch(1,
					2,
					NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index2,
					freq_alloc2,
					1,
					nb_antennas_tx);
      

      total_alloc = freq_alloc|freq_alloc2;
      total_groups=0;

      for (i=0;i<16;i++)
	if ((total_alloc&(1<<i)) > 0)
	  total_groups++;

      printf("total_groups = %d\n",total_groups);

      phy_generate_sch(0,
		       MRSCH_INDEX,
		       SYMBOL_OFFSET_MRSCH,
		       0xffff,
		       0,
		       NB_ANTENNAS_TX);
      
      for (i=0; i<MRBCH_PDU_SIZE; i++)
	dummy_mrbch_pdu[i]=i;
      
      mrbch_tx_power = phy_generate_mrbch(MRSCH_INDEX,
					  0,
					  NB_ANTENNAS_TX,
					  dummy_mrbch_pdu);
      
    }
    else {
      phy_generate_chbch(1+ch_index,               // chsch index
			 1,               // extension			 
			 nb_antennas_tx,  // nb_antennas_tx
			 dummy_chbch_pdu);
    


    /*
    if (((mac_xface->frame/5) % 200) == 0)
      msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generating MRBCH %d in symbol %d\n", 
	  mac_xface->frame, 
	  last_slot,
	  MRSCH_INDEX,
	  SYMBOL_OFFSET_MRSCH+1
	  );
    */

    }

    if (mac_xface->is_cluster_head == 0) {
	// Generate one SACH (SACH)
      
      phy_generate_sach1(ch_index,
			 1,
			 SCH,
			 1+ch_index,
			 sach_pdu,
			 sacch_pdu,
			 TIME_ALLOC_UE,
			 freq_alloc,
			 CODING_FMT,
			 nb_antennas_tx,
			 TB_SIZE_BYTES,
			 tb_alloc,
			 tb_alloc,
			 1,
			 total_groups);
                        
      if ((sach_dual_stream == 1) && (two_user_flag == 0))
	phy_generate_sach1(ch_index2,
			   1,
			   SCH,
			   1+ch_index2,
			   sach_pdu2,
			   sacch_pdu2,
			   TIME_ALLOC_UE,
			   freq_alloc2,
			   CODING_FMT2,
			   nb_antennas_tx,
			   TB_SIZE_BYTES2,
			   tb_alloc2,
			   tb_alloc2,
			   0,
			   total_groups);
      
      
      /*
      if (number_of_rbs<8)
	phy_generate_sach1(1,
			   SCH,
			   1,
			   sach_pdu,
			   sacch_pdu,
			   TIME_ALLOC_UE,
			   freq_alloc>>8,
			   CODING_FMT,
			   nb_antennas_tx,
			   TB_SIZE_BYTES,
			   tb_alloc,
			   tb_alloc,
			   0,
			   total_groups);
      */

      sach_tx_power = phy_generate_sach2(1,
					 FIRST_UL_SACH_SYMBOL,
					 NUMBER_OF_SYMBOLS,
					 nb_antennas_tx);
    }
    else {
      // Generate one SACH (SACH)
      
      phy_generate_sach1(0,
			 0,
			 CHSCH,
			 1+ch_index,
			 sach_pdu,
			 NULL,
			 TIME_ALLOC_NODEB,
			 freq_alloc,
			 CODING_FMT,
			 nb_antennas_tx,
			 TB_SIZE_BYTES,
			 tb_alloc,
			 tb_alloc,
			 1,
			 1);
      /*
      if (number_of_rbs<8)
	phy_generate_sach1(0,
			   CHSCH,
			   1,
			   sach_pdu,
			   NULL,
			   TIME_ALLOC_NODEB,
			   freq_alloc>>8,
			   CODING_FMT,
			   nb_antennas_tx,
			   TB_SIZE_BYTES,
			   tb_alloc,
			   tb_alloc,
			   0,
			   1);
      */

      sach_tx_power = phy_generate_sach2(1,
					 FIRST_DL_SACH_SYMBOL,
					 NUMBER_OF_SYMBOLS,nb_antennas_tx);
    }
    
    delay = 0; //1032;
    delay = 0;
    //    msg("tx_power %d\n",sach_tx_power);

    
    
    gain = pow(10.0,.1*(atof(argv[1]) - (double)sach_tx_power))/nb_antennas_tx;
    gain_N0 = pow(10.0,.1*(atof(argv[2])));
    
    printf("sach_tx_power = %d, gain = %f (%f), gain_N0 = %f (%d)\n",
	   sach_tx_power,
	   gain,
	   atof(argv[1])-(double)sach_tx_power,
	   gain_N0,
	   argv[2]);
    

    // generate channels
    //  printf("Generating MIMO Channels\n");
    
    for (i=0;i<nb_antennas_rx;i++)      // RX Antenna loop
      for (j=0;j<nb_antennas_tx;j++) {  // TX Antenna loop
	
	phase.r = cos(2*M_PI*(i-j)/aoa);
	phase.i = sin(2*M_PI*(i-j)/aoa);
	
	random_channel(amps,Td, 8,BW,ch[j + (i*nb_antennas_rx)],ricean_factor,&phase);
	
      }
    /*    
    //  printf("sach_pdu %x (%d)\n",PHY_vars->sach_data[0].demod_pdu,sach_size);
    for (ii=0;ii<nb_antennas_rx;ii++) {
      for (j=0;
	   j<nb_antennas_tx;
	   j++) {
	printf("Channel RX %d TX %d\n",ii,j);
	for (l=0;l<(11+2*BW*Td);l++)
	  printf("%f+sqrt(-1)*%f\n",ch[j+(ii*nb_antennas_rx)][l]);
      }
    }
    */
	 
    // MIMO channel
    
    
    
    for (i=(10+2*BW*Td);
	 i<NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;
	 i++) {
      for (ii=0;
	   ii<nb_antennas_rx;
	   ii++) {
	rx_tmp.r = 0;
	rx_tmp.i = 0;
	n.r = sqrt(.5*gain_N0)*gaussdouble(0.0,1.0);
	n.i = sqrt(.5*gain_N0)*gaussdouble(0.0,1.0);
	for (j=0;
	     j<nb_antennas_tx;
	     j++) {
	  for (l = 0;
	       l<(11+2*BW*Td);
	       l++) {
#ifndef BIT8_TXMUX	    
	    tx.r = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[2*(i-l)])*sqrt(gain);
	    tx.i = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[1+(2*(i-l))])*sqrt(gain);
#else
	    tx.r = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+4*(i-l)])*sqrt(gain);
	    tx.i = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+1+(4*(i-l))])*sqrt(gain);
#endif	    
	    rx_tmp.r += (tx.r * ch[j+(ii*nb_antennas_rx)][l].r) - (tx.i * ch[j+(ii*nb_antennas_rx)][l].i);
	    rx_tmp.i += (tx.i * ch[j+(ii*nb_antennas_rx)][l].r) + (tx.r * ch[j+(ii*nb_antennas_rx)][l].i);
	    
	  }
	}
	
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[2*i] =(short)(rx_tmp.r + n.r) ; 
	((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[1+(2*i)] =(short)(rx_tmp.i + n.i); 
      }
    }


    // Second SACH/CHBCH/MRBCH
    if (sach_dual_stream == 1) {

      printf("Doing TX for second stream\n");
      for (j=0;j<nb_antennas_tx;j++)      
	Zero_Buffer(PHY_vars->tx_vars[j].TX_DMA_BUFFER,FRAME_LENGTH_SAMPLES*2);

      if (mac_xface->is_cluster_head == 0) {
	if (two_user_flag == 1) {
	  total_groups2 = phy_generate_sch(1,
					   1,
					   NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index2,
					   freq_alloc2,
					   1,
					   nb_antennas_tx);
	  
	  phy_generate_sch(0,
			   MRSCH_INDEX,
			   SYMBOL_OFFSET_MRSCH,
			   0xffff,
			   0,
			   NB_ANTENNAS_TX);
	  
	  for (i=0; i<MRBCH_PDU_SIZE; i++)
	    dummy_mrbch_pdu[i]=i;
	  
	  mrbch_tx_power = phy_generate_mrbch(MRSCH_INDEX,
					      0,
					      NB_ANTENNAS_TX,
					      dummy_mrbch_pdu);
	  
	}
      }
      else {
	phy_generate_chbch(1+ch_index2,               // chsch index
			   1,               // extension			 
			   nb_antennas_tx,  // nb_antennas_tx
			   dummy_chbch_pdu2);
	
	
	
	/*
	  if (((mac_xface->frame/5) % 200) == 0)
	  msg("[OPENAIR][PHY_PROCEDURES] Frame %d: last_slot %d, Generating MRBCH %d in symbol %d\n", 
	  mac_xface->frame, 
	  last_slot,
	  MRSCH_INDEX,
	  SYMBOL_OFFSET_MRSCH+1
	  );
	*/
	
      }
      
      if (mac_xface->is_cluster_head == 0) {
	// Generate one SACH (SACH)
	if (two_user_flag == 1) {
	  phy_generate_sach1(ch_index2,
			     1,
			     SCH,
			     1,
			     sach_pdu2,
			     sacch_pdu2,
			     TIME_ALLOC_UE,
			     freq_alloc2,
			     CODING_FMT2,
			     nb_antennas_tx,
			     TB_SIZE_BYTES2,
			     tb_alloc2,
			     tb_alloc2,
			     1,
			     total_groups2);
	  
	  sach_tx_power2 = phy_generate_sach2(1,
					      FIRST_UL_SACH_SYMBOL,
					      NUMBER_OF_SYMBOLS,
					      nb_antennas_tx);
	}
      }
      else {
	// Generate one SACH (SACH)
	
	phy_generate_sach1(0,
			   0,
			   CHSCH,
			   1+ch_index2,
			   sach_pdu2,
			   NULL,
			   TIME_ALLOC_NODEB,
			   freq_alloc2,
			   CODING_FMT2,
			   nb_antennas_tx,
			   TB_SIZE_BYTES2,
			   tb_alloc2,
			   tb_alloc2,
			   1,
			   1);
	/*
	  if (number_of_rbs<8)
	  phy_generate_sach1(0,
	  CHSCH,
	  1,
	  sach_pdu,
	  NULL,
	  TIME_ALLOC_NODEB,
	  freq_alloc>>8,
	  CODING_FMT,
	  nb_antennas_tx,
	  TB_SIZE_BYTES,
	  tb_alloc,
	  tb_alloc,
	  0,
	  1);
	*/
	
	sach_tx_power2 = phy_generate_sach2(1,
					   FIRST_DL_SACH_SYMBOL,
					   NUMBER_OF_SYMBOLS,nb_antennas_tx);
          

      
	gain2 = pow(10.0,.1*(atof(argv[14]) - (double)sach_tx_power))/nb_antennas_tx;
	
	printf("sach_tx_power2 = %d, gain = %f (%f), gain_N0 = %f (%d)\n",
	       sach_tx_power2,
	       gain2,
	       atof(argv[1])-(double)sach_tx_power2,
	       gain_N0,
	       argv[2]);
	
	for (i=(10+2*BW*Td);
	     i<NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;
	     i++) {
	  for (ii=0;
	       ii<nb_antennas_rx;
	       ii++) {
	    rx_tmp.r = 0;
	    rx_tmp.i = 0;
	    for (j=0;
		 j<nb_antennas_tx;
		 j++) {
	      for (l = 0;
		   l<(11+2*BW*Td);
		   l++) {
#ifndef BIT8_TXMUX	    
		tx.r = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[2*(i-l)])*sqrt(gain2);
		tx.i = (double)(((short *)&PHY_vars->tx_vars[j].TX_DMA_BUFFER[0])[1+(2*(i-l))])*sqrt(gain2);
#else
		tx.r = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+4*(i-l)])*sqrt(gain2);
		tx.i = (double)(((char *)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0])[(2*j)+1+(4*(i-l))])*sqrt(gain2);
#endif	    
		rx_tmp.r += (tx.r * ch[j+(ii*nb_antennas_rx)][l].r) - (tx.i * ch[j+(ii*nb_antennas_rx)][l].i);
		rx_tmp.i += (tx.i * ch[j+(ii*nb_antennas_rx)][l].r) + (tx.r * ch[j+(ii*nb_antennas_rx)][l].i);
		
	      }
	    }
	    
	    ((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[2*i] +=(short)(rx_tmp.r) ; 
	    ((s16*)&PHY_vars->rx_vars[ii].RX_DMA_BUFFER[0])[1+(2*i)] +=(short)(rx_tmp.i); 
	  }
	}
      } // Clusterhead == 1
    }
#ifdef DEBUG_PHY  
    write_output("rxsig.m","rxs",
		 (s16 *)&PHY_vars->rx_vars[0].RX_DMA_BUFFER[0],
		 NUMBER_OF_SYMBOLS_PER_FRAME*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES,
		 1,
		 1);
#endif //DEBUG_PHY  

    // Switch roles
    if (mac_xface->is_cluster_head == 1) {
      mac_xface->is_cluster_head = 0;
      SACH_resources.Time_alloc = TIME_ALLOC_NODEB;
      SACH_resources.Coding_fmt = CODING_FMT;
      SACH_resources.Freq_alloc = freq_alloc;
      SACH_resources.Ifreq_alloc = freq_alloc & freq_alloc2;

      SACH_resources2.Time_alloc = TIME_ALLOC_NODEB;
      SACH_resources2.Coding_fmt = CODING_FMT2;
      SACH_resources2.Freq_alloc = freq_alloc2;
      SACH_resources2.Ifreq_alloc = freq_alloc & freq_alloc2;

    }
    else {
      mac_xface->is_cluster_head = 1;
      SACH_resources.Time_alloc = TIME_ALLOC_UE;
      SACH_resources.Coding_fmt = CODING_FMT;
      SACH_resources.Freq_alloc = freq_alloc;
      SACH_resources.Ifreq_alloc = freq_alloc;

      SACH_resources2.Time_alloc = TIME_ALLOC_UE;
      SACH_resources2.Coding_fmt = CODING_FMT2;
      SACH_resources2.Freq_alloc = freq_alloc2;
      SACH_resources2.Ifreq_alloc = freq_alloc2;

    }


    

    

    if (mac_xface->is_cluster_head == 1) {
      printf("Doing SCH channel estimation\n");
      phy_channel_estimation_top(0,NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index,0,1+ch_index,nb_antennas_rx,SCH);
      if (sach_dual_stream == 1)
	phy_channel_estimation_top(0,NUMBER_OF_GUARD_RACH_SYMBOLS+TX_RX_SWITCH_SYMBOL+ch_index2,0,1+ch_index2,nb_antennas_rx,SCH);
    }
    else {
    printf("Doing CHSCH channel estimation\n");
      phy_channel_estimation_top(0,1+ch_index,0,1+ch_index,nb_antennas_rx,CHSCH);
      if (sach_dual_stream == 1)
	phy_channel_estimation_top(0,1+ch_index2,0,1+ch_index2,nb_antennas_rx,CHSCH);
    }
        
    if (mac_xface->is_cluster_head == 0) {


      if (sach_dual_stream == 0) {
	printf("Doing dual-stream\n");
	phy_decode_chbch(1+ch_index,nb_antennas_rx,nb_antennas_tx,chbch_pdu_rx[0],CHBCH_PDU_SIZE);
	phy_decode_sach_common(FIRST_DL_SACH_SYMBOL,NUMBER_OF_SYMBOLS,nb_antennas_rx,1);
	phy_decode_sach(0,
			1,
			&SACH_resources,
			sach_pdu_rx,
			sacch_pdu_rx,
			nb_antennas_rx,
			nb_antennas_tx,
			1,
			1+ch_index,
			0,
			NUM_TBS,
			TB_SIZE_BYTES,
			tb_alloc,
			&crc_status[0]);
      }
      else {

	phy_decode_chbch_2streams_ml(chsch_indices,
				     0,
				     nb_antennas_rx,
				     nb_antennas_tx,
				     chbch_pdu_rx,
				     ret,
				     CHBCH_PDU_SIZE);

      
      	phy_decode_sach_common(FIRST_DL_SACH_SYMBOL,NUMBER_OF_SYMBOLS,nb_antennas_rx,1);
	printf("Decoding first stream\n");
	phy_decode_sach_2streams_ml(0,
				    1,
				    &SACH_resources,
				    sach_pdu_rx,
				    sacch_pdu_rx,
				    nb_antennas_rx,
				    nb_antennas_tx,
				    1,
				    1+ch_index,
				    0,
				    NUM_TBS,
				    TB_SIZE_BYTES,
				    tb_alloc,
				    &crc_status[0]);
	printf("Decoding second stream\n");
	phy_decode_sach_2streams_ml(0,
				    0,
				    &SACH_resources2,
				    sach_pdu_rx2,
				    sacch_pdu_rx2,
				    nb_antennas_rx,
				    nb_antennas_tx,
				    1,
				    1+ch_index2,
				    0,
				    NUM_TBS2,
				    TB_SIZE_BYTES2,
				    tb_alloc2,
				    &crc_status2[0]);
      }
	/*
      if (number_of_rbs < 8) {
	SACH_resources.Freq_alloc = freq_alloc>>8;


	phy_decode_sach(0,
			0,
			&SACH_resources,
			sach_pdu_rx,
			sacch_pdu_rx,
			nb_antennas_rx,
			nb_antennas_tx,
			1,
			NUM_TBS,
			TB_SIZE_BYTES,
			tb_alloc,
			&crc_status2[0]);
      }
      */
    }
    else {

      phy_channel_estimation_top(0,
				 SYMBOL_OFFSET_MRSCH,
				 1,
				 MRSCH_INDEX,
				 NB_ANTENNAS_RX,
				 SCH);
      
      mrbch_crc = phy_decode_mrbch(MRSCH_INDEX,
				   NB_ANTENNAS_RX,
				   NB_ANTENNAS_TXRX,
				   dummy_mrbch_pdu,
				   MRBCH_PDU_SIZE);

      phy_decode_sach_common(FIRST_UL_SACH_SYMBOL,NUMBER_OF_SYMBOLS,nb_antennas_rx,1);
      printf("Doing dual-stream decode CH 1\n");
      phy_decode_sach_2streams_ml(1,
				  1,
				  &SACH_resources,
				  sach_pdu_rx,
				  sacch_pdu_rx,
				  nb_antennas_rx,
				  nb_antennas_tx,
				  1,
				  1+ch_index,
				  0,
				  NUM_TBS,
				  TB_SIZE_BYTES,
				  tb_alloc,
				  &crc_status[0]);

      printf("Doing dual-stream decode CH 2\n");
      phy_decode_sach_2streams_ml(1,
				  1,
				  &SACH_resources2,
				  sach_pdu_rx2,
				  sacch_pdu_rx2,
				  nb_antennas_rx,
				  nb_antennas_tx,
				  1,
				  1+ch_index2,
				  1,
				  NUM_TBS2,
				  TB_SIZE_BYTES2,
				  tb_alloc2,
				  &crc_status[0]);

	/*
      SACH_resources.Freq_alloc = freq_alloc>>8;

      if (number_of_rbs<8)
	phy_decode_sach(1,
			1,
			&SACH_resources,
			sach_pdu_rx,
			sacch_pdu_rx,
			nb_antennas_rx,
			nb_antennas_tx,
			1,
			NUM_TBS,
			TB_SIZE_BYTES,
			tb_alloc,
			&crc_status2[0]);
	*/

    }
    if (crc_status[0] == -2)
      printf("Frame %d SACCH Error %d\n",mac_xface->frame,++sacch_errors);
    else {
      for (i=0;i<NUM_TBS;i++) 
	if (crc_status[i] == -1)
	  printf("Frame %d SACH Error %d (TB %d)\n",mac_xface->frame,++sach_errors,i);
    }
    /*
    if (number_of_rbs<8) {
      if (crc_status2[0] == -2)
	printf("Frame %d SACCH2 Error %d\n",mac_xface->frame,++sacch_errors);
      
      if (crc_status2[0] == -1)
	printf("Frame %d SACH2 Error %d\n",mac_xface->frame,++sach_errors);
    }
    */

    mac_xface->frame++;
  }

    //  mac_cleanup();

  //  phy_cleanup();
  //  printf("Exiting\n");

  printf("Exiting, SACCH Error Rate = %e, SACH Error Rate (Conditional) = %e\n",
	 (double)sacch_errors/(NUM_TBS*(mac_xface->frame-1)),
	 (double)sach_errors/(NUM_TBS*(mac_xface->frame-1)));
  
}
