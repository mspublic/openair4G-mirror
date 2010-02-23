
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif
#ifdef PLATON
#include "daq.h"
#endif

#include "PHY/CONFIG/vars.h"
#include "MAC_INTERFACE/vars.h"
//#include "PHY/TOOLS/defs.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include <arpa/inet.h>
#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <ctype.h>
#include <fcntl.h>
//#include <errno.h>
#include <unistd.h>
//#include <string.h>
//#include <netdb.h>

#define PAGE_SIZE 4096

PHY_CONFIG PHY_config_mem;
PHY_CONFIG *PHY_config;

//float estimate_freq(short *);

//unsigned int revbits(unsigned int x);

//-----------------------------------------------------------------------------
int main (int argc, char **argv) {
  //-----------------------------------------------------------------------------
  // arg[1] = fpga_file
  // arg[2] = freq



  //  int ifreq,target_freq;
  //  int index, fd, offset;
  //  char file_name[] = "                                                  ";
  //  char file_name_start[] = "/opt/eurecom/etc/PLATON_";
  //  char file_tx_extension[] = "TX.cal";
  //  char file_rx_extension[] = "RX.cal";
  //  const char *calibration_file_tx_name;
  //  const char *calibration_file_rx_name;  //  int frequence;
  //  int nb_rf_cards;
  //  int num_slots = 100000;

  int openair_fd,rx_sig_fifo_fd,frequency,i;

  unsigned int action;
  char *dma_buffer_local;
  char device_name[16];
  unsigned char temp[4];
  unsigned int tmp;
  int result=-1;

  FILE *chbch_file,*rx_frame_file,*tx_frame_file;
  unsigned int fc;
//#ifdef PHY_EMUL_IOCTL
  unsigned int Topo_info[100];//H.A
//#endif //PHY_EMUL_IOCTL
  int             ifreq, target_freq;

  char *chbch_pdu;
  int chbch_size;

 
#ifdef PLATON
  freq_t          freq;
#endif 


  if (argc < 3) {
    printf("[openair][INFO] Usage %s  frequency(0,1,2,3)  action (0-14) params . . .  \n" , argv[0]);
    printf("[openair][INFO] ACTION DESCRIPTIONS\n");
    printf("[openair][INFO] Action 0  : Configure PHY/MAC (Kernel module and FPGA) - param dual_tx (0/1) - param tdd (0/1)\n");
    printf("[openair][INFO] Action 1  : Start Primary Clusterhead - param 0/1 = frequency offset on/off - param NODE_ID\n");
    printf("[openair][INFO] Action 2  : Start Secondary Clusterhead - param 0/1 = frequency offset on/off - param NODE_ID\n");
    printf("[openair][INFO] Action 3  : Start Node - param 0/1 = frequency offset on/off - param NODE_ID\n");
    printf("[openair][INFO] Action 4  : Standby - param 0/1 = frequency offset on/off\n");
    printf("[openair][INFO] Action 5  : Get frame and save to rx_frame.dat - param 0/1 = frequency offset on/off\n");
    printf("[openair][INFO] Action 6  : Set TX gains - param 2394_0 (0-255),9862_TX_0(0-255),2394_1(0-255),9862_TX_1(0-255)\n");
    printf("[openair][INFO] Action 7  : Set RX gains - param 2391_0 (0-255),9862_RX_0(0-19),2391_1(0-255),9862_RX_1(0-19) (also turns off AGC)\n");
    printf("[openair][INFO] Action 8  : TX C/W @ fs/4 + offset - param 0/1 = 66 kHz frequency offset on/off\n");
    printf("[openair][INFO] Action 9  : TX OFDM/QPSK - param 0/1 = 66 kHz frequency offset on/off\n");
    printf("[openair][INFO] Action 10 : TX SCCP/QPSK param 0/1 = 66 kHz frequency offset on/off\n");
    printf("[openair][INFO] Action 11 : TX SCCP/16QAM param 0/1 = 66 kHz frequency offset on/off\n");
    printf("[openair][INFO] Action 12 : TX I/Q impulses (delay test)\n");
    printf("[openair][INFO] Action 13 : TX real C/W fs/4 + offset with DC component - param 0/1 = 66 kHz frequency offset off/on\n");  
    printf("[openair][INFO] Action 14 : SET RX RF Mode - param (CONFIG 0 (0-255), OPCNTL 0 (0-255), CONFIG 1 (0-255), OPCNTL 1 (0-255) )\n");
    printf("[openair][INFO] Action 15 : SET TCXO param (set > 255 to use calibrated value)\n");
    printf("[openair][INFO] Action 16 : SET CALIBRATED RX GAIN param (also turns off AGC)\n");
    printf("[openair][INFO] Action 22 : Update SoC firmware\n");
    printf("[openair][INFO] Action 25 : SET TIMING ADVANCE param\n");
    printf("[openair][INFO] Action 26 : SET FREQ OFFSET param\n");
    printf("[openair][INFO] Action 27 : Start Primary Clusterhead in cognitive mode - param 0/1 = frequency offset on/off - param NODE_ID\n");
    printf("[openair][INFO] Action 39 : Send EMOS recording flag\n"); 
    exit (-1);
  }


  frequency = atoi(argv[1]);
  //  printf ("[openair][INFO][START] RBT file            is %s\n", argv[2]);
  printf ("[openair][INFO][START] Frequency           is %d\n", frequency);
  //  printf ("[openair][INFO][START] number of antennas  is %d\n", nb_rf_cards);

  action = atoi(argv[2]);

  if (action == 0) // configure
    printf("[openair][INFO][START] Action              is : configuration\n");
  else if (action == 1) // start clusterhead
    printf("[openair][INFO][START] Action              is : start primary clusterhead\n");
  else if (action == 2) // start terminode
    printf("[openair][INFO][START] Action              is : start secondary clusterhead\n");
  else if (action == 3) // stop
    printf("[openair][INFO][START] Action              is : start node\n");
  else if (action == 4) // stop
    printf("[openair][INFO][START] Action              is : stop\n");
  else if (action == 5) // stop
    printf("[openair][INFO][START] Action              is : GET_FRAME\n");
  else if (action == 6) // stop
    printf("[openair][INFO][START] Action              is : SET_TX_GAIN\n");
  else if (action == 7) // stop
    printf("[openair][INFO][START] Action              is : SET_RX_GAIN\n");
  else if (action == 8) // stop
    printf("[openair][INFO][START] Action              is : START_FS4_TEST\n");
  else if (action == 9) // stop
    printf("[openair][INFO][START] Action              is : START_OFDM_TEST\n");
  else if (action == 11) // stop
    printf("[openair][INFO][START] Action              is : START_QAM16_TEST\n");
  else if (action == 10) // stop
    printf("[openair][INFO][START] Action              is : START_QPSK_TEST\n");
  else if (action == 12) // stop
    printf("[openair][INFO][START] Action              is : START_IQ_IMPULSES_TEST\n");
  else if (action == 13) // stop
    printf("[openair][INFO][START] Action              is : START_REAL_FS4_WITH_DC_TEST\n");
  else if (action == 14) // stop
    printf("[openair][INFO][START] Action              is : SET_RX_RF MODE\n");
  else if (action == 15) // stop
    printf("[openair][INFO][START] Action              is : SET_TCXO_DAC\n");
  else if (action == 16) // Set calibrated gain
    printf("[openair][INFO][START] Action              is : SET_CALIBRATED_RX_GAIN\n");
  else if (action == 22) // Update firmware
    printf("[openair][INFO][START] Action              is : UPDATE_FIRMWARE\n");
  else if (action == 23) // Set Topology info
    printf("[openair][INFO][START] Action              is : SET_TOPOLOGY_INFORMATION\n");
  else if (action == 24) // Stop_EMULATION_TX_RX for node// disconnect from CH
    printf("[openair][INFO][START] Action              is : STOP_EMULATION_TX_RX\n");
  else if (action == 25) // Set timing advance
    printf("[openair][INFO][START] Action              is : SET_TIMING_ADVANCE\n");
  else if (action == 26) // Set freq offset
    printf("[openair][INFO][START] Action              is : SET_FREQ_OFFSET\n");
  else if (action == 39) // Send EMOS Rec flag
    printf("[openair][INFO][START] Action              is : START_EMOS_NODEB\n");
  else {
    printf("[openair][INFO][START] Action              is : unknown action %d\n",action);
  }


  printf("[openair][INFO][START] Opening /dev/openair0\n");
  if ((openair_fd = open("/dev/openair0", O_RDWR,0)) <0) {
    fprintf(stderr,"Error %d opening /dev/openair0\n",openair_fd);
    exit(-1);
  }

  if((config = fopen("./config.cfg","r")) == NULL) // this can be configured
    {
      printf("[openair][CONFIG][INFO] openair configuration file <config.cfg> could not be found!");
      exit(0);
    }

  
  if ((scenario= fopen("./scenario.scn","r")) ==NULL)
    {
      printf("[openair][CONFIG][INFO] openair scenario file <scenario.scn> could not be found!");
      exit(0);
    }
  
  PHY_config = (PHY_CONFIG *)&PHY_config_mem;
  PHY_vars = malloc(sizeof(PHY_VARS));
  mac_xface = malloc(sizeof(MAC_xface));

  reconfigure_MACPHY(scenario);
  printf("reconfigure_MACPHY() done.\n");

#ifndef OPENAIR_LTE  
  phy_init(NB_ANTENNAS_TX);
#else
  lte_frame_parms = &(PHY_config->lte_frame_parms);
  lte_ue_common_vars = &(PHY_vars->lte_ue_common_vars);
  lte_ue_dlsch_vars = &(PHY_vars->lte_ue_dlsch_vars[0]);
  lte_ue_pbch_vars = &(PHY_vars->lte_ue_pbch_vars[0]);
  lte_eNB_common_vars = &PHY_vars->lte_eNB_common_vars;

  lte_frame_parms->N_RB_DL            = 25;
  lte_frame_parms->N_RB_UL            = 25;
  lte_frame_parms->Ncp                = 1;
  lte_frame_parms->Nid_cell           = 0;
  lte_frame_parms->nushift            = 0;
  lte_frame_parms->nb_antennas_tx     = NB_ANTENNAS_TX;
  lte_frame_parms->nb_antennas_rx     = NB_ANTENNAS_RX;
  lte_frame_parms->first_dlsch_symbol = 2;
  lte_frame_parms->Csrs = 2;
  lte_frame_parms->Bsrs = 0;
  lte_frame_parms->kTC = 0;
  lte_frame_parms->n_RRC = 0;
  
  init_frame_parms(lte_frame_parms);
  
  phy_init_top(NB_ANTENNAS_TX);
	  
  lte_frame_parms->twiddle_fft      = twiddle_fft;
  lte_frame_parms->twiddle_ifft     = twiddle_ifft;
  lte_frame_parms->rev              = rev;
  
  lte_gold(lte_frame_parms);

  /*
  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();
  */

  phy_init_lte_ue(lte_frame_parms,lte_ue_common_vars,lte_ue_dlsch_vars,lte_ue_pbch_vars);
  phy_init_lte_eNB(lte_frame_parms, lte_eNB_common_vars);
#endif
  printf("Initialized PHY variables\n");


  //mac_init();
  //printf("Initialized MAC variables\n");

#ifndef OPENAIR_LTE
  chbch_size = (NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_CHBCH_SYMBOLS)>>3;
  chbch_pdu  = malloc(chbch_size);

  for (i=0;i<chbch_size-4;i++) {
    chbch_pdu[i] = i;
  }

  printf("Filled CHBCH PDU with random data\n");
#endif //OPENAIR_LTE

  dma_buffer_local = (char *)malloc(NB_ANTENNAS_RX*FRAME_LENGTH_BYTES);

  if (!dma_buffer_local) {
    printf("could not allocated dma buffer\n");
    exit(-1);
  }

  printf("Running action %d\n",action);
  switch (action) {

  case 0 :


    if (argc<5) {
      printf("Please provide fdd parameter (0/1)\n");
      exit(-1);
    }
    if (argc<4) {
      printf("Please provide dual_tx parameter (0/1)\n");
      exit(-1);
    }
    PHY_config->dual_tx = atoi(argv[3]);
    PHY_config->tdd = atoi(argv[4]);

#ifdef PLATON
    if (loadFPGA2(openair_fd,argv[3]) < 0) {
      printf("[openair][LOAD_FPGA][INFO] problem loading fpga firmware, check the filename\n");
      close(openair_fd);
      exit(-1);
    }
    else
      printf("[openair][LOAD_FPGA][INFO] fpga firmare loaded successfully\n");
    if (usr_load_calibration_data(openair_fd,NB_ANTENNAS_RX,argv) <0) {
      printf("[openair][LOAD_CALIBRATION_DATA][INFO] problem loading RF calibration data, check the filename\n");
    }
#endif //PLATON    
    result=ioctl(openair_fd, openair_DUMP_CONFIG,(char *)PHY_config);
    if (result == 0) {
      printf ("[openair][CONFIG][INFO] loading openair configuration in kernel space\n");
    } else {
      printf ("[openair][START][INFO] loading openair configuration in kernel space failed \n");
    }


    break;
    
  case 1 :
    printf("[openair][START][INFO] Starting clusterhead\n");
#ifdef CBMIMO1
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4) | ((atoi(argv[4])&0xFF) << 7);
    printf("[openair][START][INFO] fc = %d\n",fc);
#endif //CBMIMO1
#ifdef PLATON
    fc = atoi(argv[1]);
    ioctl (openair_fd, DAQ_SET_RF_IDLE, 0);
    sleep(1);
    //usleep (10000);
    ioctl (openair_fd, DAQ_SET_NORMAL_RX, 0);
    sleep(1);

    freq.synth_id = 0;
    ioctl (openair_fd, DAQ_RESET_FSYN, &freq);
    sleep(1);

    freq.synth_id = 1;
    ioctl (openair_fd, DAQ_RESET_FSYN, &freq);
    sleep(1);

    freq.synth_id = 2;
    ioctl (openair_fd, DAQ_RESET_FSYN, &freq);
    sleep(1);

    freq.synth_id = 3;
    ioctl (openair_fd, DAQ_RESET_FSYN, &freq);
    sleep(1);



    printf (" Philips board detected, set default values for the synths \n");
    freq.synth_id = 0;
    target_freq = fc + 190000;
    ifreq = 1000 * target_freq;
    freq.freq = ifreq;
    ioctl (openair_fd, DAQ_SET_FSYN, &freq);
    usleep (10000);
    printf ("LO automatically adjusted for LoId = %d to %d Hz \n", freq.synth_id, freq.freq);
    
    freq.synth_id = 1;
    target_freq = 247600;
    ifreq = 1000 * target_freq;
    freq.freq = ifreq;
    ioctl (openair_fd, DAQ_SET_FSYN, &freq);
    usleep (10000);
    printf ("LO automatically adjusted for LoId = %d to %d Hz \n", freq.synth_id, freq.freq);
    
    freq.synth_id = 2;
    target_freq = fc;
    ifreq = 1000 * target_freq;
    freq.freq = ifreq;
    ioctl (openair_fd, DAQ_SET_FSYN, &freq);
    usleep (10000);
    printf ("LO automatically adjusted for LoId = %d to %d Hz \n", freq.synth_id, freq.freq);
    
    freq.synth_id = 3;
    target_freq = 15360;
    ifreq = 15363636; //1000 * target_freq; // Adjusted to match CBMIMO1 sampling clock
    freq.freq = ifreq;
    ioctl (openair_fd, DAQ_SET_FSYN, &freq);
    printf ("LO automatically adjusted for LoId = %d to %d Hz \n", freq.synth_id, freq.freq);
    
    sleep (1);
#endif //PLATON    
    result=ioctl(openair_fd,openair_START_1ARY_CLUSTERHEAD, &fc);
    if (result == 0) {
      printf ("[openair][START][INFO] primary clusterhead running\n");
    } else {
      printf ("[openair][START][INFO] starting primary clusterhead failed \n");
    }
    break;
  case 2 :
    printf("[openair][START][INFO] Starting secondary clusterhead\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4) |  ((atoi(argv[4])&0xFF) << 7);
    result=ioctl(openair_fd,openair_START_2ARY_CLUSTERHEAD,&fc);
    if (result == 0) {
      printf ("[openair][START][INFO] secondary clusterhead running\n");
    } else {
      printf ("[openair][START][INFO] starting secondary clusterhead failed \n");
    }

    break;


#ifdef CBMIMO1
  case 3 :
    printf("[openair][START][INFO] Starting NODE ...(%x)\n",openair_START_NODE);
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4) |  ((atoi(argv[4])&0xFF) << 7);
    result=ioctl(openair_fd,openair_START_NODE, &fc);

    if (result == 0) {
      printf ("[openair][START][INFO] ok! \n");
    } else {
      printf ("[openair][START][INFO] not ok! \n");
    }



    break;
#endif
  case 4 : 
    printf("[openair][START][INFO] Stoping ...\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);

    result=ioctl(openair_fd,openair_STOP,(void *)&fc);
    if (result == 0) {
      printf ("[openair][STOP][INFO] ok! \n");
    } else {
      printf ("[openair][STOP][INFO] not ok! \n");
    }

    if ((rx_frame_file = fopen("rx_frame.dat","w")) == NULL)
      {
	printf("[openair][STOP][INFO] Cannot open rx_frame.m data file\n");
	exit(0);
      }

    fwrite(dma_buffer_local,4,NB_ANTENNAS_RX*FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);
    fclose(rx_frame_file);

    break;
#ifdef CBMIMO1
//#ifndef PHY_EMUL_IOCTL
  case 5 : 


    if ((rx_frame_file = fopen("rx_frame.dat","w")) == NULL)
      {
	printf("[openair][INFO] Cannot open rx_frame.m data file\n");
	exit(0);
      }

    if ((rx_sig_fifo_fd = open("/dev/rtf59",O_RDONLY,0)) <0)
      {
	printf("[openair][INFO] Cannot open rx_sig_fifo\n");
	exit(0);
      }

    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);
    ioctl(openair_fd,openair_GET_BUFFER,(void *)&fc);

    sleep(2);

    read(rx_sig_fifo_fd,(void *)dma_buffer_local,NB_ANTENNAS_RX*FRAME_LENGTH_BYTES);
    fwrite(dma_buffer_local,4,NB_ANTENNAS_RX*FRAME_LENGTH_COMPLEX_SAMPLES,rx_frame_file);

    fclose(rx_frame_file);
    break;

  case 6 :
    printf("[openair][START][INFO] Setting TX gains to %d,%d,%d,%d\n",atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
    temp[0] = atoi(argv[3]);
    temp[1] = atoi(argv[4]);
    temp[2] = atoi(argv[5]);
    temp[3] = atoi(argv[6]);

    result=ioctl(openair_fd,openair_SET_TX_GAIN,&temp[0]);
    break;

  case 7 :
    printf("[openair][START][INFO] Setting RX gains to %d,%d,%d,%d\n",atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),atoi(argv[6]));
    temp[0] = atoi(argv[3]);
    temp[1] = atoi(argv[4]);
    temp[2] = atoi(argv[5]);
    temp[3] = atoi(argv[6]);


    result=ioctl(openair_fd,openair_SET_RX_GAIN,&temp[0]);
    break;

  case 8 :
    printf("[openair][START][INFO] TX Test FS/4\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);
    result=ioctl(openair_fd,openair_START_FS4_TEST,&fc);
    break;

  case 9 :
    printf("[openair][START][INFO] TX Test OFDM\n");

    if ((tx_frame_file = fopen("tx_frame.dat","w")) == NULL)
      {
	printf("[openair][INFO] Cannot open tx_frame.dat data file\n");
	exit(0);
      }
    
    //openair_generate_ofdm(1,0xffff,chbch_pdu);
    openair_generate_ofdm(3,0,0);

    printf("[openair][START][INFO] TX_DMA_BUFFER = %p\n",PHY_vars->tx_vars[0].TX_DMA_BUFFER);
    result=ioctl(openair_fd,openair_START_TX_SIG,(void *)PHY_vars->tx_vars);
#ifdef IFFT_FPGA
    fwrite(PHY_vars->tx_vars[0].TX_DMA_BUFFER,1,NUMBER_OF_USEFUL_CARRIERS*NUMBER_OF_SYMBOLS_PER_FRAME,tx_frame_file);
#else
    fwrite(PHY_vars->tx_vars[0].TX_DMA_BUFFER,4,FRAME_LENGTH_COMPLEX_SAMPLES,tx_frame_file);
#endif
    fclose(tx_frame_file);


    break;

  case 10 :
    printf("[openair][START][INFO] TX Test QAM16\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);;
    result=ioctl(openair_fd,openair_START_QAM16_TEST,&fc);
    break;

  case 11 :
    printf("[openair][START][INFO] TX Test QPSK\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);;
    result=ioctl(openair_fd,openair_START_QPSK_TEST,&fc);
    break;

  case 12 :
    printf("[openair][START][INFO] TX Test I/Q Impulses\n");
    fc = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);;
    result=ioctl(openair_fd,openair_START_IQ_IMPULSES_TEST,&fc);
    break;

  case 13:
    //printf("[openair][START][INFO] FS4 Test with DC\n");
    printf("[openair][START][INFO] FS4 Test\n");
    fc = atoi(argv[3]);
    //result=ioctl(openair_fd,openair_START_REAL_FS4_WITH_DC_TEST,&fc);
    result=ioctl(openair_fd,openair_START_FS4_TEST,&fc);
    break;

  case 14:        // SET RX RF MODE
    printf("[openair][START][INFO] SET RX RF MODE\n");
    fc = atoi(argv[3]);
    result=ioctl(openair_fd,openair_RX_RF_MODE,&fc);
    break;

  case 15:        // SET TCXO DAC level
    printf("[openair][START][INFO] SET TCXO DAC\n");
    fc = atoi(argv[3]);
    result=ioctl(openair_fd,openair_SET_TCXO_DAC,&fc);
    break;

  case 16:        // SET Calibrated
    printf("[openair][START][INFO] SET RX GAIN\n");
    fc = atoi(argv[3]);
    result=ioctl(openair_fd,openair_SET_CALIBRATED_RX_GAIN,&fc);

    break;


  case 17:        // DO SYNCHRONIZATION
    printf("[openair][INFO] Do CHBCH Synchronization\n");
    

    
    ((unsigned char *)&dma_buffer_local[0])[0] = (unsigned char)((atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4));
    ((unsigned char *)&dma_buffer_local[0])[1] = (unsigned char)(atoi(argv[4]));
    ioctl(openair_fd,openair_DO_SYNCH,(void *)dma_buffer_local);

    break;
   

  case 18:        // GET SIGNALS

    temp[0] = atoi(argv[4]);
    
    if (temp[0]==0)
      printf("[openair][START][INFO] Get Synch Symbols\n");
    else if (temp[0]==1)
      printf("[openair][START][INFO] Get Channel Estimate\n");
    else if (temp[0]==2)
      printf("[openair][START][INFO] Get Estimated data\n");
    else if (temp[0]==3)
      printf("[openair][START][INFO] Get Decoded Data\n");
    else
      printf("[openair][START][INFO] Get Signals : wrong selection (choose from 0-3)\n");
    

    if ((rx_frame_file = fopen("rx_frame.dat","w")) == NULL)
      {
	printf("[openair][GET_SIGNALS][INFO] Cannot open rx_frame.dat data file\n");
	exit(0);
      }
    
    ((unsigned int *)&dma_buffer_local[0])[0] = (atoi(argv[3])&1) | ((frequency&7)<<1) | ((frequency&7)<<4);
    ((unsigned char *)&dma_buffer_local[0])[1] = (unsigned char)(atoi(argv[4]));

    if (temp[0] < 4)
      ioctl(openair_fd,openair_GET_SIGNALS,(void *)dma_buffer_local);
    //    sleep(2);
    
    fwrite(dma_buffer_local,4,7*256,rx_frame_file);
    
    fclose(rx_frame_file);
    break;
    
    
  case 19:        // SET FFT SCALE
    printf("[openair][START][INFO] Set FFT Scale to %d, %d\n",atoi(argv[3]),atoi(argv[4]));
    
    temp[0] = atoi(argv[3]);
    temp[1] = atoi(argv[4]);
    result=ioctl(openair_fd,openair_SET_FFT_SCALE,&temp[0]);
    break;


  case 21:        // FFT TEST
    fc = atoi(argv[3]);
    result=ioctl(openair_fd,openair_SET_CALIBRATED_RX_GAIN,&fc);
    break;

  case 22:
    result=ioctl(openair_fd, openair_UPDATE_FIRMWARE);
    if (result) printf("[openair][START][INFO] NOK, ioctl failed\n");
    break;
//#else //PHY_EMUL_IOCTL

  case 23 :
    printf("[OPENAIR][START][Topology] Configuring Topology,\n",argc);
    unsigned short ri=0,rj=3,Nb_inst,Nb_ch,Nb_ue,kk;
    Topo_info[ri++]=atoi(argv[rj++]);//NUMBER OF EMULATION MASTER (Machines)
    printf("NUMBER OF MASTER %d\n",Topo_info[ri-1]);
    Topo_info[ri++]=atoi(argv[rj++]);//LOCAL MASTER ID
    printf("LOCAL MASTER ID %d\n",Topo_info[ri-1]);
    Nb_inst=atoi(argv[rj++]);//LOCAL NUMBER OF INSTANCES
    Topo_info[ri++]=Nb_inst;
    printf("NUMBER OF INSTANCE %d\n",Topo_info[ri-1]);
    for(i=0;i<Nb_inst;i++){
      printf("INSTANCE %d\n",i);
      Topo_info[ri++]=atoi(argv[rj++]);//Node_id of Instance i 
      printf("NODE_ID %d\n",Topo_info[ri-1]);
      Nb_ch=atoi(argv[rj++]);//Number of CH in the range
      Topo_info[ri++]=Nb_ch;
      printf("NB_CH %d\n",Topo_info[ri-1]);
      for(kk=0;kk<Nb_ch;kk++)
	Topo_info[ri++]=atoi(argv[rj++]);//List of CH IDs in the range
      Nb_ue=atoi(argv[rj++]);//Number of UE in the range
      Topo_info[ri++]=Nb_ue;
      printf("NB_UE %d\n",Topo_info[ri-1]);
      for(kk=0;kk<Nb_ue;kk++)
	Topo_info[ri++]=atoi(argv[rj++]);//List of UE IDs in the range
    }
    result=ioctl(openair_fd,openair_config_topology, &Topo_info[0]);
    
    if (result == 0) {
      printf ("[openair][START][Topology] ok! \n");
    } else {
      printf ("[openair][START][Topology] not ok! \n");
    }
    break;

case 24 :
    printf("[OPENAIR][STOP][EMULTION] Disconnect Node\n");
    Topo_info[0]=atoi(argv[3]);
    result=ioctl(openair_fd,openair_stop_emulation, &Topo_info[0]);

    if (result == 0) {
      printf ("[openair][STOP][EMULATION] ok! \n");
    } else {
      printf ("[openair][STOP][EMULATION] not ok! \n");
    }
    break;
//#endif //PHY_EMUL_IOCTL
#endif //CBMIMO1

  case 25:
    
    fc = atoi(argv[3]);
    printf("[openair][START][INFO] SET TIMING ADVANCE to %d\n",fc);
    result = ioctl(openair_fd,openair_SET_TIMING_ADVANCE, &fc);
    break;

  case 26:
    
    fc = atoi(argv[3]);
    printf("[openair][START][INFO] SET FREQ OFFSET to %d\n",fc);
    result = ioctl(openair_fd,openair_SET_FREQ_OFFSET, &fc);
    break;

  case 27 :
    printf("[openair][START][INFO] Starting clusterhead in cognitive mode\n");
    fc = 0; //dummy value  - not needed
    result=ioctl(openair_fd,openair_START_1ARY_CLUSTERHEAD_COGNITIVE, &fc);
    if (result == 0) {
      printf ("[openair][START][INFO] primary clusterhead running\n");
    } else {
      printf ("[openair][START][INFO] starting primary clusterhead failed \n");
    }
    break;

	
  case 39:
    result=ioctl(openair_fd, openair_START_EMOS_NODEB);
    if (result) printf("[openair][START][INFO][EMOS] NOK, ioctl failed\n");
    break;

  }
  
  close(openair_fd);
  
  return 0;
}

/*
float estimate_freq(short *buffer) {

  int i,j,tmp,tmp_re,tmp_im,max_f,max_point;
  float avg = 0.0;
  
  for (i=2048;i<16*2048;i+=2048)
    fix_fft(&buffer[i],10,0);
  
  for (j=1;j<16;j++) {
    
    max_f = 0;
    
    for (i=0;i<2048;i+=2) {
      tmp_re = buffer[i + (j<<11)];
      tmp_im = buffer[i+1 + (j<<11)];
      tmp = (tmp_re*(int)tmp_re) + (tmp_im*(int)tmp_im);
      if ( tmp > max_f) {
	max_f = tmp;
	max_point = i>>1;
      }
    }
    avg += ((float)max_point/15);
    //    printf("%d (%f)\n",max_point,avg);
    
  }
  return(avg);
}

unsigned int revbits(unsigned int x) {

  int i;
  unsigned int ret = 0 ;


  for (i=0;i<31;i++)

    ret += (((x&(1<<i)) == 0) ? 0 : ((1<<(31-i))));

  return ret;
}
*/
