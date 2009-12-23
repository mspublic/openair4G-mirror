#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <sys/user.h>
//#include <errno.h>
#include <math.h>
#include <signal.h>
#include <strings.h>
#include "forms.h"
#include "lte_scope.h"

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

//#include "PHY/CONFIG/vars.h"
//#include "MAC_INTERFACE/vars.h"
//#include "PHY/TOOLS/defs.h"


FD_lte_scope *form;


//short channel[2048];
//short channel_f[2048];
//char demod_data[2048];

short *channel[4],*channel_f[4],*rx_sig[4];
int *sync_corr;
short *pbch_llr,*pbch_comp;
short *dlsch_llr,*dlsch_comp;

int length,offset;
float avg=1;
int sach_flag; 

unsigned char nb_ant_tx, nb_ant_rx;


void lte_scope_idle_callback(void) {

  int i,j,ind,k,s;

  float Re,Im;
  float mag_sig[NB_ANTENNAS_RX*NB_ANTENNAS_TX*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig_time[NB_ANTENNAS_RX*NB_ANTENNAS_TX*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig2[FRAME_LENGTH_COMPLEX_SAMPLES],
    time2[FRAME_LENGTH_COMPLEX_SAMPLES],
    I[25*12*12], Q[25*12*12],
    //llr[384];
    llr[25*12*4*7];

  /*
    mag_h[NB_ANTENNAS_RX*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8],
    mag_sig2[NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8],
    scat_sig_re[NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8],
    scat_sig_im[NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8],
    scat_sig_re2[NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8],
    scat_sig_im2[NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT*8];
  */
  float cum_avg;
  
  /*
  for (i=0;i<2*CYCLIC_PREFIX_LENGTH;i++){
    sig_time[i] = (float)i;
    Re = (float)(channel[0][4*(NUMBER_OF_OFDM_CARRIERS - 2*CYCLIC_PREFIX_LENGTH) + 4*i]);
    Im = (float)(channel[0][4*(NUMBER_OF_OFDM_CARRIERS - 2*CYCLIC_PREFIX_LENGTH) + 1+(4*i)]);
    mag_sig[i] = 10*log10(1+Re*Re + Im*Im);
    Re = (float)(channel[1][4*(NUMBER_OF_OFDM_CARRIERS - 2*CYCLIC_PREFIX_LENGTH) + 4*i]);
    Im = (float)(channel[1][4*(NUMBER_OF_OFDM_CARRIERS - 2*CYCLIC_PREFIX_LENGTH) + 1+(4*i)]);
    mag_sig2[i] = 10*log10(1+Re*Re + Im*Im);
  }
  //fl_set_xyplot_data(form->scatter_sig,real_mf,imag_mf,136,"","","");
  fl_set_xyplot_ybounds(form->channel_t_re,30,80);
  fl_set_xyplot_ybounds(form->channel_t_im,30,80);
  fl_set_xyplot_data(form->channel_t_re,sig_time,mag_sig,2*CYCLIC_PREFIX_LENGTH,"","","");
  fl_set_xyplot_data(form->channel_t_im,sig_time,mag_sig2,2*CYCLIC_PREFIX_LENGTH,"","","");
  */

  cum_avg = 0;
  ind = 0;
  for (k=0;k<1;k++){
    for (j=0;j<1;j++) {
      
      for (i=0;i<PHY_config->lte_frame_parms.symbols_per_tti*PHY_config->lte_frame_parms.ofdm_symbol_size;i++){
	sig_time[ind] = (float)ind;
	Re = (float)(channel_f[k+2*j][2*i]);
	Im = (float)(channel_f[k+2*j][2*i+1]);
	//mag_sig[ind] = (short) rand(); 
	mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	cum_avg += (short)sqrt((double)Re*Re + (double)Im*Im) ;
	ind++;
      }
      //      ind+=NUMBER_OF_OFDM_CARRIERS/4; // spacing for visualization
    }
  }

  avg = cum_avg/NUMBER_OF_USEFUL_CARRIERS;

  fl_set_xyplot_ybounds(form->channel_f,30,70);
  fl_set_xyplot_data(form->channel_f,sig_time,mag_sig,ind,"","","");

  // channel_t_re = sync_corr
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)  {
    sig2[i] = (float) (sync_corr[i]);
    time2[i] = (float) i;
  }

  //fl_set_xyplot_ybounds(form->channel_t_re,10,90);
  fl_set_xyplot_data(form->channel_t_re,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");

  /*
  cum_avg = 0;
  ind = 0;
  for (k=0;k<1;k++){
    for (j=0;j<1;j++) {
      
      for (i=0;i<PHY_config->lte_frame_parms.ofdm_symbol_size;i++){
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

  fl_set_xyplot_ybounds(form->channel_t_re,10,90);
  fl_set_xyplot_data(form->channel_t_re,sig_time,mag_sig,ind,"","","");
  */


  // channel_t_im = rx_sig
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES; i++)  {
    sig2[i] = (float) (rx_sig[0][2*i]);
    time2[i] = (float) i;
  }

  //fl_set_xyplot_ybounds(form->channel_t_re,0,100);
  fl_set_xyplot_data(form->channel_t_im,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES,"","","");

  j=0;
  for(i=0;i<384;i++) {
    llr[j] = (float) pbch_llr[i];
    time2[j] = (float) j;
    if (i==63)
      i=127;
    else if (i==191)
      i=319;
    j++;
  }

  fl_set_xyplot_data(form->decoder_input,time2,llr,192,"","","");
  fl_set_xyplot_ybounds(form->decoder_input,-50,50);

  j=0;
  for(i=0;i<6*12*4;i++) {
    I[j] = pbch_comp[2*i];
    Q[j] = pbch_comp[2*i+1];
    j++;
    if (i==47)
      i=96;
    else if (i==191)
      i=239;
  }

  fl_set_xyplot_data(form->scatter_plot,I,Q,3*12*4,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot,-50,50);
  fl_set_xyplot_ybounds(form->scatter_plot,-50,50);

  for(i=0;i<12*12*2*7;i++) {
    llr[i] = (float) dlsch_llr[i];
    time2[i] = (float) i;
  }

  fl_set_xyplot_data(form->demod_out,time2,llr,12*12*2*7,"","","");
  //  fl_set_xyplot_data(form->demod_out,time2,llr,25*12*4,"","","");
  fl_set_xyplot_ybounds(form->demod_out,-50,50);

  j=0;
  for (s=2;s<12;s++) {
    for(i=0;i<12*12;i++) {
      I[j] = dlsch_comp[(2*25*12*s)+2*i];
      Q[j] = dlsch_comp[(2*25*12*s)+2*i+1];
      j++;
    }
    if (s==2)
      s=3;
    else if (s==5)
      s=6;
    else if (s==8)
      s=9;
  }

  fl_set_xyplot_data(form->scatter_plot2,I,Q,j,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot2,-50,50);
  fl_set_xyplot_ybounds(form->scatter_plot2,-50,50);

  usleep(100000);
}
//-----------------------------------------------------------------------------
do_scope(){

//-----------------------------------------------------------------------------
  char ch;

  fl_set_idle_callback(lte_scope_idle_callback, NULL);
  fl_do_forms() ;      /* SIGSCOPE */



}

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
//-----------------------------------------------------------------------------

  int openair_fd,i;
  unsigned int mem_base;
  unsigned int first_symbol;
  char title[20];

  PHY_vars = malloc(sizeof(PHY_VARS));

  PHY_config = malloc(sizeof(PHY_CONFIG));

  /*
   if((config = fopen("./widens_config.cfg","r")) == NULL) // this can be configured
	{
	  printf("[Main USER] The widens configuration file <widens_config.cfg> could not be found!");
	  exit(0);
	}		
  
  if ((scenario= fopen("./widens_scenario.scn","r")) ==NULL)
    {
      printf("[Main USER] The widens scenario file <widens_scenario.scn> could not be found!");
      exit(0);
    }
  
  printf("Opened configuration files\n");

  reconfigure_MACPHY(scenario);
  */

  printf("Opening /dev/openair0\n");
  if ((openair_fd = open("/dev/openair0", O_RDONLY)) <0) {
    fprintf(stderr,"Error %d opening /dev/openair0\n",openair_fd);
    exit(-1);
  }

  printf("Getting PHY_vars ...\n");

  ioctl(openair_fd,openair_GET_VARS,PHY_vars);

  printf("Getting PHY_config ...\n");

  ioctl(openair_fd,openair_GET_CONFIG,PHY_config);

  printf("PHY_vars->tx_vars[0].TX_DMA_BUFFER = %p\n",PHY_vars->tx_vars[0].TX_DMA_BUFFER);
  printf("PHY_vars->rx_vars[0].RX_DMA_BUFFER = %p\n",PHY_vars->rx_vars[0].RX_DMA_BUFFER);
  printf("PHY_vars->lte_ue_common_vars.dl_ch_estimates = %p\n",PHY_vars->lte_ue_common_vars.dl_ch_estimates);
  printf("PHY_vars->lte_ue_common_vars.sync_corr = %p\n",PHY_vars->lte_ue_common_vars.sync_corr);
  printf("PHY_vars->lte_ue_pbch_vars.rxdataF_comp = %p\n",PHY_vars->lte_ue_pbch_vars.rxdataF_comp);

  printf("NUMBER_OF_OFDM_CARRIERS = %d\n",NUMBER_OF_OFDM_CARRIERS);

  nb_ant_tx = PHY_config->lte_frame_parms.nb_antennas_tx;
  nb_ant_rx = PHY_config->lte_frame_parms.nb_antennas_rx;
  printf("(TX, RX) ANTENNAS = %d, %d\n",nb_ant_tx,nb_ant_rx);
  
  mem_base = mmap(0,
		  2048*4096,
		  PROT_READ,
		  MAP_PRIVATE,
		  openair_fd,
		  0);

  if (mem_base != -1)
    msg("MEM base= %x\n",mem_base);
  else
    msg("Could not map physical memory\n");

  for (i=0;i<nb_ant_tx*nb_ant_rx;i++) {

    channel_f[i] = (short*)(mem_base + 
			    (unsigned int)PHY_vars->lte_ue_common_vars.dl_ch_estimates + 
			    nb_ant_rx*nb_ant_tx*sizeof(int*) + 
			    i*(PHY_config->lte_frame_parms.symbols_per_tti*sizeof(int)*PHY_config->lte_frame_parms.ofdm_symbol_size) - 
			    (unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);

    channel[i] = (short*)(mem_base + 
			  (unsigned int)PHY_vars->lte_ue_common_vars.dl_ch_estimates_time + 
			  nb_ant_rx*nb_ant_tx*sizeof(int*) + 
			  i*(PHY_config->lte_frame_parms.symbols_per_tti*sizeof(int)*PHY_config->lte_frame_parms.ofdm_symbol_size) - 
			  (unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);

    rx_sig[i] = (short *)(mem_base + (unsigned int)PHY_vars->rx_vars[i].RX_DMA_BUFFER-(unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);

  }

  sync_corr = (short*)(mem_base + (unsigned int)PHY_vars->lte_ue_common_vars.sync_corr - (unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);

  pbch_comp = (short*)(mem_base + 
		       (unsigned int)PHY_vars->lte_ue_pbch_vars.rxdataF_comp + 
		       nb_ant_rx*nb_ant_tx*sizeof(int*) - 
		       (unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);
  
  pbch_llr = (short*) (mem_base + 
		       (unsigned int)PHY_vars->lte_ue_pbch_vars.llr - 
		       (unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);
  
  dlsch_comp = (short*)(mem_base + 
			(unsigned int)PHY_vars->lte_ue_dlsch_vars.rxdataF_comp + 
			nb_ant_rx*nb_ant_tx*sizeof(int*) - 
			(unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);
  
  dlsch_llr = (short*) (mem_base + 
			(unsigned int)PHY_vars->lte_ue_dlsch_vars.llr[0] - 
			(unsigned int)&PHY_vars->tx_vars[0].TX_DMA_BUFFER[0]);
  
  sprintf(title, "LTE SCOPE"),

  fl_initialize(&argc, argv, title, 0, 0);    /* SIGSCOPE */
  form = create_form_lte_scope();                 /* SIGSCOPE */
  fl_show_form(form->lte_scope,FL_PLACE_HOTSPOT,FL_FULLBORDER,title);   /* SIGSCOPE */

  do_scope();
  sleep(5);
  close(openair_fd);
  return(0);
}
