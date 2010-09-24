/*!\brief Some macros for fixed-point arithmetic in C
*/

//#ifndef __FIXED_POINT_H__
//#define __FIXED_POINT_H__
int tmp1;

#define FIX_MPY(DEST,A,B)       DEST = ((int)(A) * (int)(B))>>15 


#define SAT_ADD(DEST,A,B)       tmp1 = ((int)(A) + (int)(B)) ; if (tmp1 >= (ONE << 15)) DEST = ((ONE << 15) - 1); else if (tmp1 <= -(ONE << 15)) DEST = (-(ONE << 15)); else DEST = (short)tmp1;
																	    //#endif
