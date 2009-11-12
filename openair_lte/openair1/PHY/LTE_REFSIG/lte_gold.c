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

unsigned int lte_gold_table[20][2][14];  // need 55 bytes for sequence
// slot index x pilot within slot x sequence

void lte_gold(LTE_DL_FRAME_PARMS *frame_parms) {

  unsigned char ns,l;
  unsigned int n,x1,x2;

  for (ns=0;ns<20;ns++) {

    for (l=0;l<2;l++) {
      
      x2 = frame_parms->Ncp + (frame_parms->Nid_cell<<1) + (((1+(frame_parms->Nid_cell<<1))*(1 + ((frame_parms->Ncp==0?4:3)*l) + (7*(1+ns))))<<10); //cinit
      //x2 = frame_parms->Ncp + (frame_parms->Nid_cell<<1) + (1+(frame_parms->Nid_cell<<1))*(1 + (3*l) + (7*(1+ns))); //cinit
      x1 = 1+ (1<<31);
      x2=x2 + ((x2 ^ (x2>>1) ^ (x2>>2) ^ (x2>>3))<<31);
      // skip first 50 double words (1600 bits)
      //      printf("n=0 : x1 %x, x2 %x\n",x1,x2);
      for (n=1;n<50;n++) {
	x1 = (x1>>1) ^ (x1>>4);
	x1 = x1 ^ (x1<<31) ^ (x1<<28);
	x2 = (x2>>1) ^ (x2>>2) ^ (x2>>3) ^ (x1>>4);
	x2 = x2 ^ (x2<<31) ^ (x2<<30) ^ (x2<<29) ^ (x2<<28);
      }
      for (n=0;n<14;n++) {
	x1 = (x1>>1) ^ (x1>>4);
	x1 = x1 ^ (x1<<31) ^ (x1<<28);
	x2 = (x2>>1) ^ (x2>>2) ^ (x2>>3) ^ (x1>>4);
	x2 = x2 ^ (x2<<31) ^ (x2<<30) ^ (x2<<29) ^ (x2<<28);
	lte_gold_table[ns][l][n] = x1^x2;
	//	printf("n=%d : c %x\n",n,x1^x2);	
      }

    }

  }
}


#ifdef LTE_GOLD_MAIN
main() {

  lte_gold(423,0);

}
#endif
