#include "PHY/TOOLS/defs.h"

/** @defgroup _numerical_ Useful Numerical Functions
 *@{
The present clause specifies several numerical functions for testing of digital communication systems.

-# Generation of Uniform Random Bits
-# Generation of Quantized Gaussian Random Variables
-# Generation of Floating-point Gaussian Random Variables
-# Generic Multipath Channel Generator
 * @defgroup _taus Tausworthe Uniform Random Variable Generator
 * @ingroup numerical 
 * @{
\fn inline unsigned int taus()
\brief Tausworthe Uniform Random Generator.  This is based on the hardware implementation described in 
  Lee et al, "A Hardware Gaussian Noise Generator Usign the Box-Muller Method and its Error Analysis," IEEE Trans. on Computers, 2006.
*/
inline unsigned int taus();

/* \fn void set_taus_seed(void)
\brief Sets the seed for the Tausworthe generator.
*@} */
void set_taus_seed(void);
/** @defgroup _gauss Generation of Quantized Gaussian Random Variables 
 * @ingroup numerical
 * @{
This set of routines are used to generate quantized (i.e. fixed-point) Gaussian random noise efficiently. 
The use of these routines allows for rapid computer simulation of digital communication systems. The method 
is based on a lookup-table of the quantized normal probability distribution.  The routines assume that the 
continuous-valued Gaussian random-variable,\f$x\f$ is quantized
to \f$N\f$ bits over the interval \f$[-L\sigma,L\sigma)\f$ where \f$N\f$ and \f$L\f$ control the precision 
and range of the quantization.  The 
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


/** \fn unsigned int *generate_gauss_LUT(unsigned char Nbits,unsigned char L)
\brief This routine generates a Gaussian pdf lookup table (LUT).  The table has \f$2^{\mathrm{Nbits}-1}\f$ entries which represent
the right half of the pdf.  The data stored in position \f$i\f$ is actually the scaled cumulative probability distribution, 
\f$2^{31}\mathrm{erf}\left(\frac{iL}{2^{N-1}}\right)\f$.  This represents the average number of times that the random variable
falls in the interval \f$\left[0,\frac{i}{2^{N-1}}\right)\f$.  This format allows for rapid conversion of uniform 32-bit
random variables to \f$N\f$-bit Gaussian random variables using binary search.
@see gauss
@param Nbits Number of bits for the output variable
@param L Number of standard deviations in range
*/
unsigned int *generate_gauss_LUT(unsigned char Nbits,unsigned char L);
 
/** \fn int gauss(unsigned int *gauss_LUT,unsigned char Nbits);
\brief This routine returns a zero-mean unit-variance Gaussian random variable.  
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
int gauss(unsigned int *gauss_LUT,unsigned char Nbits);

/* *@} */

double gaussdouble(double,double);
void randominit(void);






/** \fn void random_channel(double *amps,
                            double t_max, 
		    int channel_length,
		    double bw,
		    struct complex *ch,
		    double ricean_factor,
		    struct complex *phase);
\brief This routine generates a random channel response (time domain) according to a tapped delay line model. 
\param amps Linear amplitudes of the taps (length(amps)=channel_length). The taps are assumed to be spaced equidistantly between 0 and t_max. The values should sum up to 1.
\param t_max Maximum path delay in mus.
\param channel_length Number of taps.
\param bw Channel bandwidth in MHz.
\param ch Returned channel (length(ch)=(int)11+2*bw*t_max).
\param ricean_factor Ricean factor applied to all taps.
\param phase Phase of the first tap.
*/
void random_channel(double *amps,
		    double t_max, 
		    int channel_length,
		    double bw,
		    struct complex *ch,
		    double ricean_factor,
		    struct complex *phase);


/*\fn void multipath_channel(struct complex **ch,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       double *amps, 
		       double Td, 
		       double BW, 
		       double ricean_factor,
		       double aoa,
		       unsigned char nb_antennas_tx,
		       unsigned char nb_antennas_rx,
		       unsigned int length,
		       unsigned int channel_length,
		       unsigned int path_loss_dB);
\brief This function generates and applys a random frequency selective random channel model.
@param ch Pointer to spatio-temporal channel coefficients
@param tx_sig_re input signal (real component) 
@param tx_sig_im input signal (imaginary component) 
@param rx_sig_re output signal (real component)
@param rx_sig_im output signal (imaginary component)
@param amps Amplitude of multipath components (average)
@param Td maximum time-delay spread
@param BW system bandwidth (sampling time)
@param ricean_factor Ricean factor (linear)
@param aoa angle of arrival of wavefront
@param nb_antennas_tx Number of TX antennas
@param nb_antennas_rx Number of RX antennas
@param length Length of input signal
@param channel_length Length of channel (time samples)
@param path_loss_dB Path loss in dB
*/
void multipath_channel(struct complex **ch,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       double *amps, 
		       double Td, 
		       double BW, 
		       double ricean_factor,
		       double aoa,
		       unsigned char nb_antennas_tx,
		       unsigned char nb_antennas_rx,
		       unsigned int length,
		       unsigned int channel_length,
		       unsigned int path_loss_dB);
/* *@} */
