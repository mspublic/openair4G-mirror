#include <math.h>
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "PHY/TOOLS/defs.h"
#include "defs.h"

void freq_channel(channel_desc_t *desc,u16 nb_rb) {

  double delta_f = 90e3,freq;  // 90 kHz spacing
  s16 f;
  u8 aarx,aatx,l;

  for (f=-nb_rb;f<nb_rb;f++) {
    freq=delta_f*(double)f*1e-6;


      for (aarx=0;aarx<desc->nb_rx;aarx++) {
	for (aatx=0;aatx<desc->nb_tx;aatx++) {
	  desc->chF[aarx+(aatx*desc->nb_rx)][nb_rb+f].r=0.0;
	  desc->chF[aarx+(aatx*desc->nb_rx)][nb_rb+f].i=0.0;
	  for (l=0;l<(int)desc->nb_taps;l++) {
	    desc->chF[aarx+(aatx*desc->nb_rx)][f+nb_rb].r+=(desc->a[l][aarx+(aatx*desc->nb_rx)].r*cos(2*M_PI*freq*desc->delays[l])+
						      desc->a[l][aarx+(aatx*desc->nb_rx)].i*sin(2*M_PI*freq*desc->delays[l]));
	    desc->chF[aarx+(aatx*desc->nb_rx)][f+nb_rb].i+=(-desc->a[l][aarx+(aatx*desc->nb_rx)].r*sin(2*M_PI*freq*desc->delays[l])+
						      desc->a[l][aarx+(aatx*desc->nb_rx)].i*cos(2*M_PI*freq*desc->delays[l]));
	  }
	  //	  	  	  printf("chF(%f) => (%f,%f)\n",freq,desc->chF[aarx+(aatx*desc->nb_rx)][f].r,desc->chF[aarx+(aatx*desc->nb_rx)][f].i);
	}
      }
  }
}

double compute_pbch_sinr(channel_desc_t *desc,
			 channel_desc_t *desc_i1, 
			 channel_desc_t *desc_i2,
			 double snr_dB,double snr_i1_dB,
			 double snr_i2_dB,
			 u16 nb_rb) {

  double avg_sinr,snr=pow(10.0,.1*snr_dB),snr_i1=pow(10.0,.1*snr_i1_dB),snr_i2=pow(10.0,.1*snr_i2_dB);
  u16 f;
  u8 aarx,aatx;
  double S;
  struct complex S_i1;
  struct complex S_i2;

  avg_sinr=0.0;
  //  printf("nb_rb %d\n",nb_rb);
  for (f=(nb_rb-6);f<(nb_rb+6);f++) {
    S = 0.0;
    S_i1.r =0.0;
    S_i1.i =0.0;
    S_i2.r =0.0;
    S_i2.i =0.0;
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
	S    += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc->chF[aarx+(aatx*desc->nb_rx)][f].r + 
	         desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc->chF[aarx+(aatx*desc->nb_rx)][f].i);
	//	printf("%d %d chF[%d] => (%f,%f)\n",aarx,aatx,f,desc->chF[aarx+(aatx*desc->nb_rx)][f].r,desc->chF[aarx+(aatx*desc->nb_rx)][f].i);
	       
	if (desc_i1) {
	  S_i1.r += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].r + 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].i);
	  S_i1.i += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].i - 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].r);
	}
	if (desc_i2) {
	  S_i2.r += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].r + 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].i);
	  S_i2.i += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].i - 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].r);
	}
      }
    } 
    //    printf("snr %f f %d : S %f, S_i1 %f, S_i2 %f\n",snr,f-nb_rb,S,snr_i1*sqrt(S_i1.r*S_i1.r + S_i1.i*S_i1.i),snr_i2*sqrt(S_i2.r*S_i2.r + S_i2.i*S_i2.i));
    avg_sinr += (snr*S/(desc->nb_tx+snr_i1*sqrt(S_i1.r*S_i1.r + S_i1.i*S_i1.i)+snr_i2*sqrt(S_i2.r*S_i2.r + S_i2.i*S_i2.i)));
  }
  //  printf("avg_sinr %f (%f,%f,%f)\n",avg_sinr/12.0,snr,snr_i1,snr_i2);
  return(10*log10(avg_sinr/12.0));
}
 

double compute_sinr(channel_desc_t *desc,
		    channel_desc_t *desc_i1, 
		    channel_desc_t *desc_i2,
		    double snr_dB,double snr_i1_dB,
		    double snr_i2_dB,
		    u16 nb_rb) {

  double avg_sinr,snr=pow(10.0,.1*snr_dB),snr_i1=pow(10.0,.1*snr_i1_dB),snr_i2=pow(10.0,.1*snr_i2_dB);
  u16 f;
  u8 aarx,aatx;
  double S;
  struct complex S_i1;
  struct complex S_i2;

  avg_sinr=0.0;
  //  printf("nb_rb %d\n",nb_rb);
  for (f=0;f<2*nb_rb;f++) {
    S = 0.0;
    S_i1.r =0.0;
    S_i1.i =0.0;
    S_i2.r =0.0;
    S_i2.i =0.0;
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
	S    += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc->chF[aarx+(aatx*desc->nb_rx)][f].r + 
	         desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc->chF[aarx+(aatx*desc->nb_rx)][f].i);
	if (desc_i1) {
	  S_i1.r += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].r + 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].i);
	  S_i1.i += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].i - 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i1->chF[aarx+(aatx*desc->nb_rx)][f].r);
	}
	if (desc_i2) {
	  S_i2.r += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].r + 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].i);
	  S_i2.i += (desc->chF[aarx+(aatx*desc->nb_rx)][f].r*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].i - 
		     desc->chF[aarx+(aatx*desc->nb_rx)][f].i*desc_i2->chF[aarx+(aatx*desc->nb_rx)][f].r);
	}
      }
    }
    //        printf("f %d : S %f, S_i1 %f, S_i2 %f\n",f-nb_rb,snr*S,snr_i1*sqrt(S_i1.r*S_i1.r + S_i1.i*S_i1.i),snr_i2*sqrt(S_i2.r*S_i2.r + S_i2.i*S_i2.i));
    avg_sinr += (snr*S/(desc->nb_tx+snr_i1*sqrt(S_i1.r*S_i1.r + S_i1.i*S_i1.i)+snr_i2*sqrt(S_i2.r*S_i2.r + S_i2.i*S_i2.i)));
  }
  //  printf("avg_sinr %f (%f,%f,%f)\n",avg_sinr/12.0,snr,snr_i1,snr_i2);
  return(10*log10(avg_sinr/(nb_rb*2)));
}

u8 pbch_polynomial_degree;
double a[6];

void load_pbch_desc(FILE *pbch_file_fd) {

  int i;
  char dummy[25];

  fscanf(pbch_file_fd,"%d",&pbch_polynomial_degree);
  if (pbch_polynomial_degree>5) {
    printf("Illegal degree for pbch interpolation polynomial\n",pbch_polynomial_degree);
    exit(-1);
  }

  printf("PBCH polynomial : ");

  for (i=0;i<=pbch_polynomial_degree;i++) {
    fscanf(pbch_file_fd,"%s",dummy);
    a[i] = strtod(dummy,NULL);
    printf("%f ",a[i]);
  }
  printf("\n");
} 

double pbch_bler(double sinr) {

  int i;
  double log10_bler=a[pbch_polynomial_degree];
  double sinrpow=sinr;
  //  printf("log10bler %f\n",log10_bler);
  if (sinr<-7.9)
    return(1.0);
  else if (sinr>=0.0)
    return(1e-4);

  for (i=1;i<=pbch_polynomial_degree;i++) {
    //    printf("sinrpow %f\n",sinrpow);
    log10_bler += (a[pbch_polynomial_degree-i]*sinrpow);
    sinrpow *= sinr;
    //    printf("log10bler %f\n",log10_bler);
  }
  return(pow(10.0,log10_bler));
}
