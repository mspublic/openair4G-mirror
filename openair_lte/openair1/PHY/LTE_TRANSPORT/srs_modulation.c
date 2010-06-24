#include "PHY/defs.h"
#include "extern.h"

unsigned short msrsb_6_40[8][4] = {{36,12,4,4},
				   {32,16,8,4},
				   {24,4,4,4},
				   {20,4,4,4},
				   {16,4,4,4},
				   {12,4,4,4},
				   {8,4,4,4},
				   {4,4,4,4}};

unsigned short msrsb_41_60[8][4] = {{48,24,12,4},
				    {48,16,8,4},
				    {40,20,4,4},
				    {36,12,4,4},
				    {32,16,8,4},
				    {24,4,4,4},
				    {20,4,4,4},
				    {16,4,4,4}};

unsigned short msrsb_61_80[8][4] = {{72,24,12,4},
				    {64,32,16,4},
				    {60,20,4,4},
				    {48,24,12,4},
				    {48,16,8,4},
				    {40,20,4,4},
				    {36,12,4,4},
				    {32,16,8,4}};

unsigned short msrsb_81_110[8][4] = {{96,48,24,4},
				     {96,32,16,4},
				     {80,40,20,4},
				     {72,24,12,4},
				     {64,32,16,4},
				     {60,20,4,4},
				     {48,24,12,4},
				     {48,16,8,4}};

unsigned short Nb_6_40[8][4] = {{36,12,4,4},
				{32,16,8,4},
				{24,4,4,4},
				{20,4,4,4},
				{16,4,4,4},
				{12,4,4,4},
				{8,4,4,4},
				{4,4,4,4}};

unsigned short Nb_41_60[8][4] = {{48,24,12,4},
				 {48,16,8,4},
				 {40,20,4,4},
				 {36,12,4,4},
				 {32,16,8,4},
				 {24,4,4,4},
				 {20,4,4,4},
				 {16,4,4,4}};

unsigned short Nb_61_80[8][4] = {{72,24,12,4},
				 {64,32,16,4},
				 {60,20,4,4},
				 {48,24,12,4},
				 {48,16,8,4},
				 {40,20,4,4},
				 {36,12,4,4},
				 {32,16,8,4}};

unsigned short Nb_81_110[8][4] = {{96,48,24,4},
				  {96,32,16,4},
				  {80,40,20,4},
				  {72,24,12,4},
				  {64,32,16,4},
				  {60,20,4,4},
				  {48,24,12,4},
				  {48,16,8,4}};

int compareints (const void * a, const void * b)
{
  return ( *(unsigned short*)a - *(unsigned short*)b );
}


int generate_srs_tx(LTE_DL_FRAME_PARMS *frame_parms,
		    mod_sym_t *txdataF,
		    short amp,
		    unsigned int sub_frame_number) {

  unsigned short msrsb,Nb,nb,b,msrs0,k,l,Msc_RS,Msc_RS_idx,carrier_pos,symbol_offset;
  unsigned short *Msc_idx_ptr;
  int k0;
  int sub_frame_offset;

  if (frame_parms->N_RB_UL < 41) {
    msrs0 = msrsb_6_40[frame_parms->Csrs][0];
    msrsb = msrsb_6_40[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_6_40[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL < 61) {
    msrs0 = msrsb_41_60[frame_parms->Csrs][0];
    msrsb = msrsb_41_60[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_41_60[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL < 81) {
    msrs0 = msrsb_61_80[frame_parms->Csrs][0];
    msrsb = msrsb_61_80[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_61_80[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL <111) {
    msrs0 = msrsb_81_110[frame_parms->Csrs][0];
    msrsb = msrsb_81_110[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_81_110[frame_parms->Csrs][frame_parms->Bsrs];
  }

  Msc_RS = msrsb * 6;
  k0 = (((frame_parms->N_RB_UL>>1)-(msrs0>>1))*12) + frame_parms->kTC;
  nb  = (4*frame_parms->n_RRC/msrsb)%Nb;

  for (b=0;b<=frame_parms->Bsrs;b++) {
    k0 += 2*nb*Msc_RS;
  }

  if (k0<0) {
    msg("generate_srs: invalid parameter set msrs0=%d, msrsb=%d, Nb=%d => nb=%d, k0=%d\n",msrs0,msrsb,Nb,nb,k0);
    return(-1);
  }

#ifdef USER_MODE
  Msc_idx_ptr = (unsigned short*) bsearch((unsigned short*) &Msc_RS, (unsigned short*) dftsizes, 33, sizeof(unsigned short), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("generate_srs: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else //stdlib not availiable in RTAI
  if (Msc_RS==216)
    Msc_RS_idx = 12;
  else if (Msc_RS==144)
    Msc_RS_idx = 9;
  else {
    msg("generate_srs: index for Msc_RS=%d not implemented\n",Msc_RS);
    return(-1);
  }
#endif

#ifdef DEBUG_SRS
  msg("generate_srs_tx: Msc_RS = %d, Msc_RS_idx = %d\n",Msc_RS, Msc_RS_idx);
#endif

#ifndef IFFT_FPGA_UE
  carrier_pos = (frame_parms->first_carrier_offset + k0) % frame_parms->ofdm_symbol_size;
  //msg("carrier_pos = %d\n",carrier_pos);
  
  sub_frame_offset = sub_frame_number*frame_parms->symbols_per_tti*frame_parms->ofdm_symbol_size;
  symbol_offset = sub_frame_offset+(frame_parms->symbols_per_tti-1)*frame_parms->ofdm_symbol_size;

  for (k=0;k<Msc_RS;k++) {
    ((short*) txdataF)[2*(symbol_offset + carrier_pos)]   = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][k<<1])>>15);
    ((short*) txdataF)[2*(symbol_offset + carrier_pos)+1] = (short) (((int) amp * (int) ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1])>>15);
    carrier_pos+=2;
    if (carrier_pos >= frame_parms->ofdm_symbol_size)
      carrier_pos=0;
  }
#else
  carrier_pos = (frame_parms->N_RB_UL*12/2 + k0) % (frame_parms->N_RB_UL*12);
  //msg("carrier_pos = %d\n",carrier_pos);

  sub_frame_offset = sub_frame_number*frame_parms->symbols_per_tti*frame_parms->N_RB_UL*12;
  symbol_offset = sub_frame_offset+(frame_parms->symbols_per_tti-1)*frame_parms->N_RB_UL*12;

  for (k=0;k<Msc_RS;k++) {
    if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] >= 0)) 
      txdataF[symbol_offset+carrier_pos] = (mod_sym_t) 4;
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] < 0)) 
      txdataF[symbol_offset+carrier_pos] = (mod_sym_t) 2;
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] >= 0)) 
      txdataF[symbol_offset+carrier_pos] = (mod_sym_t) 3;
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] < 0)) 
      txdataF[symbol_offset+carrier_pos] = (mod_sym_t) 1;

    carrier_pos+=2;
    if (carrier_pos >= frame_parms->N_RB_UL*12)
      carrier_pos=0;
  }
#endif
  //  write_output("srs_tx.m","srstx",&txdataF[symbol_offset],512,1,1);
  return(0);
}

int generate_srs_rx(LTE_DL_FRAME_PARMS *frame_parms,
		    int *txdataF) {

  unsigned short msrsb,Nb,nb,b,msrs0,k,Msc_RS,Msc_RS_idx,carrier_pos;
  unsigned short *Msc_idx_ptr;
  int k0;

  if (frame_parms->N_RB_UL < 41) {
    msrs0 = msrsb_6_40[frame_parms->Csrs][0];
    msrsb = msrsb_6_40[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_6_40[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL < 61) {
    msrs0 = msrsb_41_60[frame_parms->Csrs][0];
    msrsb = msrsb_41_60[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_41_60[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL < 81) {
    msrs0 = msrsb_61_80[frame_parms->Csrs][0];
    msrsb = msrsb_61_80[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_61_80[frame_parms->Csrs][frame_parms->Bsrs];
  }
  else if (frame_parms->N_RB_UL <111) {
    msrs0 = msrsb_81_110[frame_parms->Csrs][0];
    msrsb = msrsb_81_110[frame_parms->Csrs][frame_parms->Bsrs];
    Nb    = Nb_81_110[frame_parms->Csrs][frame_parms->Bsrs];
  }

  Msc_RS = msrsb * 6;
  k0 = (((frame_parms->N_RB_UL>>1)-(msrs0>>1))*12) + frame_parms->kTC;
  nb  = (4*frame_parms->n_RRC/msrsb)%Nb;

  for (b=0;b<=frame_parms->Bsrs;b++) {
    k0 += 2*nb*Msc_RS;
  }

  if (k0<0) {
    msg("Invalid parameter set msrs0=%d, msrsb=%d, Nb=%d => nb=%d, k0=%d\n",msrs0,msrsb,Nb,nb,k0);
    return(-1);
  }

#ifdef USER_MODE
  Msc_idx_ptr = (unsigned short*) bsearch((unsigned short*) &Msc_RS, (unsigned short*) dftsizes, 33, sizeof(unsigned short), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("generate_srs: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else //stdlib not availiable in RTAI
  if (Msc_RS==216)
    Msc_RS_idx = 12;
  else if (Msc_RS==144)
    Msc_RS_idx = 9;
  else {
    msg("generate_srs: index for Msc_RS=%d not implemented\n",Msc_RS);
    return(-1);
  }
#endif

#ifdef DEBUG_SRS
  msg("generate_srs_rx: Msc_RS = %d, Msc_RS_idx = %d, k0=%d\n",Msc_RS, Msc_RS_idx,k0);
#endif

  carrier_pos = (frame_parms->first_carrier_offset + k0) % frame_parms->ofdm_symbol_size;

  for (k=0;k<Msc_RS;k++) {
    ((short*) txdataF)[carrier_pos<<2]   = ul_ref_sigs_rx[0][0][Msc_RS_idx][k<<2];
    ((short*) txdataF)[(carrier_pos<<2)+1] = ul_ref_sigs_rx[0][0][Msc_RS_idx][(k<<2)+1];
    ((short*) txdataF)[(carrier_pos<<2)+2] = ul_ref_sigs_rx[0][0][Msc_RS_idx][(k<<2)+2];
    ((short*) txdataF)[(carrier_pos<<2)+3] = ul_ref_sigs_rx[0][0][Msc_RS_idx][(k<<2)+3];
    carrier_pos+=2;
    if (carrier_pos >= frame_parms->ofdm_symbol_size) 
#ifdef OFDMA_ULSCH
      carrier_pos=1;
#else
      carrier_pos=0;
#endif
  }
  /*
  for (k=0;k<Msc_RS;k++) {
    if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] >= 0)) {
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+1] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+2] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+3] = ONE_OVER_SQRT2_Q15;
    }
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] >= 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] < 0)) {
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+1] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+2] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+3] = ONE_OVER_SQRT2_Q15;
    }
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] >= 0)) {
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+1] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+2] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+3] = -ONE_OVER_SQRT2_Q15;
    }
    else if ((ul_ref_sigs[0][0][Msc_RS_idx][k<<1] < 0) && (ul_ref_sigs[0][0][Msc_RS_idx][(k<<1)+1] < 0)) {
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+1] = -ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+2] = ONE_OVER_SQRT2_Q15;
      ((short*) txdataF)[4*(symbol_offset + carrier_pos)+3] = -ONE_OVER_SQRT2_Q15;
    }

    carrier_pos+=2;
    if (carrier_pos >= frame_parms->ofdm_symbol_size)
      carrier_pos=0;
  }
  */

  //  write_output("srs_rx.m","srsrx",txdataF,1024,2,1);
  return(0);
}


#ifdef MAIN
main() {


}
#endif
