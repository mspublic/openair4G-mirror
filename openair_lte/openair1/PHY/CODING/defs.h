#ifndef __CODING_DEFS__H__
#define __CODING_DEFS__H__

/** @ingroup _PHY_CODING_BLOCKS_
 * @{
\fn void ccodedot11_encode(unsigned int numbytes,unsigned char *inPtr,unsigned char *outPtr,unsigned char puncturing)
\brief This function implements a rate 1/2 constraint length 7 convolutional code.
@param numbytes Number of bytes to encode
@param inPtr Pointer to input buffer
@param outPtr Pointer to output buffer
@param puncturing Puncturing pattern (Not used at present, to be removed)
*/
void ccodedot11_encode (unsigned int numbytes, 
			unsigned char *inPtr, 
			unsigned char *outPtr, 
			unsigned char puncturing);

/*!\fn void ccodedot11_init(void)
\brief This function initializes the generator polynomials for an 802.11 convolutional code.*/
void ccodedot11_init(void);		   

/*!\fn void ccodedot11_init_inv(void)
\brief This function initializes the trellis structure for decoding an 802.11 convolutional code.*/
void ccodedot11_init_inv(void);		   

/*\fn void threegpplte_turbo_encoder(unsigned char *input,unsigned short input_length_bytes,unsigned char *output,unsigned short interleaver_f1,unsigned short interleaver_f2)
\brief This function implements a rate 1/3 8-state parralel concatenated turbo code (3GPP-LTE).
@param input Pointer to input buffer
@param input_length_bytes Number of bytes to encode
@param output Pointer to output buffer
@param interleaver_f1 F1 generator
@param interleaver_f2 F2 generator
*/
void threegpplte_turbo_encoder(unsigned char *input,
			       unsigned short input_length_bytes,
			       unsigned char *output,
			       unsigned short interleaver_f1,
			       unsigned short interleaver_f2);


/*!\fn void crcTableInit(void)
\brief This function initializes the different crc tables.*/
void crcTableInit (void);

/*!\fn void crc24a(unsigned char *inPtr, int bitlen)
\brief This computes a 24-bit crc ('a' variant for overall transport block) 
based on 3GPP UMTS/LTE specifications.
@param inPtr Pointer to input byte stream
@param bitlen length of inputs in bits
*/
unsigned int crc24a (unsigned char *inPtr, int bitlen);

/*!\fn void crc24b(unsigned char *inPtr, int bitlen)
\brief This computes a 24-bit crc ('b' variant for transport-block segments) 
based on 3GPP UMTS/LTE specifications.
@param inPtr Pointer to input byte stream
@param bitlen length of inputs in bits
*/
unsigned int crc24b (unsigned char *inPtr, int bitlen);

/*!\fn void crc16(unsigned char *inPtr, int bitlen)
\brief This computes a 16-bit crc based on 3GPP UMTS specifications.
@param inPtr Pointer to input byte stream
@param bitlen length of inputs in bits*/
unsigned int crc16 (unsigned char *inPtr, int bitlen);

/*!\fn void crc12(unsigned char *inPtr, int bitlen)
\brief This computes a 12-bit crc based on 3GPP UMTS specifications.
@param inPtr Pointer to input byte stream
@param bitlen length of inputs in bits*/
unsigned int crc12 (unsigned char *inPtr, int bitlen);

/*!\fn void crc8(unsigned char *inPtr, int bitlen)
\brief This computes a 8-bit crc based on 3GPP UMTS specifications.
@param inPtr Pointer to input byte stream
@param bitlen length of inputs in bits*/
unsigned int crc8  (unsigned char *inPtr, int bitlen);

/*!\fn void phy_viterbi_dot11_sse2(char *y, unsigned char *decoded_bytes, unsigned short n)
\brief This routine performs a SIMD optmized Viterbi decoder for the 802.11 64-state convolutional code.
@param y Pointer to soft input (coded on 8-bits but should be limited to 4-bit precision to avoid overflow)
@param decoded_bytes Pointer to decoded output
@param n Length of input/trellis depth in bits*/
void phy_viterbi_dot11_sse2(char *y,unsigned char *decoded_bytes,unsigned short n);

/*!\fn void phy_generate_viterbi_tables(void)
\brief This routine initializes metric tables for the optimized Viterbi decoder.
*/
void phy_generate_viterbi_tables( void );


/*!\fn int rate_matching(unsigned int N_coded, 
		         unsigned int N_input,
		         unsigned char *inPtr, 
		         unsigned char N_bps,
		         unsigned int off)
\brief This routine performs random puncturing of a coded sequence.
@param N_coded Number of coding bits to be output
@param N_input Number of input bits
@param *inPtr Pointer to coded input
@param N_bps Number of modulation bits per symbol (1,2,4)
@param off Offset for seed

*/
int rate_matching(unsigned int N_coded, 
		   unsigned int N_input,
		   unsigned char *inPtr, 
		   unsigned char N_bps,
		   unsigned int off);


int rate_matching_lte(unsigned int N_coded, 
		      unsigned int N_input, 
		      unsigned char *inPtr,
		      unsigned int off);

/*!\fn void threegpp_turbo_decoder(short *y, unsigned char *decoded_bytes,unsigned short n,unsigned short interleaver_f1,unsigned short interleaver_f2,unsigned char max_iterations,unsigned char crc_len)
\brief This routine performs max-logmap detection for the 3GPP turbo code (with termination)
@param decoded_bytes Pointer to decoded output
@param n number of coded bits (including tail bits)
@param max_iterations The maximum number of iterations to perform
@param interleaver_f1 F1 generator
@param interleaver_f2 F2 generator
@param crc_len Length of 3GPP crc (0 none, 1 8-bit, 2 16-bit, 3 24-bit)
@returns number of iterations used (this is 1+max if incorrect crc or if crc_len=0)
*/
unsigned char phy_threegpplte_turbo_decoder(short *y,
					    unsigned char *decoded_bytes,
					    unsigned short n,			       
					    unsigned short interleaver_f1,
					    unsigned short interleaver_f2,
					    unsigned char max_iterations,
					    unsigned char crc_len);


/** @} */

unsigned int crcbit (unsigned char * , 
		     int, 
		     unsigned int);

short reverseBits(int ,int);
void phy_viterbi_dot11(char *,unsigned char *,unsigned short);

#endif
