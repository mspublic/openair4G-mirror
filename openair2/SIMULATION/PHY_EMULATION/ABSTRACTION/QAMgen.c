/*! @defgroup _QAMgen 
@ingroup numerical

This set of routines are used to generate quantized (i.e. fixed-point) random quadrature-amplitude modulated signal sequences.  These are
random number generators which can be used to feed pulse-shaping or OFDM modulation systems.*/
struct complex
{
  double r;
  double i;
};



/*This routine generates a random QAM sequence for and OFDM input
@param data Pointer to the modulated output data sequence
@param amp Amplitude \f$0--2^{15}\f$
@param N Length of the output sequence
@param Nu Number of useful output dimensions (for OFDM modulator)
@param M Modulation order (0 - QPSK, 1 - 16QAM)
*/


void QAM_input(struct complex *data,  
	  short amp,                   
	  int N,                       
	  int Nu,                      
	  char M) {                    

  int i,rv;
  int FCO = (N-(Nu>>1));   // First non-zero carrier offset


  for (i=0;i<N;i++) {
    data[i].r = 0;
    data[i].i = 0;
  }

  for (i=0;i<Nu;i++) {

    rv = taus();           // get a 32-bit uniform random variable

    switch (M) {

    case 0 :   // QPSK
      data[(i+FCO)%N].r = (rv&1) ? -amp : amp;
      data[(i+FCO)%N].i = ((rv>>1)&1) ? -amp : amp;

      //      printf("data[%d] = %f\n",(i+FCO)%N,data[(i+FCO)%N].r);

      break;
    case 1 :   // 16QAM 
      data[(i+FCO)%N].r = (2*(rv&3) - 1)*amp/3;
      data[(i+FCO)%N].i = (2*((rv>>2)&3) - 1)*amp/3;
      break;
    default:
      break;
    }
    
  }
}
