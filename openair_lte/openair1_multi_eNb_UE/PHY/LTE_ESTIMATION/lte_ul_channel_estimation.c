#include "PHY/defs.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_CH

// For Channel Estimation in Distributed Alamouti Scheme
static short temp_out_ifft[2048*4] __attribute__((aligned(16)));
static short temp_out_fft_0[2048*4] __attribute__((aligned(16)));
static short temp_out_fft_1[2048*4] __attribute__((aligned(16)));

int lte_ul_channel_estimation(int **ul_ch_estimates,
			      int **ul_ch_estimates_0,
			      int **ul_ch_estimates_1,
			      int **rxdataF_ext,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char l,
			      unsigned char Ns,
			      unsigned int N_rb_alloc,
			      unsigned char relay_flag,
			      unsigned char diversity_scheme) {

  unsigned short aa,b,k,Msc_RS,Msc_RS_idx,symbol_offset,i,j,re_offset;
  unsigned short * Msc_idx_ptr;
  unsigned short pilot_pos1 = 3 - frame_parms->Ncp, pilot_pos2 = 10 - 2*frame_parms->Ncp;
  short alpha, beta;
  int *ul_ch1, *ul_ch2;

  int *ul_ch1_0,*ul_ch2_0,*ul_ch1_1,*ul_ch2_1;
  int *temp_in_ifft,**temp_in_fft_0,*temp_in_fft_1;
  Msc_RS = N_rb_alloc*12;


  int *temp_out_ifft_ptr = (int*)0,*in_fft_ptr_0 = (int*)0,*in_fft_ptr_1 = (int*)0,*temp_out_fft_0_ptr = (int*)0,*out_fft_ptr_0 = (int*)0,*temp_out_fft_1_ptr = (int*)0,*out_fft_ptr_1 = (int*)0,*in_ifft_ptr = (int*)0,*temp_in_ifft_ptr = (int*)0;
  
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
#ifdef DEBUG_PHY
  write_output("drs_seq.m","drs",ul_ref_sigs_rx[0][0][Msc_RS_idx],2*Msc_RS,2,1);
#endif
#endif
#endif

  if (l == (3 - frame_parms->Ncp)) {

    symbol_offset = frame_parms->N_RB_UL*12*(l+((7-frame_parms->Ncp)*(Ns&1)));

    for (aa=0; aa<frame_parms->nb_antennas_rx; aa++){
      //      msg("Componentwise prod aa %d, symbol_offset %d\n",aa,symbol_offset);
      mult_cpx_vector_norep2((short*) &rxdataF_ext[aa][symbol_offset<<1],
			     (short*) ul_ref_sigs_rx[0][0][Msc_RS_idx],
			     (short*) &ul_ch_estimates[aa][symbol_offset],
			     Msc_RS,
			     15);

}

    for (aa=0; aa<frame_parms->nb_antennas_rx; aa++){


      if((relay_flag == 2) && (diversity_scheme == 2))// Memory Allocation for temporary pointers to Channel Estimates
	{
	  temp_in_ifft = (int*)malloc16(frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	  memset(temp_in_ifft,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
      
	  temp_in_fft_0 = (int*)malloc16(frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	  memset(temp_in_fft_0,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
      
	  temp_in_fft_1 = (int*)malloc16(frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	  memset(temp_in_fft_1,0,frame_parms->ofdm_symbol_size*sizeof(int*)*2);
	}
  



      //Extracting Channel Estimates for Distributed Alamouti Receiver Combining
      if((relay_flag == 2) && (diversity_scheme == 2))
	{
	  

	  re_offset = frame_parms->ofdm_symbol_size - frame_parms->N_RB_UL*12;
	  temp_in_ifft_ptr = &temp_in_ifft[0];
	
	  i = symbol_offset;
	 
	  for(j=0;j<(frame_parms->N_RB_UL*12);j++){
	    temp_in_ifft_ptr[j] = (int*)ul_ch_estimates[aa][i];
	    i++;
	  }


	  fft((short*) &temp_in_ifft[0],                          // Perfomring IFFT on Combined Channel Estimates
	      temp_out_ifft, 
	      frame_parms->twiddle_ifft,
	      frame_parms->rev,
	      (frame_parms->log2_symbol_size),
	      (frame_parms->log2_symbol_size)/2,
	      0);





	  in_fft_ptr_0 = &temp_in_fft_0[0];
	  temp_out_ifft_ptr = (int*)temp_out_ifft;
	 
	  for(j= 0;j<(1<<(frame_parms->log2_symbol_size))/2;j++)
	    {
	      in_fft_ptr_0[j] = temp_out_ifft_ptr[2*j];
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
	  

	  for(j=(1<<frame_parms->log2_symbol_size)/2;j<(1<<(frame_parms->log2_symbol_size));j++)
	    {
	      in_fft_ptr_1[j] = temp_out_ifft_ptr[2*j];
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
	    out_fft_ptr_1[i] = pow(-1,j+1)*temp_out_fft_1_ptr[2*j];
	    i++;
	  }


	  /*	  if(aa == 0){
	    write_output("test1.m","t1",temp_in_ifft,512,1,1);
	    write_output("test2.m","t2",temp_out_ifft,512*2,2,1);
	    write_output("test3.m","t3",temp_in_fft_0,512,1,1);  
	    write_output("test4.m","t4",temp_out_fft_0,512,1,1);
	    write_output("test5.m","t5",temp_in_fft_1,512,1,1);  
	    write_output("test6.m","t6",temp_out_fft_1,512,1,1);
	    }*/
	

	}//relay_flag ==2 && diversity_scheme == 2

      if (Ns&1) {//we are in the second slot of the sub-frame, so do the interpolation

	ul_ch1 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	ul_ch2 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos2];


	if((relay_flag == 2) && (diversity_scheme == 2))// For Distributed Alamouti
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
	 
	    if((relay_flag == 2) && (diversity_scheme == 2))// For Distributed Alamouti
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

	if((relay_flag == 2) && (diversity_scheme == 2))// For Distributed Alamouti
	  {
	    multadd_complex_vector_real_scalar((short*) ul_ch1_0,0x3FFF,(short*) ul_ch1_0,1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2_0,0x3FFF,(short*) ul_ch2_0,1,N_rb_alloc*12);

	    multadd_complex_vector_real_scalar((short*) ul_ch1_1,0x3FFF,(short*) ul_ch1_1,1,N_rb_alloc*12);
	    multadd_complex_vector_real_scalar((short*) ul_ch2_1,0x3FFF,(short*) ul_ch2_1,1,N_rb_alloc*12);
	  }


		/*if((relay_flag == 2) && (diversity_scheme == 2))// For Distributed Alamouti
		{
		write_output("drs_est.m","drsest",ul_ch_estimates[0],300*12,1,1);
		write_output("drs_est0.m","drsest0",ul_ch_estimates_0[0],300*12,1,1);
		write_output("drs_est1.m","drsest1",ul_ch_estimates_1[0],300*12,1,1);
		}*/


      } //if (Ns&1)

      // Freeing temporary pointers to Channel Estimates
      if((relay_flag == 2) && (diversity_scheme == 2))// For Distributed Alamouti
	{
	  free(temp_in_ifft);
	  free(temp_in_fft_0);
	  free(temp_in_fft_1);
	}

    } //for(aa=...
    
  } //if(l==...
  
  return(0);
}       

