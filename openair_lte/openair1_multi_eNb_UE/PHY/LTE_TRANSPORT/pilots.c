//#include "defs.h"
#include "PHY/defs.h"

void generate_pilots(mod_sym_t **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned char eNb_id,
		     unsigned short Ntti) {

  
  unsigned int tti,tti_offset,slot_offset,Nsymb,samples_per_symbol;
  unsigned char second_pilot;



  Nsymb = (frame_parms->Ncp==0)?14:12;
  second_pilot = (frame_parms->Ncp==0)?4:3;

  for (tti=0;tti<Ntti;tti++) {
	  
	
    //    printf("Doing TX pilots for TTI %d\n",tti);

#ifdef IFFT_FPGA
    tti_offset = tti*frame_parms->N_RB_DL*12*Nsymb;
    samples_per_symbol = frame_parms->N_RB_DL*12;
#else    
    tti_offset = tti*frame_parms->ofdm_symbol_size*Nsymb;
    samples_per_symbol = frame_parms->ofdm_symbol_size;
#endif
    slot_offset = (tti*2)%20;
    
    //    printf("tti %d : offset %d (slot %d)\n",tti,tti_offset,slot_offset);
    //Generate Pilots
    
    //antenna 0 symbol 0 slot 0
    lte_dl_cell_spec(&txdataF[0][tti_offset],
		     amp,
		     frame_parms,
		     eNb_id,
		     slot_offset,
		     0,
		     0);

    
    
    //antenna 0 symbol 3 slot 0
    lte_dl_cell_spec(&txdataF[0][tti_offset+(second_pilot*samples_per_symbol)],
		     amp,
		     frame_parms,
		     eNb_id,
		     slot_offset,
		     1,
		     0);
    
    //antenna 0 symbol 0 slot 1
    lte_dl_cell_spec(&txdataF[0][tti_offset+((Nsymb>>1)*samples_per_symbol)],
		     amp,
		     frame_parms,
		     eNb_id,
		     1+slot_offset,
		     0,
		     0);
        
    //antenna 0 symbol 3 slot 1
    lte_dl_cell_spec(&txdataF[0][tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol)],
		     amp,
		     frame_parms,
		     eNb_id,
		     1+slot_offset,
		     1,
		     0);
    

    if (frame_parms->nb_antennas_tx > 1) {

      // antenna 1 symbol 0 slot 0
      lte_dl_cell_spec(&txdataF[1][tti_offset],
		       amp,
		       frame_parms,
		       eNb_id,
		       slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 0
      lte_dl_cell_spec(&txdataF[1][tti_offset+(second_pilot*samples_per_symbol)],
		       amp,
		       frame_parms,
		       eNb_id,
		       slot_offset,
		       1,
		       1);
      
      //antenna 1 symbol 0 slot 1
      lte_dl_cell_spec(&txdataF[1][tti_offset+(Nsymb>>1)*samples_per_symbol],
		       amp,
		       frame_parms,
		       eNb_id,
		       1+slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 1
      lte_dl_cell_spec(&txdataF[1][tti_offset+(((Nsymb>>1)+second_pilot)*samples_per_symbol)],
		       amp,
		       frame_parms,
		       eNb_id,
		       1+slot_offset,
		       1,
		       1);
    }
  }
}
	    
int generate_pilots_slot(mod_sym_t **txdataF,
			 short amp,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 unsigned char eNb_id,
			 unsigned short slot) {

  
  unsigned int tti,tti_offset,slot_offset,Nsymb,samples_per_symbol;
  unsigned char second_pilot;

  if (slot<0 || slot>= 20) {
    msg("generate_pilots_slot: slot not in range (%d)\n",slot);
    return(-1);
  }

  Nsymb = (frame_parms->Ncp==0)?7:6;
  second_pilot = (frame_parms->Ncp==0)?4:3;


#ifdef IFFT_FPGA
  slot_offset = slot*frame_parms->N_RB_DL*12*Nsymb;
  samples_per_symbol = frame_parms->N_RB_DL*12;
#else    
  slot_offset = slot*frame_parms->ofdm_symbol_size*Nsymb;
  samples_per_symbol = frame_parms->ofdm_symbol_size;
#endif
    
    //    printf("tti %d : offset %d (slot %d)\n",tti,tti_offset,slot_offset);
    //Generate Pilots
    
    //antenna 0 symbol 0 slot 0
    lte_dl_cell_spec(&txdataF[0][slot_offset],
		     amp,
		     frame_parms,
		     eNb_id,
		     slot,
		     0,
		     0);

    
    
    //antenna 0 symbol 3 slot 0
    lte_dl_cell_spec(&txdataF[0][slot_offset+(second_pilot*samples_per_symbol)],
		     amp,
		     frame_parms,
		     eNb_id,
		     slot,
		     1,
		     0);
    

    if (frame_parms->nb_antennas_tx > 1) {

      // antenna 1 symbol 0 slot 0
      lte_dl_cell_spec(&txdataF[1][slot_offset],
		       amp,
		       frame_parms,
		       eNb_id,
		       slot,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 0
      lte_dl_cell_spec(&txdataF[1][slot_offset+(second_pilot*samples_per_symbol)],
		       amp,
		       frame_parms,
		       eNb_id,
		       slot,
		       1,
		       1);
      
    }

    return(0);
}

