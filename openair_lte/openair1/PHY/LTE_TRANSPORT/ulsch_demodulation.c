#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
//#define DEBUG_ULSCH

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
    
    rxF_ext   = &rxdataF_ext[aarx][(symbol*frame_parms->N_RB_UL*12)*2];
    rxF       = &rxdataF[aarx][(nb_rb*12 + frame_parms->first_carrier_offset + symbol*frame_parms->ofdm_symbol_size)*2];
    
    if ((first_rb<=frame_parms->N_RB_UL>>1) && ((nb_rb+first_rb)>frame_parms->N_RB_UL>>1)) {
      // the RB go over the DC
      nb_rb1 = (frame_parms->N_RB_UL>>1) - first_rb; // no. full RBs before the DC
      nb_rb2 = nb_rb - nb_rb1 - 1;                 // no. full RBs after the DC
      memcpy(rxF_ext, rxF, (nb_rb1+6)*2*sizeof(int));
      rxF = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
      memcpy(rxF_ext, rxF, (nb_rb2+6)*2*sizeof(int));
    } 
    else {
      memcpy(rxF_ext,rxF,nb_rb*2*sizeof(int));
    }
  }

  _mm_empty();
  _m_empty();

}
