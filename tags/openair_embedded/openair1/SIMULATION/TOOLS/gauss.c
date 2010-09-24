/*! @defgroup _gauss 
@ingroup numerical

This set of routines are used to generate quantized (i.e. fixed-point) Gaussian random noise efficiently. The use of these routines
allows for rapid computer simulation of digital communication systems. The method is based on a lookup-table
of the quantized normal probability distribution.  The routines assume that the continuous-valued Gaussian random-variable,\f$x\f$ is quantized
to \f$N\f$ bits over the interval \f$[-L\sigma,L\sigma)\f$ where \f$N\f$ and \f$L\f$ control the precision and range of the quantization.  The 
random variable, \f$l\in\{-2^{N-1},-2^{N-1}+1,\cdots,0,1,\cdots,2^{N-1}-1\}\f$ corresponds to the event, 
\f$E_l = 
\begin{cases} 
x\in\left[-\infty,-L\sigma\right) & l=-2^{N-1}, \\  
x\in\left[\frac{lL\sigma}{2^{N-1}},\frac{(l+1)L\sigma}{2^{N-1}}\right) & <l>-2^{N-1}, \\  
x\in\left[L\sigma,\infty\right) & l>-2^{N-1},
\end{cases}\f$
which occurs with probability
\f$\Pr(E_l) = 
\begin{cases} 
\mathrm{erfc}(L) & l=-2^{N-1}, \\  
\mathrm{erfc}(L) & l>-2^{N-1}, \\  
\mathrm{erf}\left(\frac{lL}{2^{N-1}}\right) \mathrm{erfc}\left(\frac{(l-1)L}{2^{N-1}}\right)& l>-2^{N-1}.
\end{cases}\f$
*/

#include <assert.h>


/*!\brief This routine generates a Gaussian pdf lookup table (LUT).  The table has \f$2^{\mathrm{Nbits}-1}\f$ entries which represent
the right half of the pdf.  The data stored in position \f$i\f$ is actually the scaled cumulative probability distribution, 
\f$2^{31}\mathrm{erf}\left(\frac{iL}{2^{N-1}}\right)\f$.  This represents the average number of times that the random variable
falls in the interval \f$\left[0,\frac{i}{2^{N-1}}\right)\f$.  This format allows for rapid conversion of uniform 32-bit
random variables to \f$N\f$-bit Gaussian random variables using binary search.
@see gauss
@param Nbits Number of bits for the output variable
@param L Number of standard deviations in range
*/
unsigned int *generate_gauss_LUT(unsigned char Nbits,  
				 unsigned char L       
				 )
{

  unsigned int *LUT_ptr,i;

  LUT_ptr = (unsigned int *)malloc((1<<(Nbits-1))*sizeof(int));
  assert(LUT_ptr);


  for (i=0;i<(1<<(Nbits-1));i++) {
    LUT_ptr[i] = (unsigned int)((double)((unsigned int)(1<<31))*erf(i*L/(double)(1<<(Nbits-1))));

#ifdef LUTDEBUG
    printf("pos %d : LUT_ptr[%d]=%x (%f)\n",i,i,LUT_ptr[i],(double)(erf(i*L/(double)(1<<(Nbits-1)))));
#endif LUTDEBUG
  }

  return(LUT_ptr);
}  


/*!\brief This routine returns a zero-mean unit-variance Gaussian random variable.  
 Given a 32-bit uniform random variable, 
\f$\mathrm{u}\f$ (from \ref _taus, we first extract the sign and then search in the monotonically increasing Gaussian LUT for 
the two entries \f$(i,i+1)\f$ for which 
\f$ 2^{31}\mathrm{erf}\left(\frac{i}{2^{Nbits-1}}\right) < |u| \leq 2^{31}\mathrm{erf}\left(\frac{i+1}{2^{Nbits-1}}\right) \f$ and assign 
the value \f$\mathrm{sgn}(u)i\f$.  The search requires at most \f$Nbits-1\f$ comparisons.
@see generate_gauss_LUT
@see taus
@param gauss_LUT pointer to lookup-table
@param Nbits number of bits for output variable ( between 1 and 16) 
*/
int gauss(unsigned int *gauss_LUT, 
	  unsigned char Nbits     
	  ) {
  
  unsigned int search_pos,step_size,u,tmp,tmpm1,tmpp1,s;

  // Get a 32-bit uniform random-variable
  u = taus();

#ifdef DEBUG
  printf("u = %u\n",u); 
#endif DEBUG

  // if it is larger than 2^31 (here negative), save the sign and rescale down to 31-bits.

  s = u & 0x80000000;
  u &= 0x7fffffff;


#ifdef DEBUG  
  printf("u = %x,s=%d\n",u,s); 
#endif DEBUG

  search_pos = (1<<(Nbits-2));   // starting position of the binary search
  step_size  = search_pos;
	       
  do {

    step_size >>= 1;
    
    tmp = gauss_LUT[search_pos];
    tmpm1 = gauss_LUT[search_pos-1];
    tmpp1 = gauss_LUT[search_pos+1];
#ifdef DEBUG
    printf("search_pos %d, step_size %d: t %x tm %x,tp %x\n",search_pos,step_size,tmp,tmpm1,tmpp1);
#endif DEBUG
    if (u <= tmp) 
      if (u >tmpm1) 
	return s==0 ? (search_pos-1) : 1-search_pos;
      else
	search_pos -= step_size;
    else
      if (u <= tmpp1) 
	return s==0 ? search_pos : - search_pos;
      else
	search_pos += step_size;
    
  }
  while (step_size > 0);

  // If it gets here we're beyond the positive edge  so return max
  return s==0 ? (1<<(Nbits-1))-1 : 1-((1<<(Nbits-1)));

}


#ifdef GAUSSMAIN

#define Nhistbits 8

void main(int argc,char **argv) {

  unsigned int *gauss_LUT_ptr,i;
  unsigned int hist[(1<<Nhistbits)];
  int gvar,maxg=0,ming=9999999,maxnum=0,L,Ntrials,Nbits;
  double meang=0.0,varg=0.0;

  if (argc < 4) {
    printf("Not enough arguments: %s Nbits L Ntrials\n",argv[0]);
    exit(-1);
  }

  Nbits   = atoi(argv[1]);
  L       = atoi(argv[2]);
  Ntrials = atoi(argv[3]);

  set_taus_seed();
  // Generate Gaussian LUT 12-bit quantization over 5 standard deviations
  gauss_LUT_ptr = generate_gauss_LUT(Nbits,L);

  for (i=0;i<(1<<Nhistbits);i++)
    hist[i] = 0;

  for (i=0;i<Ntrials;i++) {

    gvar = gauss(gauss_LUT_ptr,Nbits);
    if (gvar == ((1<<(Nbits-1))-1))
      maxnum++;
    maxg = gvar > maxg ? gvar : maxg;
    ming = gvar < ming ? gvar : ming;
    meang += (double)gvar/Ntrials;
    varg += (double)gvar*gvar/Ntrials;
    gvar += (1<<(Nbits-1))-1;
    hist[(gvar/(1<<(Nbits-Nhistbits)))]++;
    //    printf("%d\n",gauss(gauss_LUT_ptr,Nbits));
  }

  printf("Tail probability = %e(%x)\n",2*erfc((double)L*gauss_LUT_ptr[(1<<(Nbits-1))-1]/(unsigned int)(1<<31)),gauss_LUT_ptr[(1<<(Nbits-1))-1]);
  printf("max %d, min %d, mean %f, stddev %f, Pr(maxnum)=%e(%d)\n",maxg,ming,meang,sqrt(varg),(double)maxnum/Ntrials,maxnum);

	 //  for (i=0;i<(1<<Nhistbits);i++)
	 //    printf("%d : %u\n",i,hist[i]);

  free(gauss_LUT_ptr);
}

#endif GAUSSMAIN
