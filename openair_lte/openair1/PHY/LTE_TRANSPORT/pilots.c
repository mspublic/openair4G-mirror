#include "defs.h"
#include "PHY/defs.h"

void generate_pilots(int **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned short Ntti) {

  
  unsigned int tti,tti_offset,slot_offset,Nsymb;
  unsigned char second_pilot;



  Nsymb = (frame_parms->Ncp==0)?14:12;
  second_pilot = (frame_parms->Ncp==0)?4:3;

  for (tti=0;tti<Ntti;tti++) {
	  
	
    //    printf("Doing TX pilots for TTI %d\n",tti);
    
    tti_offset = tti*frame_parms->ofdm_symbol_size*Nsymb;
    slot_offset = (tti*2)%20;
    
    //    printf("tti %d : offset %d (slot %d)\n",tti,tti_offset,slot_offset);
    //Generate Pilots
    
    //antenna 0 symbol 0 slot 0
    lte_dl_cell_spec(&txdataF[0][tti_offset],
		     amp,
		     frame_parms,
		     slot_offset,
		     0,
		     0);

    
    
    //antenna 0 symbol 3 slot 0
    lte_dl_cell_spec(&txdataF[0][tti_offset+(second_pilot*frame_parms->ofdm_symbol_size)],
		     amp,
		     frame_parms,
		     slot_offset,
		     1,
		     0);
    
    //antenna 0 symbol 0 slot 1
    lte_dl_cell_spec(&txdataF[0][tti_offset+((Nsymb>>1)*frame_parms->ofdm_symbol_size)],
		     amp,
		     frame_parms,
		     1+slot_offset,
		     0,
		     0);
        
    //antenna 0 symbol 3 slot 1
    lte_dl_cell_spec(&txdataF[0][tti_offset+(((Nsymb>>1)+second_pilot)*frame_parms->ofdm_symbol_size)],
		     amp,
		     frame_parms,
		     1+slot_offset,
		     1,
		     0);
    

    if (frame_parms->nb_antennas_tx > 1) {

      // antenna 1 symbol 0 slot 0
      lte_dl_cell_spec(&txdataF[1][tti_offset],
		       amp,
		       frame_parms,
		       slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 0
      lte_dl_cell_spec(&txdataF[1][tti_offset+(second_pilot*frame_parms->ofdm_symbol_size)],
		       amp,
		       frame_parms,
		       slot_offset,
		       1,
		       1);
      
      //antenna 1 symbol 0 slot 1
      lte_dl_cell_spec(&txdataF[1][tti_offset+(Nsymb>>1)*frame_parms->ofdm_symbol_size],
		       amp,
		       frame_parms,
		       1+slot_offset,
		       0,
		       1);
      
      // antenna 1 symbol 3 slot 1
      lte_dl_cell_spec(&txdataF[1][tti_offset+(((Nsymb>>1)+second_pilot)*frame_parms->ofdm_symbol_size)],
		       amp,
		       frame_parms,
		       1+slot_offset,
		       1,
		       1);
    }
  }
}
	    
