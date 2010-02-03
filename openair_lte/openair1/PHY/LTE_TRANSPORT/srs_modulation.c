unsigned short msrsb_6_40[8][4] = {36,12,4,4,
				   32,16,8,4,
				   24,4,4,4,
				   20,4,4,4,
				   16,4,4,4,
				   12,4,4,4,
				   8,4,4,4,
				   4,4,4,4};

unsigned short msrsb_41_60[8][4] = {48,24,12,4,
				    48,16,8,4,
				    40,20,4,4,
				    36,12,4,4,
				    32,16,8,4,
				    24,4,4,4,
				    20,4,4,4,
				    16,4,4,4};

unsigned short msrsb_61_80[8][4] = {72,24,12,4,
				    64,32,16,4,
				    60,20,4,4,
				    48,24,12,4,
				    48,16,8,4,
				    40,20,4,4,
				    36,12,4,4,
				    32,16,8,4};

unsigned short msrsb_81_110[8][4] = {96,48,24,4,
				     96,32,16,4,
				     80,40,20,4,
				     72,24,12,4,
				     64,32,16,4,
				     60,20,4,4,
				     48,24,12,4,
				     48,16,8,4};

unsigned short Nb_6_40[8][4] = {36,12,4,4,
				   32,16,8,4,
				   24,4,4,4,
				   20,4,4,4,
				   16,4,4,4,
				   12,4,4,4,
				   8,4,4,4,
				   4,4,4,4};

unsigned short Nb_41_60[8][4] = {48,24,12,4,
				    48,16,8,4,
				    40,20,4,4,
				    36,12,4,4,
				    32,16,8,4,
				    24,4,4,4,
				    20,4,4,4,
				    16,4,4,4};

unsigned short Nb_61_80[8][4] = {72,24,12,4,
				    64,32,16,4,
				    60,20,4,4,
				    48,24,12,4,
				    48,16,8,4,
				    40,20,4,4,
				    36,12,4,4,
				    32,16,8,4};

unsigned short Nb_81_110[8][4] = {96,48,24,4,
				     96,32,16,4,
				     80,40,20,4,
				     72,24,12,4,
				     64,32,16,4,
				     60,20,4,4,
				     48,24,12,4,
				     48,16,8,4};

void generate_srs(unsigned char Csrs,
		  unsigned char Bsrs,
		  unsigned char kTC,
		  unsigned char n_RRC,
		  LTE_FRAME_PARMS *frame_parms,
		  int *txdataF) {

  unsigned short msrsb,Nb,msrs0,k0,Msc_RS,carrier_pos;

  if (lte_frame_parms->NB_RB_UL < 41) {
    msrs0 = msrsb_6_40[Csrs][0];
    msrsb = msrsb_6_40[Csrs][Bsrs];
    Nb    = Nb_6_40[Csrs][Bsrs];
  }
  else if (lte_frame_parms->N_RB_UL < 61) {
    msrs0 = msrsb_41_60[Csrs][0];
    msrsb = msrsb_41_60[Csrs][Bsrs];
    Nb    = Nb_41_80[Csrs][Bsrs];
  }
  else if (lte_frame_parms->N_RB_UL < 81) {
    msrs0 = msrsb_61_90[Csrs][0];
    msrsb = msrsb_61_80[Csrs][Bsrs];
    Nb    = Nb_61_80[Csrs][Bsrs];
  }
  else if (lte_frame_parms->N_RB_UL <111) {
    msrs0 = msrsb_81_110[Csrs][0];
    msrsb = msrsb_81_110[Csrs][Bsrs];
    Nb    = Nb_81_110[Csrs][Bsrs];
  }
  Msc_RS = msrsb * 6;
  k0 = (((lte_frame_parms->N_RB_UL>>1)-(msrsb0>>1))*12) + kTC;
  nb  = (4n_RRC/msrsb)%Nb;

  for (b=0;b<=Bsrs;b++) {
    k0 += 2*nb*Msc_RS;
  }

  carrier_pos = lte_frame_parms->first_carrier_offset + k0;
  if (carrier_pos >= lte_frame_parms->ofdm_symbol_size)
    carrier_pos=0;

  for (k=0;k<Msc_RS;k++) {
    txdataF[lte_frame_parms->first_carrier_offset + (k<<1)+k0] = ul_ref_sigs[0][0][Msc_RS]
    carrier_pos+=2;
    if (carrier_pos >= lte_frame_parms->ofdm_symbol_size)
      carrier_pos=0;
  }
}

#ifdef MAIN
main() {


}
#endif
