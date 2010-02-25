#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
#define DEBUG_ULSCH

void ulsch_extract_rbs_single(int **rxdataF,
			      int **rxdataF_ext,
			      unsigned int first_rb,
			      unsigned int nb_rb,
			      unsigned char l,
			      unsigned char Ns,
			      LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short nb_rb1,nb_rb2;
  unsigned char aarx;
  int *rxF,*rxF_ext;
  
  //unsigned char symbol = l+Ns*frame_parms->symbols_per_tti/2;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    nb_rb1 = min(max((int)(frame_parms->N_RB_UL) - (int)(2*first_rb),0),2*nb_rb);    // 2 times no. RBs before the DC
    nb_rb2 = 2*nb_rb - nb_rb1;                                   // 2 times no. RBs after the DC
#ifdef DEBUG_ULSCH
    msg("ulsch_extract_rbs_single: 2*nb_rb1 = %d, 2*nb_rb2 = %d\n",nb_rb1,nb_rb2);
#endif

    rxF_ext   = &rxdataF_ext[aarx][(symbol*frame_parms->N_RB_UL*12)*2];
    
    if (nb_rb1) {
      rxF = &rxdataF[aarx][(first_rb*12 + frame_parms->first_carrier_offset + symbol*frame_parms->ofdm_symbol_size)*2];
      memcpy(rxF_ext, rxF, nb_rb1*12*sizeof(int));
      rxF_ext += nb_rb1*12;
    
      if (nb_rb2)  {
	rxF = &rxdataF[aarx][(1 + symbol*frame_parms->ofdm_symbol_size)*2];
	memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
	rxF_ext += nb_rb2*12;
      } 
    }
    else { //there is only data in the second half
      rxF = &rxdataF[aarx][(1 + 6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)*2];
      memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
      rxF_ext += nb_rb2*12;
    }
  }

  _mm_empty();
  _m_empty();

}
