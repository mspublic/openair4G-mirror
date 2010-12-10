#include "PHY/defs.h"
#include "MAC_INTERFACE/extern.h"

unsigned short pcfich_reg[4];
unsigned char pcfich_first_reg_idx = 0;

//#define DEBUG_PCFICH

void generate_pcfich_reg_mapping(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned short kbar = 6 * (frame_parms->Nid_cell %(2*frame_parms->N_RB_DL));
  unsigned short first_reg;

  pcfich_reg[0] = kbar/6;
  first_reg = pcfich_reg[0];

  pcfich_reg[1] = ((kbar + (frame_parms->N_RB_DL>>1)*6)%(frame_parms->N_RB_DL*12))/6;
  if (pcfich_reg[1] < pcfich_reg[0]) {
    pcfich_first_reg_idx = 1;
    first_reg = pcfich_reg[1];
  }  
  pcfich_reg[2] = ((kbar + (frame_parms->N_RB_DL)*6)%(frame_parms->N_RB_DL*12))/6;
  if (pcfich_reg[2] < first_reg) {
    pcfich_first_reg_idx = 2;
    first_reg = pcfich_reg[2];
  }
  pcfich_reg[3] = ((kbar + ((3*frame_parms->N_RB_DL)>>1)*6)%(frame_parms->N_RB_DL*12))/6;
  if (pcfich_reg[3] < first_reg) {
    pcfich_first_reg_idx = 3;
    first_reg = pcfich_reg[3];
  }
  
#ifdef DEBUG_PCFICH
  debug_msg("[PHY] pcfich_reg : %d,%d,%d,%d\n",pcfich_reg[0],pcfich_reg[1],pcfich_reg[2],pcfich_reg[3]);
#endif
}

void pcfich_scrambling(LTE_DL_FRAME_PARMS *frame_parms,
		       u8 subframe,
		       u8 *b,
		       u8 *bt) {
  int i;
  u8 reset;
  u32 x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = ((((2*frame_parms->Nid_cell)+1)*(1+subframe))<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.7.1
  for (i=0; i<32; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }

    bt[i] = (b[i]&1) ^ ((s>>(i&0x1f))&1);
    //    printf("scrambling %d : b %d => bt %d, c %d\n",i,b[i],bt[i],((s>>(i&0x1f))&1));
  }
}

void pcfich_unscrambling(LTE_DL_FRAME_PARMS *frame_parms,
			 u8 subframe,
			 s16 *d) {

  int i;
  u8 reset;
  u32 x1, x2, s=0;

  reset = 1;
  // x1 is set in lte_gold_generic
  x2 = ((((2*frame_parms->Nid_cell)+1)*(1+subframe))<<9) + frame_parms->Nid_cell; //this is c_init in 36.211 Sec 6.7.1

  for (i=0; i<32; i++) {
    if ((i&0x1f)==0) {
      s = lte_gold_generic(&x1, &x2, reset);
      //printf("lte_gold[%d]=%x\n",i,s);
      reset = 0;
    }

    if (((s>>(i&0x1f))&1) == 1) 
      d[i]=-d[i];

    //    printf("scrambling %d : b %d => bt %d, c %d\n",i,b[i],bt[i],((s>>(i&0x1f))&1));
  }
}

u8 pcfich_b[4][32]={{0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
		    {1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0},
		    {1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1},
		    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

void generate_pcfich(u8 num_pdcch_symbols,
		     s16 amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     mod_sym_t **txdataF,
		     u8 subframe) {

  u8 pcfich_bt[32],nsymb,pcfich_quad;
  s16 pcfich_d[2][32];
  u8 i;
  u16 symbol_offset,m,re_offset,reg_offset;

#ifdef DEBUG_PCFICH
  msg("[PHY] Generating PCFICH for %d PDCCH symbols, AMP %d\n",num_pdcch_symbols,amp);
#endif

  // scrambling
  if ((num_pdcch_symbols>0) && (num_pdcch_symbols<4)) 
    pcfich_scrambling(frame_parms,subframe,pcfich_b[num_pdcch_symbols-1],pcfich_bt);

  // modulation

  if (frame_parms->mode1_flag) { // SISO
    for (i=0;i<32;i++) {
      pcfich_d[0][i]   = ((pcfich_bt[i] == 0) ? amp : -amp);
      //      printf("pcfich %d : %d (%d,%d)\n",i,pcfich_d[0][i],pcfich_b[num_pdcch_symbols-1][i],pcfich_bt[i]);
    }
  }
  else { // ALAMOUTI
    for (i=0;i<32;i+=4) {
      pcfich_d[0][i]   = (pcfich_bt[i] == 0) ? amp : -amp;
      pcfich_d[0][i+1] = (pcfich_bt[i+1] == 0) ? amp : -amp;
      
      pcfich_d[1][i] = (pcfich_bt[i+2] == 0) ? -amp : amp;
      pcfich_d[1][i+1] = (pcfich_bt[i+3] == 0) ? amp : -amp;

      pcfich_d[0][i+2] = -pcfich_d[1][i];
      pcfich_d[0][i+3] = pcfich_d[1][i+1];
      pcfich_d[1][i+2] = pcfich_d[0][i];
      pcfich_d[1][i+3] = -pcfich_d[0][i+1];
      /*
      printf("pcfich_d %d => (%d,%d,%d,%d) (%d %d %d %d)\n",
	     i,pcfich_d[0][i],pcfich_d[0][i+1],pcfich_d[0][i+2],pcfich_d[0][i+3],
	     pcfich_d[1][i],pcfich_d[1][i+1],pcfich_d[1][i+2],pcfich_d[1][i+3]);
      */
    }
  }


  // mapping
  nsymb = (frame_parms->Ncp==0) ? 14:12;
  
#ifdef IFFT_FPGA      
  symbol_offset = (u32)frame_parms->N_RB_DL*12*((subframe*nsymb));
  re_offset = frame_parms->N_RB_DL*12/2;
  
#else
  symbol_offset = (u32)frame_parms->ofdm_symbol_size*((subframe*nsymb));
  re_offset = frame_parms->first_carrier_offset;

#endif

  // loop over 4 quadruplets and lookup REGs
  m=0;
  for (pcfich_quad=0;pcfich_quad<4;pcfich_quad++) {
    reg_offset = re_offset+((u16)pcfich_reg[pcfich_quad]*6);
#ifdef IFFT_FPGA
    if (reg_offset>=(frame_parms->N_RB_DL*12))
      reg_offset-=(frame_parms->N_RB_DL*12);
#else
    if (reg_offset>=frame_parms->ofdm_symbol_size)
      reg_offset=1 + reg_offset-frame_parms->ofdm_symbol_size;
#endif

    for (i=0;i<6;i++) {
      if ((i!=(frame_parms->nushift))&&(i!=(frame_parms->nushift+3))) {

	txdataF[0][symbol_offset+reg_offset+i] = ((s32*)pcfich_d[0])[m];
	/*
		printf("pcfich: quad %d, i %d, offset %d => m%d (%d,%d)\n",pcfich_quad,i,reg_offset+i,m,
		       ((s16*)&txdataF[0][symbol_offset+reg_offset+i])[0],
		       ((s16*)&txdataF[0][symbol_offset+reg_offset+i])[1]);
	*/
	if (frame_parms->mode1_flag==0)   // ALAMOUTI
	  txdataF[1][symbol_offset+reg_offset+i] = ((s32*)pcfich_d[1])[m];
	m++;
      }
    }
  }

}


u8 rx_pcfich(LTE_DL_FRAME_PARMS *frame_parms,
	     u8 subframe,
	     LTE_UE_PDCCH *lte_ue_pdcch_vars,
	     MIMO_mode_t mimo_mode) {

  u8 pcfich_bt[32],nsymb,pcfich_quad;
  s16 pcfich_d[2][32];
  u8 i,j;
  u16 symbol_offset,m,re_offset,reg_offset;

  s32 **rxdataF_comp = lte_ue_pdcch_vars->rxdataF_comp;
  s16 phich_d[32],*phich_d_ptr;
  s16 metric,old_metric=-16384;
  u8 num_pdcch_symbols=3;

  // demapping
  // loop over 4 quadruplets and lookup REGs
  m=0;
  phich_d_ptr = phich_d;

  for (pcfich_quad=0;pcfich_quad<4;pcfich_quad++) {
    reg_offset = (pcfich_reg[pcfich_quad]*4);

    if (frame_parms->mode1_flag==1) {  // SISO
      for (i=0;i<4;i++) {
	//	printf("rx_pcfich: quad %d, i %d, offset %d => m%d (%d,%d)\n",pcfich_quad,i,reg_offset+i,m,
	//	       ((s16*)&rxdataF_comp[0][reg_offset+i])[0],
	//	       ((s16*)&rxdataF_comp[0][reg_offset+i])[1]);
	phich_d_ptr[0] = 0;
	phich_d_ptr[1] = 0;
	for (j=0;j<frame_parms->nb_antennas_rx;j++) {
	  phich_d_ptr[0] += ((s16*)&rxdataF_comp[j][reg_offset+i])[0]; // RE component
	  phich_d_ptr[1] += ((s16*)&rxdataF_comp[j][reg_offset+i])[1]; // IM component
	} 
	/*	printf("rx_pcfich: quad %d, i %d, offset %d => m%d (%d,%d) => phich_d_ptr[0] %d \n",pcfich_quad,i,reg_offset+i,m,
	       ((s16*)&rxdataF_comp[0][reg_offset+i])[0],
	       ((s16*)&rxdataF_comp[0][reg_offset+i])[1],
	       phich_d_ptr[0]);
	*/
	phich_d_ptr+=2;
      }
    }
    else { // ALAMOUTI
      for (i=0;i<4;i+=2) {
	phich_d_ptr[0] = 0;
	phich_d_ptr[1] = 0;
	phich_d_ptr[2] = 0;
	phich_d_ptr[3] = 0;
	for (j=0;j<frame_parms->nb_antennas_rx;j++) {
	  phich_d_ptr[0] += (((s16*)&rxdataF_comp[j][reg_offset+i])[0]+
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i+1])[0]); // RE component
	  phich_d_ptr[1] += (((s16*)&rxdataF_comp[j][reg_offset+i])[1] -
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i+1])[1]);// IM component
	  
	  phich_d_ptr[2] += (((s16*)&rxdataF_comp[j][reg_offset+i+1])[0]-
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i])[0]); // RE component
	  phich_d_ptr[3] += (((s16*)&rxdataF_comp[j][reg_offset+i+1])[1] +
			     ((s16*)&rxdataF_comp[j+2][reg_offset+i])[1]);// IM component
	}
	/*
	printf("rx_pcfich: quad %d, i %d, offset %d => m%d (%d,%d) => phich_d_ptr[0] %d \n",pcfich_quad,i,reg_offset+i,m,
	       ((s16*)&rxdataF_comp[0][reg_offset+i])[0],
	       ((s16*)&rxdataF_comp[0][reg_offset+i])[1],
	       phich_d_ptr[0]);
	*/
	phich_d_ptr+=4;

      }
    }
  }

  // pcfhich unscrambling

  pcfich_unscrambling(frame_parms,subframe,phich_d);

  // pcfich detection

  for (i=0;i<3;i++) {
    metric = 0;
    for (j=0;j<32;j++) {
      //      printf("pcfich_b[%d][%d] %d => phich_d[%d] %d]\n",i,j,pcfich_b[i][j],j,phich_d[j]);
      metric += ((pcfich_b[i][j]==0) ? (phich_d[j]) : (-phich_d[j]));
    }
#ifdef DEBUG_PCFICH
    msg("metric %d : %d\n",i,metric);
#endif
    if (metric > old_metric) {
      num_pdcch_symbols = 1+i;
      old_metric = metric;
    }
  }

#ifdef DEBUG_PCFICH
  msg("[PHY] PCFICH detected for %d PDCCH symbols\n",num_pdcch_symbols);
#endif
  return(num_pdcch_symbols);
}
