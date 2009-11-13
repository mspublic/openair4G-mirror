#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#endif
#include "PHY/LTE_TRANSPORT/defs.h"

#define min(a,b) ((a)<(b) ? (a) : (b))
static unsigned int bitrev[32] = {0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31};


unsigned int sub_block_interleaving_turbo(unsigned int D, unsigned char *d,unsigned char *w) {

  unsigned int RTC = (D>>5), ND, ND3;
  unsigned int row,col,Kpi,Kpi3,index;
  int index3,k;

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  Kpi3 = Kpi*3;
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
      printf("row %d, index %d k %d w(%d,%d,%d)\n",row,index,k,w[k],w[Kpi+(k<<1)],w[Kpi+1+(k<<1)]);
      /*
      if (w[index]!= LTE_NULL)
	w[index] = 's';
      if (w[Kpi+(index<<1)] !=LTE_NULL)
	w[Kpi+(index<<1)] = 'p';
      if (w[Kpi+1+(index<<1)] !=LTE_NULL)
	w[Kpi+1+(index<<1)] = 'q';
      */
#endif
      index3+=96;
      index+=32;
      k++;
    }      
  }
  return(RTC);
}


void sub_block_deinterleaving_turbo(unsigned int D,short *d,short *w) {

  unsigned int RTC = (D>>5), ND, ND3;
  unsigned int row,col,Kpi,Kpi3,index;
  int index3,k;

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG
  printf("sub_block_interleaving_turbo : D = %d (%d)\n",D,D*3);
  printf("RTC = %d, Kpi=%d, ND=%d\n",RTC,Kpi,ND);
#endif
  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG
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

unsigned int generate_dummy_w(unsigned int D, unsigned char *w) {

  unsigned int RTC = (D>>5), ND, ND3;
  unsigned int row,col,Kpi,Kpi3,index;
  int index3,k;

  if ((D&0x1f) > 0)
    RTC++;
  Kpi = (RTC<<5);
  Kpi3 = Kpi*3;
  ND = Kpi - D;
#ifdef RM_DEBUG
  printf("dummy sub_block_interleaving_turbo : D = %d (%d)\n",D,D*3);
  printf("RTC = %d, Kpi=%d, ND=%d\n",RTC,Kpi,ND);
#endif
  ND3 = ND*3;

  // copy d02 to dD2 (for mod Kpi operation from clause (4), p.16 of 36.212
  k=0;
  for (col=0;col<32;col++) {
#ifdef RM_DEBUG
    printf("Col %d\n",col);
#endif
    index = bitrev[col];

    if (index<ND) {
      w[k]   =  LTE_NULL;
      w[Kpi+(k<<1)] = LTE_NULL;
    }
    if ((index+1)<ND)
      w[Kpi+1+(k<<1)] =  LTE_NULL;
    printf("k %d w (%d,%d,%d), index %d\n",k,w[k],w[Kpi+(k<<1)],w[Kpi+1+(k<<1)],index-ND);
    k+=RTC;
  }
  return(RTC);
}


unsigned int lte_rate_matching_turbo(unsigned int RTC,
				     unsigned int G, 
				     unsigned char *w,
				     unsigned char *e, 
				     unsigned char C, 
				     unsigned int Nsoft, 
				     unsigned char Mdlharq,
				     unsigned char Kmimo,
				     unsigned char rvidx,
				     unsigned char Qm, 
				     unsigned char Nl, 
				     unsigned char r) {
  
  
  unsigned int Nir,Ncb,Gp,GpmodC,E,Ncbmod,ind,Nbmod,k;
  
  Nir = Nsoft/Kmimo/min(8,Mdlharq);
  Ncb = min(Nir/C,3*(RTC<<5));


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

  ind = RTC * (2+(rvidx*(((Nbmod==0)?0:1) + (Ncb/(RTC<<3)))*2));

#ifdef RM_DEBUG
  printf("lte_rate_matching_turbo: E %d, k0 %d, Nbmod %d, Ncb/(RTC<<3) %d\n",E,ind,Nbmod,Ncb/(RTC<<3));
#endif

  for (k=0;k<E;k++) {

    while(w[ind] == 2) {
      printf("ind %d, NULL\n",ind);
      ind++;
      if (ind==Ncb)
	ind=0;
    }
#ifdef RM_DEBUG
    printf("k %d ind %d, w %c(%d)\n",k,ind,w[ind],w[ind]);
#endif
    e[k] = w[ind];
    ind++;
    if (ind==Ncb)
      ind=0;
  }
  return(E);
}


unsigned int lte_rate_matching_turbo_rx(unsigned int RTC,
					unsigned int G, 
					short *w,
					unsigned char *dummy_w,
					short *soft_input, 
					unsigned char C, 
					unsigned int Nsoft, 
					unsigned char Mdlharq,
					unsigned char Kmimo,
					unsigned char rvidx,
					unsigned char Qm, 
					unsigned char Nl, 
					unsigned char r) {
  
  
  unsigned int Nir,Ncb,Gp,GpmodC,E,Ncbmod,ind,Nbmod,k;
  
  Nir = Nsoft/Kmimo/min(8,Mdlharq);
  Ncb = min(Nir/C,3*(RTC<<5));
  

  Gp = G/Nl/Qm;
  GpmodC = Gp%C;

#ifdef RM_DEBUG
  printf("lte_rate_matching_turbo_rx: Kw %d, rvidx %d, G %d, Qm %d, Nl%d, r %d\n",3*(RTC<<5),rvidx, G, Qm,Nl,r);
#endif

  if (r < (C-(GpmodC)))
    E = Nl*Qm * (Gp/C);
  else
    E = Nl*Qm * ((GpmodC==0?0:1) + (Gp/C));

  Ncbmod = Ncb%(RTC<<3);

  ind = RTC * (2+(rvidx*(((Nbmod==0)?0:1) + (Ncb/(RTC<<3)))*2));

#ifdef RM_DEBUG
  printf("lte_rate_matching_turbo_rx: RTC %d, E %d, k0 %d, Nbmod %d, Ncb/(RTC<<3) %d\n",RTC,E,ind,Nbmod,Ncb/(RTC<<3));
#endif

  for (k=0;k<E;k++) {

    while(dummy_w[ind] == LTE_NULL) {
      printf("ind %d, NULL\n",ind);
      ind++;
      if (ind==Ncb)
	ind=0;
    }

    // Maximum-ratio combining of repeated bits and retransmissions
    w[ind] += soft_input[k];
#ifdef RM_DEBUG
    printf("lte_rate_matching_turbo_rx: ind %d, w %d\n",ind,w[ind]);
#endif
    ind++;
    if (ind==Ncb)
      ind=0;
  }
  return(E);

}


#ifdef MAIN



void main() {
  unsigned char d[96+3+(3*6144)];
  unsigned char w[3*6144],e[12*6144];
  unsigned int RTC,G,rvidx;
  unsigned int nb_rb=6;
  unsigned int mod_order = 4;
  unsigned int first_dlsch_symbol = 2;
  unsigned int i;

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

