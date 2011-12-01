#ifndef __PHY_TOOLS_DEFS__H__
#define __PHY_TOOLS_DEFS__H__

/** @addtogroup _PHY_DSP_TOOLS_


* @{

*/

#include "PHY/defs.h"

#ifndef EXPRESSMIMO_TARGET
#include <emmintrin.h>
#else //EXPRESSMIMO_TARGET
#include "express-mimo/simulator/baseband/C/defs.h"
#endif //EXPRESSMIMO_TARGET

//defined in rtai_math.h
#ifndef _RTAI_MATH_H
struct complex
{
  double x;
  double y;
};
#endif

struct complexf
{
  float r;
  float i;
};

struct complex16
{
  short r;
  short i;	
};

struct complex32
{
  int r;
  int i;
};

#ifndef EXPRESSMIMO_TARGET
/*!\fn void multadd_real_vector_complex_scalar(short *x,short *alpha,short *y,unsigned int N)
This function performs componentwise multiplication and accumulation of a complex scalar and a real vector.
@param x Vector input (Q1.15)  
@param alpha Scalar input (Q1.15) in the format  |Re0 Im0|
@param y Output (Q1.15) in the format  |Re0  Im0 Re1 Im1|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N Length of x WARNING: N>=8

The function implemented is : \f$\mathbf{y} = y + \alpha\mathbf{x}\f$
*/
void multadd_real_vector_complex_scalar(short *x,
					short *alpha,
					short *y,
					unsigned int N
					);

/*!\fn void multadd_complex_vector_real_scalar(short *x,short alpha,short *y,unsigned char zero_flag,unsigned int N)
This function performs componentwise multiplication and accumulation of a real scalar and a complex vector.
@param x Vector input (Q1.15) in the format |Re0 Im0|Re1 Im 1| ... 
@param alpha Scalar input (Q1.15) in the format  |Re0|
@param y Output (Q1.15) in the format  |Re0  Im0 Re1 Im1|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param zero_flag Set output (y) to zero prior to accumulation
@param N Length of x WARNING: N>=8

The function implemented is : \f$\mathbf{y} = y + \alpha\mathbf{x}\f$
*/
void multadd_complex_vector_real_scalar(short *x,
					short alpha,
					short *y,
					unsigned char zero_flag,
					unsigned int N);


/*!\fn int mult_cpx_vector(short *x1,short *x2,short *y,unsigned int N,unsigned short output_shift)
This function performs optimized componentwise multiplication of two Q1.15 vectors in repeated format.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0 -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N  Length of Vector WARNING: N must be a multiple of 8 (4x loop unrolling and two complex multiplies per cycle) 
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 

The function implemented is : \f$\mathbf{y} = \mathbf{x_1}\odot\mathbf{x_2}\f$
*/
int mult_cpx_vector(short *x1, 
		    short *x2, 
		    short *y, 
		    unsigned int N, 
		    int output_shift);

/*!\fn int mult_cpx_vector_norep(short *x1,short *x2,short *y,unsigned int N,unsigned short output_shift)
This function performs optimized componentwise multiplication of two Q1.15 vectors with normal formatted output.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0 -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
@param N  Length of Vector WARNING: N must be a multiple of 8 (4x loop unrolling and two complex multiplies per cycle) 
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 

The function implemented is : \f$\mathbf{y} = \mathbf{x_1}\odot\mathbf{x_2}\f$
*/
int mult_cpx_vector_norep(short *x1, 
			  short *x2, 
			  short *y, 
			  unsigned int N, 
			  int output_shift); // __attribute__ ((force_align_arg_pointer));


/*!\fn int mult_cpx_vector_norep2(short *x1,short *x2,short *y,unsigned int N,int output_shift)
This function performs optimized componentwise multiplication of two Q1.15 vectors with normal formatted output.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0 -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
@param N  Length of Vector WARNING: N must be a multiple of 8 (no unrolling and two complex multiplies per cycle) 
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 

The function implemented is : \f$\mathbf{y} = \mathbf{x_1}\odot\mathbf{x_2}\f$
*/
int mult_cpx_vector_norep2(short *x1, 
			   short *x2, 
			   short *y, 
			   unsigned int N, 
			   int output_shift); //__attribute__ ((force_align_arg_pointer));
 
int mult_cpx_vector_norep_conj(short *x1, 
			       short *x2, 
			       short *y, 
			       unsigned int N, 
			       int output_shift);

int mult_cpx_vector_norep_conj2(short *x1, 
				short *x2, 
				short *y, 
				unsigned int N, 
				int output_shift);

/*!\fn int mult_cpx_vector2(short *x1,short *x2,short *y,unsigned int N,unsigned short output_shift)
This function performs optimized componentwise multiplication of two Q1.15 vectors.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0 -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N  Length of Vector WARNING: N must be a multiple of 2 (2 complex multiplies per cycle)
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 

The function implemented is : \f$\mathbf{y} = \mathbf{x_1}\odot\mathbf{x_2}\f$
*/
int mult_cpx_vector2(short *x1, 
		     short *x2, 
		     short *y, 
		     unsigned int N, 
		     int output_shift);

/*!\fn int mult_cpx_vector_add(short *x1,short *x2,short *y,unsigned int N,unsigned short output_shift)
This function performs optimized componentwise multiplication of two Q1.15 vectors. The output IS ADDED TO y. WARNING: make sure that output_shift is set to the right value, so that the result of the multiplication has the comma at the same place as y.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0 -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N  Length of Vector WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 

The function implemented is : \f$\mathbf{y} += \mathbf{x_1}\odot\mathbf{x_2}\f$
*/

int mult_cpx_vector_add(short *x1, 
			short *x2, 
			short *y, 
			unsigned int N, 
			int output_shift);


int mult_cpx_vector_h_add32(short *x1, 
			    short *x2, 
			    short *y, 
			    unsigned int N, 
			    short sign);

int mult_cpx_vector_add32(short *x1, 
			  short *x2, 
			  short *y, 
			  unsigned int N);

int mult_vector32(short *x1, 
		  short *x2, 
		  short *y, 
		  unsigned int N);

int mult_vector32_scalar(short *x1, 
			 int x2, 
			 short *y, 
			 unsigned int N);

int mult_cpx_vector32_conj(short *x, 
			   short *y, 
			   unsigned int N);

int mult_cpx_vector32_real(short *x1, 
			   short *x2, 
			   short *y, 
			   unsigned int N);

int shift_and_pack(short *y, 
		   unsigned int N, 
		   int output_shift);

/*!\fn int mult_cpx_vector_h(short *x1,short *x2,short *y,unsigned int N,unsigned short output_shift,short sign)
This function performs optimized componentwise multiplication of the vector x1 with the conjugate of the vector x2. The output IS ADDED TO y. WARNING: make sure that output_shift is set to the right value, so that the result of the multiplication has the comma at the same place as y.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1) Im(N-1) Re(N-1) Im(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N  Length of Vector (complex samples) WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 
@param sign +1..add, -1..substract

The function implemented is : \f$\mathbf{y} = \mathbf{y} + \mathbf{x_1}\odot\mathbf{x_2}^*\f$
*/
int mult_cpx_vector_h(short *x1, 
		      short *x2, 
		      short *y, 
		      unsigned int N, 
		      int output_shift,
		      short sign);

/*!\fn int mult_cpx_matrix_h(short *x1[2][2],short *x2[2][2],short *y[2][2],unsigned int N,unsigned short output_shift,short hermitian)
This function performs optimized componentwise matrix multiplication of the 2x2 matrices x1 with the 2x2 matrices x2. The output IS ADDED TO y (i.e. make sure y is initilized correctly). WARNING: make sure that output_shift is set to the right value, so that the result of the multiplication has the comma at the same place as y.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1) Im(N-1) Re(N-1) Im(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N  Length of Vector (complex samples) WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 
@param hermitian if !=0 the hermitian transpose is returned (i.e. A^H*B instead of A*B^H)
*/
int mult_cpx_matrix_h(short *x1[2][2], 
		    short *x2[2][2], 
		    short *y[2][2], 
		    unsigned int N, 
		    unsigned short output_shift,
		    short hermitian);

/*!\fn int mult_cpx_matrix_vector(int *x1[2][2],int *x2[2],int *y[2],unsigned int N,unsigned short output_shift)
This function performs optimized componentwise matrix-vector multiplication of the 2x2 matrices x1 with the 2x1 vectors x2. The output IS ADDED TO y (i.e. make sure y is initilized correctly). WARNING: make sure that output_shift is set to the right value, so that the result of the multiplication has the comma at the same place as y.

@param x1 Input 1 in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param x2 Input 2 in the format  |Re0  -Im0 Im0 Re0|,......,|Re(N-1) -Im(N-1) Im(N-1) Re(N-1)|
@param y  Output in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)| WARNING: y must be different for x2
@param N  Length of Vector (complex samples) WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!! 
*/
int mult_cpx_matrix_vector(int *x1[2][2], 
		    int *x2[2], 
		    int *y[2], 
		    unsigned int N, 
		    unsigned short output_shift);

/*!\fn void init_fft(unsigned short size,unsigned char logsize,unsigned short *rev)
\brief Initialize the FFT engine for a given size
@param size Size of the FFT
@param logsize log2(size)
@param rev Pointer to bit-reversal permutation array
*/

void init_fft(unsigned short size, 
	      unsigned char logsize, 
	      unsigned short *rev);

/*!\fn void fft(short *x,short *y,short *twiddle,unsigned short *rev,unsigned char log2size,unsigned char scale,unsigned char input_fmt)
This function performs optimized fixed-point radix-2 FFT/IFFT.
@param x Input
@param y Output in format: [Re0,Im0,Re0,Im0, Re1,Im1,Re1,Im1, ....., Re(N-1),Im(N-1),Re(N-1),Im(N-1)]
@param twiddle Twiddle factors
@param rev bit-reversed permutation
@param log2size Base-2 logarithm of FFT size
@param scale Total number of shifts (should be log2size/2 for normalized FFT)
@param input_fmt (0 - input is in complex Q1.15 format, 1 - input is in complex redundant Q1.15 format)
*/
void fft(short *x,   
	 short *y,
	 short *twiddle,
	 unsigned short *rev,
	 unsigned char log2size,
	 unsigned char scale,
	 unsigned char input_fmt
	);

void ifft1536(s16 *sigF,s16 *sig);

void ifft6144(s16 *sigF,s16 *sig);

void ifft12288(s16 *sigF,s16 *sig);

void ifft18432(s16 *sigF,s16 *sig);

void ifft3072(s16 *sigF,s16 *sig);

void ifft24576(s16 *sigF,s16 *sig);


/*!\fn int rotate_cpx_vector(short *x,short *alpha,short *y,unsigned int N,unsigned short output_shift, unsigned char format)
This function performs componentwise multiplication of a vector with a complex scalar.
@param x Vector input (Q1.15)  in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param alpha Scalar input (Q1.15) in the format  |Re0 Im0|
@param y Output (Q1.15) in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N Length of x WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!!
@param format Format 0 indicates that alpha is in shuffled format during multiply (Re -Im Im Re), whereas 1 indicates that input is in this format (i.e. a matched filter)

The function implemented is : \f$\mathbf{y} = \alpha\mathbf{x}\f$
*/
int rotate_cpx_vector(short *x, 
		      short *alpha, 
		      short *y, 
		      unsigned int N, 
		      unsigned short output_shift,
		      unsigned char format);

/*!\fn int rotate_cpx_vector2(short *x,short *alpha,short *y,unsigned int N,unsigned short output_shift,unsigned char format)
This function performs componentwise multiplication of a vector with a complex scalar.
@param x Vector input (Q1.15)  in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param alpha Scalar input (Q1.15) in the format  |Re0 Im0|
@param y Output (Q1.15) in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N Length of x WARNING: N must be multiple of 2 (the routine performs two complex multiplies per cycle)
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!!
@param format Format 0 indicates that alpha is in shuffled format during multiply (Re -Im Im Re), whereas 1 indicates that input is in this format (i.e. a matched filter)
The function implemented is : \f$\mathbf{y} = \alpha\mathbf{x}\f$
*/
int rotate_cpx_vector2(short *x, 
		       short *alpha, 
		       short *y, 
		       unsigned int N, 
		       unsigned short output_shift,
		       unsigned char format);

/*!\fn int rotate_cpx_vector_norep(short *x,short *alpha,short *y,unsigned int N,unsigned short output_shift)
This function performs componentwise multiplication of a vector with a complex scalar.
@param x Vector input (Q1.15)  in the format  |Re0  Im0|,......,|Re(N-1) Im(N-1)|
@param alpha Scalar input (Q1.15) in the format  |Re0 Im0|
@param y Output (Q1.15) in the format  |Re0  Im0|,......,|Re(N-1) Im(N-1)|
@param N Length of x WARNING: N>=4
@param output_shift Number of bits to shift output down to Q1.15 (should be 15 for Q1.15 inputs) WARNING: log2_amp>0 can cause overflow!!

The function implemented is : \f$\mathbf{y} = \alpha\mathbf{x}\f$
*/
int rotate_cpx_vector_norep(short *x, 
			       short *alpha, 
			       short *y, 
			       unsigned int N, 
			       unsigned short output_shift);



/*!\fn int add_cpx_vector(short *x,short *alpha,short *y,unsigned int N)
This function performs componentwise addition of a vector with a complex scalar.
@param x Vector input (Q1.15)  in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param alpha Scalar input (Q1.15) in the format  |Re0 Im0|
@param y Output (Q1.15) in the format  |Re0  Im0 Re0 Im0|,......,|Re(N-1)  Im(N-1) Re(N-1) Im(N-1)|
@param N Length of x WARNING: N>=4

The function implemented is : \f$\mathbf{y} = \alpha + \mathbf{x}\f$
*/
int add_cpx_vector(short *x, 
		   short *alpha, 
		   short *y, 
		   unsigned int N);

int add_cpx_vector32(short *x, 
		      short *y, 
		      short *z, 
		      unsigned int N);

int add_real_vector64(short *x, 
		      short *y, 
		      short *z, 
		      unsigned int N);

int sub_real_vector64(short *x, 
		      short* y, 
		      short *z, 
		      unsigned int N);

int add_real_vector64_scalar(short *x, 
			     long long int a, 
			     short *y, 
			     unsigned int N);

/*!\fn int add_vector16(short *x,short *y,short *z,unsigned int N)
This function performs componentwise addition of two vectors with Q1.15 components.
@param x Vector input (Q1.15)  
@param y Scalar input (Q1.15) 
@param z Scalar output (Q1.15) 
@param N Length of x WARNING: N must be a multiple of 32

The function implemented is : \f$\mathbf{z} = \mathbf{x} + \mathbf{y}\f$
*/
int add_vector16(short *x, 
		 short *y, 
		 short *z, 
		 unsigned int N);

int add_vector16_64(short *x, 
		    short *y, 
		    short *z, 
		    unsigned int N);

int complex_conjugate(short *x1, 
		      short *y, 
		      unsigned int N);

void bit8_txmux(int length,int offset);

void bit8_rxdemux(int length,int offset);

#ifdef USER_MODE
/*!\fn int write_output(const char *fname, const char *vname, void *data, int length, int dec, char format);
\brief Write output file from signal data
@param fname output file name
@param vname  output vector name (for MATLAB/OCTAVE)
@param data   point to data 
@param length length of data vector to output
@param dec    decimation level
@param format data format (0 = real 16-bit, 1 = complex 16-bit,2 real 32-bit, 3 complex 32-bit,4 = real 8-bit, 5 = complex 8-bit)
*/
int write_output(const char *fname, const char *vname, void *data, int length, int dec, char format);
#endif

void Zero_Buffer(void *,unsigned int);
void Zero_Buffer_nommx(void *buf,unsigned int length);

void mmxcopy(void *dest,void *src,int size);

/*!\fn int signal_energy(int *,unsigned int);
\brief Computes the signal energy per subcarrier
*/
int signal_energy(int *,unsigned int);

/*!\fn int signal_energy_nodc(int *,unsigned int);
\brief Computes the signal energy per subcarrier, without DC removal
*/
int signal_energy_nodc(int *,unsigned int);

/*!\fn double signal_energy_fp(double **, double **,unsigned int, unsigned int,unsigned int);
\brief Computes the signal energy per subcarrier
*/
double signal_energy_fp(double **s_re, double **s_im, unsigned int nb_antennas, unsigned int length,unsigned int offset);

/*!\fn double signal_energy_fp2(struct complex *, unsigned int);
\brief Computes the signal energy per subcarrier
*/
double signal_energy_fp2(struct complex *s, unsigned int length);


int iSqrt(int value);
unsigned char log2_approx(unsigned int);
unsigned char log2_approx64(unsigned long long int x);
short invSqrt(short x);
unsigned int angle(struct complex16 perrror);

/*!\fn int phy_phase_compensation_top (unsigned int pilot_type, unsigned int initial_pilot,
				unsigned int last_pilot, int ignore_prefix);
Compensate the phase rotation of the RF. WARNING: This function is currently unused. It has not been tested!
@param pilot_type indicates whether it is a CHBCH (=0) or a SCH (=1) pilot
@param initial_pilot index of the first pilot (which serves as reference)
@param last_pilot index of the last pilot in the range of pilots to correct the phase
@param ignore_prefix set to 1 if cyclic prefix has not been removed (by the hardware)

*/

#else // EXPRESSMIMO_TARGET

#define fft(x,y,twiddle,rev,log2size,scale,input_fmt) \
(((twiddle)==0) ? \
 (fft(1<<(log2size),((unsigned long*)(x)),((unsigned long*)(y)))) : \
 (ifft(1<<(log2size),((unsigned long*)(x)),((unsigned long*)(y)))))

#define mult_cpx_vector(x1,x2,y,N,os)  component_wise_product(N,(unsigned long *)(x1),(unsigned long *)(x2),(unsigned long *)(y))

#define mult_cpx_vector2(x1,x2,y,N,os)  component_wise_product(N,(unsigned long *)(x1),(unsigned long *)(x2),(unsigned long *)(y))

#define add_vector16(x,y,z,N) component_wise_addition(N,(unsigned long*)(x),(unsigned long*)(y),(unsigned long*)(z)) 

#endif // EXPRESSMIMO_TARGET

char dB_fixed(unsigned int x);

char dB_fixed2(unsigned int x,unsigned int y);

int phy_phase_compensation_top (unsigned int pilot_type, unsigned int initial_pilot,
				unsigned int last_pilot, int ignore_prefix);

/*!\fn void phy_phase_compensation (short *ref_sch, short *tgt_sch, short *out_sym, int ignore_prefix, int aa, struct complex16 *perror_out);
This function is used by the EMOS to compensate the phase rotation of the RF. It has been designed for symbols of type CHSCH or SCH, but cannot be used for the data channels.
@param ref_sch reference symbol
@param tgt_sch target symbol
@param out_sym output of the operation
@param ignore_prefix  set to 1 if cyclic prefix has not been removed (by the hardware)
@param aa antenna index
@param perror_out phase error (output parameter)
*/
void phy_phase_compensation (short *ref_sch, short *tgt_sch, short *out_sym, int ignore_prefix, int aa, struct complex16 *perror_out );

int dot_product(short *x,
		short *y,
		unsigned int N, //must be a multiple of 8
		unsigned char output_shift);


void dft24(int *x,int *y,unsigned char scale_flag);
void dft36(int *x,int *y,unsigned char scale_flag);
void dft48(int *x,int *y,unsigned char scale_flag);
void dft60(int *x,int *y,unsigned char scale_flag);
void dft72(int *x,int *y,unsigned char scale_flag);
void dft96(int *x,int *y,unsigned char scale_flag);
void dft108(int *x,int *y,unsigned char scale_flag);
void dft120(int *x,int *y,unsigned char scale_flag);
void dft144(int *x,int *y,unsigned char scale_flag);
void dft180(int *x,int *y,unsigned char scale_flag);
void dft192(int *x,int *y,unsigned char scale_flag);
void dft216(int *x,int *y,unsigned char scale_flag);
void dft240(int *x,int *y,unsigned char scale_flag);
void dft288(int *x,int *y,unsigned char scale_flag);
void dft300(int *x,int *y,unsigned char scale_flag);

/** @} */ 


#endif //__PHY_TOOLS_DEFS__H__
