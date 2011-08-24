#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"


#define fmax 5000
  
short *synch_tmp0; 

short *synch_tmp1[4];


short *synch_tmp_out[4];


void phy_synch_time_init() {

  int i;

  synch_tmp0 = (short*)malloc16(8*1024);

  //printk("[PHY][INIT][SYNC] synch_tmp0 = %p\n",(void *)synch_tmp0);

  for (i=0;i<4;i++) {

    
    synch_tmp1[i]    = (short *)malloc16(8*1024);
    synch_tmp_out[i] = (short *)malloc16(8*1024);

    /*
    printk("[PHY][INIT][SYNC] ind %d: synch_tmp1 = %p, sync_tmp_out = %p\n",i,
	(void *)synch_tmp1[i],
	(void *)synch_tmp_out[i]);
    */
  }
}

void phy_synch_time(short *in,
		    unsigned int *sync_pos,   // calculated position of sync 
		    unsigned int length,
		    unsigned int skip,
		    SCH_t synch_type,     // can be SCH or CHSCH
		    unsigned char synch_source) { // specifies the CHSCH/SCH index to be synced to

  int mag,max_lev,max_pos,aa;
  int mean_lev_int[64], max_lev_int[64], max_pos_int[64], peak_avg[64];
  int k, max_peak_avg;

  //  int k=0,k2=0,neg,pos,zero,re,im;

  //  int f_est;
  unsigned int i,j;
  //  unsigned int save_output;

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif //USER_MODE

  max_lev = 0;
  max_pos = 0;

  fft(&in[0],
      synch_tmp0,
      twiddle_fft_times4,
      rev_times4,
      10,
      6,
      0);
  
  //  msg("[PHY SYNC TIME] sync_type %s, sync_source %d\n",(synch_type==CHSCH ? "CHSCH" : "SCH") ,synch_source);


  k=0;
  for (i=0;i<(length<<1);i+=(skip<<1)) {

    if (k>=64) {
      msg("[PHY SYNC TIME] Error: allocate more memory to temporal variables!\n");
      return;
    }

    fft(&in[i],
	synch_tmp0,
	twiddle_fft_times4,
	rev_times4,
	10,
	6,
	0);
   
    
    for (aa=0;aa<4;aa++) {
      
      mult_cpx_vector(synch_tmp0,
		      (synch_type==CHSCH ? 
		       (s16 *)PHY_vars->chsch_data[synch_source].CHSCH_f_sync[aa] :
		       (s16 *)PHY_vars->sch_data[synch_source].SCH_f_sync[aa]),
		      synch_tmp_out[aa],
		      1024,
		      LOG2_CHSCH_RX_F_AMP);
      
    /*
      if (i==0) {
      fprintf(fid,"fftprod = [");
      for (k=0;k<4096;k+=4)
      fprintf(fid,"%d + (%d)*sqrt(-1),\n",tmp_out[k],tmp_out[k+1]);
      fprintf(fid,"];\n");
      }
    */
      
      fft(synch_tmp_out[aa],
	  synch_tmp1[aa],
	  twiddle_ifft_times4,
	  rev_times4,
	  10,5,
	  1);
      
    }

    //    save_output = 0;
    mean_lev_int[k] = 0;
    max_lev_int[k] = 0;
    max_pos_int[k] = 0;
    peak_avg[k] = 0;
    for (j=0;j<4*1024;j+=4) {
      mag = 0;
      for (aa=0;aa<4;aa++) {
	mag += synch_tmp1[aa][j]*synch_tmp1[aa][j] + 
	  synch_tmp1[aa][j+1]*synch_tmp1[aa][j+1];
      }
      mean_lev_int[k] += mag;

      if (mag > max_lev) {
	max_lev = mag;
	max_pos = (i>>1) + (j>>2);
	//	save_output = 1;
      }

      if (mag > max_lev_int[k]) {
	max_lev_int[k] = mag;
	max_pos_int[k] = (i>>1) + (j>>2);
	//	save_output = 1;
      }
    }

    mean_lev_int[k] = mean_lev_int[k]/1024;
    if (mean_lev_int[k]==0) {
      peak_avg[k] = max_lev_int[k];
    }
    else {
      peak_avg[k] = max_lev_int[k]/mean_lev_int[k];
    }

#ifdef DEBUG_PHY
    //msg("[OPENAIR][PHY][SYNCH] group %d, max_lev %d, max_pos %d\n",i,max_lev,max_pos);
    msg("[OPENAIR][PHY][SYNCH] group %d, mean_lev_int %d, max_lev_int %d, max_pos_int %d, peak_avg %d\n",k,mean_lev_int[k],max_lev_int[k],max_pos_int[k],peak_avg[k]);
#endif //DEBUG_PHY
 
#ifdef DEBUG_PHY
#ifdef USER_MODE    
    if (i==0) {//(save_output==1) {

      write_output("synch_input.m","sync_in",&in[i],2*1024,1,1);
      write_output("synch_inputF.m","sync_inF",&synch_tmp0[i],2*1024,1,1);
      write_output("synch_output_F.m","sync_outF",&synch_tmp_out[0][0],2*1024,2,1);
      write_output("synch_output.m","sync_out",
		   &synch_tmp1[0][0],
		   2*1024,2,1);
    }
#endif //USER_MODE
#endif //DEBUG_PHY   
    k++; 
  }

#ifdef DEBUG_PHY
    msg("[OPENAIR][PHY][SYNCH] max_pos (old algorithm) %d\n",max_pos);
#endif //DEBUG_PHY


  max_peak_avg = 0;
  for (i=0; i<k; i++) {
    if (peak_avg[i] > max_peak_avg) {
      max_pos = max_pos_int[i];
      max_peak_avg = peak_avg[i];
    }
  }

#ifdef DEBUG_PHY
    msg("[OPENAIR][PHY][SYNCH] max_pos (new algorithm) %d\n",max_pos);
#endif //DEBUG_PHY


  switch (synch_type) {
  case SCH: 
    // offset due to position of SCH 
    *sync_pos = max_pos - (SYMBOL_OFFSET_MRSCH)*(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);
    break;
  case CHSCH: 
    // offset due to position of CHSCH  
    *sync_pos = max_pos - PHY_config->PHY_chsch[synch_source].symbol*(OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES);     
    break;
  default:
    msg("[PHY SYNC TIME] Unknow synch_type %d\n",synch_type);
    *sync_pos = max_pos;
  }

  if ( sync_pos < 0 )
        sync_pos += FRAME_LENGTH_COMPLEX_SAMPLES;


  /*
  for (i=0;i<256;i++) {
    tmp[(i<<2)]   = in[(*sync_pos + i+32)<<1];
    tmp[1+(i<<2)] = in[1+((*sync_pos + i+32)<<1)];
    tmp[2+(i<<2)] = tmp[(i<<2)];
    tmp[3+(i<<2)] = tmp[1+(i<<2)];
    //    msg("tmp %d : %d %d \n",i,tmp[(i<<2)],tmp[1+(i<<2)]);
  }
  msg("[PHY SYNCH TIME] : mult_cpx_vector (pos)\n");

  mult_cpx_vector(tmp,
		  CHSCH_pos,
		  tmp_out,
		  256,
		  0);

  re = 0;
  im = 0;
  for (i=0;i<256;i++) {
    //    msg("pos %d : %d %d \n",i,tmp_out[(i<<2)],tmp_out[1+(i<<2)]);
    re += (int)(tmp_out[(i<<2)]);
    im += (int)(tmp_out[1+(i<<2)]);
    //    msg("re,im %d : %d %d \n",i,re,im);
  }

  re>>=2;
  im>>=2;
  pos = iSqrt(re*re + im*im);
  //  msg("pos:%d,%d -> %d\n",re,im,re*re + im*im);
    msg("[PHY SYNCH TIME] : mult_cpx_vector (neg)\n");
  
  mult_cpx_vector(tmp,
		  CHSCH_neg,
		  tmp_out,
		  256,
		  0);
  re = 0;
  im = 0;

  for (i=0;i<256;i++) {
    re += (int)(tmp_out[(i<<2)]);
    im += (int)(tmp_out[1+(i<<2)]);
  }
  re>>=2;
  im>>=2;
  neg = iSqrt(re*re + im*im);

  //  msg("neg:%d,%d -> %d\n",re,im,re*re + im*im);

  msg("[PHY SYNCH TIME] : mult_cpx_vector (zero)\n");
  mult_cpx_vector(tmp,
		  CHSCH_zero,
		  tmp_out,
		  256,
		  0);
  re = 0;
  im = 0;

  for (i=0;i<256;i++) {
    re += (int)(tmp_out[(i<<2)]);
    im += (int)(tmp_out[1+(i<<2)]);
  }
  //  msg("zero:%d,%d -> %d\n",re,im,re*re + im*im);

  re>>=2;
  im>>=2;
  zero = iSqrt(re*re + im*im);

  if (neg+pos-(2*zero) > 0)
    f_est = -(pos - neg)*fmax/((neg+pos-(2*zero))<<1);
  else
    f_est = -1;
  msg("Freq estimate = %d Hz (%d,%d,%d)\n",f_est,neg,zero,pos);
  */


}
