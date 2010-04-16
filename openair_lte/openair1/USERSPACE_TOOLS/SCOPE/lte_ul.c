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
#include "ARCH/COMMON/defs.h"
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

short *channel_drs[4],*channel_srs[4],*rx_sig[4],*ulsch_ext[2],*ulsch_comp,*ulsch_llr,**rx_sig_ptr;
int* sync_corr;

int length,offset;
float avg=1;

unsigned char nb_ant_tx, nb_ant_rx;


void lte_scope_idle_callback(void) {

  int i,j,ind,k,s;

  float Re,Im;
  float mag_sig[NB_ANTENNAS_RX*NB_ANTENNAS_TX*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig_time[NB_ANTENNAS_RX*NB_ANTENNAS_TX*NUMBER_OF_OFDM_CARRIERS*NUMBER_OF_OFDM_SYMBOLS_PER_SLOT],
    sig2[FRAME_LENGTH_COMPLEX_SAMPLES],
    time2[FRAME_LENGTH_COMPLEX_SAMPLES],
    I[25*12*12], Q[25*12*12],
    //llr[8*(3*8*6144+12)],
    //llr_time[8*(3*8*6144+12)];
    llr[25*12*4*7],
    llr_time[25*12*4*7];

  float cum_avg;
  
  /*
  // channel_t_re = sync_corr
  for (i=0; i<640*3; i++)  {
    sig2[i] = (float) (sync_corr[i]);
    time2[i] = (float) i;
  }

  //fl_set_xyplot_ybounds(form->channel_t_re,10,90);
  fl_set_xyplot_data(form->channel_t_re,time2,sig2,640*3,"","","");
  */

  // channel_t_re = srs
  cum_avg = 0;
  ind = 0;
  for (k=0;k<2;k++){
    for (j=0;j<2;j++) {
      
      s = PHY_config->lte_frame_parms.first_carrier_offset;
      for (i=0;i<PHY_config->lte_frame_parms.N_RB_UL*12;i++){
	sig_time[ind] = (float)ind;
	Re = (float)(channel_srs[k+2*j][2*s]);
	Im = (float)(channel_srs[k+2*j][2*s+1]);
	//mag_sig[ind] = (short) rand(); 
	mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	cum_avg += (short)sqrt((double)Re*Re + (double)Im*Im) ;
	ind++;
	s++;
	if (s>= PHY_config->lte_frame_parms.ofdm_symbol_size)
	  s=1;
      }
      //ind+=16; // spacing for visualization
    }
  }

  avg = cum_avg/NUMBER_OF_USEFUL_CARRIERS;

  fl_set_xyplot_ybounds(form->channel_t_re,30,70);
  fl_set_xyplot_data(form->channel_t_re,sig_time,mag_sig,ind,"","","");

  // channel_t_im = rx_sig
  for (i=0; i<FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX; i++)  {
    sig2[i] = (float) (rx_sig[0][2*i]);
    time2[i] = (float) i;
  }

  //fl_set_xyplot_ybounds(form->channel_t_re,0,100);
  fl_set_xyplot_data(form->channel_t_im,time2,sig2,FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX,"","","");

  // channel_f = drs
  cum_avg = 0;
  ind = 0;
  for (k=0;k<1;k++){
    for (j=0;j<1;j++) {
      
      for (i=0;i<PHY_config->lte_frame_parms.N_RB_UL*12*PHY_config->lte_frame_parms.symbols_per_tti;i++){
	sig_time[ind] = (float)ind;
	Re = (float)(channel_drs[k+2*j][2*i]);
	Im = (float)(channel_drs[k+2*j][2*i+1]);
	mag_sig[ind] = (short)10*log10(1.0+((double)Re*Re + (double)Im*Im)); 
	cum_avg += sqrt((double)Re*Re + (double)Im*Im) ;
	ind++;
      }
    }
  }

  avg = cum_avg/NUMBER_OF_USEFUL_CARRIERS;

  //fl_set_xyplot_ybounds(form->channel_f,30,70);
  fl_set_xyplot_data(form->channel_f,sig_time,mag_sig,ind,"","","");
  //fl_set_xyplot_data(form->decoder_input,sig_time,mag_sig,ind,"","","");

  // demod_out = ulsch_llr
  for(i=0;i<12*12*12;i++) {
    llr[i] = (float) ulsch_llr[i];
    llr_time[i] = (float) i;
  }

  fl_set_xyplot_data(form->demod_out,llr_time,llr,i,"","","");
  //  fl_set_xyplot_data(form->demod_out,time2,llr,25*12*4,"","","");
  fl_set_xyplot_ybounds(form->demod_out,-50,50);

  // scatter_plot2 = ulsch_comp
  for(i=0;i<12*12*12;i++) {
      I[i] = ulsch_comp[2*i];
      Q[i] = ulsch_comp[2*i+1];
  }

  fl_set_xyplot_data(form->scatter_plot2,I,Q,i,"","","");
  fl_set_xyplot_xbounds(form->scatter_plot2,-50,50);
  fl_set_xyplot_ybounds(form->scatter_plot2,-50,50);

  usleep(100000);
}
//-----------------------------------------------------------------------------
void do_scope(void){

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
  char title[20];
  unsigned int     bigphys_top;

  LTE_eNB_ULSCH *lte_eNb_ulsch;

  PHY_vars = malloc(sizeof(PHY_VARS));

  PHY_config = malloc(sizeof(PHY_CONFIG));

  printf("Opening /dev/openair0\n");
  if ((openair_fd = open("/dev/openair0", O_RDONLY)) <0) {
    fprintf(stderr,"Error %d opening /dev/openair0\n",openair_fd);
    exit(-1);
  }

  printf("Getting PHY_vars ...\n");

  ioctl(openair_fd,openair_GET_VARS,PHY_vars);

  printf("Getting PHY_config ...\n");

  ioctl(openair_fd,openair_GET_CONFIG,PHY_config);
  ioctl(openair_fd,openair_GET_BIGPHYSTOP,(void *)&bigphys_top);

  printf("Bigphys_top = %p\n",bigphys_top);
  printf("TX_DMA_BUFFER = %p\n",PHY_vars->lte_eNB_common_vars.txdataF);
  printf("RX_DMA_BUFFER = %p\n",PHY_vars->lte_eNB_common_vars.rxdata);
  printf("PHY_vars->lte_eNB_common_vars.sync_corr = %p\n",PHY_vars->lte_ue_common_vars.sync_corr);
  printf("PHY_vars->lte_eNB_ulsch_vars[0] = %p\n",PHY_vars->lte_eNB_ulsch_vars[0]);
  printf("PHY_vars->lte_eNB_common_vars.srs_ch_estimates[0] = %p\n",PHY_vars->lte_eNB_common_vars.srs_ch_estimates[0]);

  printf("NUMBER_OF_OFDM_CARRIERS = %d\n",NUMBER_OF_OFDM_CARRIERS);

  nb_ant_tx = PHY_config->lte_frame_parms.nb_antennas_tx;
  nb_ant_rx = PHY_config->lte_frame_parms.nb_antennas_rx;
  printf("(TX, RX) ANTENNAS = %d, %d\n",nb_ant_tx,nb_ant_rx);
  
  mem_base = (unsigned int) mmap(0,
				 BIGPHYS_NUMPAGES*4096,
				 PROT_READ,
				 MAP_PRIVATE,
				 openair_fd,
				 0);

  if (mem_base != -1)
    msg("MEM base= %x\n",mem_base);
  else
    msg("Could not map physical memory\n");

  sync_corr = (int*) (mem_base +
		      (unsigned int)PHY_vars->lte_eNB_common_vars.sync_corr -
		      bigphys_top);

  lte_eNb_ulsch = (LTE_eNB_ULSCH *) (mem_base + 
				     (unsigned int)PHY_vars->lte_eNB_ulsch_vars[0] - 
				     bigphys_top);

  printf("lte_eNb_ulsch = %p\n",lte_eNb_ulsch);


  for (i=0;i<nb_ant_tx*nb_ant_rx;i++) {

    channel_srs[i] = (short*)(mem_base + 
			      (unsigned int)PHY_vars->lte_eNB_common_vars.srs_ch_estimates[0] + 
			      nb_ant_rx*nb_ant_tx*sizeof(int*) + 
			      i*(sizeof(int)*PHY_config->lte_frame_parms.ofdm_symbol_size) - 
			      bigphys_top);

    channel_drs[i] = (short*)(mem_base + 
			      (unsigned int)lte_eNb_ulsch->drs_ch_estimates[0] + 
			      nb_ant_rx*nb_ant_tx*sizeof(int*) + 
			      i*(PHY_config->lte_frame_parms.symbols_per_tti*sizeof(int)*PHY_config->lte_frame_parms.N_RB_UL*12) - 
			      bigphys_top);

    ulsch_ext[i] = (short*)(mem_base + 
			    (unsigned int)lte_eNb_ulsch->rxdataF_ext[0] + 
			      nb_ant_rx*sizeof(int*) + 
			      i*(PHY_config->lte_frame_parms.symbols_per_tti*sizeof(int)*PHY_config->lte_frame_parms.N_RB_UL*12) - 
			    bigphys_top);
    /*
    ulsch_ext[i] = (short*)(mem_base + 
			(unsigned int)PHY_vars->lte_eNB_common_vars.rxdataF[0] + 
			nb_ant_rx*sizeof(int*) + 
			i*(PHY_config->lte_frame_parms.symbols_per_tti*sizeof(int)*PHY_config->lte_frame_parms.ofdm_symbol_size) - 
			bigphys_top);
    */
  }
    
  rx_sig_ptr = (short **)(mem_base + 
			  (unsigned int)PHY_vars->lte_eNB_common_vars.rxdata[0] -
			  bigphys_top);

  for (i=0;i<nb_ant_rx;i++) {
    rx_sig[i] = (short *)(mem_base + 
			  (unsigned int)rx_sig_ptr[i] -
			  bigphys_top);
  }
    
  ulsch_comp = (short*)(mem_base + 
			(unsigned int)lte_eNb_ulsch->rxdataF_comp[0] + 
			nb_ant_rx*sizeof(int*) -
			bigphys_top);


  ulsch_llr = (short*)(mem_base + 
		       (unsigned int)lte_eNb_ulsch->llr -
			bigphys_top);

  sprintf(title, "LTE SCOPE"),

  fl_initialize(&argc, argv, title, 0, 0);    /* SIGSCOPE */
  form = create_form_lte_scope();                 /* SIGSCOPE */
  fl_show_form(form->lte_scope,FL_PLACE_HOTSPOT,FL_FULLBORDER,title);   /* SIGSCOPE */

  do_scope();
  sleep(5);
  close(openair_fd);
  return(0);
}
