#ifndef __LTE_ESTIMATION_DEFS__H__
#define __LTE_ESTIMATION_DEFS__H__

/** @addtogroup _PHY_STRUCTURES_
* @{
\fn void phy_synch_time_init(void)
\brief Initialize the Initial Timing synchronization engine

*@} */

#include "PHY/defs.h"

/** @addtogroup _PHY_PARAMETER_ESTIMATION_BLOCKS_
* @{
*/

/*!\brief Timing drift hysterisis in samples*/
#define SYNCH_HYST 4


/*! \fn void lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms)
\brief This function allocates memory needed for the synchronization.
\param frame_parms LTE DL frame parameter structure
*/

int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms, LTE_UE_COMMON *common_vars);

/*! \fn void lte_sync_time_free()
\brief This function frees the memory allocated by lte_sync_time_init.
*/
void lte_sync_time_free(void);

/*! \fn short lte_sync_time(int **rxdata, LTE_DL_FRAME_PARMS *frame_parms)
\brief This function performs the coarse timing synchronization.

The algorithm uses a time domain correlation with a downsampled version of the received signal. 

\param rxdata Received time domain data for all rx antennas
\param frame_parms LTE DL frame parameter structure
\return sync_pos Position of the sync within the frame (downsampled) if successfull and -1 if there was an error or no peak was detected.
*/

short lte_sync_time(int **rxdata, LTE_DL_FRAME_PARMS *frame_parms);

int lte_dl_channel_estimation(int **dl_ch_estimates,
			      int **rxdataF,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char Ns,
			      unsigned char p,
			      unsigned char l,
			      unsigned char symbol);


/** @} */ 
#endif
