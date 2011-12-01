/* file: pss.c
   purpose: generate the primary synchronization signals of LTE
   author: florian.kaltenberger@eurecom.fr, oscar.tonelli@yahoo.it
   date: 21.10.2009 
*/

//#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

int generate_pss(mod_sym_t **txdataF,
		 short amp,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 unsigned short symbol,
		 unsigned short slot_offset) {

  unsigned int Nsymb;
  unsigned short k,m,aa;
  u8 Nid2;
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
  unsigned char *primary_sync_tab;
#else
  short *primary_sync;
#endif
#else
  short *primary_sync;
#endif

  Nid2 = frame_parms->Nid_cell % 3;
  debug_msg("[PHY][PSS] Using sequence %d\n",Nid2);

  switch (Nid2) {
  case 0:
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
    primary_sync_tab = primary_synch0_tab;
#else
    primary_sync = primary_synch0;
#endif
#else
    primary_sync = primary_synch0;
#endif
    break;
  case 1:
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
    primary_sync_tab = primary_synch1_tab;
#else
    primary_sync = primary_synch1;
#endif
#else
    primary_sync = primary_synch1;
#endif
    break;
  case 2:
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
    primary_sync_tab = primary_synch2_tab;
#else
    primary_sync = primary_synch2;
#endif
#else
    primary_sync = primary_synch2;
#endif
    break;
  default:
    msg("[PSS] eNb_id has to be 0,1,2\n");
    return(-1);
  }

  //a = (amp*ONE_OVER_SQRT2_Q15)>>15;
  //printf("[PSS] amp=%d, a=%d\n",amp,a);

  Nsymb = (frame_parms->Ncp==0)?14:12;

  //for (aa=0;aa<frame_parms->nb_antennas_tx;aa++) {
  aa = 0;

  // The PSS occupies the inner 6 RBs, which start at
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
  k = (frame_parms->N_RB_DL-3)*12+5;
#else
  k = frame_parms->ofdm_symbol_size-3*12+5;
#endif
#else
  k = frame_parms->ofdm_symbol_size-3*12+5;
#endif
  //printf("[PSS] k = %d\n",k);
  for (m=5;m<67;m++) {
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
    txdataF[aa][slot_offset*Nsymb/2*frame_parms->N_RB_DL*12 + symbol*frame_parms->N_RB_DL*12 + k] = primary_sync_tab[m];
#else
    ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			    symbol*frame_parms->ofdm_symbol_size + k)] = 
      (amp * primary_sync[2*m]) >> 15; 
    ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			    symbol*frame_parms->ofdm_symbol_size + k) + 1] = 
      (amp * primary_sync[2*m+1]) >> 15;
#endif
#else
    //    printf("generate_pss: offset is %d (Nsymb %d)\n",slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
    //	   symbol*frame_parms->ofdm_symbol_size,Nsymb);
    ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			    symbol*frame_parms->ofdm_symbol_size + k)] = 
      (amp * primary_sync[2*m]) >> 15; 
    ((short*)txdataF[aa])[2*(slot_offset*Nsymb/2*frame_parms->ofdm_symbol_size +
			    symbol*frame_parms->ofdm_symbol_size + k) + 1] = 
      (amp * primary_sync[2*m+1]) >> 15;
#endif	
    k+=1;
#ifdef IFFT_FPGA
#ifndef RAW_IFFT
    if (k >= frame_parms->N_RB_DL*12) 
      k-=frame_parms->N_RB_DL*12;
#else
    if (k >= frame_parms->ofdm_symbol_size) {
      k++; //skip DC
      k-=frame_parms->ofdm_symbol_size;
    }
#endif
#else
    if (k >= frame_parms->ofdm_symbol_size) {
      k++; //skip DC
      k-=frame_parms->ofdm_symbol_size;
    }
#endif
  }
  //}
  return(0);
}

int generate_pss_emul(PHY_VARS_eNB *phy_vars_eNb,u8 sect_id) {
  
  msg("[PHY] EMUL eNB generate_pss_emul eNB %d, sect_id %d\n",phy_vars_eNb->Mod_id,sect_id);
  eNB_transport_info[phy_vars_eNb->Mod_id].cntl.pss=sect_id;
  return(0);
}
