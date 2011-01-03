#include "PHY/defs.h"
#include "PHY/extern.h"

//u8 ncs_cell[20][7];

#define DEBUG_PUCCH

void init_ncs_cell(LTE_DL_FRAME_PARMS *frame_parms,u8 ncs_cell[20][7]) {

  u8 ns,l,reset=1,i,N_UL_symb;
  u32 x1,x2,j=0,s;
  
  N_UL_symb = (frame_parms->Ncp==0) ? 7 : 6;
  x2 = frame_parms->Nid_cell;

  for (ns=0;ns<20;ns++) {

    for (l=0;l<N_UL_symb;l++) {
      ncs_cell[ns][l]=0;

      for (i=0;i<8;i++) {
	if ((j%32) == 0) {
	  s = lte_gold_generic(&x1,&x2,reset);
	  //	  printf("s %x\n",s);
	  reset=0;
	}
	if (((s>>(j%32))&1)==1)
	  ncs_cell[ns][l] += (1<<i);
	j++;
      }
#ifdef DEBUG_PUCCH
      printf("[PHY] PUCCH ncs_cell init (j %d): Ns %d, l %d => ncs_cell %d\n",j,ns,l,ncs_cell[ns][l]);
#endif
    }

  }
}

s16 alpha_re[12] = {32767, 28377, 16383,     0,-16384,  -28378,-32768,-28378,-16384,    -1, 16383, 28377};
s16 alpha_im[12] = {0,     16383, 28377, 32767, 28377,   16383,     0,-16384,-28378,-32768,-28378,-16384};

s16 W4[3][4] = {{32767, 32767, 32767, 32767},
		{32767,-32768, 32767,-32768},
		{32767,-32768,-32768, 32767}};
s16 W3_re[3][6] = {{32767, 32767, 32767},
		   {32767,-16384,-16384},
		   {32767,-16384,-16384}};

s16 W3_im[3][6] = {{0    ,0     ,0     },
		   {0    , 28377,-28378},
		   {0    ,-28378, 28377}};

void generate_pucch(LTE_DL_FRAME_PARMS *frame_parms,
		    u8 ncs_cell[20][7],
		    PUCCH_FMT_t fmt,
		    u8 deltaPUCCH_Shift,
		    u8 NRB2,
		    u8 Ncs1_div_deltaPUCCH_Shift,
		    u16 n1_pucch,
		    u16 n2_pucch,
		    u8 shortened_format,
		    u32 payload,
		    s16 amp,
		    u8 subframe) {
  
  u8 u = frame_parms->Nid_cell % 30,n;
  u32 z[12*14],*zptr;
  s16 d0;
  u8 ns,N_UL_symb,n_oc,n_oc0,n_oc1;
  u8 c = (frame_parms->Ncp==0) ? 3 : 2;
  u16 nprime,nprime0,nprime1;
  u16 thres,h;
  u8 Nprime_div_deltaPUCCH_Shift,Nprime,d;
  u8 m,refs;
  u8 n_cs,S,alpha_ind,rem;
  s16 tmp_re,tmp_im,ref_re,ref_im,W_re=0,W_im=0;

  if ((deltaPUCCH_Shift==0) || (deltaPUCCH_Shift>3)) {
    msg("[PHY] generate_pucch: Illegal deltaPUCCH_shift %d (should be 1,2,3)\n",deltaPUCCH_Shift);
    return;
  }

  if (NRB2 > 2047) {
    msg("[PHY] generate_pucch: Illegal NRB2 %d (should be 0...2047)\n",NRB2);
    return;
  }

  if (Ncs1_div_deltaPUCCH_Shift > 7) {
    msg("[PHY] generate_pucch: Illegal Ncs1_div_deltaPUCCH_Shift %d (should be 0...7)\n",Ncs1_div_deltaPUCCH_Shift);
    return;
  }

  zptr = z;
  thres = (c*Ncs1_div_deltaPUCCH_Shift);
  Nprime_div_deltaPUCCH_Shift = (n1_pucch < thres) ? Ncs1_div_deltaPUCCH_Shift : (12/deltaPUCCH_Shift);
  Nprime = Nprime_div_deltaPUCCH_Shift * deltaPUCCH_Shift;

#ifdef DEBUG_PUCCH
  printf("[PHY] PUCCH: cNcs1/deltaPUCCH_Shift %d, Nprime %d\n",thres,Nprime);
#endif

  N_UL_symb = (frame_parms->Ncp==0) ? 7 : 6;

  if (n1_pucch < thres)
    nprime0=n1_pucch;
  else
    nprime0 = (n1_pucch - thres)%thres; 
  
  if (n1_pucch >= thres)
    nprime1= ((c*(nprime0+1))%(thres+1))-1;
  else {
    d = (frame_parms->Ncp==0) ? 2 : 0;
    h= (nprime0+d)%(c*Nprime_div_deltaPUCCH_Shift);
    nprime1 = (h/c) + (h%c)*Nprime_div_deltaPUCCH_Shift; 
  }

#ifdef DEBUG_PUCCH
  printf("[PHY] PUCCH: nprime0 %d nprime1 %d\n",nprime0,nprime1);
#endif

  n_oc0 = nprime0/Nprime_div_deltaPUCCH_Shift;
  if (frame_parms->Ncp==1)
    n_oc0<<=1;

  n_oc1 = nprime1/Nprime_div_deltaPUCCH_Shift;
  if (frame_parms->Ncp==1)  // extended CP
    n_oc1<<=1;

#ifdef DEBUG_PUCCH
  printf("[PHY] PUCCH: noc0 %d noc11 %d\n",n_oc0,n_oc1);
#endif

  nprime=nprime0;
  n_oc  =n_oc0;

  // loop over 2 slots
  for (ns=(subframe<<1);ns<(2+(subframe<<1));ns++) {

    if ((nprime&1) == 0)
      S=0;  // 1
    else
      S=1;  // j

    //loop over symbols in slot
    for (m=0;m<N_UL_symb;m++) {
      // Compute n_cs (36.211 p. 18)
      n_cs = ncs_cell[ns][m];
      if (frame_parms->Ncp==0) { // normal CP
	n_cs = ((u16)n_cs + (nprime*deltaPUCCH_Shift + (n_oc%deltaPUCCH_Shift))%Nprime)%12;
      }
      else {
	n_cs = ((u16)n_cs + (nprime*deltaPUCCH_Shift + (n_oc>>1))%Nprime)%12;	
      }


      refs=0;
      // Comput W_noc(m) (36.211 p. 19)
      if ((ns==(1+(subframe<<1))) && (shortened_format==1)) {  // second slot and shortened format

	if (m<2) {                                         // data
	  W_re=W3_re[n_oc][m];
	  W_im=W3_im[n_oc][m];
	}
	else if ((m<N_UL_symb-2)&&(frame_parms->Ncp==0)) { // reference and normal CP 
	  W_re=W3_re[n_oc][m-2];
	  W_im=W3_im[n_oc][m-2];
	  refs=1;
	}
	else if ((m<N_UL_symb-2)&&(frame_parms->Ncp==1)) {  // reference and extended CP 
	  W_re=W4[n_oc][m-2];                          
	  W_im=0;
	  refs=1;
	}     
	else if ((m>=N_UL_symb-2)) {                        // data
	  W_re=W3_re[n_oc][m-N_UL_symb+4];
	  W_im=W3_im[n_oc][m-N_UL_symb+4];
	}
      }
      else {
	if (m<2) {                                         // data
	  W_re=W4[n_oc][m];
	  W_im=0;
	}
	else if ((m<N_UL_symb-2)&&(frame_parms->Ncp==0)) { // reference and normal CP 
	  W_re=W3_re[n_oc][m-2];
	  W_im=W3_im[n_oc][m-2];
	  refs=1;
	}
	else if ((m<N_UL_symb-2)&&(frame_parms->Ncp==1)) { // reference and extended CP 
	  W_re=W4[n_oc][m-2];
	  W_im=0;
	  refs=1;
	}
	else if ((m>=N_UL_symb-2)) {                       // data
	  W_re=W4[n_oc][m-N_UL_symb+4];
	  W_im=0;
	}
      }

      // multiply W by S(ns) (36.211 p.17). only for data, reference symbols do not have this factor  
      if ((S==1)&&(refs==1)) {
	tmp_re = W_re;
	W_re = -W_im;
	W_im = tmp_re;
      }

#ifdef DEBUG_PUCCH
      printf("[PHY] PUCCH: ncs[%d][%d]=%d, W_re %d, W_im %d, S %d, refs %d\n",ns,m,n_cs,W_re,W_im,S,refs);
#endif
      alpha_ind=0;
	// compute output sequence

      for (n=0;n<12;n++) {

	// this is r_uv^alpha(n), u=v=0 (no group/sequence hopping)
	tmp_re = (alpha_re[alpha_ind] * ul_ref_sigs[u][0][0][n<<1] - alpha_im[alpha_ind] * ul_ref_sigs[u][0][0][1+(n<<1)])>>15;
	tmp_im = (alpha_re[alpha_ind] * ul_ref_sigs[u][0][0][1+(n<<1)] - alpha_im[alpha_ind] * ul_ref_sigs[u][0][0][n<<1])>>15;

	// this is S(ns)*w_noc(m)*r_uv^alpha(n)
	ref_re = (tmp_re*W_re - tmp_im*W_im)>>15;
	ref_im = (tmp_re*W_im + tmp_im*W_re)>>15;

	if ((m<2)||(m>=(N_UL_symb-2))) { //these are PUCCH data symbols
	  switch (fmt) {
	  case pucch_format1:   //OOK 1-bit
	    
	    ((short *)&zptr[n])[0] = ((s32)amp*ref_re)>>15;
	    ((short *)&zptr[n])[1] = ((s32)amp*ref_im)>>15;
	    
	    break;
	    
	  case pucch_format1a:  //BPSK 1-bit
	    d0 = (payload&1)==0 ? amp : -amp;
	    
	    ((short *)&zptr[n])[0] = ((s32)d0*ref_re)>>15;
	    ((short *)&zptr[n])[1] = ((s32)d0*ref_im)>>15;
	    
	    break;
	    
	  case pucch_format1b:  //QPSK 2-bits
	    if ((payload&3)==0)  {// 1
	      ((short *)&zptr[n])[0] = ((s32)amp*ref_re)>>15;
	      ((short *)&zptr[n])[1] = ((s32)amp*ref_im)>>15;
	    }
	    else if ((payload&3)==1) {// -j
	      ((short *)&zptr[n])[0] = ((s32)amp*ref_im)>>15;
	      ((short *)&zptr[n])[1] = (-(s32)amp*ref_re)>>15;
	    }
	    else if ((payload&3)==2) {// j
	      ((short *)&zptr[n])[0] = (-(s32)amp*ref_im)>>15;
	      ((short *)&zptr[n])[1] = ((s32)amp*ref_re)>>15;
	    }
	    else if ((payload&3)==3) {// -1
	      ((short *)&zptr[n])[0] = (-(s32)amp*ref_re)>>15;
	      ((short *)&zptr[n])[1] = (-(s32)amp*ref_im)>>15;
	    }
	    break;
	    
	  case pucch_format2:
	    msg("[PHY] PUCCH format 2 not implemented\n");
	    break;
	    
	  case pucch_format2a:
	    msg("[PHY] PUCCH format 2a not implemented\n");
	    break;
	    
	  case pucch_format2b:
	    msg("[PHY] PUCCH format 2b not implemented\n");
	    break;
	  } // switch fmt
	}
	else {   // These are PUCCH reference symbols

	  ((short *)&zptr[n])[0] = ((s32)amp*ref_re)>>15;
	  ((short *)&zptr[n])[1] = ((s32)amp*ref_im)>>15;

	}

	alpha_ind = (alpha_ind + n_cs)%12;
      } // n
      zptr+=12;
    } // m

  nprime=nprime1;
  n_oc  =n_oc1;
  } // ns

  rem = ((((12*Ncs1_div_deltaPUCCH_Shift)>>3)&7)>0) ? 1 : 0;

  m = (n1_pucch < thres) ? NRB2 : (((n1_pucch-thres)/thres)+NRB2+((12*Ncs1_div_deltaPUCCH_Shift)>>3)+rem);

#ifdef DEBUG_PUCCH
  printf("[PHY] PUCCH: m %d\n",m);
#endif
}
