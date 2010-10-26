#ifndef __LTE_ESTIMATION_DEFS__H__
#define __LTE_ESTIMATION_DEFS__H__

#include "PHY/defs.h"
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

/** @addtogroup _PHY_PARAMETER_ESTIMATION_BLOCKS_
* @{
*/

/*!\brief Timing drift hysterisis in samples*/
#define SYNCH_HYST 4


/*! \fn void lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms, LTE_UE_COMMON *common_vars)
\brief This function allocates memory needed for the synchronization.
\param frame_parms LTE DL frame parameter structure
\param common_vars LTE DL common RX variables structure
*/

int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms); //LTE_UE_COMMON *common_vars

/*! \fn void lte_sync_time_free()
\brief This function frees the memory allocated by lte_sync_time_init.
*/
void lte_sync_time_free(void);

/*! \fn short lte_sync_time(int **rxdata, LTE_DL_FRAME_PARMS *frame_parms)
\brief This function performs the coarse timing synchronization.

The algorithm uses a time domain correlation with a downsampled version of the received signal. 

\param rxdata Received time domain data for all rx antennas
\param frame_parms LTE DL frame parameter structure
\param eNb_id return value with the eNb_id
\return sync_pos Position of the sync within the frame (downsampled) if successfull and -1 if there was an error or no peak was detected.
*/

int lte_sync_time(int **rxdata, LTE_DL_FRAME_PARMS *frame_parms, int length, int* eNb_id);

int lte_sync_time_eNb(int **rxdata, ///rx data in time domain
		      LTE_DL_FRAME_PARMS *frame_parms,
		      int eNb_id,
		      int length,
		      int* peak_val,
		      unsigned int* sync_corr_eNb);

int lte_sync_time_eNb_emul(PHY_VARS_eNB *phy_vars_eNb,
			   u8 sect_id,
			   s32 *sync_val);

/*! \fn int lte_dl_channel_estimation(int **dl_ch_estimates,
			      int **rxdataF,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char Ns,
			      unsigned char p,
			      unsigned char l,
			      unsigned char symbol)
\brief This function performs channel estimation including frequency and temporal interpolation
\param dl_ch_estimates pointer to structure that holds channel estimates (one slot)
\param rxdataF pointer to received data in freq domain
\param frame_parms pointer to LTE frame parameters
\param Ns slot number (0..19)
\param p antenna port 
\param l symbol within slot
\param symbol symbol within frame
*/

int lte_dl_channel_estimation(int **dl_ch_estimates,
			      int **rxdataF,
			      unsigned char eNb_id,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char Ns,
			      unsigned char p,
			      unsigned char l,
			      unsigned char symbol);

#ifdef EMOS
int lte_dl_channel_estimation_emos(int dl_ch_estimates_emos[NB_ANTENNAS_RX*NB_ANTENNAS_TX][N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS],
				   int **rxdataF,
				   LTE_DL_FRAME_PARMS *frame_parms,
				   unsigned char Ns,
				   unsigned char p,
				   unsigned char l,
				   unsigned char sector);
#endif

/*! \fn int lte_est_freq_offset(int **dl_ch_estimates,
			LTE_DL_FRAME_PARMS frame_parms,
			int aa,
			int l)
\brief Frequency offset estimation for LTE
We estimate the frequency offset by calculating the phase difference between channel estimates for symbols carrying pilots (l==0 or l==3/4). We take a moving average of the phase difference.
\param dl_ch_estimates pointer to structure that holds channel estimates (one slot)
\param frame_parms pointer to LTE frame parameters
\param l symbol within slot
*/
int lte_est_freq_offset(int **dl_ch_estimates,
			LTE_DL_FRAME_PARMS *frame_parms,
			int l,
			int* freq_offset);

/*! \brief Tracking of timing for LTE
This function computes the time domain channel response, finds the peak and adjusts the timing in pci_interface.offset accordingly.
\param frame_parms LTE DL frame parameter structure
\param phy_vars_ue 
\param ue_common LTE DL common RX variables structure
\param clear If clear==1 moving average filter is reset
\param coeff Coefficient of the moving average filter (Q1.15)
*/

void lte_adjust_synch(LTE_DL_FRAME_PARMS *frame_parms,
		      PHY_VARS_UE *phy_vars_ue,
		      unsigned char eNb_id,
		      unsigned char clear,
		      short coef);

//! \brief this function fills the PHY_VARS_UE->PHY_measurement structure
void lte_ue_measurements(PHY_VARS_UE *phy_vars_ue,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 unsigned int subframe_offset,
			 unsigned char N0_symbol,
			 unsigned char init_averaging);

void lte_ue_measurements_emul(PHY_VARS_UE *phy_vars_ue,u8 last_slot,u8 eNB_id);

//! Automatic gain control
void phy_adjust_gain (unsigned char clear,
		      short coef,
		      unsigned char chsch_ind,
		      PHY_VARS_UE *phy_vars_ue);


int lte_ul_channel_estimation(int **ul_ch_estimates,
			      int **ul_ch_estimates_0,
			      int **ul_ch_estimates_1,
			      int **rxdataF_ext,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      unsigned char l,
			      unsigned char Ns,
			      unsigned int N_rb_alloc,
			      unsigned char relay_flag,
			      unsigned char diversity_scheme);

int lte_srs_channel_estimation(LTE_DL_FRAME_PARMS *frame_parms,
			       LTE_eNB_COMMON *eNb_common_vars,
			       LTE_eNB_SRS *eNb_srs_vars,
			       SRS_param_t *SRS_parms,
			       unsigned char sub_frame_number,
			       unsigned char eNb_id);

int lte_est_timing_advance(LTE_DL_FRAME_PARMS *frame_parms,
			   LTE_eNB_SRS *lte_eNb_srs,
			   unsigned int *eNb_id,
			   unsigned char clear,
			   unsigned char number_of_cards,
			   short coef);

void lte_eNB_I0_measurements(PHY_VARS_eNB *phy_vars_eNb,
			     unsigned char eNB_id,
			     unsigned char clear);

void lte_eNB_I0_measurements_emul(PHY_VARS_eNB *phy_vars_eNb,
				  u8 sect_id);


void lte_eNB_srs_measurements(PHY_VARS_eNB *phy_vars_eNb,
			      unsigned char eNB_id,
			      unsigned char UE_id,
			      unsigned char init_averaging);


void freq_equalization(LTE_DL_FRAME_PARMS *frame_parms,
		       int **rxdataF_comp,
		       int **ul_ch_mag,
		       int **ul_ch_mag_b,
		       unsigned char symbol,
		       unsigned short Msc_RS,
		       unsigned char Qm);

/** @} */ 
#endif
