#include <stdio.h>
#include <math.h>

static unsigned long seed, iy, ir[98];
/*
@defgroup _uniformdouble 
@ingroup numerical Uniform linear congruential random number generator.
*/

/*!\brief Initialization routine for Uniform/Gaussian random number generators. */

#define a 1664525lu
#define mod 4294967296.0                /* is 2**32 */


void randominit()
{
  int i;
  unsigned seedfather;

  srand((unsigned)time(NULL));

  seed = (unsigned long) rand();
  if (seed % 2 == 0) seed += 1; /* seed and mod are relative prime */
  for (i=1; i<=97; i++)
  {
      seed = a*seed;                 /* mod 2**32  */
      ir[i]= seed;                   /* initialize the shuffle table    */
  }
  iy=1;
}

/*!\brief Uniform linear congruential random number generator on \f$[0,1)\f$.  Returns a double-precision floating-point number.*/

double uniformrandom()
{
  #define a 1664525lu
  #define mod 4294967296.0                /* is 2**32 */

  int j;

  j=1 + 97.0*iy/mod;
  iy=ir[j];
  seed = a*seed;                          /* mod 2**32 */
  ir[j] = seed;
  return( (double) iy/mod );
}

/*
@defgroup _gaussdouble Gaussian random number generator based on modified Box-Muller transformation.   
@ingroup numerical 
*/

/*!\brief Gaussian random number generator based on modified Box-Muller transformation.Returns a double-precision floating-point number. */

double gaussdouble(double mean, double variance)
{
  static int iset=0;
  static double gset;
  double fac,r,v1,v2;

  if (iset == 0){
     do{
           v1 = 2.0*uniformrandom()-1.0;
           v2 = 2.0*uniformrandom()-1.0;
           r = v1*v1+v2*v2;
        }  while (r >= 1.0);
     fac = sqrt(-2.0*log(r)/r);
     gset= v1*fac;
     iset=1;
     return(sqrt(variance)*v2*fac + mean);
  } else {
     iset=0;
     return(sqrt(variance)*gset + mean);
  }
}



#ifdef MAIN
main(int argc,char **argv) {

  int i;

  randominit();
  for (i=0;i<10;i++) {
    printf("%f\n",gaussdouble(0.0,1.0));
  }
}
#endif

