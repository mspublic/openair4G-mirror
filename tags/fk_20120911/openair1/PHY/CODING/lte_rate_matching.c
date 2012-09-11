/* file: lte_rate_matching.c
   purpose: Procedures for rate matching/interleaving for LTE (turbo-coded transport channels) (TX/RX)
   author: raymond.knopp@eurecom.fr
   date: 21.10.2009 
*/
#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#endif
#include "PHY/defs.h" 

//#define cmin(a,b) ((a)<(b) ? (a) : (b))

static u32 bitrev[32] = {0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31};
static u32 bitrev_cc[32] = {1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31,0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30};

//#define RM_DEBUG 1
//#define RM_DEBUG2 1
// #define RM_DEBUG_CC 1
 
u32 sub_block_interleaving_turbo(u32 D, u8 *d,u8 *w) {

  u32 RTC = (D>>5), ND, ND3;
  u32 row,col,Kpi,index;
  u32 index3,k;
  #ifdef RM_DEBUG
  u32 nulled=0;
  #endif

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
  #ifdef RM_DEBUG
  printf("sub_block_interleaving_turbo : D = %d (%d)\n",D,D*3);
  printf("RTC = %d, Kpi=%d, ND=%d\n",RTC,Kpi,ND);
  #endif
  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  d[(3*D)+2] = d[2];
  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG
    printf("Col %d\n",col);
#endif
    index = bitrev[col];
    index3 = 3*index;
    for (row=0;row<RTC;row++) {

      w[k]   =  d[index3-ND3];
      w[Kpi+(k<<1)] =  d[index3-ND3+1];
      w[Kpi+1+(k<<1)] =  d[index3-ND3+5]; 
#ifdef RM_DEBUG
      printf("row %d, index %d, index-Nd %d (k,Kpi+2k,Kpi+2k+1) (%d,%d,%d) w(%d,%d,%d)\n",row,index,index-ND,k,Kpi+(k<<1),Kpi+(k<<1)+1,w[k],w[Kpi+(k<<1)],w[Kpi+1+(k<<1)]);
      
      if (w[k]== LTE_NULL)
	nulled++;
      if (w[Kpi+(k<<1)] ==LTE_NULL)
	nulled++;
      if (w[Kpi+1+(k<<1)] ==LTE_NULL)
	nulled++;
      
#endif
      index3+=96;
      index+=32;
      k++;
    }      
  }
  #ifdef RM_DEBUG
  printf("RM_TX: Nulled %d\n",nulled);
  #endif
  return(RTC);
}


u32 sub_block_interleaving_cc(u32 D, u8 *d,u8 *w) {

  u32 RCC = (D>>5), ND, ND3;
  u32 row,col,Kpi,index;
  u32 index3,k;
#ifdef RM_DEBUG_CC
  u32 nulled=0;
#endif

  if ((D&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG_CC
  printf("sub_block_interleaving_cc : D = %d (%d), d %p, w %p\n",D,D*3,d,w);
  printf("RCC = %d, Kpi=%d, ND=%d\n",RCC,Kpi,ND);
#endif
  ND3 = ND*3;

  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG_CC
    printf("Col %d\n",col);
#endif
    index = bitrev_cc[col];
    index3 = 3*index;
    for (row=0;row<RCC;row++) {
      w[k]          =  d[index3-ND3];
      w[Kpi+k]     =   d[index3-ND3+1];
      w[(Kpi<<1)+k] =  d[index3-ND3+2]; 
#ifdef RM_DEBUG_CC
      printf("row %d, index %d k %d w(%d,%d,%d)\n",row,index,k,w[k],w[Kpi+k],w[(Kpi<<1)+k]);
      
      if (w[k]== LTE_NULL)
	nulled++;
      if (w[Kpi+k] ==LTE_NULL)
	nulled++;
      if (w[(Kpi<<1)+k] ==LTE_NULL)
	nulled++;
      
#endif
      index3+=96;
      index+=32;
      k++;
    }      
  }
#ifdef RM_DEBUG_CC
  printf("RM_TX: Nulled %d\n",nulled);
#endif
  return(RCC);
}

void sub_block_deinterleaving_turbo(u32 D,s16 *d,s16 *w) {

  u32 RTC = (D>>5), ND, ND3;
  u32 row,col,Kpi,index;
  u32 index3,k;

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG2
  printf("sub_block_interleaving_turbo : D = %d (%d)\n",D,D*3);
  printf("RTC = %d, Kpi=%d, ND=%d\n",RTC,Kpi,ND);
#endif
  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG2
    printf("Col %d\n",col);
#endif
    index = bitrev[col];
    index3 = 3*index;
    for (row=0;row<RTC;row++) {

      d[index3-ND3]   = w[k];
      d[index3-ND3+1] = w[Kpi+(k<<1)];  
      d[index3-ND3+5] = w[Kpi+1+(k<<1)];  
      index3+=96;
      index+=32;
      k++;
    }      
  }
  d[2] = d[(3*D)+2];

}

void sub_block_deinterleaving_cc(u32 D,s8 *d,s8 *w) {

  u32 RCC = (D>>5), ND, ND3;
  u32 row,col,Kpi,index;
  u32 index3,k;

  if ((D&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG2
  printf("sub_block_interleaving_cc : D = %d (%d), d %p, w %p\n",D,D*3,d,w);
  printf("RCC = %d, Kpi=%d, ND=%d\n",RCC,Kpi,ND);
#endif
  ND3 = ND*3;

  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG2
    printf("Col %d\n",col);
#endif
    index = bitrev_cc[col];
    index3 = 3*index;
    for (row=0;row<RCC;row++) {

      d[index3-ND3]   = w[k];
      d[index3-ND3+1] = w[Kpi+k];  
      d[index3-ND3+2] = w[(Kpi<<1)+k];  
#ifdef RM_DEBUG2
      printf("row %d, index %d k %d index3-ND3 %d w(%d,%d,%d)\n",row,index,k,index3-ND3,w[k],w[Kpi+k],w[(Kpi<<1)+k]);
#endif
      index3+=96;
      index+=32;
      k++;
    }      
  }

}

u32 generate_dummy_w(u32 D, u8 *w,u8 F) {

  u32 RTC = (D>>5), ND;
  u32 col,Kpi,index;
  s32 k;
#ifdef RM_DEBUG
  u32 nulled=0;
#endif

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG
  printf("dummy sub_block_interleaving_turbo : D = %d (%d)\n",D,D*3);
  printf("RTC = %d, Kpi=%d, ND=%d, F=%d (Nulled %d)\n",RTC,Kpi,ND,F,(2*F + 3*ND));
#endif
  //  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  k=0;

  for (col=0;col<32;col++) {
#ifdef RM_DEBUG
    printf("Col %d\n",col);
#endif
    index = bitrev[col];
    
    if (index<ND+F) {
      w[k]   =  LTE_NULL;
      w[Kpi+(k<<1)] = LTE_NULL;
#ifdef RM_DEBUG
      nulled+=2;
#endif
    }

    //bits beyond 32 due to "filler" bits
    if (index+32<ND+F) {
      w[k+1]   =  LTE_NULL;
      w[Kpi+2+(k<<1)] = LTE_NULL;
#ifdef RM_DEBUG
      nulled+=2;
#endif
    }
    if (index+64<ND+F) {
      w[k+2]   =  LTE_NULL;
      w[Kpi+4+(k<<1)] = LTE_NULL;
#ifdef RM_DEBUG
      nulled+=2;
#endif
    }

    if ((index+1)<ND) {
      w[Kpi+1+(k<<1)] =  LTE_NULL;
#ifdef RM_DEBUG
      nulled+=1;
#endif
    }
#ifdef RM_DEBUG
    printf("k %d w (%d,%d,%d) w+1 (%d,%d,%d), index-ND-F %d index+32-ND-F %d\n",k,w[k],w[Kpi+(k<<1)],w[Kpi+1+(k<<1)],w[k+1],w[2+Kpi+(k<<1)],w[2+Kpi+1+(k<<1)],index-ND-F,index+32-ND-F);
#endif
    k+=RTC;
  }

#ifdef RM_DEBUG
  printf("Nulled = %d\n",nulled);
#endif
  return(RTC);
}
 
u32 generate_dummy_w_cc(u32 D, u8 *w){

  u32 RCC = (D>>5), ND;
  u32 col,Kpi,index;
  s32 k;
#ifdef RM_DEBUG_CC
  u32 nulled=0;
#endif

  if ((D&0x1f) > 0)
    RCC++;
  Kpi = (RCC<<5);
  //  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG_CC
  printf("dummy sub_block_interleaving_cc : D = %d (%d)\n",D,D*3);
  printf("RCC = %d, Kpi=%d, ND=%d, (Nulled %d)\n",RCC,Kpi,ND,3*ND);
#endif
  //  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  k=0;

  for (col=0;col<32;col++) {
#ifdef RM_DEBUG_CC
    printf("Col %d\n",col);
#endif
    index = bitrev_cc[col];
    
    if (index<ND) {
      w[k]          = LTE_NULL;
      w[Kpi+k]      = LTE_NULL;
      w[(Kpi<<1)+k] = LTE_NULL;
#ifdef RM_DEBUG_CC
      nulled+=3;
#endif
    }
    /*
    //bits beyond 32 due to "filler" bits
    if (index+32<ND) {
      w[k+1]          = LTE_NULL;
      w[Kpi+1+k]      = LTE_NULL;
      w[(Kpi<<1)+1+k] = LTE_NULL;
#ifdef RM_DEBUG
      nulled+=3;
#endif
    }
    if (index+64<ND) {
      w[k+2]          = LTE_NULL;
      w[Kpi+2+k]      = LTE_NULL;
      w[(Kpi<<1)+2+k] = LTE_NULL;
#ifdef RM_DEBUG
      nulled+=3;
#endif
    }


    if ((index+1)<ND) {
      w[Kpi+1+(k<<1)] =  LTE_NULL;
#ifdef RM_DEBUG
      nulled+=1;
#endif
    }
*/
#ifdef RM_DEBUG_CC
    printf("k %d w (%d,%d,%d), index-ND %d index+32-ND %d\n",k,w[k],w[Kpi+k],w[(Kpi<<1)+k],index-ND,index+32-ND);
#endif
    k+=RCC;
  }

#ifdef RM_DEBUG_CC
  printf("Nulled = %d\n",nulled);
#endif
  return(RCC);
}


u32 lte_rate_matching_turbo(u32 RTC,
				     u32 G, 
			             u8 *w,
				     u8 *e, 
				     u8 C, 
				     u32 Nsoft, 
				     u8 Mdlharq,
				     u8 Kmimo,
				     u8 rvidx,
				     u8 Qm, 
				     u8 Nl, 
				     u8 r) {
  
  
  u32 Nir,Ncb,Gp,GpmodC,E,Ncbmod,ind,k;
  int cnt=0;
  u8 *e2;
  #ifdef RM_DEBUG
  u32 nulled=0;
  #endif

  Nir = Nsoft/Kmimo/cmin(8,Mdlharq);
  Ncb = cmin(Nir/C,3*(RTC<<5));

  if (Ncb<(3*(RTC<<5))) {
    msg("Exiting, RM condition (Nir %d, Nsoft %d, Kw %d\n",Nir,Nsoft,3*(RTC<<5));
    return(0);
  }
  Gp = G/Nl/Qm;
  GpmodC = Gp%C;

  #ifdef RM_DEBUG
  printf("lte_rate_matching_turbo: Kw %d, rvidx %d, G %d, Qm %d, Nl%d, r %d\n",3*(RTC<<5),rvidx, G, Qm,Nl,r);
  #endif

  if (r < (C-(GpmodC)))
    E = Nl*Qm * (Gp/C);
  else
    E = Nl*Qm * ((GpmodC==0?0:1) + (Gp/C));

  Ncbmod = Ncb%(RTC<<3);

  ind = RTC * (2+(rvidx*(((Ncbmod==0)?0:1) + (Ncb/(RTC<<3)))*2));

  #ifdef RM_DEBUG
  printf("lte_rate_matching_turbo: E %d, k0 %d, Ncbmod %d, Ncb/(RTC<<3) %d\n",E,ind,Ncbmod,Ncb/(RTC<<3));
  #endif

  e2=e+(r*E);

  for (k=0;k<E;k++) {


    while(w[ind] == LTE_NULL) {
      #ifdef RM_DEBUG
      printf("RM_tx : ind %d, NULL\n",ind);
      nulled++;
      #endif
      ind++;
      if (ind==Ncb)
	ind=0;
    }

    e2[k] = w[ind];
    cnt = cnt+1;
    #ifdef RM_DEBUG
    //    printf("k %d ind %d, w %c(%d)\n",k,ind,w[ind],w[ind]);
     printf("RM_TX %d (%d) Ind: %d (%d)\n",k,k+r*E,ind,e2[k]);
    #endif
    ind++;
    if (ind==Ncb)
      ind=0;
  }

  #ifdef RM_DEBUG
  printf("nulled %d\n",nulled);
  #endif
  return(E);
}


u32 lte_rate_matching_cc(u32 RCC,
				  u16 E,
				  u8 *w,
				  u8 *e) {

  
  u32 ind=0,k;

  u16 Kw = 3*(RCC<<5);

#ifdef RM_DEBUG_CC
  u32 nulled=0;

  printf("lte_rate_matching_cc: Kw %d, E %d\n",Kw, E);
#endif

  for (k=0;k<E;k++) {


    while(w[ind] == LTE_NULL) {

#ifdef RM_DEBUG_CC
      nulled++;
      printf("RM_TX_CC : ind %d, NULL\n",ind);
#endif
      ind++;
      if (ind==Kw)
	ind=0;
    }


    e[k] = w[ind];
#ifdef RM_DEBUG_CC
//    printf("k %d ind %d, w %c(%d)\n",k,ind,w[ind],w[ind]);
    printf("RM_TX_CC %d Ind: %d (%d)\n",k,ind,e[k]);
#endif
    ind++;
    if (ind==Kw)
      ind=0;
  }
#ifdef RM_DEBUG_CC
  printf("nulled %d\n",nulled);
#endif
  return(E);
}


int lte_rate_matching_turbo_rx(u32 RTC,
			       u32 G, 
			       s16 *w,
			       u8 *dummy_w,
			       s16 *soft_input, 
			       u8 C, 
			       u32 Nsoft, 
			       u8 Mdlharq,
			       u8 Kmimo,
			       u8 rvidx,
			       u8 clear,
			       u8 Qm, 
			       u8 Nl, 
			       u8 r,
			       u32 *E_out) {
  
  
  u32 Nir,Ncb,Gp,GpmodC,E,Ncbmod,ind,k;
  s16 *soft_input2;
  s32 w_tmp;

  if (Kmimo==0 || Mdlharq==0 || C==0 || Qm==0 || Nl==0) {
    msg("lte_rate_matching.c: invalid paramters\n");
    return(-1);
  }

  Nir = Nsoft/Kmimo/cmin(8,Mdlharq);
  Ncb = cmin(Nir/C,3*(RTC<<5));
  

  Gp = G/Nl/Qm;
  GpmodC = Gp%C;



  if (r < (C-(GpmodC)))
    E = Nl*Qm * (Gp/C);
  else
    E = Nl*Qm * ((GpmodC==0?0:1) + (Gp/C));

  Ncbmod = Ncb%(RTC<<3);

  ind = RTC * (2+(rvidx*(((Ncbmod==0)?0:1) + (Ncb/(RTC<<3)))*2));

#ifdef RM_DEBUG
  printf("lte_rate_matching_turbo_rx: Clear %d, E %d, Ncb %d, Kw %d, rvidx %d, G %d, Qm %d, Nl%d, r %d\n",clear,E,Ncb,3*(RTC<<5),rvidx, G, Qm,Nl,r);
#endif

  if (clear==1)
    memset(w,0,Ncb*sizeof(s16));
 
  soft_input2 = soft_input + (r*E);
 
  for (k=0;k<E;k++) {


    while(dummy_w[ind] == LTE_NULL) {
#ifdef RM_DEBUG
      printf("RM_rx : ind %d, NULL\n",ind);
#endif
      ind++;
      if (ind==Ncb)
	ind=0;
    }
    /*
    if (w[ind] != 0)
      printf("repetition %d (%d,%d,%d)\n",ind,rvidx,E,Ncb);
    */
    // Maximum-ratio combining of repeated bits and retransmissions
    /*
    w_tmp = (int) w[ind] + (int) soft_input2[k];
    if (w_tmp > 32767) {
#ifdef DEBUG_RM
      printf("OVERFLOW!!!!!, w_tmp = %d\n",w_tmp);
#endif
      w[ind] = 32767;
    }
    else if (w_tmp < -32768) {
#ifdef DEBUG_RM
      printf("UNDERFLOW!!!!!, w_tmp = %d\n",w_tmp);
#endif
      w[ind] = -32768;
    }
    else
    */
    w[ind] += soft_input2[k];
#ifdef RM_DEBUG
      printf("RM_RX k%d Ind: %d (%d)\n",k,ind,w[ind]);
#endif
    ind++;
    if (ind==Ncb)
      ind=0;
  }

  *E_out = E;
  return(0);

}

u32 lte_rate_matching_turbo_rx_abs(u32 RTC,
			       u32 G, 
			       double *w,
			       u8 *dummy_w,
			       double *soft_input, 
			       u8 C, 
			       u32 Nsoft, 
			       u8 Mdlharq,
			       u8 Kmimo,
			       u8 rvidx,
			       u8 clear,
			       u8 Qm, 
			       u8 Nl, 
			       u8 r,
			       u32 *E_out) {
  
  
  u32 Nir,Ncb,Gp,GpmodC,E,Ncbmod,ind,k,start_ind;
  double *soft_input2;
  double w_tmp;
  u32 nulled=0;
  int stop_counting_null=0;
  if (Kmimo==0 || Mdlharq==0 || C==0 || Qm==0 || Nl==0) {
    msg("lte_rate_matching.c: invalid paramters\n");
    return(-1);
  }

  Nir = Nsoft/Kmimo/cmin(8,Mdlharq);
  Ncb = cmin(Nir/C,3*(RTC<<5));
  

  Gp = G/Nl/Qm;
  GpmodC = Gp%C;



  if (r < (C-(GpmodC)))
    E = Nl*Qm * (Gp/C);
  else
    E = Nl*Qm * ((GpmodC==0?0:1) + (Gp/C));

  Ncbmod = Ncb%(RTC<<3);
  
  ind = RTC * (2+(rvidx*(((Ncbmod==0)?0:1) + (Ncb/(RTC<<3)))*2));
  start_ind=ind;
#ifdef RM_DEBUG
  printf("lte_rate_matching_turbo_rx: Clear %d, E %d, Ncb %d, Kw %d, rvidx %d, G %d, Qm %d, Nl%d, r %d\n",clear,E,Ncb,3*(RTC<<5),rvidx, G, Qm,Nl,r);
#endif

  if (clear==1)
    memset(w,0,Ncb*sizeof(s16));
 
  soft_input2 = soft_input + (r*E);
 
  for (k=0;k<E;k++) {


    while(dummy_w[ind] == LTE_NULL) {
#ifdef RM_DEBUG
      printf("RM_rx : ind %d, NULL\n",ind);
#endif
      if(stop_counting_null==0 || (ind<start_ind && stop_counting_null==1) ){
      nulled++;
      }
      
      ind++;
      
      
      if (ind==Ncb){
	ind=0;
	stop_counting_null+=1;
      }
    }

      w[ind] = w[ind] + soft_input2[k] - w[ind]*soft_input2[k];
#ifdef RM_DEBUG
      printf("RM_RX k%d Ind: %d (%d)\n",k,ind,w[ind]);
#endif
    ind++;
    if (ind==Ncb){
      ind=0;
    stop_counting_null+=1;
    }
  }

  *E_out = E;
  return(start_ind);

}


void lte_rate_matching_cc_rx(u32 RCC,
			     u16 E, 
			     s8 *w,
			     u8 *dummy_w,
			     s8 *soft_input) {

  
  
  u32 ind=0,k;
  u16 Kw = 3*(RCC<<5);
  u32 acc=1;
  s16 w16[Kw];
#ifdef RM_DEBUG_CC
  u32 nulled=0;

  printf("lte_rate_matching_cc_rx: Kw %d, E %d, w %p, soft_input %p\n",3*(RCC<<5),E,w,soft_input);
#endif


  memset(w,0,Kw);
  memset(w16,0,Kw*sizeof(s16));

  for (k=0;k<E;k++) {


    while(dummy_w[ind] == LTE_NULL) {
#ifdef RM_DEBUG_CC
      nulled++;
      printf("RM_RX : ind %d, NULL\n",ind);
#endif
      ind++;
      if (ind==Kw)
	ind=0;
    }
    /*
    if (w[ind] != 0)
      printf("repetition %d (%d,%d,%d)\n",ind,rvidx,E,Ncb);
    */
    // Maximum-ratio combining of repeated bits and retransmissions
#ifdef RM_DEBUG_CC
      printf("RM_RX_CC k %d (%d) ind: %d (%d)\n",k,soft_input[k],ind,w16[ind]);
#endif


      w16[ind] += soft_input[k];

      ind++;
      if (ind==Kw) {
	ind=0;
	acc++;
      }
  }
  // rescale
  for (ind=0;ind<Kw;ind++) {
    //    w16[ind]=(w16[ind]/acc);
    if (w16[ind]>7)
      w[ind]=7;
    else if (w16[ind]<-8)
      w[ind]=-8;
    else
      w[ind]=(s8)w16[ind];
  }
#ifdef RM_DEBUG_CC

  printf("Nulled %d\n",nulled);
#endif
}


#ifdef MAIN

void main() {
  u8 d[96+3+(3*6144)];
  u8 w[3*6144],e[12*6144];
  u32 RTC,G,rvidx;
  u32 nb_rb=6;
  u32 mod_order = 4;
  u32 first_dlsch_symbol = 2;
  u32 i;

  G = ( nb_rb * (12 * mod_order) * (12-first_dlsch_symbol-3)) ;//( nb_rb * (12 * mod_order) * (14-first_dlsch_symbol-3)) :
    
  // initialize 96 first positions to "LTE_NULL"
  for (i=0;i<96;i++)
    d[i]=LTE_NULL;

  RTC = sub_block_interleaving_turbo(4+(192*8), &d[96], w);
  for (rvidx=0;rvidx<4;rvidx++) {
    lte_rate_matching_turbo(RTC,
			G, 
			w,
			e, 
			1,           //C 
			1827072,     //Nsoft, 
			8,           //Mdlharq,
			1,           //Kmimo,
			rvidx,       //rvidx,
			mod_order,   //Qm, 
			1,           //Nl, 
			0            //r
			); 
  }    
}

#endif

