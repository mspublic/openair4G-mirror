/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
#ifndef __LTE_ESTIMATION_DEFS__H__
#define __LTE_ESTIMATION_DEFS__H__

#include "PHY/defs.h"
/*
#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif
 */

/** @addtogroup _PHY_PARAMETER_ESTIMATION_BLOCKS_
 * @{
 */

/*!\brief Timing drift hysterisis in samples*/
#define SYNCH_HYST 1


/*!
\brief This function allocates memory needed for the synchronization.
\param frame_parms LTE DL frame parameter structure
 */

int lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms); //LTE_UE_COMMON *common_vars

/*! \fn void lte_sync_time_free()
\brief This function frees the memory allocated by lte_sync_time_init.
 */
void lte_sync_time_free(void);

/*! 
\brief This function performs the coarse timing synchronization.
The algorithm uses a time domain correlation with a downsampled version of the received signal. 
\param rxdata Received time domain data for all rx antennas
\param frame_parms LTE DL frame parameter structure
\param eNB_id return value with the eNb_id
\return sync_pos Position of the sync within the frame (downsampled) if successfull and -1 if there was an error or no peak was detected.
 */
int lte_sync_time(int **rxdata, 
    LTE_DL_FRAME_PARMS *frame_parms,
    int *eNB_id);

/*! 
\brief This function performs detection of the PRACH (=SRS) at the eNb to estimate the timing advance
The algorithm uses a time domain correlation with a downsampled version of the received signal. 
\param rxdata Received time domain data for all rx antennas
\param frame_parms LTE DL frame parameter structure
\param length Length for correlation
\param peak_val pointer to value of returned peak 
\param sync_corr_eNb pointer to correlation buffer
\return sync_pos Position of the sync within the frame (downsampled) if successfull and -1 if there was an error or no peak was detected.
 */
int lte_sync_time_eNB(int32_t **rxdata,
    LTE_DL_FRAME_PARMS *frame_parms,
    uint32_t length,
    uint32_t *peak_val,
    uint32_t *sync_corr_eNb);

int lte_sync_time_eNB_emul(PHY_VARS_eNB *phy_vars_eNb,
    uint8_t sect_id,
    int32_t *sync_val);

/*!
\brief This function performs channel estimation including frequency and temporal interpolation
\param phy_vars_ue Pointer to UE PHY variables
\param eNB_id Index of target eNB
\param eNB_offset Offset for interfering eNB (in terms cell ID mod 3)
\param Ns slot number (0..19)
\param p antenna port 
\param l symbol within slot
\param symbol symbol within frame
*/
int lte_dl_channel_estimation(PHY_VARS_UE *phy_vars_ue,
    module_id_t eNB_id,
    uint8_t eNB_offset,
    uint8_t Ns,
    uint8_t p,
    uint8_t l,
    uint8_t symbol);


int lte_dl_msbfn_channel_estimation(PHY_VARS_UE *phy_vars_ue,
    module_id_t eNB_id,
    uint8_t eNB_offset,
    int subframe,
    unsigned char l,
    unsigned char symbol);

int lte_dl_mbsfn_channel_estimation(PHY_VARS_UE *phy_vars_ue,
    module_id_t eNB_id,
    uint8_t eNB_offset,
    int subframe,
    unsigned char l);

/*
#ifdef EMOS
int lte_dl_channel_estimation_emos(int dl_ch_estimates_emos[NB_ANTENNAS_RX*NB_ANTENNAS_TX][N_RB_DL_EMOS*N_PILOTS_PER_RB*N_SLOTS_EMOS],
				   int **rxdataF,
				   LTE_DL_FRAME_PARMS *frame_parms,
				   unsigned char Ns,
				   unsigned char p,
				   unsigned char l,
				   unsigned char sector);
#endif
 */

/*!
\brief Frequency offset estimation for LTE
We estimate the frequency offset by calculating the phase difference between channel estimates for symbols carrying pilots (l==0 or l==3/4). We take a moving average of the phase difference.
\param dl_ch_estimates pointer to structure that holds channel estimates (one slot)
\param frame_parms pointer to LTE frame parameters
\param l symbol within slot
\param freq_offset pointer to the returned frequency offset
 */
int lte_est_freq_offset(int **dl_ch_estimates,
    LTE_DL_FRAME_PARMS *frame_parms,
    int l,
    int* freq_offset);

int lte_mbsfn_est_freq_offset(int **dl_ch_estimates,
    LTE_DL_FRAME_PARMS *frame_parms,
    int l,
    int* freq_offset);

/*! \brief Tracking of timing for LTE
This function computes the time domain channel response, finds the peak and adjusts the timing in pci_interface.offset accordingly.
\param frame_parms LTE DL frame parameter structure
\param phy_vars_ue Pointer to UE PHY data structure
\param eNb_id 
\param clear If clear==1 moving average filter is reset
\param coef Coefficient of the moving average filter (Q1.15)
 */

void lte_adjust_synch(LTE_DL_FRAME_PARMS *frame_parms,
    PHY_VARS_UE *phy_vars_ue,
    module_id_t eNb_id,
    unsigned char clear,
    short coef);

//! \brief this function fills the PHY_VARS_UE->PHY_measurement structure
void lte_ue_measurements(PHY_VARS_UE *phy_vars_ue,
    unsigned int subframe_offset,
    unsigned char N0_symbol,
    unsigned char abstraction_flag);

//! \brief This function performance RSRP/RSCP measurements
void ue_rrc_measurements(PHY_VARS_UE *phy_vars_ue,
    uint8_t slot,
    uint8_t abstraction_flag);

void lte_ue_measurements_emul(PHY_VARS_UE *phy_vars_ue,uint8_t last_slot,uint8_t eNB_id);

/*! \brief Function to return the path-loss based on the UE cell-specific reference signal strength and transmission power of eNB
@param Mod_id Module ID for UE
@param eNB_index Index of eNB on which to act
@returns Path loss in dB
 */
int16_t get_PL(module_id_t Mod_id,uint8_t CC_id,uint8_t eNB_index);
uint32_t get_RSRP(module_id_t Mod_id,uint8_t CC_id,uint8_t eNB_index);
uint32_t get_RSRQ(module_id_t Mod_id,uint8_t CC_id,uint8_t eNB_index);
uint8_t get_n_adj_cells(module_id_t Mod_id,uint8_t CC_id);
uint32_t get_rx_total_gain_dB(module_id_t Mod_id,uint8_t CC_id);
uint32_t get_RSSI(module_id_t Mod_id,uint8_t CC_id);
int8_t set_RSRP_filtered(module_id_t Mod_id,uint8_t CC_id,uint8_t eNB_index,float rsrp);
int8_t set_RSRQ_filtered(module_id_t Mod_id,uint8_t CC_id,uint8_t eNB_index,float rstq);

//! Automatic gain control
void phy_adjust_gain (PHY_VARS_UE *phy_vars_ue,
    unsigned char eNB_id);

int lte_ul_channel_estimation(PHY_VARS_eNB *phy_vars_eNB,
    module_id_t eNB_id,
    module_id_t UE_id,
    uint8_t subframe,
    uint8_t l,
    uint8_t Ns,
    uint8_t cooperation_flag);

int16_t lte_ul_freq_offset_estimation(LTE_DL_FRAME_PARMS *frame_parms,
                                  int32_t *ul_ch_estimates,
                                  uint16_t nb_rb);

int lte_srs_channel_estimation(LTE_DL_FRAME_PARMS *frame_parms,
			       LTE_eNB_COMMON *eNb_common_vars,
			       LTE_eNB_SRS *eNb_srs_vars,
			       SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,
			       unsigned char sub_frame_number,
			       unsigned char eNb_id);

int lte_est_timing_advance(LTE_DL_FRAME_PARMS *frame_parms,
			   LTE_eNB_SRS *lte_eNb_srs,
			   unsigned int *eNb_id,
			   unsigned char clear,
			   unsigned char number_of_cards,
			   short coef);

int lte_est_timing_advance_pusch(PHY_VARS_eNB* phy_vars_eNB,module_id_t UE_id,uint8_t subframe);

void lte_eNB_I0_measurements(PHY_VARS_eNB *phy_vars_eNb,
    module_id_t eNB_id,
    unsigned char clear);

void lte_eNB_I0_measurements_emul(PHY_VARS_eNB *phy_vars_eNb,
				  uint8_t sect_id);


void lte_eNB_srs_measurements(PHY_VARS_eNB *phy_vars_eNb,
    module_id_t eNB_id,
    module_id_t UE_id,
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
