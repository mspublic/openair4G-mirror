#include <time.h>
#include "defs.h"
#include <stdint.h>
#include "STS_LTS_F.h"
#include "PHY/TOOLS/defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PHY/TOOLS/twiddle512.h"

#define CA_THRESHOLD 40

extern int16_t twiddle_fft64[63*4*2];
extern int16_t twiddle_ifft64[63*4*2];

uint16_t rev512[512];
extern uint16_t rev64[64];
int init_rev=0;

int rate2ind[16] = {0,0,0,0,0,0,0,0,6,4,2,0,7,5,3,1};

//#define DEBUG_SYNC 1

//#define EXECTIME 1

RX_VECTOR_t rxv;
uint16_t chest[128] __attribute__((aligned(16)));
extern int interleaver_bpsk[48];
int16_t signalF[512*2*2];
int16_t corrT[1024*2] __attribute__((aligned(16)));
int16_t corrF[1024*2] __attribute__((aligned(16)));
int16_t rxLTS_F[128*2] __attribute__((aligned(16)));
int16_t rxSIGNAL_F[128*2] __attribute__((aligned(16)));
uint32_t rxSIGNAL_F_comp[64*2] __attribute__((aligned(16)));
uint32_t rxSIGNAL_F_comp2[48] __attribute__((aligned(16)));
int8_t rxSIGNAL_llr[48] __attribute__((aligned(16)));


#ifdef EXECTIME
long is_t1=0,is_t2=0,is_t3=0,is_t4=0,is_t5,is_t6,is_t7,is_t8,is_t9;
int init_synch_trials=0;
int signal_trials=0;

void print_is_stats() {

  if (init_synch_trials>0)
    printf("Initial sync stats (STSLTS Corr) : Trials %d, is_t1 (512pt FFT) %d ns, is_t2 %d ns (STSLTS Corr), is_t3 %d ns (512pt IFFT), is_t4 %d ns (Peak Search/Detection)\n",init_synch_trials,
	   is_t1/init_synch_trials,is_t2/init_synch_trials,is_t3/init_synch_trials,is_t4/init_synch_trials);
  if (signal_trials>0)
    printf("Initial sync stats (SIGNAL)      : Trials %d, is_t5 (LTS FFT) %d ns, is_t6 (LTS Est.) %d ns, is_t7 (SIGNAL FFT) %d ns, is_t8 (SIGNAL Comp. + Deinter) %d ns, is_t9 (SIGNAL Viterbi) %d ns\n",signal_trials,
	   is_t5/signal_trials,is_t6/signal_trials,is_t7/signal_trials,is_t8/signal_trials,is_t9/signal_trials);
  init_synch_trials=0;
  signal_trials=0;
  is_t1=0;
  is_t2=0;
  is_t3=0;
  is_t4=0;
  is_t5=0;
  is_t6=0;
  is_t7=0;
  is_t8=0;
  is_t9=0;
}
#endif

CHANNEL_STATUS_t initial_sync(RX_VECTOR_t **rx_vector,int *rx_offset,uint32_t *rx_frame,int rx_frame_length,int one_shot) {

  int32_t energy,peak_energy,mean_energy;
  int n,i,i2,j,j2,k,peak_pos,found_sync=0,LTS2_pos,SIGNAL_pos,offset;
  uint32_t pilot1,pilot2,pilot3,pilot4;
  int16_t tmp;
  int pos;
  uint8_t signal_sdu[3] __attribute__((aligned(16)));

  int signal_parity;
#ifdef EXECTIME
  struct timespec tin,tout;
#endif
  int ret;
  //  int32_t m[640];
  //  int16_t re,im;

  if (init_rev == 0)
    init_fft(512,9,rev512);

  // Check for start of PLCP in buffer (scan over 640 sample span)

  peak_energy = 0;
  peak_pos = 0;
  //  m[0] = 0;

  //  for (n=0;n<640;n+=160) {
  //    for (j=n;j<n+160;j++) {
  //      re = ((int16_t *)(rx_frame+rx_frame_pos+j))[0];
  //      im = ((int16_t *)(rx_frame+rx_frame_pos+j))[1];
  //      m[j+1] = ((3*m[j])>>2) + (((int32_t)re*re + (int32_t)im*im)>>2);
#ifdef DEBUG_SYNC
  //      printf("j %d (%d) : m[j] %d,m[j-10] %d, re %d, im %d, abs %d\n",j,rx_frame_pos,m[j],m[j-10],re,im,(int32_t)re*re + (int32_t)im*im);
#endif
  //      if ((j>20) && (m[j] > (m[j-10]<<3))) {
  n = 0;
  while (1) {
    //    is_wait(n);

#ifdef EXECTIME
    ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif
	// ensure 64-bit alignment on FFT input
    //    if ((j&1) == 0)
    //      offset=6;
    //	else
    //	  offset=5;

    fft((int16_t *)(rx_frame+n),         /// complex input
	signalF,           /// complex output
	&twiddle_fft512[0],  /// complex twiddle factors
	rev512,           /// bit reversed permutation vector
	9,               /// log2(FFT_SIZE)
	4,               /// scale (energy normalized for 64-point)
	0);              /// 0 means 64-bit complex interleaved format else complex-multiply ready repeated format
#ifdef EXECTIME
    ret=clock_gettime(CLOCK_REALTIME,&tout);
    is_t1 += (tout.tv_nsec-tin.tv_nsec);
    
    ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif
    mult_cpx_vector(signalF,STS_LTS_F,corrF,512,15);
    
#ifdef EXECTIME
    ret=clock_gettime(CLOCK_REALTIME,&tout);
    is_t2 += (tout.tv_nsec-tin.tv_nsec);
    
    
    ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif
    fft(corrF,        /// complex input
	corrT,          /// complex output
	&twiddle_ifft512[0],  /// complex twiddle factors
	rev512,         /// bit reversed permutation vector
	9,              /// log2(FFT_SIZE)
	5,               /// scale (energy normalized for 64-point)
	1);              /// 0 means 64-bit complex interleaved format else complex-multiply ready repeated format
    
#ifdef EXECTIME
    ret=clock_gettime(CLOCK_REALTIME,&tout);
    is_t3 += (tout.tv_nsec-tin.tv_nsec);
    
    
    ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif
    
    // look for peak and compare to average output
    mean_energy = 0;
    peak_energy = 0;
    for (i2=0,j2=0;i2<512;i2++,j2+=4) {
      energy = corrT[j2]*corrT[j2] + corrT[j2+1]*corrT[j2+1];
      if (energy > peak_energy) {
	peak_energy = energy;
	peak_pos = n+i2;
	if ((peak_pos&1)==1)
	  peak_pos--; // ensure peak_pos is 64-bit aligned
      }
      mean_energy += energy;
    }
    
    mean_energy -= peak_energy;
    mean_energy>>=9;
#ifdef DEBUG_SYNC
    printf("n %d: mean energy %d/%d dB, peak_energy %d/%d dB, pos %d\n",n,mean_energy,dB_fixed(mean_energy),peak_energy,dB_fixed(peak_energy),peak_pos);
    //    for (j2=n;j2<=j;j2++)
    //      printf("m[%d] %d\n",j2,m[j2]);
#endif
    
#ifdef EXECTIME
    ret=clock_gettime(CLOCK_REALTIME,&tout);
    is_t4+=(tout.tv_nsec-tin.tv_nsec);
    init_synch_trials++;
#endif
    if (peak_energy>(80*mean_energy)) {
#ifdef DEBUG_SYNC
      write_output("STSLTScorr.m","STSLTScorrT", corrT,512,2,1);
#endif
      //    *rx_vector = &rxv;
      *rx_vector = NULL;
      
      // now try to estimate channel and decode SIGNAL field
      LTS2_pos = peak_pos + 240;
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif
      fft((int16_t *)(rx_frame+16+LTS2_pos),         /// complex input
	  rxLTS_F,           /// complex output
	  &twiddle_fft64[0],  /// complex twiddle factors
	  rev64,           /// bit reversed permutation vector
	  6,               /// log2(FFT_SIZE)
	  3,               /// scale (energy normalized for 64-point)
	  0);              /// 0 means 64-bit complex interleaved format else complex-multiply ready repeated format
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tout);
      is_t5+=(tout.tv_nsec-tin.tv_nsec);
      ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif    
      mult_cpx_vector(rxLTS_F,LTS_F,(int16_t*)chest,64,0);
      
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tout);
      is_t6+=(tout.tv_nsec-tin.tv_nsec);
#endif    
      
#ifdef DEBUG_SYNC
      write_output("rxLTS.m","rLTS", rx_frame+16+LTS2_pos,64,1,1);
      write_output("rxLTS_F.m","rLTS_F", rxLTS_F,256,1,1);
      write_output("chest.m","ch", chest,128,1,1);
#endif
      
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif      
      SIGNAL_pos = LTS2_pos + 80;
      fft((int16_t *)(rx_frame+16+SIGNAL_pos),         /// complex input
	  rxSIGNAL_F,           /// complex output
	  &twiddle_fft64[0],  /// complex twiddle factors
	  rev64,           /// bit reversed permutation vector
	  6,               /// log2(FFT_SIZE)
	  3,               /// scale (energy normalized for 64-point)
	  0);              /// 0 means 64-bit complex interleaved format else complex-multiply ready repeated format
      
      mult_cpx_vector_norep_unprepared_conjx2(rxSIGNAL_F,(int16_t*)chest,(int16_t*)rxSIGNAL_F_comp,64,10);
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tout);
      is_t7+=(tout.tv_nsec-tin.tv_nsec);
      ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif         
      // extract 48 statistics and 4 pilot symbols 
      // -ve portion
      for (i=0;i<5;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(38+i)];
      pilot1 = rxSIGNAL_F_comp[(38+5)];
      
      for (;i<18;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(38+1+i)];
      pilot2 = rxSIGNAL_F_comp[(38+19)];
      
      for (;i<24;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(38+2+i)];
      
      // +ve portion
      for (;i<30;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(-24+1+i)];
      pilot3 = rxSIGNAL_F_comp[(6+1)];
      for (;i<43;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(-24+2+i)];
      pilot4 = rxSIGNAL_F_comp[(19+2)];((int16_t *)&pilot4)[0]=-((int16_t *)&pilot4)[0];((int16_t *)&pilot4)[1]=((int16_t *)&pilot4)[1];
      for (;i<48;i++)
	rxSIGNAL_F_comp2[i] = rxSIGNAL_F_comp[(-24+3+i)];
      
#ifdef DEBUG_SYNC
      write_output("rxSIGNAL_F.m","rxSIG_F", rxSIGNAL_F,128,2,1);
      write_output("rxSIGNAL_F_comp.m","rxSIG_F_comp", rxSIGNAL_F_comp,64,1,1);
      write_output("rxSIGNAL_F_comp2.m","rxSIG_F_comp2", rxSIGNAL_F_comp2,48,1,1);
#endif
      
      // CFO compensation
      
      // now deinterleave SIGNAL
      for (k=0;k<48;k++) {
	//      printf("interleaver_bpsk[%d] = %d\n",k,interleaver_bpsk[k]);
	tmp = ((int16_t*)rxSIGNAL_F_comp2)[k<<1]>>4;
	pos = interleaver_bpsk[k];
	if (tmp<-8)
	  rxSIGNAL_llr[pos] = -8;
	else if (tmp>7)
	  rxSIGNAL_llr[pos] = 7;
	else
	  rxSIGNAL_llr[pos] = (int8_t)tmp;
	//      rxSIGNAL_llr[k] = (tmp > 7) ? 7 : ((tmp < -8) ? -8 : (int8_t)tmp);
      }
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tout);
      is_t8+=(tout.tv_nsec-tin.tv_nsec);
      ret=clock_gettime(CLOCK_REALTIME,&tin);
#endif         
      // Viterbi decoding
      signal_sdu[0]=0;
      signal_sdu[1]=0;
      signal_sdu[2]=0;
      
      phy_viterbi_dot11_sse2(rxSIGNAL_llr,signal_sdu,24,0,1);
      
      // check parity and reconstruct RX_VECTOR
      signal_parity = 0;
      for (i=0;i<8;i++) {
	signal_parity += ((signal_sdu[0]&(1<<i))>0)? 1 : 0;
	signal_parity += ((signal_sdu[1]&(1<<i))>0)? 1 : 0;
	signal_parity += ((signal_sdu[2]&(1<<i))>0)? 1 : 0;
      }
      
      if ((signal_parity&1) == 0) {
	rxv.rate = rate2ind[signal_sdu[0]&0xF];
	rxv.sdu_length =  signal_sdu[0]    >> 5;
	rxv.sdu_length += signal_sdu[1]    << 3;
	rxv.sdu_length += (signal_sdu[2]&1) << 11;
	*rx_vector = &rxv;
	*rx_offset = peak_pos+400;
      }
#ifdef EXECTIME
      ret=clock_gettime(CLOCK_REALTIME,&tout);
      is_t9+=(tout.tv_nsec-tin.tv_nsec);
      signal_trials++;
#endif   
#ifdef DEBUG_SYNC
      printf("Decoded RX_VECTOR: %x,%x,%x\n",
	     //	   ((uint8_t *)&rxv)[0],((uint8_t *)&rxv)[1],((uint8_t *)&rxv)[2]);
	     signal_sdu[0],signal_sdu[1],signal_sdu[2]);
      
      write_output("rxSIGNAL_llr.m","rxSIG_llr", rxSIGNAL_llr,48,1,4);
#endif
      
      return(BUSY);
    }
    n+=160;
    if (n>=rx_frame_length) {
      n=0;
      if (one_shot == 1)
	return(IDLE);
    }

  }
  // shouldn' get here
  assert("Exited while(1)\n");
  return(IDLE);
}
