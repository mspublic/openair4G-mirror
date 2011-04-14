/*________________________radio_emulation.c________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr , knopp@eurecom.fr
________________________________________________________________*/

#include "misc_proto.h"
#include "radio_emulation.h"
#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/extern.h"
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/defs.h"
#include "SIMULATION/simulation_defs.h"
#include "LAYER2/MAC/extern.h"


//#include "mac_extern.h"

#define ONEL 0x00000001
#define ONES 0x0001
#define ONEC 0x01
unsigned int *gauss_LUT_ptr;

complex16 H1f[NUMBER_OF_FREQUENCY_GROUPS];
complex16 Hf[NUMBER_OF_FREQUENCY_GROUPS];
complex16 V_gauss[NUMBER_OF_FREQUENCY_GROUPS];
/******************************************************************************************************/ 
short  FIX_MPY_SAT(short A,short B){
/******************************************************************************************************/ 
  return (short)(((int)((int)A * (int)B))>>15);
}
/******************************************************************************************************/ 
short SAT_ADD_FIX(short A,short B){       
/******************************************************************************************************/ 
     int tmp1 = (int)((int)(A) + (int)(B)) ;
     if (tmp1 >= (ONES << 15)){ 
        return ((ONES << 15) - 1);
     } 
     else if (tmp1 <= -(ONES << 15)){ 
       return (-(ONES << 15)); 
     }
     else
       return (short)(tmp1);
}
/******************************************************************************************************/ 
void radio_emulation_init(){
  /******************************************************************************************************/ 
  int i,j;
  set_taus_seed();
  gauss_LUT_ptr = generate_gauss_LUT(N_BIT,L_DEV);

  //  for(i=0;i<NB_NODE;i++)
  //    for(j=0;j<NB_NODE;j++) {
  //      generate_inst_rssi(i,j);
  //    }

  msg("[RADIO_EMULATION INIT] OK for Emul index[0] %d\n",Emul_idx[0]);
  //  exit(0);
}

/******************************************************************************************************/ 
void SAT_CMPLX_ADD(complex16 *DEST, complex16 A, complex16 B){
  /******************************************************************************************************/ 
  //  SAT_ADD_FIX((int*)&DEST->r,A.r,B.r);
  //SAT_ADD_FIX((int*)&DEST->i,A.i,B.i);
}

/******************************************************************************************************/ 
void FIX_CMPLX_MPY(complex16 *DEST, complex16 A, complex16 B){
  /******************************************************************************************************/ 
  /*complex16 Tmp;
  FIX_MPY_SAT((int*)&Tmp.r,A.r,B.r);
  FIX_MPY_SAT((int*)&Tmp.i,A.i,B.i);
  SAT_ADD_FIX((int*)&DEST->r,Tmp.r,-Tmp.i);
  FIX_MPY_SAT((int*)&Tmp.r,A.r,B.i);
  FIX_MPY_SAT((int*)&Tmp.i,A.i,B.r);
  SAT_ADD_FIX((int*)&DEST->i,Tmp.r,Tmp.i);
  */
}
/******************************************************************************************************/ 
void FIX_CMPLX_CONJ(complex16 *DEST, complex16 *SRC, unsigned int N){
  /******************************************************************************************************/ 
  int i;
  for (i=0;i<N;i++){
    DEST[i].r=SRC[i].r;
    DEST[i].i=-SRC[i].i;
  }
}

/******************************************************************************************************/ 
void FIX_CMPLX_NORM(unsigned short *DEST,complex16 *SRC, unsigned int N){
  /******************************************************************************************************/ 
  int i;
  complex16 Conj[N];
  FIX_CMPLX_CONJ(Conj,SRC,N);
  for (i=0;i<N;i++){
    FIX_CMPLX_MPY(&SRC[i],SRC[i],Conj[i]);
    // SAT_ADD_FIX((int*)DEST,*DEST,SRC[i].r);
  }
}


/******************************************************************************************************/ 
void generate_inst_rssi(unsigned short Dest, unsigned short Src){
  /******************************************************************************************************/ 
  unsigned short i,j;
  unsigned int EHf;
  //  msg("generate_inst_rssi in.....\n");
  for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++) {
    V_gauss[i].r=gauss(gauss_LUT_ptr,N_BIT);
    V_gauss[i].i=gauss(gauss_LUT_ptr,N_BIT);
    //msg("[RADIO_EMULATION][DEST %d][SRC %d] G_V on Frequency [%d]=%d;%d\n",Dest,Src,i,V_gauss[i].r,V_gauss[i].i);
  }
  //  msg("gauss ok\n");
  for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++){
    H1f[i].r=0;
    H1f[i].i=0;
    for(j=0;j<NUMBER_OF_FREQUENCY_GROUPS;j++){
      H1f[i].r=
	SAT_ADD_FIX(H1f[i].r,SAT_ADD_FIX(FIX_MPY_SAT(KH[i][j].r,V_gauss[j].r),-FIX_MPY_SAT(KH[i][j].i,V_gauss[j].i)));
      //SAT_ADD_FIX(H1f[i].r,SAT_ADD_FIX(FIX_MPY_SAT(KH[i][j].r,V_gauss[j].r),-FIX_MPY_SAT(KH[i][j].i,V_gauss[j].i)));
      H1f[i].i=
	SAT_ADD_FIX(H1f[i].i,SAT_ADD_FIX(FIX_MPY_SAT(KH[i][j].r,V_gauss[j].i),FIX_MPY_SAT(KH[i][j].i,V_gauss[j].r)));
      /*        if(mac_xface->frame > 0){
	      Hf[i].r= SAT_ADD_FIX(FIX_MPY_SAT(Hf[i].r,2) , FIX_MPY_SAT(H1f[i].r ,2) )>>1; 
	      Hf[i].i= SAT_ADD_FIX(FIX_MPY_SAT(Hf[i].i,2) , FIX_MPY_SAT(H1f[i].i ,2) )>>1; 
	      }
	      else*/
       {
	Hf[i].r= H1f[i].r; 
	Hf[i].i= H1f[i].i; 
      }
       
       //       msg("[RADIO_EMULATION][DEST %d][SRC %d] H1f on Frequency [%d]=%d(%d);%d(%d)\n",Dest,Src,i,Hf[i].r,H1f[i].r,Hf[i].i,H1f[i].i);
      
    }
    //   exit(0);
  }



  for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++){
    //    EHf=FIX_MPY_SAT(H1f[i].r<<8,H1f[i].r<<8);
    //msg("EHF=%d\t",EHf);
    //EHf=FIX_MPY_SAT(H1f[i].i<<8,H1f[i].i<<8);
    //msg("EHF=%d\t",EHf);
    //EHf=SAT_ADD_FIX(FIX_MPY_SAT(H1f[i].r<<8,H1f[i].r<<8),FIX_MPY_SAT(H1f[i].i<<8,H1f[i].i<<8));
    //msg("EHF=%d\t",EHf);
    //E=SAT_ADD_FIX(H1f[i].r,Tmp_mpy.r);
    //    Tmp_mpy->r=H1f[i].r;
    //Tmp_mpy->i=-H1f[i].i;
    //    FIX_CMPLX_MPY(&EHf,H1f[i],*Tmp_mpy);
    //SAT_ADD(EHf_dB,dB_fixed(EHf),-dB_fixed(FIX_SCAL_FACT>>4));  //4=log_2(NUMBER_OF_FREQUENCY_GROUP)
    //SAT_ADD(Rssi_dB[Src][Dest][k],RSSI[Src][Dest],EHf_dB);
    //EHf_dB>>=(FIX_SCAL_FAC+4);
    Rssi[Dest][Src][i]= RSSI[Src][Dest] - NB_SUBBANDS_IN_dB;

      // FIX_MPY_SAT(RSSI[Src][Dest],SAT_ADD_FIX(FIX_MPY_SAT(Hf[i].r,Hf[i].r),FIX_MPY_SAT(Hf[i].i,Hf[i].i)));
    //    if(Rssi[Dest][Src][i]<0){
    //      msg("[RADIO_EMULATION][DEST %d][SRC %d] RSSI on Frequency [%d]=%d\n",Dest,Src,i,(Rssi[Dest][Src][i]));
    //      Rssi[Dest][Src][i]=0;
      //      exit(0);
    //    }
  }
  //  msg("generate_inst_rssi done\n");
  //exit(0);
}

/******************************************************************************************************/ 
void reset_rssi_sinr(void){
/******************************************************************************************************/ 
  unsigned short i,j,k,kk;
  for(i=0;i<NB_NODE;i++){
    for(j=0;j<NB_NODE;j++){
      if(i!=j)
	generate_inst_rssi(i,j);
    }
    //   for(k=0;k<NUMBER_OF_FREQUENCY_GROUPS;k++)
    //for(kk=0;kk<NB_TIME_ALLOC;kk++)
    //Sinr[i][k][kk]=N0_LN/NUMBER_OF_FREQUENCY_GROUPS;
  }
}

/******************************************************************************************************/ 
unsigned short rssi_dB_2_fixed(char x ){ //x between -120dB & -40dB
/******************************************************************************************************/ 
  unsigned int s=0;
  if(x<-120) x=-120;
  else if(x>-40) x=-40;
  unsigned char y=(unsigned char)((x+120));//*0xff/80);//8 bits representation of x in dB
  //  msg("x=%d,y=%d\t",x,y);
  unsigned char i=0;
  for(i=0;i<7;i++)
    if((y & (ONEC << i))){
      s+=( (int)(  ONEL   << ( (int)(ONEC << i)*3/10 )) );
      msg("i=%d,s_d=%d,s=%d \t",i, ( (int)(  ONEL   << ( (int)(ONEC << i)*3/10 )) ),s);
    }
  //  msg("[sssssss]=%d",)
  return s;
}
