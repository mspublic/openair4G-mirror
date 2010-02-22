/* Definitions for LTE Reference signals */
/* Author R. Knopp / EURECOM / OpenAirInterface.org */
#ifndef __LTE_REFSIG_DEFS__H__
#define __LTE_REFSIG_DEFS__H__
#include "PHY/defs.h"

/** @ingroup _PHY_REF_SIG
 * @{
 */

/*! \brief gold sequenquence generator
\param x1 
\param x2 this should be set to c_init if reset=1
\param reset resets the generator
\return 32 bits of the gold sequence
*/
unsigned int lte_gold_generic(unsigned int *x1, unsigned int *x2, unsigned char reset);


/*! \fn void lte_gold(LTE_DL_FRAME_PARMS *frame_parms)
\brief This function generates the LTE Gold sequence (36-211, Sec 7.2)
@param frame_parms LTE DL Frame parameters
*/
void lte_gold(LTE_DL_FRAME_PARMS *frame_parms);

/*!
\fn void lte_dl_cell_spec(mod_sym_t *output,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      unsigned char eNb_id,
		      unsigned char Ns,
		      unsigned char l,
		      unsigned char p)
\brief This function generates the cell-specific reference signal sequence (36-211, Sec 6.10.1.1)
@param output Output vector for OFDM symbol (Frequency Domain)
@param amp Q15 amplitude
@param frame_parms LTE DL Frame Parameters
@param eNb_id Nid2 (0,1,2)
@param Ns Slot number (0..19)
@param l symbol (0,1) - Note 1 means 3!
@param p antenna intex
*/
int lte_dl_cell_spec(mod_sym_t *output,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned char eNb_id,
		     unsigned char Ns,
		     unsigned char l,
		     unsigned char p);

/*!
\fn void lte_dl_cell_spec_rx(int *output,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 unsigned char Ns,
			 unsigned char l,
			 unsigned char p)
\brief This function generates the cell-specific reference signal sequence (36-211, Sec 6.10.1.1) for channel estimation upon reception
@param output Output vector for OFDM symbol (Frequency Domain)
@param frame_parms LTE DL Frame Parameters
@param Ns Slot number (0..19)
@param l symbol (0,1) - Note 1 means 3!
@param p antenna intex
*/
int lte_dl_cell_spec_rx(int *output,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 unsigned char Ns,
			 unsigned char l,
			 unsigned char p);

void generate_ul_ref_sigs(void);
void generate_ul_ref_sigs_rx(void);

void free_ul_ref_sigs(void);

/*!
\brief This function generate the sounding reference symbol (SRS) for the uplink. The SRS is always transmitted in the last symbol of the slot and uses the full bandwidth. This function makes the following simplifications wrt LTE Rel.8:
 1) the SRS in OpenAir is quantized to a QPSK sequence.
 2) no group hopping, no sequence hopping
 3) u = N_id_cell%30, v=0, alpha=0, 
 4) Msc_RS = 300, k_0=0
@param txdataF pointer to the frequency domain TX signal
@param amp amplitudte of the transmit signal (irrelevant for #ifdef IFFT_FPGA)
@param frame_parms LTE DL Frame Parameters
@sub_frame_offset  Offset of this subframe in units of subframes
*/

int lte_generate_srs(mod_sym_t **txdataF,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
		     unsigned int sub_frame_offset);


#endif
