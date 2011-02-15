#include "defs.h"

#ifndef EXPRESSMIMO_TARGET
#include "mmintrin.h"
#endif //EXPRESSMIMO_TARGET

// Compute Energy of a complex signal vector, removing the DC component!
// input  : points to vector
// length : length of vector in complex samples

#define shift 4
#define shift_DC 0


#ifndef EXPRESSMIMO_TARGET
int signal_energy(int *input,unsigned int length) {

  int i;
  int temp,temp2;
  register __m64 mm0,mm1,mm2,mm3;
  __m64 *in = (__m64 *)input;

#ifdef MAIN
  short *printb;
#endif

  mm0 = _m_pxor(mm0,mm0);
  mm3 = _m_pxor(mm3,mm3);

  for (i=0;i<length>>1;i++) {
    
    mm1 = in[i]; 
    mm2 = mm1;
    mm1 = _m_pmaddwd(mm1,mm1);
    mm1 = _m_psradi(mm1,shift);
    mm0 = _m_paddd(mm0,mm1);
    //    temp2 = mm0;
    //    printf("%d %d\n",((int *)&temp2)[0],((int *)&temp2)[1]);


    //    printb = (short *)&mm2;
    //    printf("mm2 %d : %d %d %d %d\n",i,printb[0],printb[1],printb[2],printb[3]);

    mm2 = _m_psrawi(mm2,shift_DC);
    mm3 = _m_paddw(mm3,mm2);

    //    printb = (short *)&mm3;
    //    printf("mm3 %d : %d %d %d %d\n",i,printb[0],printb[1],printb[2],printb[3]);

  }

  /*
#ifdef MAIN
  printb = (short *)&mm3;
  printf("%d %d %d %d\n",printb[0],printb[1],printb[2],printb[3]);
#endif
  */
  mm1 = mm0;

  mm0 = _m_psrlqi(mm0,32);

  mm0 = _m_paddd(mm0,mm1);

  temp = _m_to_int(mm0);

  temp/=length;
  temp<<=shift;   // this is the average of x^2

  // now remove the DC component
  

  mm2 = _m_psrlqi(mm3,32);
  mm2 = _m_paddw(mm2,mm3);

  mm2 = _m_pmaddwd(mm2,mm2);

  temp2 = _m_to_int(mm2);

  temp2/=(length*length);

  temp2<<=(2*shift_DC);
#ifdef MAIN
  printf("E x^2 = %d\n",temp);  
#endif
  temp -= temp2;
#ifdef MAIN
  printf("(E x)^2=%d\n",temp2);
#endif
  _mm_empty();
  _m_empty();



  return((temp>0)?temp:1);
}

int signal_energy_nodc(int *input,unsigned int length) {

  int i;
  int temp,temp2;
  register __m64 mm0,mm1,mm2,mm3;
  __m64 *in = (__m64 *)input;

#ifdef MAIN
  short *printb;
#endif

  mm0 = _m_pxor(mm0,mm0);
  mm3 = _m_pxor(mm3,mm3);

  for (i=0;i<length>>1;i++) {
    
    mm1 = in[i]; 
    mm2 = mm1;
    mm1 = _m_pmaddwd(mm1,mm1);
    mm1 = _m_psradi(mm1,shift);
    mm0 = _m_paddd(mm0,mm1);
    //    temp2 = mm0;
    //    printf("%d %d\n",((int *)&in[i])[0],((int *)&in[i])[1]);


    //    printb = (short *)&mm2;
    //    printf("mm2 %d : %d %d %d %d\n",i,printb[0],printb[1],printb[2],printb[3]);


  }

  /*
#ifdef MAIN
  printb = (short *)&mm3;
  printf("%d %d %d %d\n",printb[0],printb[1],printb[2],printb[3]);
#endif
  */
  mm1 = mm0;

  mm0 = _m_psrlqi(mm0,32);

  mm0 = _m_paddd(mm0,mm1);

  temp = _m_to_int(mm0);

  temp/=length;
  temp<<=shift;   // this is the average of x^2

#ifdef MAIN
  printf("E x^2 = %d\n",temp);  
#endif
  _mm_empty();
  _m_empty();



  return((temp>0)?temp:1);
}

double signal_energy_fp(double **s_re,double **s_im,unsigned int nb_antennas,unsigned int length,unsigned int offset) {

  int aa,i;
  double V=0.0;

  for (i=0;i<length;i++) {
    for (aa=0;aa<nb_antennas;aa++) {
      V= V + (s_re[aa][i+offset]*s_re[aa][i+offset]) + (s_im[aa][i+offset]*s_im[aa][i+offset]); 
    }
  }
  return(V/length/nb_antennas);
}

double signal_energy_fp2(struct complex *s,unsigned int length) {

  int aa,i;
  double V=0.0;

  for (i=0;i<length;i++) {
      V= V + (s[i].y*s[i].x) + (s[i].y*s[i].x); 
    }
  return(V/length);
}
#else

int signal_energy(int *input,unsigned int length) {
}

#endif

#ifdef MAIN
#define LENGTH 256
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
main(int argc,char **argv) {

  int input[LENGTH];
  int energy=0,dc_r=0,dc_i=0;
  short s=1,i;
  int amp;

  amp = atoi(argv[1]);
  if (argc>1)
    printf("Amp = %d\n",amp);

  for (i=0;i<LENGTH;i++) {
    s = -s;
    ((short*)input)[2*i]     = 31 + (short)(amp*sin(2*M_PI*i/LENGTH));
    ((short*)input)[1+(2*i)] = 30 + (short)(amp*cos(2*M_PI*i/LENGTH));
    energy += (((short*)input)[2*i]*((short*)input)[2*i]) + (((short*)input)[1+(2*i)]*((short*)input)[1+(2*i)]);
    dc_r += ((short*)input)[2*i];
    dc_i += ((short*)input)[1+(2*i)];


  }
  energy/=LENGTH;
  dc_r/=LENGTH;
  dc_i/=LENGTH;

  printf("signal_energy = %d dB(%d,%d,%d,%d)\n",dB_fixed(signal_energy(input,LENGTH)),signal_energy(input,LENGTH),energy-(dc_r*dc_r+dc_i*dc_i),energy,(dc_r*dc_r+dc_i*dc_i));
  printf("dc = (%d,%d)\n",dc_r,dc_i);
}
#endif

