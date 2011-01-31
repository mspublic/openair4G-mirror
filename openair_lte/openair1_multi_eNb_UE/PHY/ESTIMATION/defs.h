/** @addtogroup _PHY_STRUCTURES_
* @{
\fn void phy_synch_time_init(void)
\brief Initialize the Initial Timing synchronization engine

*@} */

#include "PHY/defs.h"

void phy_synch_time_init(void );

/** @addtogroup _PHY_PARAMETER_ESTIMATION_BLOCKS_
* @{
*/

/*!\brief Timing drift hysterisis in samples*/
#define SYNCH_HYST 4

/*!\fn void phy_synch_time(short *in,unsigned int *sync_pos,unsigned int length,unsigned int skip,SCH_t synch_type,unsigned char synch_source)
This routine performs a sliding block correlation (using FFT based correlator) by looping across overlapping blocks in a received frame.

@param in Input signal frame (usually a whole TTI) for correlation
@param sync_pos Pointer to integer to store position of absolute maximum of correlation modulus.
@param length Total length of signal frame (usually the number of samples in one TTI)
@param skip Increment in samples between windows (usually an overlap of one complete OFDM symbols is guaranteed
@param synch_type Type of synch sequence (SCH or CHSCH).  CHSCH is used by UE/MR to synch to CH, SCH is used by CH to synch to MR
@param synch_source Index of CHSCH or SCH to use for timing acquisition
*/
void phy_synch_time(short *in,
		    unsigned int *sync_pos,
		    unsigned int length,
		    unsigned int skip,
		    SCH_t synch_type,
		    unsigned char synch_source);

/*!\fn  phy_channel_estimation_top(int rx_offset,int pilot_offset,int ignore_prefix,int sch_index,unsigned char nb_antennas_rx,SCH_t sch_type)
This routine is the top-level entry-point for wideband multi-source channel estimation.

@param rx_offset Sample offset in signal buffer for beginning of TTI
@param pilot_offset Symbol offset for position of pilot symbol for channel estimation in TTI 
@param ignore_prefix Flag to indicate that the cyclic extension should be ignored in the event that the HW removes the extension samples
@param sch_index Index of SCH pilot to use
@param nb_antennas_rx Number of RX antennas
@param sch_type Type of SCH (CHSCH or SCH). CHSCH is used by UE/MR and SCH is used by CH and MR (in mesh topology only).
*/
void phy_channel_estimation_top(int rx_offset,
				int pilot_offset,
				int ignore_prefix,
				int sch_index,
				unsigned char nb_antennas_rx,
				SCH_t sch_type);

/*!\fn void phy_channel_estimation(short *rxsig_f,short *channel_est_t,short *channel_est_f,short *channel_matched_filter_f,short *pilot_conj_f,char log2_pilot_amp,char smoothen_flag)
This routine performs least-squares channel estimation with optional noise reduction via smoothing (time-windowing).
@param rxsig_f Pointer to eceived symbol in frequency-domain with which channel estimation is to be performed. 
@param channel_est_t Pointer to least-squares channel estimate in time-domain
@param channel_est_f Pointer to least-squares channel estimation in frequency-domain
@param channel_matched_filter_f Pointer to channel matched filter in frequency-domain (conjugate of channel_est_f)
@param pilot_conj_f Pointer to conjugate of pilot symbol in frequency-domain 
@param log2_pilot_amp Log base2 of pilot amplitude
@param smoothen_flag Flag to activate time-domain windowing for noise-reduction or multi-channel separation

The implemented function is a follows:  let the rxsig_f be denoted \f$\mathbf{R}\f$, channel_est_t by \f$\mathbf{\hat{h}}\f$,
channel_est_f by \f$\mathbf{\hat{H}}\f$, channel_matched_filter_f by \f$\mathbf{\hat{H}}^*\f$, pilot_conj_f by \f$\mathbf{P}^*\f$.  
The procedures are:
-# \f$\mathbf{\hat{H}'} = \mathbf{R}\odot\mathbf{P}^*\f$ 
-# \f$\mathbf{\hat{h}} = \mathrm{IDFT}(\mathbf{\hat{H}})\f$
-# if smoothen_flag = 0 \f$\mathbf{\hat{H}} = \mathbf{\hat{H}'}\f$ else \f$\mathbf{\hat{H}} = \mathrm{DFT}(\mathbf{\hat{h}}\odot\mathbf{W}_c)\f$
-# \f$\mathbf{\hat{H}}^* = \mathbf{\hat{H}}\f$

Signals and pilots are stored in complex Q1.15 format, with complex pairs repeated in adjacent positions for efficient SIMD processing, that is
a signal vector in memory looks like: 

\f${\small\mathbf{X} = [\mathrm{Re}(X_0)\;\;\mathrm{Im}(X_0)\;\;\mathrm{Re}(X_0)\;\;\mathrm{Im}(X_0)\cdots\mathrm{Re}(X_{N_d-1})\;\;\mathrm{Im}(X_{N_d-1})\;\;\mathrm{Re}(X_{N_d-1})\;\;\mathrm{Im}(X_{N_d-1})]}\f$.  

The pilot signals are assumed to be precomputed and in the correct format for SIMD Q1.15 complex multiplication, that is:

\f${\small \mathbf{Y} = [\mathrm{Re}(Y_0)\;\;\mathrm{Im}(Y_0)\;\;-\mathrm{Im}(Y_0)\;\;\mathrm{Re}(Y_0)\cdots\mathrm{Re}(Y_{N_d-1})\;\;\mathrm{Im}(Y_{N_d-1})\;\;-\mathrm{Im}(Y_{N_d-1})\;\;\mathrm{Re}(Y_{N_d-1})]}\f$
*/
void phy_channel_estimation(short *rxsig_f,
			    short *channel_est_t,
			    short *channel_est_f,
			    short *channel_matched_filter_f,
			    short *pilot_conj_f, 
			    char log2_pilot_amp, 
			    char smoothen_flag);
/*!\brief This function is used during the channel estimation procedure to compute subband signal strenghts from the received SCH signal.  These are used by the MAC layer for feedback signaling in support of scheduling and for deriving measurement information for higher layer protocols.
\param sch_index Index of SCH on which to perform the measurement
\param sch_type The type of SCH on which to act (SCH,CHSCH)
\param nb_antennas_tx Number of antennas in the transmitter
\param aa Receiver antenna index
\param n0 Estimate of per carrier noise variances (linear) (vector of receive antennas)
*/
void phy_subband_powers(unsigned int sch_index,
			SCH_t sch_type,
			unsigned int nb_antennas_tx,
			unsigned int aa,
			int *n0);

/*!\brief This function is used by EMOS to estimate the channel from the pilots.

\param ref_pilot index of the reference pilot
\param from_pilot index of the first pilot
\param to_pilot a index of the last pilot
\param sch_index index of the SCH (for storage)
\param perform_estimate flag to indicate if channel estimation should be performed
\param ignore_prefix flag that indicates if prefix should be ignored
  
  This function takes the pilots with pilot indices from_pilot to to_pilot of the received signal
  (which is stored in PHY_vars->rx_vars) and does a phase compensation with respect to the pilot with index ref_pilot, 
  takes their average. If ref_pilot==from_pilot, the averaging buffer is reset. If perform_estimate==TURE, channel estimation
  is performed on the averaging buffer. The result is stored in PHY_vars->sch_data[sch_index].channel
  and PHY_vars->sch_data[sch_index].channel_f respectively.
*/
void phy_channel_est_emos(int ref_pilot, int from_pilot, int to_pilot, int sch_index, int perform_estimate, int ignore_prefix);


/*! \brief This routine calculates the MMSE filter matrix.
\param channel_est_f1 Pointer to channel estimation in frequency-domain from CH1
\param channel_est_f2 Pointer to channel estimation in frequency-domain from CH2
\param channel_mmse_filter Pointer to MMSE filter
\param det Pointer to determinant (32 bit)
\param idet Pointer to inverse determinant (16 bit) 
\param sigma2 Pointer to noise estimates of both RX antennas
*/
int phy_calc_mmse_filter(int **channel_est_f1,
			 int **channel_est_f2,
			 int *channel_mmse_filter[NB_ANTENNAS_TX][NB_ANTENNAS_RX],
			 int *det,
			 int* idet,
			 int* sigma2 );


/*!\fn int phy_channel_interpolation(int *channel_est_f,
			      int *channel_est_f_interp,
			      int antenna_tx);
This routine performs interpolation of the channel estimates. Due to the multiplexing of the pilots over the transmit antennas, channel estimates for one particular Tx antenna are not availiable for all subcarriers. To obtain channel estimtes for the remaining subcarriers, we perform interpolation. The interpolation is done using an ifft, zero appending, and fft.
@param channel_est_f Pointer to least-squares channel estimate in frequency-domain (contains channel estimates of all tx antennas)
@param channel_est_f_interp Pointer to interpolated channel estimation in frequency-domain
@param antenna_tx Index of Tx antenna, which is interpolated 
*/int phy_channel_interpolation(int *channel_est_f,
			      int *channel_est_f_interp,
			      int antenna_tx);

/*!\fn  void phy_adjust_gain(unsigned char clear,short coef,unsigned char ind)
This routine performs digital automatic gain control with hysterisis. It is used only during real-time RF execution both for UE/MR and CH (in MESH topology only!).
The function looks at the measured power levels of CHSCH chsch_ind in PHY_vars->PHY_measurements. 

@param clear Flag to initialize the gain control loop
@param coef Q1.15 forgetting factor (must be positive!)
@param ind Index of channel estimate (in PHY_measurements structure)

Note that RF Hardware is assumed to be calibrated. Support routines for adjusting the digital gain of the receiver every TTI are
also assumed.  Makes use of the generic call openair_set_rx_gain_cal_openair() which uses a calibration table to adjust the total (RF->DSP)
gain of the receiver.
*/
void phy_adjust_gain(unsigned char clear,short coef,unsigned char ind);

/*!\fn  void phy_adjust_gain_mesh(unsigned char clear,short coef)
This routine performs digital automatic gain control with hysterisis with respect to several measurements. 
This function looks at the measured power levels of CHSCH 
0-NUMBER_OF_CHSCH_SYMBOLS_MAX in PHY_vars->PHY_measurements 
and adjusts the gain level to the weakest CHSCH under the 
condition that the strongest one does not saturate.

It is used only during real-time RF execution both for UE/MR and CH(in
MESH topology only!).
@param clear Flag to initialize the gain control loop
@param coef Q1.15 forgetting factor (must be positive!)

Note that RF Hardware is assumed to be calibrated. Support routines for adjusting the digital gain of the receiver every TTI are
also assumed.  Makes use of the generic call openair_set_rx_gain_cal_openair() which uses a calibration table to adjust the total (RF->DSP)
gain of the receiver.
*/
void phy_adjust_gain_mesh (unsigned char clear,short coef);



/*!\fn void phy_adjust_synch(unsigned char clear,int sch_index,short coef,SCH_t sch_type)
This routine performs timing drift adjustment by tracking the peak of a channel estimate in time.

@param clear Flag to initialize the gain control loop
@param sch_index Index of channel estimate (in CHSCH/SCH structure)
@param coef Q1.15 forgetting factor (must be positive!)
@param sch_type Type of SCH (CHSCH or SCH)

This is used by UE/MR in both topologies and CH (sometimes) in Mesh topology. Support routines for adjusting the synchronization 
point (start of TTI based on acquisition sample counter) is assumed.  Makes use of the shared parameter pci_interface.frame_offset 
to adjust start of TTI in HW ACQ system. The drift tracking is done as follows:

Let \f$\mathbf{h}_i[0:N_c-1]\f$ be the channel response for RX antenna \f$i\f$, that is either a CHSCH_data.channel or SCH_data.channel. 
Define the position of the maximum modulus as
\f$n_\mathrm{max} = \operatornamewithlimits{argmax}_{n=0,\cdots,N_c-1}\sum_{i=0}^{\mathrm{nb\_antennas\_rx}}|\mathbf{h}_{i}[n]|^2\f$ and
let the TTI offset be \f$\mathrm{tti\_offset}\f$ which would be RX_VARS.offset. The drift estimation is done adaptively every TTI as

-# if clear\f$=1\f$ then \f$\mathrm{max\_pos\_filtered} = n_\mathrm{max}\f$
   else \f$\mathrm{max\_pos\_filtered} = (1-\mathrm{coef})n_\mathrm{max} + \mathrm{coef}\cdot\mathrm{max\_pos\_filtered}\f$
-# \f$\Delta t = \mathrm{max\_pos\_filtered} - \mathrm{target\_peak\_position}\f$
-# if \f$\Delta t > \f$ SYNCH_HYST then \f${\mathrm{tti\_offset}}++\f$ else if \f$\Delta t < \f$ SYNCH_HYST then \f${\mathrm{tti\_offset}}--\f$
*/
void phy_adjust_synch(unsigned char clear,int sch_index,short coef, SCH_t sch_type);


/*!\fn void phy_calc_timing_offset(int clear, int sch_index, short coef, SCH_t sch_type, int* max_pos_fil)
This routine is based on phy_adjust_sync with the difference, that it does not actually adjust the offset, but returns the peak position as a parameter. Additionally, a test is performed if a peak is actually present (the peak value has to be 10dB higher than the average). 

@param clear If 0, the last value in max_pos_fil is used to filter the peak position, else no filter is applied
@param sch_index Index of channel estimate (in CHSCH/SCH structure)
@param coef Q1.15 forgetting factor (must be positive!)
@param sch_type Type of SCH (CHSCH or SCH)
@param max_pos_fil overwritten with the filtered calculated peak position
*/
int phy_calc_timing_offset(int clear, int sch_index, short coef, SCH_t sch_type, int* max_pos_fil);

/*!\fn void phy_adjust_synch_multi_CH(unsigned char clear, short coef, SCH_t sch_type)
This routine performs timing drift adjustment by tracking the peaks of the channel estimates from several CHs in time. 

@param clear Flag to initialize the gain control loop
@param coef Q1.15 forgetting factor (must be positive!)
@param sch_type Type of SCH (CHSCH or SCH)

This is used by UE/MR in the Mesh topology with several clusters. Support routines for adjusting the synchronization point (start of TTI based on acquisition sample counter) is assumed.  Makes use of the shared parameter pci_interface.frame_offset to adjust start of TTI in HW ACQ system. The drift tracking is adjusted to the first peak found in the channel estimates of all CHs.
*/
void phy_adjust_synch_multi_CH(unsigned char clear, short coef, SCH_t sch_type);

/*!\fn void phy_adjust_synch_CH2(int clear, int sch_index, short coef, SCH_t sch_type)
This routine performs timing drift adjustment by tracking the first peak of the channel estimate. This is used for the synchronization of the secondary CH, where multiple MRSCHes can be received. The peak is defined as the point that has 10x higher energy than the mean. 

@param clear Flag to initialize the gain control loop
@param sch_index Index of channel estimate (in CHSCH/SCH structure)
@param coef Q1.15 forgetting factor (must be positive!)
@param sch_type Type of SCH (CHSCH or SCH)

*/
int phy_adjust_sync_CH2(int clear, int sch_index, short coef, SCH_t sch_type);

int model_based_detection(void);

/*! \fn void lte_sync_time_init(LTE_DL_FRAME_PARMS *frame_parms)
\brief This function allocates memory needed for the synchronization.
\param frame_parms LTE DL frame parameter structure
*/

/** @} */ 
