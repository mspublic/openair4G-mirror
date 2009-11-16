#ifndef __CODING_DEFS__H__
#define __CODING_DEFS__H__

#define CRC24_A 0
#define CRC24_B 1
#define CRC16 2
#define CRC8 3

#define MAX_TURBO_ITERATIONS 6

#define LTE_NULL 2

/** @ingroup _PHY_CODING_BLOCKS_
 * @{
*/

/* \fn lte_segmentation(unsigned char *input_buffer,
	  	        unsigned char **output_buffers,
		        unsigned int B,
		        unsigned int *C,
		        unsigned int *Cplus,
		        unsigned int *Cminus,
		        unsigned int *Kplus,
		        unsigned int *Kminus,
		        unsigned int *F)
\brief This function implements the LTE transport block segmentation algorithm from 36-212, V8.6 2009-03.
@param input_buffer
@param output_buffers
@param B
@param C
@param Cplus
@param Cminus
@param Kplus
@param Kminus
@param F
*/
void lte_segmentation(unsigned char *input_buffer,
		      unsigned char **output_buffers,
		      unsigned int B,
		      unsigned int *C,
		      unsigned int *Cplus,
		      unsigned int *Cminus,
		      unsigned int *Kplus,
		      unsigned int *Kminus,
		      unsigned int *F);

unsigned int sub_block_interleaving_turbo(unsigned int D, unsigned char *d,unsigned char *w);

void sub_block_deinterleaving_turbo(unsigned int D, short *d,short *w);

/** \fn generate_dummy_w(unsigned int D, unsigned char *w,unsigned char F)
\brief This function generates a dummy interleaved sequence (first row) for receiver, in order to identify
the NULL positions used to make the matrix complete.
\param D Number of systematic bits plus 4 (plus 4 for termination)
\param w This is the dummy sequence (first row), it will contain zeros and at most 31 "LTE_NULL" values
\param F Number of filler bits due added during segmentation
\returns Interleaving matrix cardinality (Kpi from 36-212)
*/

unsigned int generate_dummy_w(unsigned int D, unsigned char *w, unsigned char F);


/** \fn lte_rate_matching_turbo(unsigned int RTC[8],
			     unsigned int G, 
			     unsigned char *w,
			     unsigned char *e, 
			     unsigned char C, 
			     unsigned int Nsoft, 
			     unsigned char Mdlharq,
			     unsigned char Kmimo,
			     unsigned char rvidx,
			     unsigned char Qm, 
			     unsigned char Nl, 
			     unsigned char r)

\brief This is the LTE rate matching algorithm for Turbo-coded channels (e.g. DLSCH,ULSCH).  It is taken directly from 36-212 (Rel 8 8.6, 2009-03), pp.16-18 )
\param RTC R^TC_subblock from subblock interleaver (number of rows in interleaving matrix) for up to 8 segments
\param G This the number of coded transport bits allocated in sub-frame
\param w This is a pointer to the w-sequence (second interleaver output)
\param e This is a pointer to the e-sequence (rate matching output, channel input/output bits)
\param C Number of segments (codewords) in the sub-frame
\param Nsoft Total number of soft bits (from UE capabilities in 36-306)
\param Mdlharq Number of HARQ rounds 
\param Kmimo MIMO capability for this DLSCH (0 = no MIMO)
\param rvidx round index (0-3)
\param Qm modulation order (2,4,6)
\param Nl number of layers (1,2)
\param r segment number
*/


unsigned int lte_rate_matching_turbo(unsigned int RTC,
				     unsigned int G, 
				     unsigned char *w,
				     unsigned char *e, 
				     unsigned char C, 
				     unsigned int Nsoft, 
				     unsigned char Mdlharq,
				     unsigned char Kmimo,
				     unsigned char rvidx,
				     unsigned char Qm, 
				     unsigned char Nl, 
				     unsigned char r);

/** \fn lte_rate_matching_turbo_rx(unsigned int RTC,
				unsigned int G, 
				short *w,
				unsigned char *dummy_w,
				short *soft_input, 
				unsigned char C, 
				unsigned int Nsoft, 
				unsigned char Mdlharq,
				unsigned char Kmimo,
				unsigned char rvidx,
				unsigned char Qm, 
				unsigned char Nl, 
				unsigned char r)

\brief This is the LTE rate matching algorithm for Turbo-coded channels (e.g. DLSCH,ULSCH).  It is taken directly from 36-212 (Rel 8 8.6, 2009-03), pp.16-18 )
\param RTC R^TC_subblock from subblock interleaver (number of rows in interleaving matrix)
\param G This the number of coded transport bits allocated in sub-frame
\param w This is a pointer to the soft w-sequence (second interleaver output) with soft-combined outputs from successive HARQ rounds 
\param dummy_w This is the first row of the interleaver matrix for identifying/discarding the "LTE-NULL" positions
\param soft_input This is a pointer to the soft channel output 
\param C Number of segments (codewords) in the sub-frame
\param Nsoft Total number of soft bits (from UE capabilities in 36-306)
\param Mdlharq Number of HARQ rounds 
\param Kmimo MIMO capability for this DLSCH (0 = no MIMO)
\param rvidx round index (0-3)
\param Qm modulation order (2,4,6)
\param Nl number of layers (1,2)
\param r segment number
*/

unsigned int lte_rate_matching_turbo_rx(unsigned int RTC,
					unsigned int G, 
					short *w,
					unsigned char *dummy_w,
					short *soft_input, 
					unsigned char C, 
					unsigned int Nsoft, 
					unsigned char Mdlharq,
					unsigned char Kmimo,
					unsigned char rvidx,
					unsigned char Qm, 
					unsigned char Nl, 
					unsigned char r);


/** \fn void ccodedot11_encode(unsigned int numbytes,unsigned char *inPtr,unsigned char *outPtr,unsigned char puncturing)
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

/*\fn void threegpplte_turbo_encoder(unsigned char *input,unsigned short input_length_bytes,unsigned char *output,unsigned char F,unsigned short interleaver_f1,unsigned short interleaver_f2)
\brief This function implements a rate 1/3 8-state parralel concatenated turbo code (3GPP-LTE).
@param input Pointer to input buffer
@param input_length_bytes Number of bytes to encode
@param output Pointer to output buffer
@param F Number of filler bits at input
@param interleaver_f1 F1 generator
@param interleaver_f2 F2 generator
*/
void threegpplte_turbo_encoder(unsigned char *input,
			       unsigned short input_length_bytes,
			       unsigned char *output,
			       unsigned char F,
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



/*!\fn void threegpp_turbo_decoder(short *y, unsigned char *decoded_bytes,unsigned short n,unsigned short interleaver_f1,unsigned short interleaver_f2,unsigned char max_iterations,unsigned char crc_len)
\brief This routine performs max-logmap detection for the 3GPP turbo code (with termination)
@param decoded_bytes Pointer to decoded output
@param n number of coded bits (including tail bits)
@param max_iterations The maximum number of iterations to perform
@param interleaver_f1 F1 generator
@param interleaver_f2 F2 generator
@param crc_type Length of 3GPPLTE crc (CRC24a,CRC24b,CRC16,CRC8)
@param F Number of filler bits at start of packet 
@returns number of iterations used (this is 1+max if incorrect crc or if crc_len=0)
*/
unsigned char phy_threegpplte_turbo_decoder(short *y,
					    unsigned char *decoded_bytes,
					    unsigned short n,			       
					    unsigned short interleaver_f1,
					    unsigned short interleaver_f2,
					    unsigned char max_iterations,
					    unsigned char crc_type,
					    unsigned char F);


/** @} */

unsigned int crcbit (unsigned char * , 
		     int, 
		     unsigned int);

short reverseBits(int ,int);
void phy_viterbi_dot11(char *,unsigned char *,unsigned short);

#endif
