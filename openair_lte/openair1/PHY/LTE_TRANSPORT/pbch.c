#include <string.h>
#include "PHY/defs.h"
#include "PHY/CODING/extern.h"
#include "PHY/CODING/lte_interleaver_inline.h"

#ifdef OPENAIR2
#include "PHY_INTERFACE/defs.h"
#endif


#ifdef OPENAIR2
/*!\brief This routine implements the MAC interface for the CHBCH on transmission.  It scans the MACPHY_REQ table
@param chbch_ind Index for the chbch to be generated.  The index is in correspondance with an associated chsch_ind.
*/

void phy_generate_chbch_top(unsigned char chbch_ind) {

  unsigned char i; //,j;

  
  for(i=0;i<NB_REQ_MAX;i++) {
         
    /*
      if (((mac_xface->frame/5) % 20) == 0)
      msg("[PHY][CODING] Frame %d: Req %d(%p), Active %d, Type %d\n",
      mac_xface->frame,
      i,
      &Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req,
      Macphy_req_table[0].Macphy_req_table_entry[i].Active,
      (Macphy_req_table[0].Macphy_req_table_entry[i].Active == 1)?
      Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type:-1);
        
    */

    if(Macphy_req_table[0].Macphy_req_table_entry[i].Active){
      if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Pdu_type == CHBCH) {

	
	Macphy_req_table[0].Macphy_req_table_entry[i].Active=0;
	//	Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_req.Phy_Resources_Entry->Active=0;
	Macphy_req_table[0].Macphy_req_cnt--;
	/*
	  if ((mac_xface->frame % 100) == 0) {
	  for (j=0;j<136;j+=4)
	  msg("[PHY][CODING] CHBCH_PDU(%d) %x : %x %x %x %x\n",sizeof(CHBCH_PDU),j,
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+1],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+2],
	  ((unsigned char*)Macphy_req_table.Macphy_req_table_entry[i].Macphy_data_ind.Pdu.Chbch_pdu)[j+3]);
	  }
	*/

#ifdef DEBUG_PHY
	if (((mac_xface->frame/5) % 200) == 0)
	  msg("[OPENAIR1][PHY][CODING] Frame %d: Calling generate chbch %d (%d,%d,%d,%d)\n",
	      mac_xface->frame,
	      chbch_ind,
	      NUMBER_OF_CHSCH_SYMBOLS,
	      NUMBER_OF_CHBCH_SYMBOLS,
	      SAMPLE_OFFSET_CHSCH_NO_PREFIX,
	      SAMPLE_OFFSET_CHBCH_NO_PREFIX);
#endif //DEBUG_PHY
	/*	
	if (Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Chbch_pdu->Num_ul_sach > 0)
	  msg("[OPENAIR1][PHY][CODING] Frame %d: Chbch has UL_SACH allocation\n", 
	      mac_xface->frame);
	
	*/

	phy_generate_chbch(chbch_ind,
			   0,
			   NB_ANTENNAS_TX,
			   (unsigned char *)Macphy_req_table[0].Macphy_req_table_entry[i].Macphy_data_req.Dir.Req_tx.Pdu.Chbch_pdu);
      }
    }
  }

}

#endif //OPENAIR2

/* This implements the LTE PBCH channel with the following differences form the spec 3GPP 36.212-860
 *  * we do not use the OFDM symbols 0 and 3/4 (normal/extended CP respectively)
 *      (in a later version, when we use 4 Tx antennas, we should also skip over the pilots in symbol 1)
 *  * that gives us pbch_coded_bits = 36*6*2=384 (RE/RB * #RB * bits/RB) for normal CP and
 *                  pbch_coded_bits = 24*6*2=240 for extended CP
 *  * the input length (information bits) is 64 input bits (including CRC)
 *  * we use turbo code with rate 1/3, rate matching to the corresponding size
 */

int generate_pbch(int **txdataF,
		  int amp,
		  LTE_DL_FRAME_PARMS *frame_parms,
		  unsigned char *pbch_pdu) {



  int i, l, j, j2;
  unsigned int crc;

  unsigned int  pbch_crc_bits,pbch_crc_bytes,pbch_coded_bits,pbch_coded_bytes;
  pbch_crc_bits    = 64;
  pbch_crc_bytes   = pbch_crc_bits>>3;
  pbch_coded_bits  = (frame_parms->Ncp==0) ? 36*6*2 : 24*6*2; //RE/RB * #RB * bits/RB (QPSK)
  pbch_coded_bytes = pbch_coded_bits>>3;

  unsigned char pbch_data[pbch_crc_bytes];
  unsigned char pbch_coded_data[pbch_coded_bits], pbch_coded_data2[pbch_coded_bits], pbch_scrambled[pbch_coded_bits];  //one bit per byte

  bzero(pbch_data,pbch_crc_bytes);
  bzero(pbch_coded_data,pbch_coded_bits);
  bzero(pbch_scrambled,pbch_coded_bits);

#ifdef USER_MODE
#ifdef DEBUG_PHY
  char fname[40],vname[40];
#endif //DEBUG_PHY
#endif // USER_MODE

  
  // Encode data

  // CRC attachment
  crc = crc16(pbch_pdu, pbch_crc_bits-16); 
  printf("crc = %x\n", crc);

  // scramble crc with PBCH CRC mask (Table 5.3.1.1-1 of 3GPP 36.212-860)
  switch (frame_parms->nb_antennas_tx) {
  case 1:
    crc = crc ^ (unsigned short) 0;
    break;
  case 2:
    crc = crc ^ (unsigned short) 1;
    break;
  case 4:
    crc = crc ^ (unsigned short) 0xAAAA;
    break;
  default:
    msg("[openair][PBCH] Unknown number of TX antennas!\n");
    break;
  }

  // Place crc
  // double check!!!
  for (i=0;i<6;i++) 
    pbch_data[i] = pbch_pdu[i];
  pbch_data[6] = ((char*) &crc)[2];
  pbch_data[7] = ((char*) &crc)[3];
  for (i=0;i<8;i++) 
    printf(" pbch_data[%d] = %x\n",i,pbch_data[i]);

  // this is not LTE compliant! LTE uses a rate 1/3 convolutional code
  threegpplte_turbo_encoder(pbch_data,
			    pbch_crc_bytes,
			    pbch_coded_data,
			    f1f2mat[threegpp_interleaver_parameters(pbch_crc_bytes)*2],   // f1 (see 36121-820, page 14)
			    f1f2mat[(threegpp_interleaver_parameters(pbch_crc_bytes)*2)+1]  // f2 (see 36121-820, page 14)
			    );

  // rate matching
  if (rate_matching_lte(pbch_coded_bits, 
			pbch_crc_bits*3+12, 
			pbch_coded_data,
			0) !=0 ) {
    msg("[openair1][PBCH] Rate matching problem!\n");
    return(-1);
  }

#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_encoded_output.m","pbch_encoded_out",
	       pbch_coded_data2,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // Bit collection
  j2=0;
  for (j=0;j<pbch_crc_bits*3+12;j++) {
    if ((pbch_coded_data[j]&0x80) > 0) { // bit is to be transmitted
      pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
      //Bit is repeated
      if ((pbch_coded_data[j]&0x40)>0)
	pbch_coded_data2[j2++] = pbch_coded_data[j]&1;
    }
  }					
  printf("[openair][PHCH] rate matched bits=%d, pbch_coded_bits=%d, pbch_crc_bits=%d\n",j2,pbch_coded_bits,pbch_crc_bits);


#ifdef DEBUG_PHY
#ifdef USER_MODE
  write_output("pbch_encoded_output2.m","pbch_encoded_out2",
	       pbch_coded_data2,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // scrambling
  // TBD

  // modulation and mapping 
  unsigned int nsymb = (frame_parms->Ncp==0) ? 14:12;
  unsigned int pilots, first_pilot;
  unsigned int second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  unsigned int jj=0;
  unsigned int re_allocated=0;
  unsigned int rb, re_offset;
  for (l=(nsymb>>1);l<(nsymb>>1)+4;l++) {
    
    pilots=0;
    first_pilot = 0;
    if ((l==0) || (l==(nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((l==second_pilot)||(l==(second_pilot+(nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

    printf("l=%d, pilots=%d, first_pilot=%d\n",l,pilots,first_pilot);

    if (pilots==0) { // don't skip pilot symbols
      // This is not LTE, it guarantees that
      // pilots from adjacent base-stations
      // do not interfere with data
      // LTE is eNb centric.  "Smart" Interference
      // cancellation isn't possible
      re_offset = frame_parms->ofdm_symbol_size-3*12;
      
      for (rb=frame_parms->N_RB_DL/2-3;rb<frame_parms->N_RB_DL/2+3;rb++) {
	
	allocate_REs_in_RB(txdataF,
			   &jj,
			   re_offset,
			   frame_parms->ofdm_symbol_size*l,
			   pbch_coded_data2,
			   SISO,
			   pilots,
			   first_pilot,
			   2,
			   amp,
			   &re_allocated,
			   0,
			   frame_parms);

	re_offset+=12; // go to next RB
	
	// check if we crossed the symbol boundary and skip DC
	if (re_offset >= frame_parms->ofdm_symbol_size) {
	    re_offset=1;
	}
      }
	
    }
  }


  return(0);
}

unsigned short pbch_extract_single(int **rxdataF,
				   int **dl_ch_estimates,
				   int **rxdataF_ext,
				   int **dl_ch_estimates_ext,
				   unsigned char symbol,
				   LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=6;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;

  unsigned char nsymb = (frame_parms->Ncp==0) ? 7:6;
  unsigned char symbol_mod = symbol % nsymb;

  int rx_offset = frame_parms->ofdm_symbol_size-3*12;
  int ch_offset = frame_parms->N_RB_DL*6-3*12;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    printf("extract_rbs: symbol_mod=%d, rx_offset=%d, ch_offset=%d\n",symbol_mod,
	   (rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2,
	   5+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size)));

    dl_ch0     = &dl_ch_estimates[aarx][LTE_CE_OFFSET+ch_offset+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    rxF        = &rxdataF[aarx][(rx_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(6*12)];
    rxF_ext    = &rxdataF_ext[aarx][symbol_mod*(6*12)];

    for (rb=0; rb<nb_rb; rb++) {
    
      // skip DC carrier
      if (rb==3) {
	rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	dl_ch0++;
      }
      
      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  
      for (i=0;i<12;i++) {
	rxF_ext[i]=rxF[i<<1];
      }
      dl_ch0+=12;
      dl_ch0_ext+=12;
      rxF+=24;
      rxF_ext+=12;
    }
  }
  return(0);
}

/*
unsigned short dlsch_extract_rbs_dual(int **rxdataF,
				      int **dl_ch_estimates,
				      int **rxdataF_ext,
				      int **dl_ch_estimates_ext,
				      unsigned int *rb_alloc,
				      unsigned char symbol,
				      LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*dl_ch1,*dl_ch1_ext,*rxF,*rxF_ext;
  unsigned char symbol_mod;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol_mod*(frame_parms->N_RB_DL*12)];

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  dl_ch0++;
	  dl_ch1++;
	}
	
	if (rb_alloc_ind==1) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));

	    //printf("rb %d\n",rb);
	    //for (i=0;i<12;i++)
	    //printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	    //printf("\n");
	  
	  for (i=0;i<12;i++) {
	    rxF_ext[i]=rxF[i<<1];
	    //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	    //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	  }
	  nb_rb++;
	}
	dl_ch0+=12;
	dl_ch0_ext+=12;
	dl_ch1+=12;
	dl_ch1_ext+=12;
	rxF+=24;
	rxF_ext+=12;
      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	if (rb_alloc_ind==1) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  printf("RB %d (%d)\n",rb,nb_rb);

	}
	dl_ch0+=12;
	dl_ch0_ext+=12;
	dl_ch1+=12;
	dl_ch1_ext+=12;
	rxF+=24;
	rxF_ext+=12;
      }
      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;
	
      if (rb_alloc_ind==1) {

	for (i=0;i<6;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  dl_ch1_ext[i]=dl_ch1[i];
	  rxF_ext[i]=rxF[i<<1];
	}
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	for (;i<12;i++) {
	  dl_ch0_ext[i]=dl_ch0[i+1];
	  dl_ch1_ext[i]=dl_ch1[i+1];
	  rxF_ext[i]=rxF[(1+i)<<1];
	}
	nb_rb++;
	printf("RB %d (%d)\n",rb,nb_rb);
      }
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=13;
      dl_ch0_ext+=12;
      dl_ch1+=13;
      dl_ch1_ext+=12;
      rxF+=14;
      rxF_ext+=12;
      rb++;

      for (;rb<frame_parms->N_RB_DL;rb++) {
	  
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	  
	if (rb_alloc_ind==1) {
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  printf("RB %d (%d)\n",rb,nb_rb);
	}
	dl_ch0+=12;
	dl_ch0_ext+=12;
	dl_ch1+=12;
	dl_ch1_ext+=12;
	rxF+=24;
	rxF_ext+=12;
      }
    }
  }
  return(nb_rb/frame_parms->nb_antennas_rx);
}
*/

void rx_pbch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH *lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     MIMO_mode_t mimo_mode) {

  unsigned char log2_maxh,aatx,aarx;
  int avgs, avg[frame_parms->nb_antennas_tx*frame_parms->nb_antennas_rx];

  int symbol;
  int nb_rb = 6;
  int mod_order = 2;
  int nsymb = (frame_parms->Ncp==0) ? 14:12;
  unsigned int pilots, first_pilot;
  unsigned int second_pilot = (frame_parms->Ncp==0) ? 4 : 3;

  for (symbol=(nsymb>>1);symbol<(nsymb>>1)+4;symbol++) {

    pilots=0;
    first_pilot = 0;
    if ((symbol==0) || (symbol==(nsymb>>1))){
      pilots=1;
      first_pilot=1;
    }

    if ((symbol==second_pilot)||(symbol==(second_pilot+(nsymb>>1)))) {
      pilots=1;
      first_pilot=0;
    }

    if (pilots==0) { // don't skip pilot symbols

      if (frame_parms->nb_antennas_tx>1)
	/*
	  nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
	  lte_ue_common_vars->dl_ch_estimates,
	  lte_ue_dlsch_vars->rxdataF_ext,
	  lte_ue_dlsch_vars->dl_ch_estimates_ext,
	  rb_alloc,
	  symbol,
	  frame_parms);
	*/
	msg("[openair][PBCH][RX] nb_antennas_tx>1 not yet implemented\n");
      else
	pbch_extract_single(lte_ue_common_vars->rxdataF,
			    lte_ue_common_vars->dl_ch_estimates,
			    lte_ue_dlsch_vars->rxdataF_ext,
			    lte_ue_dlsch_vars->dl_ch_estimates_ext,
			    symbol,
			    frame_parms);

  /*  
  //  we should be able to use this one with nb_rb=6, given the extract function is fixed
  if (symbol==(nsymb>>1)+1) {
    dlsch_channel_level(lte_ue_dlsch_vars->dl_ch_estimates_ext,
			frame_parms,
			avg,
			nb_rb);

  }

  avgs = 0;
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
      avgs = max(avgs,avg[(aarx<<1)+aatx]);

  log2_maxh = 4+(log2_approx(avgs)/2);
  printf("log2_maxh = %d (%d,%d)\n",log2_maxh,avg[0],avgs);

  dlsch_channel_compensation(lte_ue_dlsch_vars->rxdataF_ext,
			     lte_ue_dlsch_vars->dl_ch_estimates_ext,
			     lte_ue_dlsch_vars->dl_ch_mag,
			     lte_ue_dlsch_vars->dl_ch_magb,
			     lte_ue_dlsch_vars->rxdataF_comp,
			     frame_parms,
			     symbol,
			     mod_order,
			     nb_rb,
			     log2_maxh); // log2_maxh+I0_shift

  
  if (frame_parms->nb_antennas_rx > 1)
    dlsch_detection_mrc(frame_parms,
			lte_ue_dlsch_vars->rxdataF_comp,
			lte_ue_dlsch_vars->dl_ch_mag,
			lte_ue_dlsch_vars->dl_ch_magb,
			symbol,
			nb_rb);
      
    if (mimo_mode == SISO)
      dlsch_siso(frame_parms,lte_ue_dlsch_vars->rxdataF_comp,symbol,nb_rb);
    else if (mimo_mode == ALAMOUTI)
      dlsch_alamouti(frame_parms,lte_ue_dlsch_vars->rxdataF_comp,lte_ue_dlsch_vars->dl_ch_mag,lte_ue_dlsch_vars->dl_ch_magb,symbol,nb_rb);
    else if (mimo_mode == ANTCYCLING)
      dlsch_antcyc(frame_parms,lte_ue_dlsch_vars->rxdataF_comp,lte_ue_dlsch_vars->dl_ch_mag,lte_ue_dlsch_vars->dl_ch_magb,symbol,nb_rb);
    else {
      msg("dlsch_rx: Usupported MIMO mode\n");
      exit (-1);
    }

    memcpy(&lte_ue_dlsch_vars->llr[symbol%(nsymb>>1)*72],&lte_ue_dlsch_vars->rxdataF_comp[symbol%(nsymb>>1)*72],72*2*sizeof(int));
  */
  }
}
}
