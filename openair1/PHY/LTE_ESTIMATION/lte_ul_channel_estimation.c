#include "PHY/defs.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_CH

// For Channel Estimation in Distributed Alamouti Scheme
static short temp_out_ifft[2048*4] __attribute__((aligned(16)));
static short temp_out_fft_0[2048*4] __attribute__((aligned(16)));
static short temp_out_fft_1[2048*4] __attribute__((aligned(16)));

static int temp_in_ifft[2048*2] __attribute__((aligned(16)));
static int temp_in_fft_0[2048*2] __attribute__((aligned(16)));
static int temp_in_fft_1[2048*2] __attribute__((aligned(16)));


int lte_ul_channel_estimation(int **ul_ch_estimates,
			      int **ul_ch_estimates_time,
			      int **ul_ch_estimates_0,
			      int **ul_ch_estimates_1,
			      int **rxdataF_ext,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char l,
			      unsigned char Ns,
			      unsigned int N_rb_alloc,
			      u8 cyclicShift,
			      u8 cooperation_flag) {

  unsigned short aa,b,k,Msc_RS,Msc_RS_idx,symbol_offset,i,j;
  unsigned short * Msc_idx_ptr;
  unsigned short pilot_pos1 = 3 - frame_parms->Ncp, pilot_pos2 = 10 - 2*frame_parms->Ncp;
  short alpha, beta;
  int *ul_ch1, *ul_ch2;
  int *ul_ch1_0,*ul_ch2_0,*ul_ch1_1,*ul_ch2_1;
  //s8 phase_shift;
  //phase_shift = -(2*M_PI*cyclicShift)/12;
  short ul_ch_estimates_re,ul_ch_estimates_im;
  int rx_power_correction;

  //debug_msg("lte_ul_channel_estimation: cyclic shift %d\n",cyclicShift);


  s16 alpha_re[12] = {32767, 28377, 16383,     0,-16384,  -28378,-32768,-28378,-16384,    -1, 16383, 28377};
  s16 alpha_im[12] = {0,     16383, 28377, 32767, 28377,   16383,     0,-16384,-28378,-32768,-28378,-16384};

  int *temp_out_ifft_ptr = (int*)0,*in_fft_ptr_0 = (int*)0,*in_fft_ptr_1 = (int*)0,
    *temp_out_fft_0_ptr = (int*)0,*out_fft_ptr_0 = (int*)0,*temp_out_fft_1_ptr = (int*)0,
    *out_fft_ptr_1 = (int*)0,*in_ifft_ptr = (int*)0,*temp_in_ifft_ptr = (int*)0;
  
  Msc_RS = N_rb_alloc*12;

#ifdef USER_MODE
  Msc_idx_ptr = (unsigned short*) bsearch(&Msc_RS, dftsizes, 33, sizeof(unsigned short), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("lte_ul_channel_estimation: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else
  for (b=0;b<33;b++) 
    if (Msc_RS==dftsizes[b])
      Msc_RS_idx = b;
#endif

#ifdef DEBUG_CH
  msg("lte_ul_channel_estimation: Msc_RS = %d, Msc_RS_idx = %d\n",Msc_RS, Msc_RS_idx);
#ifdef USER_MODE
  write_output("drs_seq.m","drs",ul_ref_sigs_rx[0][0][Msc_RS_idx],2*Msc_RS,2,1);
#endif
#endif

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  if (l == (3 - frame_parms->Ncp)) {

    symbol_offset = frame_parms->N_RB_UL*12*(l+((7-frame_parms->Ncp)*(Ns&1)));

    for (aa=0; aa<frame_parms->nb_antennas_rx; aa++){
      //msg("Componentwise prod aa %d, symbol_offset %d,ul_ch_estimates %p,ul_ch_estimates[aa] %p,ul_ref_sigs_rx[0][0][Msc_RS_idx] %p\n",aa,symbol_offset,ul_ch_estimates,ul_ch_estimates[aa],ul_ref_sigs_rx[0][0][Msc_RS_idx]);
      mult_cpx_vector_norep2((short*) &rxdataF_ext[aa][symbol_offset<<1],
			     (short*) ul_ref_sigs_rx[0][0][Msc_RS_idx],
			     (short*) &ul_ch_estimates[aa][symbol_offset],
			     Msc_RS,
			     15);

      memset(temp_in_ifft,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);   
      // Convert to time domain for visualization
      for(i=0;i<frame_parms->N_RB_UL*12;i++)
	temp_in_ifft[i] = ul_ch_estimates[aa][symbol_offset+i];
	    
	  
      fft( (short*) temp_in_ifft,                          
	   (short*) ul_ch_estimates_time[aa],
	   frame_parms->twiddle_ifft,
	   frame_parms->rev,
	   (frame_parms->log2_symbol_size),
	   (frame_parms->log2_symbol_size)/2,
	   0);

#ifdef DEBUG_CH      
#ifdef USER_MODE
      if (aa==0)
	write_output("drs_est1.m","drs1",ul_ch_estimates_time[aa],512*2,2,1);
#endif
#endif

      if((cyclicShift != 0) &&(cooperation_flag != 2)){
	// Compemsating for the phase shift introduced at the transmitter
	for(i=symbol_offset;i<symbol_offset+frame_parms->N_RB_UL*12;i++){
	  ul_ch_estimates_re = ((short*) ul_ch_estimates[aa])[i<<1];
	  ul_ch_estimates_im = ((short*) ul_ch_estimates[aa])[(i<<1)+1];
	  ((short*) ul_ch_estimates[aa])[i<<1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_re;
	  /*
	    (short) (((int) (i%2 == 1? 1:-1) * (int) (alpha_re[cyclicShift]) * (int) (ul_ch_estimates_re) + 
		      (int) (i%2 == 1? 1:-1) * (int) (alpha_im[cyclicShift]) * (int) (ul_ch_estimates_im))>>15);
	  */
	  ((short*) ul_ch_estimates[aa])[(i<<1)+1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_im;
	  /*
	    (short) (((int) (i%2 == 1? 1:-1) * (int) (alpha_re[cyclicShift]) * (int) (ul_ch_estimates_im) -  
		      (int) (i%2 == 1? 1:-1) * (int) (alpha_im[cyclicShift]) * (int) (ul_ch_estimates_re))>>15);
	  */
	}
      }

      /*      
      // Convert to time domain for visualization
      memset(temp_in_ifft,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
      for(i=0;i<frame_parms->N_RB_UL*12;i++)
	temp_in_ifft[i] = ul_ch_estimates[aa][symbol_offset+i];
	    
      fft( (short*) temp_in_ifft,                          
	   (short*) ul_ch_estimates_time[aa],
	   frame_parms->twiddle_ifft,
	   frame_parms->rev,
	   (frame_parms->log2_symbol_size),
	   (frame_parms->log2_symbol_size)/2,
	   0);
      

#ifdef USER_MODE
      if (aa==0)
	write_output("drs_est2.m","drs2",ul_ch_estimates_time[aa],512*2,2,1);
#endif
      */

      if(cooperation_flag == 2)// Memory Allocation for temporary pointers to Channel Estimates
	{
	  memset(temp_in_ifft,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	  memset(temp_in_fft_0,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	  memset(temp_in_fft_1,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);

	  //Extracting Channel Estimates for Distributed Alamouti Receiver Combining
	  
	  temp_in_ifft_ptr = &temp_in_ifft[0];
	
	  i = symbol_offset;
	 
	  for(j=0;j<(frame_parms->N_RB_UL*12);j++){
	    temp_in_ifft_ptr[j] = ul_ch_estimates[aa][i];
	    i++;
	  }


	  fft((short*) &temp_in_ifft[0],                          // Performing IFFT on Combined Channel Estimates
	      temp_out_ifft, 
	      //ul_ch_estimates_time[aa],
	      frame_parms->twiddle_ifft,
	      frame_parms->rev,
	      (frame_parms->log2_symbol_size),
	      (frame_parms->log2_symbol_size)/2,
	      0);




	  // because the ifft is not power preserving, we should apply the factor sqrt(power_correction) here, but we rather apply power_correction here and nothing after the next fft
	  in_fft_ptr_0 = &temp_in_fft_0[0];
	  temp_out_ifft_ptr = (int*)temp_out_ifft;
	  //temp_out_ifft_ptr = (int*)ul_ch_estimates_time[aa];
	 
	  for(j= 0;j<(1<<(frame_parms->log2_symbol_size))/2;j++)
	    {
	      ((short*)in_fft_ptr_0)[2*j] = ((short*)temp_out_ifft_ptr)[4*j]*rx_power_correction;
	      ((short*)in_fft_ptr_0)[2*j+1] = ((short*)temp_out_ifft_ptr)[4*j+1]*rx_power_correction;
	    }
	  
	 

	  fft((short*) &temp_in_fft_0[0],                        // Performing FFT to obtain the Channel Estimates for UE0 to eNB1
	      temp_out_fft_0,
	      frame_parms->twiddle_fft,
	      frame_parms->rev,
	      frame_parms->log2_symbol_size,
	      frame_parms->log2_symbol_size>>1,
	      0);

	  out_fft_ptr_0 = &ul_ch_estimates_0[aa][symbol_offset]; // CHANNEL ESTIMATES FOR UE0 TO eNB1
	  temp_out_fft_0_ptr = (int*) temp_out_fft_0;

	  i=0;

	  for(j=0;j<frame_parms->N_RB_UL*12;j++){
	    out_fft_ptr_0[i] = temp_out_fft_0_ptr[2*j];
	    i++;
	  }

	  
	  	     
	  in_fft_ptr_1 = &temp_in_fft_1[0];
	  temp_out_ifft_ptr = (int*)temp_out_ifft;
	  //temp_out_ifft_ptr = (int*)ul_ch_estimates_time[aa];

	  for(j=(1<<frame_parms->log2_symbol_size)/2;j<(1<<(frame_parms->log2_symbol_size));j++)
	    {
	      ((short*)in_fft_ptr_1)[2*j] = ((short*)temp_out_ifft_ptr)[4*j]*rx_power_correction;
	      ((short*)in_fft_ptr_1)[2*j+1] = ((short*)temp_out_ifft_ptr)[4*j+1]*rx_power_correction;
	    }
	  
	 

	  fft((short*) &temp_in_fft_1[0],                          // Performing FFT to obtain the Channel Estimates for UE1 to eNB1
	      temp_out_fft_1,
	      frame_parms->twiddle_fft,
	      frame_parms->rev,
	      frame_parms->log2_symbol_size,
	      frame_parms->log2_symbol_size>>1,
	      0);

	  out_fft_ptr_1 = &ul_ch_estimates_1[aa][symbol_offset];   // CHANNEL ESTIMATES FOR UE1 TO eNB1
	  temp_out_fft_1_ptr = (int*) temp_out_fft_1;

	  i=0;
	  
	  for(j=0;j<frame_parms->N_RB_UL*12;j++){
	    out_fft_ptr_1[i] = (j%2==0?-1:1)*temp_out_fft_1_ptr[2*j];
	    i++;
	  }

#ifdef DEBUG_CH
#ifdef USER_MODE
	  if((aa == 0)&& (cooperation_flag == 2)){
	    write_output("test1.m","t1",temp_in_ifft,512,1,1);
	    //write_output("test2.m","t2",temp_out_ifft,512*2,2,1);
	    write_output("test2.m","t2",ul_ch_estimates_time[aa],512*2,2,1);
	    write_output("test3.m","t3",temp_in_fft_0,512,1,1);  
	    write_output("test4.m","t4",temp_out_fft_0,512,1,1);
	    write_output("test5.m","t5",temp_in_fft_1,512,1,1);  
	    write_output("test6.m","t6",temp_out_fft_1,512,1,1);
	  }
#endif
#endif

	}//cooperation_flag == 2

      if (Ns&1) {//we are in the second slot of the sub-frame, so do the interpolation

	ul_ch1 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	ul_ch2 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos2];


	if(cooperation_flag == 2)// For Distributed Alamouti
	  {
	    ul_ch1_0 = &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	    ul_ch2_0 = &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*pilot_pos2];

	    ul_ch1_1 = &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	    ul_ch2_1 = &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*pilot_pos2];
	  }


#ifdef DEBUG_CH
	msg("lte_ul_channel_estimation: ul_ch1 = %p, ul_ch2 = %p, pilot_pos1=%d, pilot_pos2=%d\n",ul_ch1, ul_ch2, pilot_pos1,pilot_pos2); 
#endif
	for (k=0;k<frame_parms->symbols_per_tti;k++) {

	  // we scale alpha and beta by 0x3FFF (instead of 0x7FFF) to avoid overflows 
	  alpha = (short) (((int) 0x3FFF * (int) (pilot_pos2-k))/(pilot_pos2-pilot_pos1));
	  beta  = (short) (((int) 0x3FFF * (int) (k-pilot_pos1))/(pilot_pos2-pilot_pos1));
	  
#ifdef DEBUG_CH
	  msg("lte_ul_channel_estimation: k=%d, alpha = %d, beta = %d\n",k,alpha,beta); 
#endif
	  //symbol_offset_subframe = frame_parms->N_RB_UL*12*k;

	  // interpolate between estimates
	  if ((k != pilot_pos1) && (k != pilot_pos2))  {
	    multadd_complex_vector_real_scalar((short*) ul_ch1,alpha,(short*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2,beta ,(short*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,N_rb_alloc*12);
	 
	    if(cooperation_flag == 2)// For Distributed Alamouti
	      {
		multadd_complex_vector_real_scalar((short*) ul_ch1_0,beta ,(short*) &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*k],1,N_rb_alloc*12);
		multadd_complex_vector_real_scalar((short*) ul_ch2_0,alpha,(short*) &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*k],0,N_rb_alloc*12);

		multadd_complex_vector_real_scalar((short*) ul_ch1_1,beta ,(short*) &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*k],1,N_rb_alloc*12);
		multadd_complex_vector_real_scalar((short*) ul_ch2_1,alpha,(short*) &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*k],0,N_rb_alloc*12);
	      }

	  }
	} //for(k=...

	// because of the scaling of alpha and beta we also need to scale the final channel estimate at the pilot positions 
	multadd_complex_vector_real_scalar((short*) ul_ch1,0x3FFF,(short*) ul_ch1,1,N_rb_alloc*12);
	multadd_complex_vector_real_scalar((short*) ul_ch2,0x3FFF,(short*) ul_ch2,1,N_rb_alloc*12);

	if(cooperation_flag == 2)// For Distributed Alamouti
	  {
	    multadd_complex_vector_real_scalar((short*) ul_ch1_0,0x3FFF,(short*) ul_ch1_0,1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2_0,0x3FFF,(short*) ul_ch2_0,1,N_rb_alloc*12);

	    multadd_complex_vector_real_scalar((short*) ul_ch1_1,0x3FFF,(short*) ul_ch1_1,1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2_1,0x3FFF,(short*) ul_ch2_1,1,N_rb_alloc*12);
	  }

	//write_output("drs_est.m","drsest",ul_ch_estimates[0],300*12,1,1);
		/*if(cooperation_flag == 2)// For Distributed Alamouti
		{
		write_output("drs_est.m","drsest",ul_ch_estimates[0],300*12,1,1);
		write_output("drs_est0.m","drsest0",ul_ch_estimates_0[0],300*12,1,1);
		write_output("drs_est1.m","drsest1",ul_ch_estimates_1[0],300*12,1,1);
		}*/


      } //if (Ns&1)

    } //for(aa=...
    
  } //if(l==...
  
  return(0);
}       

extern unsigned short transmission_offset_tdd[16];
#define DEBUG_SRS

int lte_srs_channel_estimation(LTE_DL_FRAME_PARMS *frame_parms,
			       LTE_eNB_COMMON *eNb_common_vars,
			       LTE_eNB_SRS *eNb_srs_vars,
			       SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,
			       unsigned char sub_frame_number,
			       unsigned char eNb_id) {

  int T_SFC,aa;
  int N_symb,symbol;
#ifdef DEBUG_SRS
  char fname[40], vname[40];
#endif

  u8 Ssrs  = frame_parms->soundingrs_ul_config_common.srs_SubframeConfig;

  N_symb = 2*7-frame_parms->Ncp;
  symbol = (sub_frame_number+1)*N_symb-1; //SRS is always in last symbol of subframe
  T_SFC = (Ssrs<=7 ? 5 : 10);

  /* 
  msg("SRS channel estimation eNb %d, subframs %d, %d %d %d %d %d\n",eNb_id,sub_frame_number,
      SRS_parms->Csrs,
      SRS_parms->Bsrs,
      SRS_parms->kTC,
      SRS_parms->n_RRC,
      SRS_parms->Ssrs);
  */

  if ((1<<(sub_frame_number%T_SFC))&transmission_offset_tdd[Ssrs]) {

    if (generate_srs_rx(frame_parms, 
			soundingrs_ul_config_dedicated,
			eNb_srs_vars->srs)==-1) {
      msg("lte_srs_channel_estimation: Error in generate_srs_rx\n");
      return(-1);
    }

    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
#ifdef DEBUG_SRS
      msg("SRS channel estimation eNb %d, subframs %d, aarx %d, %p, %p, %p\n",eNb_id,sub_frame_number,aa,
	  &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	  eNb_srs_vars->srs,
	  eNb_srs_vars->srs_ch_estimates[eNb_id][aa]);
#endif

      //write_output("eNb_rxF.m","rxF",&eNb_common_vars->rxdataF[0][aa][2*frame_parms->ofdm_symbol_size*symbol],2*(frame_parms->ofdm_symbol_size),2,1);
      //write_output("eNb_srs.m","srs_eNb",eNb_common_vars->srs,(frame_parms->ofdm_symbol_size),1,1);

      mult_cpx_vector_norep((short*) &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
			    (short*) eNb_srs_vars->srs,
			    (short*) eNb_srs_vars->srs_ch_estimates[eNb_id][aa],
			    frame_parms->ofdm_symbol_size,
			    15);

      //msg("SRS channel estimation cmult out\n");
#ifdef USER_MODE
#ifdef DEBUG_SRS
	sprintf(fname,"eNB_id%d_an%d_srs_ch_est.m",eNb_id,aa);
	sprintf(vname,"eNB%d_%d_srs_ch_est",eNb_id,aa);
	write_output(fname,vname,eNb_srs_vars->srs_ch_estimates[eNb_id][aa],frame_parms->ofdm_symbol_size,1,1);
#endif
#endif
    }
  }
  /*
  else {
    for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) 
      bzero(eNb_srs_vars->srs_ch_estimates[eNb_id][aa],frame_parms->ofdm_symbol_size*sizeof(int));
  }
  */
  return(0);
}
