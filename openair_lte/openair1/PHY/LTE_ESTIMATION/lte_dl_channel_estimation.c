#ifdef USER_MODE
#include <string.h>
#endif
#include "defs.h"
#include "PHY/defs.h"
#include "filt96_32.h"
//#define DEBUG_CH
int lte_dl_channel_estimation(int **dl_ch_estimates,
			  int **rxdataF,
			  LTE_DL_FRAME_PARMS *frame_parms,
			  unsigned char Ns,
			  unsigned char p,
			  unsigned char l,
			  unsigned char symbol){



  int pilot[2][200] __attribute__((aligned(16)));
  unsigned char nu,aarx;
  unsigned short k;
  unsigned int rb,pilot_cnt;
  short ch[2],*pil,*rxF,*dl_ch,*dl_ch_prev,*f,*f2,*fl,*f2l2,*fr,*f2r2;
  int ch_offset,symbol_offset;
  unsigned int n;
  int i;
  
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


  ch_offset     = (l*(frame_parms->ofdm_symbol_size));
  symbol_offset = frame_parms->ofdm_symbol_size*symbol;

  k = (nu + frame_parms->nushift);
  if (k > 6)
    k -=6;
  
#ifdef DEBUG_CH
  printf("Channel Estimation : ch_offset %d, %d, %d, %d, %d, k=%d\n",ch_offset,frame_parms->ofdm_symbol_size,frame_parms->Ncp,l,Ns,k);
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
    break;
  case 1 :
    f=filt24_1;
    f2=filt24_3;
    fl=filt24_1l;
    f2l2=filt24_3l2;
    fr=filt24_1r2;
    f2r2=filt24_3r;
    break;
  case 2 :
    f=filt24_2;
    f2=filt24_4;
    fl=filt24_2l;
    f2l2=filt24_4l2;
    fr=filt24_2r2;
    f2r2=filt24_4r;
    break;
  case 3 :
    f=filt24_3;
    f2=filt24_5;
    fl=filt24_3l;
    f2l2=filt24_5l2;
    fr=filt24_3r2;
    f2r2=filt24_5r;
    break;
  case 4 :
    f=filt24_4;
    f2=filt24_6;
    fl=filt24_4l;
    f2l2=filt24_6l2;
    fr=filt24_4r2;
    f2r2=filt24_6r;
    break;
  case 5 :
    f=filt24_5;
    f2=filt24_7;
    fl=filt24_5l;
    f2l2=filt24_7l2;
    fr=filt24_5r2;
    f2r2=filt24_7r;
    break;
  default:
    break;
  }
  


  // generate pilot
  lte_dl_cell_spec_rx(&pilot[p][0],
		      frame_parms,
		      Ns,
		      (l==0)?0:1,
		      p);
  

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    pil   = (short *)&pilot[p][0];
    rxF   = (short *)&rxdataF[aarx][((symbol_offset+k+frame_parms->first_carrier_offset)<<1)]; 
    dl_ch = (short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset];
    memset(dl_ch,0,4*(frame_parms->ofdm_symbol_size));
    
    if ((frame_parms->N_RB_DL==50) || (frame_parms->N_RB_DL==100)) {
      
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

      for (pilot_cnt=2;pilot_cnt<((frame_parms->N_RB_DL>>1)-1);pilot_cnt+=2) {
	
	// printf("%d\n",dl_ch-(short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset]);	
		
	// printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	// printf("rx[%d] -> (%d,%d)\n", k, /[0], rxF[1]);
	
	
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
      // printf("Second half\n");
      // Second half of RBs
      
      k = (nu + frame_parms->nushift);
      if (k > 6)
	k -=6;
      
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+k)<<1)]; 
      
      for (pilot_cnt=0;pilot_cnt<((frame_parms->N_RB_DL>>1)-1);pilot_cnt+=2) {
	// printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
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
	pil+=2;    // Re Im
	rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
	dl_ch+=16;
      }
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      // printf("pilot 49: rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      multadd_real_vector_complex_scalar(f2r2,
					 ch,
					 dl_ch,
					 24);
      pil+=2;    // Re Im
      rxF+=24;   // remember replicated format (Re0 Im0 Re0 Im0) !!!
      dl_ch+=8;
      
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
      // printf("pilot 50: rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);
      multadd_real_vector_complex_scalar(fr,
					 ch,
					 dl_ch,
					 24);


    }
    
    else if (frame_parms->N_RB_DL==25) {
      //printf("Channel estimation\n");

      // Treat first 2 pilots specially (left edge)
      ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
      ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
      printf("pilot 0 : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
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
	// printf("rx[%d][%d] -> (%d,%d)\n",p,frame_parms->first_carrier_offset + frame_parms->nushift + 6*rb+(3*p),rxF[0],rxF[1]);

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);

#ifdef DEBUG_CH
      printf("pilot %d : rxF - > (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",pilot_cnt,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
#endif

                 
      multadd_real_vector_complex_scalar(f,
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

      ch[0] = 1024;
      ch[1] = 0;
#endif

      multadd_real_vector_complex_scalar(f2,
					 ch,
					 dl_ch,
					 24);
      pil+=2;
      rxF+=24;
      dl_ch+=16;
      
      for (pilot_cnt=0;pilot_cnt<22;pilot_cnt+=2) {

	// printf("* pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	// printf("rx[%d][%d] -> (%d,%d)\n",p,frame_parms->first_carrier_offset + frame_parms->nushift + 6*rb+(3*p),rxF[0],rxF[1]);

	ch[0] = (short)(((int)pil[0]*rxF[0] - (int)pil[1]*rxF[1])>>15);
	ch[1] = (short)(((int)pil[0]*rxF[1] + (int)pil[1]*rxF[0])>>15);
#ifdef DEBUG_CH
	printf("pilot %d rxF -> (%d,%d) ch -> (%d,%d), pil -> (%d,%d) \n",26+pilot_cnt,rxF[0],rxF[1],ch[0],ch[1],pil[0],pil[1]);	

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
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

      ch[0] = 1024;
      ch[1] = 0;
#endif

      multadd_real_vector_complex_scalar(f2r2,
					 ch,
					 dl_ch,
					 24);

    }
    else if (frame_parms->N_RB_DL==15) {
      
      //printf("First Half\n");
      for (rb=0;rb<28;rb+=4) {
	
	//printf("aarx=%d\n",aarx);
	//printf("pilot[%d][%d] (%d,%d)\n",p,rb,pil[0],pil[1]);
	//printf("rx[%d][%d] -> (%d,%d)\n",p,
	//       frame_parms->first_carrier_offset + frame_parms->nushift + 6*rb+(3*p),
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
      rxF   = (short *)&rxdataF[aarx][((symbol_offset+1+frame_parms->nushift + (3*p))<<1)]; 
      
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
	//       frame_parms->first_carrier_offset + frame_parms->nushift + 6*rb+(3*p),
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
      msg("channel estimation not implemented for frame_parms->N_RB_DL = %d\n",frame_parms->N_RB_DL);
    }
    
    
    
    // Temporal Interpolation
    // printf("ch_offset %d\n",ch_offset);
    
    dl_ch = (short *)&dl_ch_estimates[(p<<1)+aarx][ch_offset];
    if (ch_offset == 0) {
      dl_ch_prev = (short *)&dl_ch_estimates[(p<<1)+aarx][(4-frame_parms->Ncp)*(frame_parms->ofdm_symbol_size)];
      
      multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),1,frame_parms->ofdm_symbol_size);
      multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),0,frame_parms->ofdm_symbol_size);
      
      multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*((frame_parms->ofdm_symbol_size)<<1)),1,frame_parms->ofdm_symbol_size);
      multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*((frame_parms->ofdm_symbol_size)<<1)),0,frame_parms->ofdm_symbol_size);
    } // this is 1/3,2/3 combination for pilots spaced by 3 symbols
    
    else {
      //printf("Interpolating 0->%d\n",4-frame_parms->Ncp);
      
      dl_ch_prev = (short *)&dl_ch_estimates[(p<<1)+aarx][0];
      if (frame_parms->Ncp==0) {// pilot spacing 4 symbols (1/4,1/2,3/4 combination)
	multadd_complex_vector_real_scalar(dl_ch_prev,24576,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),1,frame_parms->ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,8192,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),0,frame_parms->ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,16384,dl_ch_prev+(2*((frame_parms->ofdm_symbol_size)<<1)),1,frame_parms->ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,16384,dl_ch_prev+(2*((frame_parms->ofdm_symbol_size)<<1)),0,frame_parms->ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,8192,dl_ch_prev+(3*2*(frame_parms->ofdm_symbol_size)),1,frame_parms->ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,24576,dl_ch_prev+(3*2*(frame_parms->ofdm_symbol_size)),0,frame_parms->ofdm_symbol_size);
      }
      else {
	multadd_complex_vector_real_scalar(dl_ch_prev,10923,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),1,frame_parms->ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,21845,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)),0,frame_parms->ofdm_symbol_size);
	
	multadd_complex_vector_real_scalar(dl_ch_prev,21845,dl_ch_prev+(2*(frame_parms->ofdm_symbol_size)<<1),1,frame_parms->ofdm_symbol_size);
	multadd_complex_vector_real_scalar(dl_ch,10923,dl_ch_prev+(2*((frame_parms->ofdm_symbol_size)<<1)),0,frame_parms->ofdm_symbol_size);
      } // pilot spacing 3 symbols (1/3,2/3 combination)
    }
    
    
  }
  return(0); 
}

