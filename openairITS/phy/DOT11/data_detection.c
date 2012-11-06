#include "defs.h"
#include <stdint.h>
#include "PHY/TOOLS/defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int16_t twiddle_fft64[63*4*2];
extern int16_t twiddle_ifft64[63*4*2];

extern uint16_t rev64[64];
extern int Ndbps[8];
extern int Ncbps[8];

//#define DEBUG_DATA 1

extern uint16_t chest[128] __attribute__((aligned(16)));
extern int interleaver_bpsk[48];
extern int interleaver_qpsk[48];
extern int interleaver_16qam[48];
extern int interleaver_64qam[48];

extern uint8_t scrambler[127*8];

int data_detection(RX_VECTOR_t *rxv,uint8_t *data_ind,uint32_t* rx_data,int frame_length,int rx_offset,int (*wait(int,int))) {

  int16_t rxDATA_F[128*2] __attribute__((aligned(16)));
  uint32_t rxDATA_F_comp[64*2] __attribute__((aligned(16)));
  uint32_t rxDATA_F_comp2[48] __attribute__((aligned(16)));
  int8_t rxDATA_llr[384] __attribute__((aligned(16)));
  int8_t rxDATA_llr2[432] __attribute__((aligned(16)));
  int8_t *llr_ptr;
  uint32_t pilot1,pilot2,pilot3,pilot4;
  int i,j,k,k2,tmp,pos;
  int *interleaver;
  int dlen,dlen_symb;
  int s,sprime;
  uint32_t crc_rx;
  char fname[20],vname[20];
  // loop over all symbols
  dlen      = 32+16+6+(rxv->sdu_length<<3); // data length is 32-bits CRC + sdu + 16 service + 6 tail
  dlen_symb = dlen/Ndbps[rxv->rate];
  if ((dlen%Ndbps[rxv->rate])>0)
    dlen_symb++;

  for (s=0,sprime=1;s<dlen_symb;s++,sprime++,rx_offset+=80) {

    //    printf("DATA symbol %d, rx_offset %d\n",s,rx_offset);
    // synchronize to HW if needed
    if (wait)
      *wait(rx_offset,s);

    if (rx_offset>frame_length)
      rx_offset -= frame_length;
    // index for pilot symbol lookup
    if (sprime==127)
      sprime=0;
    
    if ((rx_offset+80) > frame_length)
      memcpy((void *)(rx_data+frame_length),
	     (void *)rx_data,
	     2*sizeof(int16_t)*(rx_offset+80-frame_length));
    
    
    fft((int16_t *)(rx_data+rx_offset+16),         /// complex input
	rxDATA_F,           /// complex output
	&twiddle_fft64[0],  /// complex twiddle factors
	rev64,           /// bit reversed permutation vector
	6,               /// log2(FFT_SIZE)
	3,               /// scale (energy normalized for 64-point)
	0);              /// 0 means 64-bit complex interleaved format else complex-multiply ready repeated format
    
    mult_cpx_vector_norep_unprepared_conjx2(rxDATA_F,(int16_t*)chest,(int16_t*)rxDATA_F_comp,64,10);
    
    // extract 48 statistics and 4 pilot symbols 
    // -ve portion
    for (i=0;i<5;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(38+i)];
    pilot1 = rxDATA_F_comp[(38+5)];
    
    for (;i<18;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(38+1+i)];
    pilot2 = rxDATA_F_comp[(38+19)];
    
    for (;i<24;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(38+2+i)];
    
    // +ve portion
    for (;i<30;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(-24+1+i)];
    pilot3 = rxDATA_F_comp[(6+1)];
    for (;i<43;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(-24+2+i)];
    pilot4 = rxDATA_F_comp[(19+2)];((int16_t *)&pilot4)[0]=-((int16_t *)&pilot4)[0];((int16_t *)&pilot4)[1]=((int16_t *)&pilot4)[1];
    for (;i<48;i++)
      rxDATA_F_comp2[i] = rxDATA_F_comp[(-24+3+i)];
    
#ifdef DEBUG_DATA
    write_output("rxDATA_F.m","rxDAT_F", rxDATA_F,128,2,1);
    write_output("rxDATA_F_comp.m","rxDAT_F_comp", rxDATA_F_comp,64,1,1);
    sprintf(fname,"rxDATA_F_comp2_%d.m",s);
    sprintf(vname,"rxDAT_F_comp2_%d",s);
    write_output(fname,vname, rxDATA_F_comp2,48,1,1);
#endif


    // CFO compensation

    // LLR Computation

    switch (rxv->rate>>1) {
    case 0: // BPSK
      llr_ptr = rxDATA_llr;
      memset(rxDATA_llr,0,48);
      for (k=0;k<48;k++) {
	pos = interleaver_bpsk[k];
	tmp = ((int16_t*)rxDATA_F_comp2)[pos<<1]>>4;
	if (tmp<-8)
	  rxDATA_llr[k] = -8;
	else if (tmp>7)
	  rxDATA_llr[k] = 7;
	else
	  rxDATA_llr[k] = (int8_t)tmp;

	//	printf("rxDATA_coded %d(%d) : %d\n",k,pos,rxDATA_llr[k]);
      }
      if (rxv->rate==1) { // rate 3/4, so add zeros for punctured bits
	llr_ptr = rxDATA_llr2;
	memset(rxDATA_llr2,0,72);
	for (k=0,k2=0;k<48;k++,k2++) {
	  rxDATA_llr2[k2] = rxDATA_llr[k];
	  if ((k&3) == 2)
	    k2+=2;
	}
      }
      break;
    case 1:  // QPSK
      llr_ptr = rxDATA_llr;
      memset(rxDATA_llr,0,96);
      for (k=0;k<96;k++) {
	pos = interleaver_qpsk[k];
	tmp = ((int16_t*)rxDATA_F_comp2)[pos]>>4;

	if (tmp<-8)
	  rxDATA_llr[k] = -8;
	else if (tmp>7)
	  rxDATA_llr[k] = 7;
	else
	  rxDATA_llr[k] = (int8_t)tmp;
      }
      if (rxv->rate==3) { // rate 3/4, so add zeros for punctured bits
	llr_ptr = rxDATA_llr2;
	memset(rxDATA_llr2,0,144);
	for (k=0,k2=0;k<96;k++,k2++) {
	  rxDATA_llr2[k2] = rxDATA_llr[k];
	  if ((k&3) == 2)
	    k2+=2;
	}
      }
      break;
    case 2:  // 16QAM
      return(0==1);
      break;
    case 3:  // 64QAM
      return(0==1);
      break;
    }
    /*    
    printf("LLRs:"); 
    for (i=0;i<48;i++)
      printf("%d,",llr_ptr[i]);

      printf("viterbi s %d/%d\n",s,dlen_symb-1);
    */
    if (s < (dlen_symb-1))
      phy_viterbi_dot11_sse2(llr_ptr,data_ind,Ndbps[rxv->rate],s*Ndbps[rxv->rate],0);
    else {
      phy_viterbi_dot11_sse2(llr_ptr,data_ind,Ndbps[rxv->rate],s*Ndbps[rxv->rate],1);
      
      // scramble data
      //      printf("DATA %x.%x.",data_ind[0],data_ind[1]);
      data_ind[0] ^= scrambler[0]; // service byte 0
      data_ind[1] ^= scrambler[1]; // service byte 1

      j=2;
      for (i=0;i<(rxv->sdu_length+4);i++,j++) {  // sdu+crc
	//	printf("%x.",data_ind[i+2]);
	if (j==(127*8))
	  j=0;
	data_ind[i+2]^=scrambler[j];

      }
      //      printf("\n");

      crc_rx = 0xffffffff;
      crc32(data_ind,&crc_rx,2+rxv->sdu_length);
#ifdef DEBUG_DATA
      printf("Received CRC %x.%x.%x.%x (%x), computed %x\n",
	     data_ind[2+rxv->sdu_length],
	     data_ind[2+rxv->sdu_length+1],
	     data_ind[2+rxv->sdu_length+2],
	     data_ind[2+rxv->sdu_length+3],
	     *(uint32_t*)&data_ind[2+rxv->sdu_length],
	     crc_rx);

      printf("SDU length %d\n",rxv->sdu_length);
      for (i=0;i<rxv->sdu_length;i++) {
	if ((i&15) == 0)
	  printf("\n %04x :  %02x",i,data_ind[2+i]);
	else
	  printf(".%02x",data_ind[2+i]);
      }
      printf("\n");
#endif
    }
  }

  return(*(uint32_t*)&data_ind[2+rxv->sdu_length] == crc_rx);

}
