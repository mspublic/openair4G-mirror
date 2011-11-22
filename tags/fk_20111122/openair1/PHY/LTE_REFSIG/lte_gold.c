#include "defs.h"


/* c(n) = x1(n+Nc) + x2(n+Nc) mod 2
x1(n+31) = (x1(n+3)+x1(n))mod 2
x2(n+31) = (x2(n+3)+x2(n+2)+x2(n+1)+x2(n))mod 2
x1(0)=1,x1(n)=0,n=1...30
x2 <=> cinit = sum_{i=0}^{30} x2(i)2^i

equivalent
x1(n) = x1(n-28) + x1(n-31)
x2(n) = x2(n-28) + x2(n-29) + x2(n-30) + x2(n-31)
x1(0)=1,x1(1)=0,...x1(30)=0,x1(31)=1
x2 <=> cinit, x2(31) = x2(3)+x2(2)+x2(1)+x2(0)

N_RB^{max,DL}=110
c_init = 2^1 * (7(n_s + 1) + l + 1)*(2N_{ID}^{cell} + 1) + 2*N_{ID}^{cell} + N_CP
N_{ID}^{cell = 0..503
*/

//unsigned int lte_gold_table[3][20][2][14];  // need 55 bytes for sequence
// slot index x pilot within slot x sequence

void lte_gold(LTE_DL_FRAME_PARMS *frame_parms,u32 lte_gold_table[20][2][14],u8 offset) {

  unsigned char ns,l,Ncp=1-frame_parms->Ncp;
  unsigned int n,x1,x2;//,x1tmp,x2tmp;
  u16 Nid_cell = frame_parms->Nid_cell;
  u8 Nid1 = Nid_cell/3,Nid2=Nid_cell%3;
  
  Nid2 = (Nid2+offset)%3;
  Nid_cell = (3*Nid1)+Nid2;

  for (ns=0;ns<20;ns++) {

    for (l=0;l<2;l++) {
      
      x2 = Ncp + 
	(Nid_cell<<1) + 
	(((1+(Nid_cell<<1))*(1 + ((frame_parms->Ncp==0?4:3)*l) + (7*(1+ns))))<<10); //cinit
      //x2 = frame_parms->Ncp + (Nid_cell<<1) + (1+(Nid_cell<<1))*(1 + (3*l) + (7*(1+ns))); //cinit
      //n = 0
      //      printf("cinit (ns %d, l %d) => %d\n",ns,l,x2);
      x1 = 1+ (1<<31);
      x2=x2 ^ ((x2 ^ (x2>>1) ^ (x2>>2) ^ (x2>>3))<<31);
      // skip first 50 double words (1600 bits)
      //printf("n=0 : x1 %x, x2 %x\n",x1,x2);
      for (n=1;n<50;n++) {
	x1 = (x1>>1) ^ (x1>>4);
	x1 = x1 ^ (x1<<31) ^ (x1<<28);
	x2 = (x2>>1) ^ (x2>>2) ^ (x2>>3) ^ (x2>>4);
	x2 = x2 ^ (x2<<31) ^ (x2<<30) ^ (x2<<29) ^ (x2<<28);
	//	printf("x1 : %x, x2 : %x\n",x1,x2);
      }
      for (n=0;n<14;n++) {
	x1 = (x1>>1) ^ (x1>>4);
	x1 = x1 ^ (x1<<31) ^ (x1<<28);
	x2 = (x2>>1) ^ (x2>>2) ^ (x2>>3) ^ (x2>>4);
	x2 = x2 ^ (x2<<31) ^ (x2<<30) ^ (x2<<29) ^ (x2<<28);
	lte_gold_table[ns][l][n] = x1^x2;
	//	printf("n=%d : c %x\n",n,x1^x2);	
      }

    }

  }
}

/*! \brief gold sequenquence generator
\param x1 
\param x2 this should be set to c_init if reset=1
\param reset resets the generator
\return 32 bits of the gold sequence
*/
unsigned int lte_gold_generic(unsigned int *x1, unsigned int *x2, unsigned char reset) 
{
  int n;
  if (reset) {
    *x1 = 1+ (1<<31);
    *x2=*x2 ^ ((*x2 ^ (*x2>>1) ^ (*x2>>2) ^ (*x2>>3))<<31);
    // skip first 50 double words (1600 bits)
    //      printf("n=0 : x1 %x, x2 %x\n",x1,x2);
    for (n=1;n<50;n++) {
      *x1 = (*x1>>1) ^ (*x1>>4);
      *x1 = *x1 ^ (*x1<<31) ^ (*x1<<28);
      *x2 = (*x2>>1) ^ (*x2>>2) ^ (*x2>>3) ^ (*x2>>4);
      *x2 = *x2 ^ (*x2<<31) ^ (*x2<<30) ^ (*x2<<29) ^ (*x2<<28);
    }
  }
  *x1 = (*x1>>1) ^ (*x1>>4);
  *x1 = *x1 ^ (*x1<<31) ^ (*x1<<28);
  *x2 = (*x2>>1) ^ (*x2>>2) ^ (*x2>>3) ^ (*x2>>4);
  *x2 = *x2 ^ (*x2<<31) ^ (*x2<<30) ^ (*x2<<29) ^ (*x2<<28);
  return(*x1^*x2);
  //	printf("n=%d : c %x\n",n,x1^x2);	
  
}



#ifdef LTE_GOLD_MAIN
main() {

  lte_gold(423,0);

}
#endif
