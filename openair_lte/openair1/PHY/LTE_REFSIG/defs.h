/* Definitions for LTE Reference signals */
/* Author R. Knopp / EURECOM / OpenAirInterface.org */
#ifndef __LTE_REFSIG_DEFS__H__
#define __LTE_REFSIG_DEFS__H__
#include "PHY/defs.h"

/** @ingroup _PHY_REF_SIG
 * @{
\fn void lte_gold(LTE_DL_FRAME_PARMS *frame_parms)
\brief This function generates the LTE Gold sequence (36-211, Sec 7.2)
@param frame_parms LTE DL Frame parameters
*/
void lte_gold(LTE_DL_FRAME_PARMS *frame_parms);

/*!
\fn void lte_dl_cell_spec(mod_sym_t *output,
		      short amp,
		      LTE_DL_FRAME_PARMS *frame_parms,
		      unsigned char Ns,
		      unsigned char l,
		      unsigned char p)
\brief This function generates the cell-specific reference signal sequence (36-211, Sec 6.10.1.1)
@param output Output vector for OFDM symbol (Frequency Domain)
@param amp Q15 amplitude
@param frame_parms LTE DL Frame Parameters
@param Ns Slot number (0..19)
@param l symbol (0,1) - Note 1 means 3!
@param p antenna intex
*/
int lte_dl_cell_spec(mod_sym_t *output,
		     short amp,
		     LTE_DL_FRAME_PARMS *frame_parms,
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

#endif
