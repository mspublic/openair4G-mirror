#include <string.h>
#include <math.h>
#include <unistd.h>

#include "SIMULATION/TOOLS/defs.h"
//#include "MAC_INTERFACE/vars.h"

#ifdef IFFT_FPGA
#include "PHY/LTE_REFSIG/mod_table.h"
#endif

//#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
//#include "SCHED/defs.h"
//#include "SCHED/vars.h"

//#include "PHY/defs.h"
//#include "MAC_INTERFACE/extern.h"
//#include "SCHED/defs2.h"
//#define DEBUG_FEP

//#include "SCHED/est_freq_offset.h"



#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "PHY/LTE_ESTIMATION/defs.h"
//#include "PHY/LTE_ESTIMATION/filt96_32.h"

#include "PHY/defs.h"
#include "PHY/LTE_ESTIMATION/defs.h"
//#include "PHY/LTE_ESTIMATION/filt96_32.h"

#include "PHY/LTE_REFSIG/defs.h"
//#include "PHY/defs.h"

extern short filt24_0[24];
extern short filt24_0_dcl[24];
extern short filt24_0_dcr[24];
extern short filt24_1[24] ;
extern short filt24_1_dcl[24] ;
extern short filt24_1_dcr[24] ;
extern short filt24_2[24] ;
extern short filt24_2_dcl[24];
extern short filt24_2_dcr[24];
extern short filt24_3[24] ;
extern short filt24_3_dcl[24];
extern short filt24_3_dcr[24] ;
extern short filt24_4[24]  ;
extern short filt24_4_dcl[24] ;
extern short filt24_4_dcr[24]  ;
extern short filt24_5[24] ;
extern short filt24_5_dcl[24]  ;
extern short filt24_5_dcr[24]  ;
extern short filt24_6[24]  ;
extern short filt24_6_dcl[24]  ;
extern short filt24_6_dcr[24] ;
extern short filt24_7[24] ;
extern short filt24_7_dcl[24]  ;
extern short filt24_7_dcr[24] ;
extern short filt24_0l[24] ;
extern short filt24_1l[24] ;
extern short filt24_2l[24]  ;
extern short filt24_3l[24]  ;
extern short filt24_4l[24]  ;
extern short filt24_5l[24] ;
extern short filt24_6l[24] ;
extern short filt24_7l[24] ;
extern short filt24_0l2[24]  ;
extern short filt24_1l2[24];
extern short filt24_2l2[24] ;
extern short filt24_3l2[24] ;
extern short filt24_4l2[24] ;
extern short filt24_5l2[24] ;
extern short filt24_6l2[24]  ;
extern short filt24_7l2[24]  ;
extern short filt24_0r[24]  ;
extern short filt24_1r[24]  ;
extern short filt24_2r[24] ;
extern short filt24_3r[24]  ;
extern short filt24_4r[24] ;
extern short filt24_5r[24]  ;
extern short filt24_6r[24]  ;
extern short filt24_7r[24]  ;
extern short filt24_0r2[24] ;
extern short filt24_1r2[24]  ;
extern short filt24_2r2[24]  ;
extern short filt24_3r2[24]  ;
extern short filt24_4r2[24]  ;
extern short filt24_5r2[24] ;
extern short filt24_6r2[24] ;
extern short filt24_7r2[24] ;


extern unsigned int lte_gold_table[3][20][2][14];
//#define DEBUG_DL_CELL_SPEC
//************************************************

int lte_dl_channel_estimation_SS(PHY_VARS_UE *phy_vars_ue,
			      u8 eNB_offset,
                              int **dl_ch_estimates,
			      int **rxdataF,
			      unsigned char Ns,
			      unsigned char p,
			      unsigned char l,
			      unsigned char symbol){
  
  int pilot[2][200] __attribute__((aligned(16)));
  unsigned char nu,aarx;
  unsigned short k;
  unsigned int rb,pilot_cnt;
  short ch[2],*pil,*rxF,*dl_ch,*dl_ch_prev,*f,*f2,*fl,*f2l2,*fr,*f2r2,*f2_dc,*f_dc;
  int ch_offset,symbol_offset;
  //  unsigned int n;
  //  int i;
  u16 Nid_cell = phy_vars_ue->lte_frame_parms.Nid_cell;
  u8 Nid1 = Nid_cell/3,Nid2=Nid_cell%3;
  u8 nushift;

  // recompute nushift with eNB_offset corresponding to adjacent eNB on which to perform channel estimation
  Nid2 = (Nid2+eNB_offset)%3;
  Nid_cell = (Nid1*3) + Nid2;
  nushift =  Nid_cell%6;

  if ((p==0) && (l==0) )
    nu = 0;
  else if ((p==0) && (l>0))
    nu = 3;
  else if ((p==1) && (l==0))
    nu = 3;
  else if ((p==1) && (l>0))
    nu = 0;
  else {
    msg("lte_dl_channel_estimation: p %d, l %d -> ERROR\n",p,l);
    return(-1);
  }

  ch_offset     = (l*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
  symbol_offset = phy_vars_ue->lte_frame_parms.ofdm_symbol_size*symbol;

  k = (nu + nushift)%6;//b
  if (k > 6)//b
    k -=6;//b
  
#ifdef DEBUG_CH
  printf("Channel Estimation : ch_offset %d, OFDM size %d, Ncp=%d, l=%d, Ns=%d, k=%d\n",ch_offset,phy_vars_ue->lte_frame_parms.ofdm_symbol_size,phy_vars_ue->lte_frame_parms.Ncp,l,Ns,k);
#endif

  switch (k) {
  case 0 :
    f=filt24_0;  //for first pilot of RB, first half
    f2=filt24_2; //for second pilot of RB, first half
    fl=filt24_0; //for first pilot of leftmost RB
    f2l2=filt24_2; 
    //    fr=filt24_2r; //for first pilot of rightmost RB
    fr=filt24_0r2; //for first pilot of rightmost RB
    //    f2r2=filt24_0r2;
    f2r2=filt24_2r;

    f_dc=filt24_0_dcr;  
    f2_dc=filt24_2_dcl;  
    break;
  case 1 :
    f=filt24_1;
    f2=filt24_3;
    fl=filt24_1l;
    f2l2=filt24_3l2;
    fr=filt24_1r2;
    f2r2=filt24_3r;
    f_dc=filt24_1_dcr;  //for first pilot of RB, first half
    f2_dc=filt24_3_dcl;  //for first pilot of RB, first half
    break;
  case 2 : 
    f=filt24_2;
    f2=filt24_4;
    fl=filt24_2l;
    f2l2=filt24_4l2; 
    fr=filt24_2r2;
    f2r2=filt24_4r;
    f_dc=filt24_2_dcr;  //for first pilot of RB, first half
    f2_dc=filt24_4_dcl;  //for first pilot of RB, first half
    break;
  case 3 :
    f=filt24_3;
    f2=filt24_5;
    fl=filt24_3l;
    f2l2=filt24_5l2;
    fr=filt24_3r2;
    f2r2=filt24_5r;
    f_dc=filt24_3_dcr;  //for first pilot of RB, first half
    f2_dc=filt24_5_dcl;  //for first pilot of RB, first half
    break;
  case 4 :
    f=filt24_4;
    f2=filt24_6;
    fl=filt24_4l;
    f2l2=filt24_6l2;
    fr=filt24_4r2;
    f2r2=filt24_6r;
    f_dc=filt24_4_dcr;  //for first pilot of RB, first half
    f2_dc=filt24_6_dcl;  //for first pilot of RB, first half
    break;
  case 5 :
    f=filt24_5;
    f2=filt24_7;
    fl=filt24_5l;
    f2l2=filt24_7l2;
    fr=filt24_5r2;
    f2r2=filt24_7r;
    f_dc=filt24_5_dcr;  //for first pilot of RB, first half
    f2_dc=filt24_7_dcl;  //for first pilot of RB, first half
    break;
  default:
    msg("lte_dl_channel_estimation: k=%d -> ERROR\n",k);
    return(-1);
    break;
  }
  


  // generate pilot
  lte_dl_cell_spec_rx(phy_vars_ue,
		      eNB_offset,
		      &pilot[p][0],
		      Ns,
		      (l==0)?0:1,
		      p);
  

  for (aarx=0;aarx<phy_vars_ue->lte_frame_parms.nb_antennas_rx;aarx++) {
    pil   = (short *)&pilot[p][0];
    rxF   = (short *)&rxdataF[aarx][((symbol_offset+k+phy_vars_ue->lte_frame_parms.first_carrier_offset)<<1)]; 
    dl_ch = (short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset];

    //    if (eNb_id==0)
      memset(dl_ch,0,4*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
    
      if ((phy_vars_ue->lte_frame_parms.N_RB_DL==6)  || 
	  (phy_vars_ue->lte_frame_parms.N_RB_DL==50) || 
	  (phy_vars_ue->lte_frame_parms.N_RB_DL==100)) {
      
      //First half of pilots
      // Treat first 2 pilots specially (left edge)
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      // printf("pilot 0 : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);
      multadd_real_vector_complex_scalar(fl,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
      dl_ch+=8;
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      // printf("pilot 1 : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);
      multadd_real_vector_complex_scalar(f2l2,
					 ch,
					 dl_ch,
					 24);
      pil+=2;
      rxF+=24;
      dl_ch+=16;

      for (pilot_cnt=2;pilot_cnt<((phy_vars_ue->lte_frame_parms.N_RB_DL)-1);pilot_cnt+=2) {
	
	// printf("%d\n",dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);	
		
	//	printf("pilot[%d][%d] (%d,%d)\n",p,pilot_cnt,pil[0],pil[1]);
	//	printf("rx[%d] -> (%d,%d)\n", k, rxF[0], rxF[1]);
	
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15); //Re
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15); //Im
	// printf("**rb %d %d\n",rb,dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);
	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	
	
	pil+=2;    // Re Im
	rxF+=24;   // 6 samples in replicated format (Re0 Im0 Re0 Im0) !!!
	dl_ch+=8;
	
	// printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	// printf("rx[%d] -> (%d,%d)\n", k+6, rxF[0], rxF[1]);
	
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	// printf("**rb %d %d\n",rb,dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }
      //       printf("Second half\n");
      // Second half of RBs
      
      k = (nu + nushift)%6;//b
      if (k > 6)//b
	k -=6;//b
      
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 
      
      for (pilot_cnt=0;pilot_cnt<((phy_vars_ue->lte_frame_parms.N_RB_DL)-1);pilot_cnt+=2) {
	//printf("pilot[%d][%d] (%d,%d)\n",p,pilot_cnt,pil[0],pil[1]);
	// printf("rx[%d][%d] -> (%d,%d)\n",p,first_carrier_offset + nushift + 6*rb+(3*p),rxF[0],rxF[1]);
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	
	// printf("**rb %d %d\n",rb,dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);
	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=8;
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	
	// printf("**rb %d %d\n",rb,dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      //      printf("pilot 49: rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      multadd_real_vector_complex_scalar(fr,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
      dl_ch+=8;
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      //       printf("pilot 50: rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);
      multadd_real_vector_complex_scalar(f2r2,
					 ch,
					 dl_ch,
					 24);


    }
    
    else if (phy_vars_ue->lte_frame_parms.N_RB_DL==25) {
      //printf("Channel estimation\n");

      // Treat first 2 pilots specially (left edge)
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
      printf("pilot 0 : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif

      multadd_real_vector_complex_scalar(fl,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
      dl_ch+=8;
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
      printf("pilot 1 : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif

      multadd_real_vector_complex_scalar(f2l2,
					 ch,
					 dl_ch,
					 24);
      pil+=2;
      rxF+=24;
      dl_ch+=16;

      for (pilot_cnt=2;pilot_cnt<24;pilot_cnt+=2) {

	// printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	// printf("rx[%d][%d] -> (%d,%d)\n",p,phy_vars_ue->lte_frame_parms.first_carrier_offset + phy_vars_ue->lte_frame_parms.nushift + 6*rb+(3*p),rxF[0],rxF[1]);

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
	printf("pilot %d : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",pilot_cnt,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);
	
	//	ch[0] = 1024;
	//	ch[1] = -128;
#endif
	
	
	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	pil+=2;    // Re Im
	rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
	dl_ch+=8;
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
	printf("pilot %d : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",pilot_cnt+1,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

	//	ch[0] = 1024;
	//	ch[1] = -128;
#endif
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
#ifdef DEBUG_CH
      printf("pilot 24: rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif

                 
      multadd_real_vector_complex_scalar(f_dc,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      dl_ch+=8;
      
      // printf("Second half\n");
      // Second half of RBs
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
#ifdef DEBUG_CH
      printf("pilot 25: rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif

      multadd_real_vector_complex_scalar(f2_dc,
					 ch,
					 dl_ch,
					 24);
      pil+=2;
      rxF+=24;
      dl_ch+=16;
      
      for (pilot_cnt=0;pilot_cnt<22;pilot_cnt+=2) {

	// printf("* pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	// printf("rx[%d][%d] -> (%d,%d)\n",p,phy_vars_ue->lte_frame_parms.first_carrier_offset + phy_vars_ue->lte_frame_parms.nushift + 6*rb+(3*p),rxF[0],rxF[1]);

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
#ifdef DEBUG_CH
	printf("pilot %d rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",26+pilot_cnt,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);	

	//	ch[0] = 1024;
	//	ch[1] = -128;
#endif

	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=8;
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
#ifdef DEBUG_CH
	printf("pilot %d : rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",27+pilot_cnt,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

	//	ch[0] = 1024;
	//	ch[1] = -128;
#endif
	
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }

      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
      printf("pilot 49: rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif


      multadd_real_vector_complex_scalar(fr,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
      dl_ch+=8;
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH

      printf("pilot 50: rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      //      ch[0] = 1024;
      //      ch[1] = -128;
#endif

      multadd_real_vector_complex_scalar(f2r2,
					 ch,
					 dl_ch,
					 24);

    }
    else if (phy_vars_ue->lte_frame_parms.N_RB_DL==15) {
      
      //printf("First Half\n");
      for (rb=0;rb<28;rb+=4) {
	
	//printf("aarx=%d\n",aarx);
	//printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	//printf("rx[%d][%d] -> (%d,%d)\n",p,
	//       phy_vars_ue->lte_frame_parms.first_carrier_offset + phy_vars_ue->lte_frame_parms.nushift + 6*rb+(3*p),
	//       rxF[0],
	//       rxF[1]);
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	//printf("ch -> (%d,%d)\n",ch[0],ch[1]);
	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	pil+=2;    // Re Im
	rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
	dl_ch+=8;
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	//printf("ch -> (%d,%d)\n",ch[0],ch[1]);
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      //     printf("ch -> (%d,%d)\n",ch[0],ch[1]);
      multadd_real_vector_complex_scalar(f,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      dl_ch+=8;
      
      //printf("Second half\n");
      //Second half of RBs
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+nushift + (3*p))<<1)]; 
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      
      multadd_real_vector_complex_scalar(f2,
					 ch,
					 dl_ch,
					 24);
      pil+=2;
      rxF+=24;
      dl_ch+=16;
      
      for (rb=0;rb<28;rb+=4) {
	//printf("aarx=%d\n",aarx);
	//printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	//printf("rx[%d][%d] -> (%d,%d)\n",p,
	//       phy_vars_ue->lte_frame_parms.first_carrier_offset + phy_vars_ue->lte_frame_parms.nushift + 6*rb+(3*p),
	//       rxF[0],
	//       rxF[1]);
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	
	multadd_real_vector_complex_scalar(f,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=8;
	
	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
	
	multadd_real_vector_complex_scalar(f2,
					   ch,
					   dl_ch,
					   24);
	pil+=2;
	rxF+=24;
	dl_ch+=16;
	
      }
    } else {
      msg("channel estimation not implemented for phy_vars_ue->lte_frame_parms.N_RB_DL = %d\n",phy_vars_ue->lte_frame_parms.N_RB_DL);
    }
    
    
#ifndef PERFECT_CE    
    // Temporal Interpolation
    // printf("ch_offset %d\n",ch_offset);
    
    dl_ch = (short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset];
    if (ch_offset == 0) {
      //      printf("Interpolating %d->0\n",4-phy_vars_ue->lte_frame_parms.Ncp);
      dl_ch_prev = (short *)&dl_ch_estimates[(p<<1)+aarx][(4-phy_vars_ue->lte_frame_parms.Ncp)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)];
      
      multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      
      multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
    } // this is 1/3,2/3 combination for pilots spaced by 3 symbols
    
    else {
      //      printf("Interpolating 0->%d\n",4-phy_vars_ue->lte_frame_parms.Ncp);
      
      dl_ch_prev = (short *)&dl_ch_estimates[(p<<1)+aarx][0];
      if (phy_vars_ue->lte_frame_parms.Ncp==0) {// pilot spacing 4 symbols (1/4,1/2,3/4 combination)
	multadd_complex_vector_real_scalar(dl_ch_prev,24576,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,8192,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,16384,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,16384,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,8192,dl_ch_prev+(3*2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,24576,dl_ch_prev+(3*2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      }
      else {
	multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1),1,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*((phy_vars_ue->lte_frame_parms.ofdm_symbol_size)<<1)),0,phy_vars_ue->lte_frame_parms.ofdm_symbol_size);
      } // pilot spacing 3 symbols (1/3,2/3 combination)
    }
#endif
    
  }
  return(0); 
}

//*************************************************


void do_quantization_eNB(PHY_VARS_eNB *PHY_vars_eNB, unsigned int nsymb, u8 pilot0, u8 pilot1, int quant_v, short *drs_ch_est, int UE_id)  {

//b PHY_vars_eNB->lte_frame_parms.ofdm_size has been del, check if PHY_vars_eNB->lte_frame_parms.ofdm_size==PHY_vars_UE->lte_frame_parms.ofdm_size

	//PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size=512; //to adapt
        //printf("PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size = %d\n", PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size);
        //exit(-1);
        int k, aa, aarx;
        short tx_energy;
	short drs_ch_estimates_norm[2*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb];

        u8 pilot[2];
        pilot[0] = 0;
        pilot[1] = 0;

        //printf("nsymb = %d,  %d \n", nsymb, PHY_vars_eNB->lte_frame_parms.nb_antennas_rx); //exit(-1);        	
	//for(k=0;k<PHY_vars_eNB->lte_frame_parms.nb_antennas_tx;k++)
        	tx_energy = 8; 

        for(aa=0;aa<2;aa++) { 
		for (aarx=0;aarx<1;aarx++) {//PHY_vars_eNB->lte_frame_parms.nb_antennas_rx
			for (k=0;k<PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb;k++) { 
			    drs_ch_estimates_norm[k+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb] = ((short *)PHY_vars_eNB->lte_eNB_pusch_vars[0]->drs_ch_estimates[0][aa+aarx])[k]/tx_energy;//lte_eNB_pusch_vars[aarx] FIX IT
			}
	        }
	}

        for(aa=0;aa<2;aa++) {//PHY_vars_eNB->lte_frame_parms.nb_antennas_tx
		for (k=pilot[aa]*2*300; k<pilot[aa]*2*300+2*300-1; k+=2) {
	      		if (drs_ch_estimates_norm[k+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb]>(quant_v-1))
		        	drs_ch_est[k-pilot[aa]*2*300+aa*2*300] = quant_v-1;
			else if ((drs_ch_estimates_norm[k+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb]) < (-quant_v))
				drs_ch_est[k-pilot[aa]*2*300+aa*2*300] = -quant_v;
			else 
			 	drs_ch_est[k-pilot[aa]*2*300+aa*2*300] = drs_ch_estimates_norm[k+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb];

			if (drs_ch_estimates_norm[k+1+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb]>(quant_v-1))
		                drs_ch_est[k+1-pilot[aa]*2*300+aa*2*300] = quant_v-1;
			else if ((drs_ch_estimates_norm[k+1+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb])< (-quant_v))
				drs_ch_est[k+1-pilot[aa]*2*300+aa*2*300] = -quant_v;
			else
			 	drs_ch_est[k+1-pilot[aa]*2*300+aa*2*300] = drs_ch_estimates_norm[k+1+aa*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb];
		}
	}
}

void do_quantization_UE(PHY_VARS_UE *PHY_vars_UE, unsigned int nsymb, u8 pilot0, int quant_v, short *dl_ch_estimates, int dec_f)  {
        int k, aa, aarx;;
        short tx_energy;
	short dl_ch_estimates_norm[2*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb];
	u8 pilot[2];
	pilot[0] = 0;
        pilot[1] = 0;
        
	tx_energy = 4;  

//printf("PHY_vars_UE->lte_frame_parms.nb_antennas_rx= %d \n",PHY_vars_UE->lte_frame_parms.nb_antennas_rx);
//exit(-1);

/*
	for (k=0;k<PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb;k++)
 		dl_ch_estimates_norm[k] = ((short *)PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][0])[k]/tx_energy;
			
     for(aa=0;aa<2;aa++) { 	
	for (k=pilot0*2*512; k<pilot0*2*512+2*300-1; k+=(2*dec_f)) {
		if (dl_ch_estimates_norm[k] > (quant_v-1))
		        dl_ch_estimates[k-pilot0*2*512] = quant_v-1;

		else if (dl_ch_estimates_norm[k] < (-quant_v))
			dl_ch_estimates[k-pilot0*2*512] = -quant_v;
		else 
			dl_ch_estimates[k-pilot0*2*512] = dl_ch_estimates_norm[k];

		if (dl_ch_estimates_norm[k+1]>(quant_v-1))
		        dl_ch_estimates[k+1-pilot0*2*512] = quant_v-1;
		else if (dl_ch_estimates_norm[k+1] < (-quant_v))
			dl_ch_estimates[k+1-pilot0*2*512] = -quant_v;
		else
			dl_ch_estimates[k+1-pilot0*2*512] = dl_ch_estimates_norm[k+1];
	}
      }
*/
        for(aa=0;aa<2;aa++) { 
		for (aarx=0;aarx<PHY_vars_UE->lte_frame_parms.nb_antennas_rx;aarx++) {
			for (k=0;k<PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb;k++) { 
 			    dl_ch_estimates_norm[k+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb] = ((short *)PHY_vars_UE->lte_ue_common_vars.dl_ch_estimates[0][(aa<<1)+aarx])[k+10]/tx_energy;//b (aa*6) for est ofset ant1
			}
	        }
	}

	for(aa=0;aa<2;aa++) { 
		for (k=pilot[aa]*2*512; k<pilot[aa]*2*512+2*300-1; k+=(2*dec_f)) {
		        if (dl_ch_estimates_norm[k+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb] > (quant_v-1))
		                dl_ch_estimates[k-pilot[aa]*2*512+aa*2*300] = quant_v-1;

			else if (dl_ch_estimates_norm[k+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb] < (-quant_v))
				dl_ch_estimates[k-pilot[aa]*2*512+aa*2*300] = -quant_v;
			else 
				dl_ch_estimates[k-pilot[aa]*2*512+aa*2*300] = dl_ch_estimates_norm[k+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb];

			if (dl_ch_estimates_norm[k+1+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb]>(quant_v-1))
		                dl_ch_estimates[k+1-pilot[aa]*2*512+aa*2*300] = quant_v-1;
			else if (dl_ch_estimates_norm[k+1+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb] < (-quant_v))
				dl_ch_estimates[k+1-pilot[aa]*2*512+aa*2*300] = -quant_v;
			else
				dl_ch_estimates[k+1-pilot[aa]*2*512+aa*2*300] = dl_ch_estimates_norm[k+1+aa*PHY_vars_UE->lte_frame_parms.ofdm_symbol_size*nsymb];
		}
	}

}



void do_precoding(PHY_VARS_eNB *PHY_vars_eNB, short *drs_ch_est_ZFB, double PeNb_factor[2][600], short *prec, int nsymb, int UE_id, int aa) {

  int l, k;
  double temp[nsymb][600];
  short drs_ch_est[600*nsymb];
  //short dl_ch_estimates[600*nsymb];

  for (k=0; k<600*nsymb; k++) {
      drs_ch_est[k] = drs_ch_est_ZFB[k+aa*600*nsymb];
  }

    for (k=0; k<nsymb; k++) {
      for (l=0; l<600; l+=2) {
        temp[k][l] = drs_ch_est[k*600+l]*PeNb_factor[aa][l] - drs_ch_est[k*600+l+1]*PeNb_factor[aa][l+1];
        temp[k][l+1] = drs_ch_est[k*600+l+1]*PeNb_factor[aa][l] + drs_ch_est[k*600+l]*PeNb_factor[aa][l+1];
      
        prec[k*2*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size+l] = (short)(temp[k][l]);///(temp[k][l]*temp[k][l]+temp[k][l+1]*temp[k][l+1]));
        prec[k*2*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size+l+1] = (short)(temp[k][l+1]);///(temp[k][l]*temp[k][l]+temp[k][l+1]*temp[k][l+1]));
      }
    }

   //write_output("zfb.m","zfbF", prec,600*nsymb,1,1); exit(-1);
  //write_output("drsch.m","drschF", drs_ch_est,nsymb*600,1,1);
  //write_output("prec.m","precF", prec,nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);  
  //write_output("dlch.m","dlchF", dl_ch_estimates,nsymb*PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);

}



//calibration algo
void do_calibration(short K_dl_ch_estimates[25][2][600], short K_drs_ch_estimates[25][2][600], double PeNb_factor[2][600], int ofdm_syn, int n_K) {
       
  //Calib Algor in eNb
  int i=0, s_c=0;
  double ar=0,ai=0,br=0,bi=0,cr=0,ci=0,dr=0,di=0;
  int aa;
  int length_H_G = n_K*4;

  //double phase_inc = 2*M_PI*(4*512-4*300)*(5-1)*1/7.68e6;


  short H[length_H_G];
  short G[length_H_G];  	
  bzero(H,length_H_G);
  bzero(G,length_H_G);
 
 for(s_c=0; s_c<600; s_c+=2)
  {
  for(aa=0; aa<2; aa++)
    {
    //system for 1 ant at primary, change to perform onother prim ant
	for(i=0;i<n_K;i++)
	{ 
	//printf("i = %d\n",i);
	G[(i<<2)+0] = K_dl_ch_estimates[i][aa][s_c+0];	
	G[(i<<2)+1] = K_dl_ch_estimates[i][aa][s_c+1];	
	H[(i<<2)+0] = K_drs_ch_estimates[i][aa][s_c+0];
	H[(i<<2)+1] = K_drs_ch_estimates[i][aa][s_c+1];
	}
       
      for(i=0;i<n_K;i++)
	{ 
	  ar +=  H[(i<<2)+0]*H[(i<<2)+0] + H[(i<<2)+1]*H[(i<<2)+1];
	  br +=  H[(i<<2)+0]*G[(i<<2)+0] + H[(i<<2)+1]*G[(i<<2)+1];
	  bi += -H[(i<<2)+0]*G[(i<<2)+1] + H[(i<<2)+1]*G[(i<<2)+0];
	  dr +=  G[(i<<2)+0]*G[(i<<2)+0] + G[(i<<2)+1]*G[(i<<2)+1];
	}
	  ar = (double)(ar/100); 
	  br = (double)(br/100);
	  bi = (double)(bi/100);
	  dr = (double)(dr/100);
	  

      if( (ar-dr+iSqrt((ar-dr)*(ar-dr)+((br*br+bi*bi)*4)))==0)
      {
      PeNb_factor[aa][s_c] = 0;
      PeNb_factor[aa][s_c+1] = 0;     
      } 
      else{
      PeNb_factor[aa][s_c]   = (2*br/(ar-dr+iSqrt((ar-dr)*(ar-dr)+((br*br+bi*bi)*4))));
      PeNb_factor[aa][s_c+1] = (-2*bi/(ar-dr+iSqrt((ar-dr)*(ar-dr)+((br*br+bi*bi)*4))));  
      }
      
      ar=0; ai=0; br=0; bi=0; cr=0; ci=0; dr=0; di=0;
      //if ((s_c>>1) > 4) exit(-1);      
   }
  }
  msg("P_eNb DETERMINED.. \n");
}


int slot_fep_SS(PHY_VARS_UE *phy_vars_ue,
	     unsigned char l,
	     unsigned char Ns,
	     int sample_offset,
	     int no_prefix) {

  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;
  LTE_UE_COMMON *ue_common_vars   = &phy_vars_ue->lte_ue_common_vars;
  u8 eNB_id = 0;//ue_common_vars->eNb_id;
  unsigned char aa;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame
  unsigned int nb_prefix_samples = (no_prefix ? 0 : frame_parms->nb_prefix_samples);
  unsigned int nb_prefix_samples0 = (no_prefix ? 0 : frame_parms->nb_prefix_samples0);
  unsigned int subframe_offset;
  unsigned int slot_offset;

  if (no_prefix) {
    subframe_offset = frame_parms->ofdm_symbol_size * frame_parms->symbols_per_tti * (Ns>>1);
    slot_offset = frame_parms->ofdm_symbol_size * (frame_parms->symbols_per_tti>>1) * (Ns%2);
  }
  else {
    subframe_offset = frame_parms->samples_per_tti * (Ns>>1);
    slot_offset = (frame_parms->samples_per_tti>>1) * (Ns%2);
  }

  if (l<0 || l>=7-frame_parms->Ncp) {
    msg("slot_fep: l must be between 0 and %d\n",7-frame_parms->Ncp);
    return(-1);
  }
  if (Ns<0 || Ns>=20) {
    msg("slot_fep: Ns must be between 0 and 19\n");
    return(-1);
  }

#ifdef DEBUG_FEP
  debug_msg("slot_fep: slot %d, symbol %d, nb_prefix_samples %d, nb_prefix_samples0 %d, slot_offset %d, subframe_offset %d, sample_offset %d\n", Ns, symbol, nb_prefix_samples,nb_prefix_samples0,slot_offset,subframe_offset,sample_offset);
#endif
  

  for (aa=0;aa<frame_parms->nb_antennas_rx;aa++) {
    memset(&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],0,2*frame_parms->ofdm_symbol_size*sizeof(int));

    if (l==0) {
      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       slot_offset +
					       nb_prefix_samples0 + 
					       subframe_offset],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }
    else {

      fft((short *)&ue_common_vars->rxdata[aa][sample_offset +
					       slot_offset +
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples0+nb_prefix_samples) + 
					       (frame_parms->ofdm_symbol_size+nb_prefix_samples)*(l-1) +
					       subframe_offset],
	  (short*)&ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	  frame_parms->twiddle_fft,
	  frame_parms->rev,
	  frame_parms->log2_symbol_size,
	  frame_parms->log2_symbol_size>>1,
	  0);
    }

    memcpy(&ue_common_vars->rxdataF2[aa][2*subframe_offset+2*frame_parms->ofdm_symbol_size*symbol],
	   &ue_common_vars->rxdataF[aa][2*frame_parms->ofdm_symbol_size*symbol],
	   2*frame_parms->ofdm_symbol_size*sizeof(int));

  }
    
  if ((l==0) || (l==(4-frame_parms->Ncp))) {
    for (aa=0;aa<2;aa++) {//b	
#ifndef PERFECT_CE
#ifdef DEBUG_FEP
      debug_msg("Channel estimation eNB %d, aatx %d, slot %d, symbol %d\n",eNB_id,aa,Ns,l);
#endif
      lte_dl_channel_estimation_SS(phy_vars_ue,0,
				ue_common_vars->dl_ch_estimates[eNB_id],
				ue_common_vars->rxdataF,
				Ns,
				aa,
				l,
				symbol);
#endif

      // do frequency offset estimation here!
      // use channel estimates from current symbol (=ch_t) and last symbol (ch_{t-1}) 
#ifdef DEBUG_FEP
      msg("Frequency offset estimation\n");
#endif   
      if ((Ns == 0) & (l==(4-frame_parms->Ncp))) 
	lte_est_freq_offset(ue_common_vars->dl_ch_estimates[eNB_id][0],
			    frame_parms,
			    l,
			    &ue_common_vars->freq_offset);
    }
  }
#ifdef DEBUG_FEP
  msg("slot_fep: done\n");
#endif
  return(0);
}

