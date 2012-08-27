// 6.10.2.2 MBSFN reference signals Mapping to resource elements 

#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#endif

#include "defs.h"
#include "PHY/defs.h"

//extern unsigned int lte_gold_table[10][3][42];
#define DEBUG_DL_MBSFN

int lte_dl_mbsfn(PHY_VARS_eNB *phy_vars_eNB, mod_sym_t *output,
		 short amp,
		 unsigned char Ns,
		 unsigned char l,
		 unsigned char p) {

  unsigned char mprime,mprime_dword,mprime_qpsk_symb,m;
  unsigned short k,a;
  mod_sym_t qpsk[4];

#ifdef IFFT_FPGA
  // new mod table
  qpsk[0] = 1;
  qpsk[1] = 3;
  qpsk[2] = 2;
  qpsk[3] = 4;

#else
  a = (amp*ONE_OVER_SQRT2_Q15)>>15;
  ((short *)&qpsk[0])[0] = a;
  ((short *)&qpsk[0])[1] = a;
  
  ((short *)&qpsk[1])[0] = -a;
  ((short *)&qpsk[1])[1] = a;
  ((short *)&qpsk[2])[0] = a;
  ((short *)&qpsk[2])[1] = -a;

  ((short *)&qpsk[3])[0] = -a;
  ((short *)&qpsk[3])[1] = -a;
  
#endif

  mprime = 3*(110 - phy_vars_eNB->lte_frame_parms.N_RB_DL);

  for (m=0; m<phy_vars_eNB->lte_frame_parms.N_RB_DL*6; m++) {	

    if ((Ns % 2 == 0)  && (l!=0)) 
      
      k = m*2;
    
    else if ((Ns % 2 == 1)  && (l==0))  
      
      k = (m*2)+1;
    
    else {
      msg("lte_dl_mbsfn: l %d -> ERROR\n",l);
      return(-1);
    }

#ifdef IFFT_FPGA
    k+=phy_vars_eNB->lte_frame_parms.N_RB_DL*6;
#else  
    k+=phy_vars_eNB->lte_frame_parms.first_carrier_offset;
#endif   

#ifdef IFFT_FPGA
    if (k >= phy_vars_eNB->lte_frame_parms.N_RB_DL*12) {
      k-=phy_vars_eNB->lte_frame_parms.N_RB_DL*12;
    }
#else
    if (k >= phy_vars_eNB->lte_frame_parms.ofdm_symbol_size) {
      k++;  // skip DC carrier
      k-=phy_vars_eNB->lte_frame_parms.ofdm_symbol_size;
    }
#endif
 

    
    mprime_dword     = mprime>>4;
    mprime_qpsk_symb = mprime&0xf;   
    

  
  
    
    output[k] = qpsk[(phy_vars_eNB->lte_gold_mbsfn_table[Ns][l][mprime_dword]>>(2*mprime_qpsk_symb))&3];
    //output[k] = (lte_gold_table[eNB_offset][Ns][l][mprime_dword]>>(2*mprime_qpsk_symb))&3;
    
    
#ifdef DEBUG_DL_MBSFN
    msg("Ns %d, l %d, m %d,mprime_dword %d, mprime_qpsk_symbol %d\n",
	Ns,l,m,mprime_dword,mprime_qpsk_symb);
    msg("index = %d (k %d)\n",(phy_vars_eNB->lte_gold_mbsfn_table[Ns][l][mprime_dword]>>(2*mprime_qpsk_symb))&3,k);
#endif     
    mprime++;
    
#ifdef DEBUG_DL_MBSFN
    if (m<4)
      printf("Ns %d, l %d output[%d] = (%d,%d)\n",Ns,l,k,((short *)&output[k])[0],((short *)&output[k])[1]);
#endif



    //    printf("** k %d\n",k);
  }
  return(0);
}



int lte_dl_mbsfn_rx(PHY_VARS_UE *phy_vars_ue, u8 eNB_offset,
		    int *output,
		    unsigned char Ns,
		    unsigned char l,
		    unsigned char p) 
	
{
  
  unsigned char mprime,mprime_dword,mprime_qpsk_symb,m;
  unsigned short k=0;
  unsigned int qpsk[4];

  // This includes complex conjugate for channel estimation

  ((short *)&qpsk[0])[0] = ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[0])[1] = -ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[1])[0] = -ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[1])[1] = -ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[2])[0] = ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[2])[1] = ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[3])[0] = -ONE_OVER_SQRT2_Q15;
  ((short *)&qpsk[3])[1] = ONE_OVER_SQRT2_Q15;

  mprime = 3*(110 - phy_vars_ue->lte_frame_parms.N_RB_DL);
  
  for (m=0;m<phy_vars_ue->lte_frame_parms.N_RB_DL*6;m++) {

    mprime_dword     = mprime>>4;
    mprime_qpsk_symb = mprime&0xf;

    // this is r_mprime from 3GPP 36-211 6.10.1.2 
    output[k] = qpsk[(phy_vars_ue->lte_gold_mbsfn_table[Ns][l][mprime_dword]>>(2*mprime_qpsk_symb))&3];
	
#ifdef DEBUG_DL_MBSFN
    printf("Ns %d, l %d, m %d,mprime_dword %d, mprime_qpsk_symbol %d\n",
	   Ns,l,m,mprime_dword,mprime_qpsk_symb);
    printf("index = %d (k %d)\n",(phy_vars_ue->lte_gold_mbsfn_table[Ns][l][mprime_dword]>>(2*mprime_qpsk_symb))&3,k);
#endif 

    mprime++;
#ifdef DEBUG_DL_MBSFN
    if (m<4)
      printf("Ns %d l %d output[%d] = (%d,%d)\n",Ns,l,k,((short *)&output[k])[0],((short *)&output[k])[1]);
#endif
    k++;
    //    printf("** k %d\n",k);
  }
  return(0);
}

