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



  int i, l;
  unsigned int crc;

  unsigned int  pbch_crc_bits,pbch_crc_bytes,pbch_coded_bits,pbch_coded_bytes;
  pbch_crc_bits    = 64;
  pbch_crc_bytes   = pbch_crc_bits>>3;
  pbch_coded_bits  = (frame_parms->Ncp==0) ? 36*6*2 : 24*6*2; //RE/RB * #RB * bits/RB (QPSK)
  pbch_coded_bytes = pbch_coded_bits>>3;

  unsigned char pbch_data[pbch_crc_bytes];
  unsigned char pbch_coded_data[pbch_coded_bits], pbch_scrambled[pbch_coded_bits];  //one bit per byte

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
	       pbch_coded_data,
	       pbch_coded_bits,
	       1,
	       4);
#endif //USER_MODE
#endif //DEBUG_PHY

  // scrambling
  // TBD

  // modulation and mapping 
  int nsymb = (frame_parms->Ncp==0) ? 14:12;
  int pilots, first_pilot;
  int second_pilot = (frame_parms->Ncp==0) ? 4 : 3;
  int jj=0;
  int re_allocated=0;
  int rb, re_offset;
  for (l=(nsymb>>1);l<(nsymb>>1)+4;l++) {
    
    pilots=0;
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
			   pbch_coded_data,
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

