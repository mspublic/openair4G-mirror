#ifdef USER_MODE
#include <string.h>
#endif
#include "defs.h"
#include "PHY/defs.h"
#include "filt96_32.h"
//#define DEBUG_CH 
int lte_dl_mbsfn_channel_estimation(PHY_VARS_UE *phy_vars_ue,
				    u8 eNB_id,
					u8 eNB_offset,
				    int subframe,
				    unsigned char l,
				    unsigned char symbol){
  


  int pilot[600] __attribute__((aligned(16)));
  unsigned char aarx,aa;
  unsigned short k;
  unsigned int rb,pilot_cnt;
  short ic[3],*pil,*rxF,*ch,*ch_prev;
  int ch_offset,symbol_offset;
  int c,p;
  //  unsigned int n;
  //  int i;
  u16 Nid_cell_mbsfn = phy_vars_ue->lte_frame_parms.Nid_cell_mbsfn;

  int **dl_ch_estimates=phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[0];
  int **rxdataF=phy_vars_ue->lte_ue_common_vars.rxdataF;

  ch_offset     = (l*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
  symbol_offset = phy_vars_ue->lte_frame_parms.ofdm_symbol_size*symbol;


  for (aarx=0;aarx<phy_vars_ue->lte_frame_parms.nb_antennas_rx;aarx++) {
    // generate pilot
    if ((l==2)||(l==6)||(l==10)) {
      lte_dl_mbsfn_rx(phy_vars_ue,
			   &pilot[0],
			   subframe,
			   l>>2); 
			   } // if symbol==2, return 0 else if symbol = 6, return 1, else if symbol=10 return 2
      
      
      
      pil   = (short *)&pilot[0];
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+k+phy_vars_ue->lte_frame_parms.first_carrier_offset)<<1)]; 
      ch = (short *)&dl_ch_estimates[aarx][ch_offset];
      
      //    if (eNb_id==0)
      memset(ch,0,4*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
//***********************************************************************      
      if ((phy_vars_ue->lte_frame_parms.N_RB_DL==6)  || 
	  (phy_vars_ue->lte_frame_parms.N_RB_DL==50) || 
	  (phy_vars_ue->lte_frame_parms.N_RB_DL==100)) {
	
	// Interpolation  and extrapolation;	  
	for (rb=0; rb<phy_vars_ue->lte_frame_parms.N_RB_DL; rb++) {
		
		if (l==6) {  // l=6;
	  ch+=2;
	  rxF+=4;
	  for (c=0; c< 24; c+=4) { // total no of real and img channels in a RB;
		  for (p=0; p< 12; p+=2) { // total no of real and img pilots in a RB;
	  
	  ch[c] = (short)(((int)pil[p]*rxF[c] - (int)pil[p+1]*rxF[c+1])>>15);
	  ch[c+1] = (short)(((int)pil[p]*rxF[c+1] + (int)pil[p+1]*rxF[c])>>15);
	  ch[c+2] = ch[c]>>1;
	  ch[c+3] = ch[c+1]>>1;
	   if (rb>0) {
	  ch[c-2] += ch[c+2];
	  ch[c-1] += ch[c+3];
	  }
	  else
	  {
	  ch[c-2]= (ch[c]>>1)*3- ch[c+4];
	  ch[c-1]= (ch[c+1]>>1)*3- ch[c+5];  
	  }
	}
 }
}
	  	  
	  else  { // l=2 or 10; 
		for (c=0; c< 24; c+=4) {
		  for (p=0; p< 12; p+=2) {
	  
	  ch[c] = (short)(((int)pil[p]*rxF[c] - (int)pil[p+1]*rxF[c+1])>>15);
	  ch[c+1] = (short)(((int)pil[p]*rxF[c+1] + (int)pil[p+1]*rxF[c])>>15);
	  if (rb=phy_vars_ue->lte_frame_parms.N_RB_DL) {
	  ch[c+2]= (ch[c]>>1)*3- ch[c-2];
	  ch[c+3]= (ch[c+1]>>1)*3- ch[c-1];
      }
	  else {
	  ch[c+2] = ch[c]>>1;
	  ch[c+3] = ch[c+1]>>1;
	  ch[c-2] += ch[c+2];
	  ch[c-1] += ch[c+3];
	    
      }
	}
   }
 }
if ( rb= phy_vars_ue->lte_frame_parms.N_RB_DL>>1) {
rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 
	ch+=2;
	rxF+=2;
		}
	
}
}   
	  //pil+=12;
	  //ch+=24;
	  //rxF+=24;
//*********************************************************************	  
      else if (phy_vars_ue->lte_frame_parms.N_RB_DL==25) {
	//printf("Channel estimation\n");


	// loop over first 12 RBs
	if (l==6) {
	  // extrapolate the first channel estimate
	  ch+=2;
	  rxF+=4;
	  
	
	for (rb=0;rb<12;rb++) {
	    // 1st pilot

	  ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	  ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	  ch[2] = ch[0]>>1;
	  ch[3] = ch[1]>>1;
	  if (rb>0) {
	    ch[-2] += ch[2];
	    ch[-1] += ch[3];
	  }
	  else
	  {
	  ch[-2]= (ch[0]>>1)*3- ch[4];
	  ch[-1]= (ch[1]>>1)*3- ch[5];  
	  }
	  // 2nd pilot
	  ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	  ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	  ch[6] = ch[4]>>1;
	  ch[7] = ch[5]>>1;
	  ch[2] += ch[6];
	  ch[3] += ch[7];
	  
	  // 3rd pilot
	  ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	  ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	  ch[10] = ch[8]>>1;
	  ch[11] = ch[9]>>1;
	  ch[6] += ch[10];
	  ch[7] += ch[11];
	  
	  // 4th pilot
	  ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	  ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	  ch[14] = ch[12]>>1;
	  ch[15] = ch[13]>>1;
	  ch[10] += ch[14];
	  ch[11] += ch[15];
	  
	  // 5th pilot
	  ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	  ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	  ch[18] = ch[16]>>1;
	  ch[19] = ch[17]>>1;
	  ch[14] += ch[18];
	  ch[15] += ch[19];
	  
	  // 6th pilot
	  ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	  ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	  ch[22] = ch[20]>>1;
	  ch[23] = ch[21]>>1;
	  ch[18] += ch[22];
	  ch[19] += ch[23];
	  
	  pil+=12;
	  ch+=24;
	  rxF+=24;
	}


	// middle PRB
	// 1st pilot

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	ch[2] = ch[0]>>1;
	ch[3] = ch[1]>>1;
	
	ch[-2] += ch[2];
	ch[-1] += ch[3];
	
	// 2nd pilot
	ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	ch[6] = ch[4]>>1;
	ch[7] = ch[5]>>1;
	ch[2] += ch[6];
	ch[3] += ch[7];

	// 3rd pilot
	ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	ch[10] = (ch[8])/3;
	ch[11] = (ch[9])/3;
	ch[6] += ch[10];
	ch[7] += ch[11];

	// printf("Second half\n");
	// Second half of RBs
	rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 

	// 4th pilot
	ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	ch[14] = ch[12]>>1;
	ch[15] = ch[13]>>1;
	ch[10] += (ch[12]<<1)/3;
	ch[11] += (ch[13]<<1)/3;

	// 5th pilot
	ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	ch[18] = ch[16]>>1;
	ch[19] = ch[17]>>1;
	ch[14] += ch[18];
	ch[15] += ch[19];
	
	// 6th pilot
	ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	ch[22] = ch[20]>>1;
	ch[23] = ch[21]>>1;
	ch[18] += ch[22];
	ch[19] += ch[23];

	for (rb=0;rb<12;rb++) {
	    // 1st pilot
	  ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	  ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	  ch[2] = ch[0]>>1;
	  ch[3] = ch[1]>>1;
	  if (rb>0) {
	    ch[-2] += ch[2];
	    ch[-1] += ch[1];
	  }
	  // 2nd pilot
	  ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	  ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	  ch[6] = ch[4]>>1;
	  ch[7] = ch[5]>>1;
	  ch[2] += ch[6];
	  ch[3] += ch[7];
	  
	  // 3rd pilot
	  ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	  ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	  ch[10] = ch[8]>>1;
	  ch[11] = ch[9]>>1;
	  ch[6] += ch[10];
	  ch[7] += ch[11];
	  
	  // 4th pilot
	  ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	  ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	  ch[14] = ch[12]>>1;
	  ch[15] = ch[13]>>1;
	  ch[10] += ch[14];
	  ch[11] += ch[15];
	  
	  // 5th pilot
	  ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	  ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	  ch[18] = ch[16]>>1;
	  ch[19] = ch[17]>>1;
	  ch[14] += ch[18];
	  ch[15] += ch[19];
	  
	  // 6th pilot
	  ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	  ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	  ch[22] = ch[20]>>1;
	  ch[23] = ch[21]>>1;
	  ch[18] += ch[22];
	  ch[19] += ch[23];
	  
	  pil+=12;
	  ch+=24;
	  rxF+=24;
	}
	
}
//**********************************************************************
// for l=2 and l=1
	if (l!=6) {
	  // extrapolate last channel estimate
	for (rb=0;rb<12;rb++) {
	 // 1st pilot

	  ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	  ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	  ch[2] = ch[0]>>1;
	  ch[3] = ch[1]>>1;
	 /* if (rb>0) {
	    ch[-2] += ch[2];
	    ch[-1] += ch[1];
	} */
	  // 2nd pilot
	  ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	  ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	  ch[6] = ch[4]>>1;
	  ch[7] = ch[5]>>1;
	  ch[2] += ch[6];
	  ch[3] += ch[7];
	  
	  // 3rd pilot
	  ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	  ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	  ch[10] = ch[8]>>1;
	  ch[11] = ch[9]>>1;
	  ch[6] += ch[10];
	  ch[7] += ch[11];
	  
	  // 4th pilot
	  ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	  ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	  ch[14] = ch[12]>>1;
	  ch[15] = ch[13]>>1;
	  ch[10] += ch[14];
	  ch[11] += ch[15];
	  
	  // 5th pilot
	  ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	  ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	  ch[18] = ch[16]>>1;
	  ch[19] = ch[17]>>1;
	  ch[14] += ch[18];
	  ch[15] += ch[19];
	  
	  // 6th pilot
	  ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	  ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	  ch[22] = ch[20]>>1;
	  ch[23] = ch[21]>>1;
	  ch[18] += ch[22];
	  ch[19] += ch[23];
	  
	  pil+=12;
	  ch+=24;
	  rxF+=24;
	}
	
	// middle PRB
	// 1st pilot

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	ch[2] = ch[0]>>1;
	ch[3] = ch[1]>>1;
	
	ch[-2] += ch[2];
	ch[-1] += ch[3];
	
	// 2nd pilot
	ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	ch[6] = ch[4]>>1;
	ch[7] = ch[5]>>1;
	ch[2] += ch[6];
	ch[3] += ch[7];

	// 3rd pilot
	ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	ch[10] = (ch[8]<<1)/3;
	ch[11] = (ch[9]<<1)/3;
	ch[6] += ch[10];
	ch[7] += ch[11];

	// printf("Second half\n");
	// Second half of RBs
	rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 

	// 4th pilot
	ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	ch[14] = ch[12]>>1;
	ch[15] = ch[13]>>1;
	ch[10] += (ch[12])/3;
	ch[11] += (ch[13])/3;

	// 5th pilot
	ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	ch[18] = ch[16]>>1;
	ch[19] = ch[17]>>1;
	ch[14] += ch[18];
	ch[15] += ch[19];
	
	// 6th pilot
	ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	ch[22] = ch[20]>>1;
	ch[23] = ch[21]>>1;
	ch[18] += ch[22];
	ch[19] += ch[23];

	for (rb=0;rb<12;rb++) {
	    // 1st pilot
	  ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	  ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	  ch[2] = ch[0]>>1;
	  ch[3] = ch[1]>>1;
	  if (rb>0) {
	    ch[-2] += ch[2];
	    ch[-1] += ch[3];
	  }
	  // 2nd pilot
	  ch[4] = (short)(((int)pil[2]*rxF[4] - (int)pil[3]*rxF[5])>>15);
	  ch[5] = (short)(((int)pil[2]*rxF[5] + (int)pil[3]*rxF[4])>>15);
	  ch[6] = ch[4]>>1;
	  ch[7] = ch[5]>>1;
	  ch[2] += ch[6];
	  ch[3] += ch[7];
	  
	  // 3rd pilot
	  ch[8] = (short)(((int)pil[4]*rxF[8] - (int)pil[5]*rxF[9])>>15);
	  ch[9] = (short)(((int)pil[4]*rxF[9] + (int)pil[5]*rxF[8])>>15);
	  ch[10] = ch[8]>>1;
	  ch[11] = ch[9]>>1;
	  ch[6] += ch[10];
	  ch[7] += ch[11];
	  
	  // 4th pilot
	  ch[12] = (short)(((int)pil[6]*rxF[12] - (int)pil[7]*rxF[13])>>15);
	  ch[13] = (short)(((int)pil[6]*rxF[13] + (int)pil[7]*rxF[12])>>15);
	  ch[14] = ch[12]>>1;
	  ch[15] = ch[13]>>1;
	  ch[10] += ch[14];
	  ch[11] += ch[15];
	  
	  // 5th pilot
	  ch[16] = (short)(((int)pil[8]*rxF[16] - (int)pil[9]*rxF[17])>>15);
	  ch[17] = (short)(((int)pil[8]*rxF[17] + (int)pil[9]*rxF[16])>>15);
	  ch[18] = ch[16]>>1;
	  ch[19] = ch[17]>>1;
	  ch[14] += ch[18];
	  ch[15] += ch[19];
	  
	  // 6th pilot
	  ch[20] = (short)(((int)pil[10]*rxF[20] - (int)pil[10]*rxF[21])>>15);
	  ch[21] = (short)(((int)pil[11]*rxF[21] + (int)pil[11]*rxF[20])>>15);
	  if (rb=11){
	  ch[22]= (ch[20]>>1)*3- ch[18];
	  ch[23]= (ch[21]>>1)*3- ch[19];
      }
	  else {
	  ch[22] = ch[20]>>1;
	  ch[23] = ch[21]>>1;
      }
	  ch[18] += ch[22];
	  ch[19] += ch[23]; 
	   
      
	  pil+=12;
	  ch+=24;
	  rxF+=24;
	}
}
      }
      else if (phy_vars_ue->lte_frame_parms.N_RB_DL==15) {
	// Interpolation  and extrapolation;	  
	for (rb=0; rb<phy_vars_ue->lte_frame_parms.N_RB_DL; rb++) {
		
		if (l==6) {  // l=6;
	  ch+=2;
	  rxF+=4;
	  for (c=0; c< 24; c+=4) { // total no of real and img channels in a RB;
		  for (p=0; p< 12; p+=2) { // total no of real and img pilots in a RB;
	  
	  ch[c] = (short)(((int)pil[p]*rxF[c] - (int)pil[p+1]*rxF[c+1])>>15);
	  ch[c+1] = (short)(((int)pil[p]*rxF[c+1] + (int)pil[p+1]*rxF[c])>>15);
	  ch[c+2] = ch[c]>>1;
	  ch[c+3] = ch[c+1]>>1;
	   if (rb>0) {
	  ch[c-2] += ch[c+2];
	  ch[c-1] += ch[c+3];
	  }
	  else
	  {
	  ch[c-2]= (ch[c]>>1)*3- ch[c+4];
	  ch[c-1]= (ch[c+1]>>1)*3- ch[c+5];  
	  }
	}
 }
}
	  	  
	  else  { // l=2 or 10; 
		for (c=0; c< 24; c+=4) {
		  for (p=0; p< 12; p+=2) {
	  
	  ch[c] = (short)(((int)pil[p]*rxF[c] - (int)pil[p+1]*rxF[c+1])>>15);
	  ch[c+1] = (short)(((int)pil[p]*rxF[c+1] + (int)pil[p+1]*rxF[c])>>15);
	  if (rb=phy_vars_ue->lte_frame_parms.N_RB_DL) {
	  ch[c+2]= (ch[c]>>1)*3- ch[c-2];
	  ch[c+3]= (ch[c+1]>>1)*3- ch[c-1];
      }
	  else {
	  ch[c+2] = ch[c]>>1;
	  ch[c+3] = ch[c+1]>>1;
	  ch[c-2] += ch[c+2];
	  ch[c-1] += ch[c+3];
	    
      }
	}
   }
 }
if ( rb= phy_vars_ue->lte_frame_parms.N_RB_DL>>1) {
rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 
	ch+=2;
	rxF+=2;
		}
	
}
      } 
      else {
	msg("channel estimation not implemented for phy_vars_ue->lte_frame_parms.N_RB_DL = %d\n",phy_vars_ue->lte_frame_parms.N_RB_DL);
      }
   
      
#ifndef PERFECT_CE    
      // Temporal Interpolation
      // printf("ch_offset %d\n",ch_offset);
      
    /*  dl_ch = (short *)&dl_ch_estimates[aarx][ch_offset];
      if (ch_offset == 0) {
	//      printf("Interpolating %d->0\n",4-phy_vars_ue->lte_frame_parms.Ncp);
	dl_ch_prev = (short *)&dl_ch_estimates[aarx][(4-phy_vars_ue->lte_frame_parms.Ncp)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)];
	
	multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      } // this is 1/3,2/3 combination for pilots spaced by 3 symbols
      
      else { // do interpolation/extrapolation
	//      printf("Interpolating 0->%d\n",4-phy_vars_ue->lte_frame_parms.Ncp);*/
	
	
	ch_prev = (short *)&dl_ch_estimates[aarx][0];
		
	if (phy_vars_ue->lte_frame_parms.Ncp==1) {// pilot spacing 4 symbols (1/4,1/2,3/4 combination)
	  multadd_complex_vector_real_scalar( ch_prev,24576,ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  multadd_complex_vector_real_scalar( ch,8192,ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  
	  multadd_complex_vector_real_scalar( ch_prev,16384,ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  multadd_complex_vector_real_scalar( ch,16384,ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  
	  multadd_complex_vector_real_scalar( ch_prev,8192,ch_prev+(3*2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  multadd_complex_vector_real_scalar( ch,24576,ch_prev+(3*2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	}
	/*else {
	  multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  
	  multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	  multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	} // pilot spacing 3 symbols (1/3,2/3 combination)
      }*/
#endif          
   } 
  
  // do ifft of channel estimate
  for (aa=0;aa<phy_vars_ue->lte_frame_parms.nb_antennas_rx*phy_vars_ue->lte_frame_parms.nb_antennas_tx;aa++) {
    if (phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_offset][aa])
      fft((short*) &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_offset][aa][LTE_CE_OFFSET],
	  (short*) phy_vars_ue->lte_ue_common_vars.dl_ch_estimates_time[eNB_offset][aa],
	  phy_vars_ue->lte_frame_parms.twiddle_ifft,
	  phy_vars_ue->lte_frame_parms.rev,
	  phy_vars_ue->lte_frame_parms.log2_symbol_size,
	  phy_vars_ue->lte_frame_parms.log2_symbol_size/2,
	  0);
  }
  
  return(0); 
}





