#include "PHY/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"
#include "defs.h"
#include "extern.h"
#include "PHY/extern.h"
void dlsch_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		      u8 num_pdcch_symbols,
		      LTE_eNB_DLSCH_t *dlsch,
		      u16 G,
		      u8 q,
		      u8 Ns) {

  int i;
  u8 reset;
  u32 x1, x2, s=0;
  u8 *e=dlsch->e;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = (dlsch->rnti<<14) + (q<<13) + ((Ns>>1)<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.3.1

  //  printf("scrambling: rnti %x, q %d, Ns %d, Nid_cell %d, length %d\n",dlsch->rnti,q,Ns,frame_parms->Nid_cell, G);  
  for (i=0; i<G; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    //        printf("scrambling %d : e %d, c %d\n",i,e[i],((s>>(i&0x1f))&1));
    e[i] = (e[i]&1) ^ ((s>>(i&0x1f))&1);
  }

}


void dlsch_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			u8 num_pdcch_symbols,
			LTE_UE_DLSCH_t *dlsch,
			u16 G,
			s16* llr,
			u8 q,
			u8 Ns) {

  int i;
  u8 reset;
  u32 x1, x2, s=0;
  
  reset = 1;
  // x1 is set in first call to lte_gold_generic

  x2 = (dlsch->rnti<<14) + (q<<13) + ((Ns>>1)<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.3.1
  //  printf("unscrambling: rnti %x, q %d, Ns %d, Nid_cell %d length %d\n",dlsch->rnti,q,Ns,frame_parms->Nid_cell,G);
  for (i=0; i<G; i++) {
    if (i%32==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }
    // take the quarter of the PBCH that corresponds to this frame
    //printf("unscrambling %d : e %d, c %d\n",i,llr[i],((s>>(i&0x1f))&1));
    if (((s>>(i%32))&1)==0)
      llr[i] = -llr[i];

  }
} 
