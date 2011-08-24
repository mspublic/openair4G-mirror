/*___________________phy/transport/defs.h_______________________

 Authors : Raymond Knopp
 Company : EURECOM
 Emails  : knopp@eurecom.fr
________________________________________________________________*/

#ifndef __PHY_TRANSPORT_DEFS_H__
#define __PHY_TRANSPORT_DEFS_H__

#include "PHY/defs.h"
#include "PHY/TOOLS/defs.h"
#include "MAC_INTERFACE/defs.h"



/** @addtogroup _PHY_TRANSPORT_CHANNEL_PROCEDURES_
* @{
*/

/// CHSCH Variables
typedef struct CHSCH_data
{
  int *CHSCH_f;                             /// CHSCH symbols 
  int *CHSCH_f_tx[NB_ANTENNAS_RX];          /// CHSCH TX symbols 
  int *CHSCH_f_txr[4];                      /// CHSCH TX symbols for receiver (antenna0) 
  int *CHSCH_conj_f;                        /// Complex Conjugated CHSCH (for channel estimation)
  int *CHSCH_f_sync[4];                     /// Interpolated Complex Conjugated CHSCH (for timing synchronization)
  int *channel[NB_ANTENNAS_RX];             /// Channel Estimate (RX)
  int *mag_channel;                         /// Magnitude squared of Channel Estimate in time domain (RX)
  int *channel_f[NB_ANTENNAS_RX];           /// Channel Estimate in Freq. Domain (RX)
  int *mag_channel_f[NB_ANTENNAS_RX];       /// Magnitude Squared of Channel Estimate in Freq. Domain (RX)
  int *channel_matched_filter_f[NB_ANTENNAS_RX];               /// Channel Matched Filter in Freq. Domain (RX)
  int *channel_f_interp[NB_ANTENNAS_TX][NB_ANTENNAS_RX];       /// Interpolated Channel Estimate in Freq. Domain 
  int *channel_mmse_filter_f[NB_ANTENNAS_TX][NB_ANTENNAS_RX];  /// Channel MIMO MMSE filter in Freq. Domain (RX)
  int *det;                                 /// Determinant of MIMO MMSE filter in Freq. Domain (RX)
  int *idet;                                /// Inverse Determinant of MIMO MMSE filter in Freq. Domain (RX)
  int *rx_sig_f[NB_ANTENNAS_RX];            /// Received signal in Freq. Domain (RX)
  unsigned char subband_spatial_sinr[4][4][NUMBER_OF_FREQUENCY_GROUPS];
  unsigned char subband_aggregate_sinr[NUMBER_OF_FREQUENCY_GROUPS];
  unsigned char wideband_sinr;
} CHSCH_data;

/// SCH Variables  
typedef struct SCH_data
{
  int *SCH_f;                                /// SCH symbols 
  int *SCH_f_tx[NB_ANTENNAS_RX];             /// SCH TX symbols 
  int *SCH_f_txr[4];                         /// CHSCH TX symbols for receiver (antenna0) 
  int *SCH_conj_f;                           /// Complex Conjugated CHSCH (for channel estimation)
  int *SCH_f_sync[4];                        /// For Timing Acquisition, 4x oversampled frequency-domain synch sequence
  int *channel[NB_ANTENNAS_RX];              /// Channel Estimate (RX)
  int *mag_channel;                          /// Magnitude squared of Channel Estimate in time domain (RX)
  int *channel_f[NB_ANTENNAS_RX];            /// Channel Estimate in Freq. Domain (RX)
  int *mag_channel_f[NB_ANTENNAS_RX];        /// Magnitude Squared of Channel Estimate in Freq. Domain (RX)
  int *channel_matched_filter_f[NB_ANTENNAS_RX];          /// Channel Matched Filter in Freq. Domain (RX)
  int *rx_sig_f[NB_ANTENNAS_RX];             /// Received signal in Freq. Domain (RX)
#ifdef EMOS
  /// Phase error
  struct complex16 perror[NB_ANTENNAS_RX][48];
#endif
  //int sch_symbol_pos;                     /// Position of the SCH within the frame (in symbols)
  unsigned char subband_spatial_sinr[4][4][NUMBER_OF_FREQUENCY_GROUPS];
  unsigned char subband_aggregate_sinr[NUMBER_OF_FREQUENCY_GROUPS];
  unsigned char wideband_sinr;
} SCH_data;

/// Transport Data structure
typedef struct Transport_data
{
/// Conv. code output (TX)
  unsigned char  *encoded_data[MAX_NUM_TB];         
/// FFT Input buffer (TX)           
  int *fft_input[NB_ANTENNAS_TX];          
/// Frequency/Symbol Interleaving
  unsigned short *interleaver_tx[16];                  
/// Frequency/Symbol Interleaving
  unsigned short *interleaver_rx[16];          
  ///        
  unsigned short *pilot_indices;
/// Pilot sequence (frequency domain)
 short *pilot;                            
/// Received signal in Freq. Domain after phase derotation (RX) (1.15)
  int *rx_sig_f[NB_ANTENNAS_RX];                        
/// Received signal in Freq. Domain after channel removal (matched filter or MMSE filter) (RX) (1.15)
  int *rx_sig_f2[NB_ANTENNAS_RX];                       
/// Received signal in Freq. Domain after concatenating active frequency groups (RX) (1.15)
  int *rx_sig_f3[NB_ANTENNAS_RX];                       
///Received signal in Freq. Domain after rescaling and removal of zero-carriers(1.7)
  short *rx_sig_f4;                         
/// 16qam threshold         
  short *rx_16qam_thres;                    
/// mag squared of channel in positions, scaled for 16qam
  int *mag_channel_f_16qam[NB_ANTENNAS_RX];        
/// mag squared of channel in positions, scaled for 64qam, first part
  int *mag_channel_f_64qama[NB_ANTENNAS_RX];        
/// mag squared of channel in positions, scaled for 64qam, second part
  int *mag_channel_f_64qamb[NB_ANTENNAS_RX];        
  /// full interpolated and rotated channel estimate for all subcarriers and all symbols. Redundant Q 1.15 format! 
  int *channel_f_full[NB_ANTENNAS_TXRX][NB_ANTENNAS_RX];            /// Channel Estimate in Freq. Domain (RX)
/// Soft-decisions on transmitted bits (RX) (1.7)            
  char  *demod_data;                      
/// PDU buffer for RX
  unsigned char *demod_pdu;                     
/// PDU buffer for TX   
  unsigned char *tx_pdu[MAX_NUM_TB];                  
/// Average Received Power (RX)      
  short RX_RSSI;                          
/// Channel quality per frequency group (RX) 
  char  chan_qual[NB_GROUPS_MAX];    
  /// Total number of PDU errors
  unsigned int pdu_errors;
  /// Total number of PDU errors 128 frames ago
  unsigned int pdu_errors_last;
  /// Total number of consecutive PDU errors
  unsigned int pdu_errors_conseq;
  /// FER (in percent) 
  unsigned int pdu_fer;
  /// Phase error
  struct complex16 perror[NB_ANTENNAS_RX][64];
} Transport_data;

/*!\fn void create_times4_sync_symbol(unsigned char n)
\brief Initialize a 4 times oversampled version of a CHSCH in the frequency-domain.  This is used for initial timing acquisition by a UE/MR.
@param n Index aor the oversampled chsch to be generated.
*/
void create_times4_sync_symbol(unsigned char n);

/*!\fn void phy_chsch_init(unsigned char n,unsigned char nb_antennas_tx)
\brief This routine initializes the variables for a CHSCH
@param n Index for the chsch to be generated
@param nb_antennas_tx Number of TX antennas  
*/
void phy_chsch_init(unsigned char n,
		    unsigned char nb_antennas_tx);

/*!\fn void phy_chsch_init_rt_part(unsigned char n)
\brief This routine initializes the part of the CHSCH requiring processing that is only possible from a real-time context (SSE/MMX). 
@param n Index for the chsch to be generated.

It is used only in real-time execution of OpenAirInterface under RTAI.  In user-space, this is part of phy_chsch_init.
*/
void phy_chsch_init_rt_part(unsigned char n);

/*!\fn void phy_sch_init_rt_part(unsigned char n)
\brief This routine initializes the part of the SCH requiring processing that is only possible from a real-time context (SSE/MMX). 
@param n Index for the chsch to be generated.

It is used only in real-time execution of OpenAirInterface under RTAI.  In user-space, this is part of phy_sch_init.
*/
void phy_sch_init_rt_part(unsigned char n);

void phy_sch_init(unsigned char n,unsigned char nb_antennas_tx);

/*!\fn void phy_generate_chbch_top(unsigned char chbch_ind)
\brief This routine implements the MAC interface for the CHBCH on transmission.  
@param chbch_ind Index for the chbch to be generated.  The index is in correspondance with an associated chsch_ind.

It scans the MACPHY_DATA_REQ_TABLE whenever called (typically
towards the end of the TTI \f$N-1\f$ for transmission during TTI \f$N\f$)
in order to check if a pending request for CHBCH has be generated by the MAC.

*/
void phy_generate_chbch_top(unsigned char chbch_ind);

/*!\fn unsigned char phy_generate_chbch(unsigned char chsch_ind,unsigned char extension_switch,unsigned char nb_antennas_tx,unsigned char *chbch_pdu)
\brief This routine generates the CHBCH.  
@param chsch_ind Index for an associated CHSCH
@param extension_switch Flag for generation of cyclic extension
@param nb_antennas_tx Number of TX antennas to use
@param chbch_pdu Pointer to memory containing PDU
*/ 
unsigned char phy_generate_chbch(unsigned char chsch_ind,
				 unsigned char extension_switch,
				 unsigned char nb_antennas_tx,
				 unsigned char *chbch_pdu);

/*!\fn void phy_decode_chbch_top(unsigned char chbch_ind)
\brief This routine implements the MAC interface for the CHBCH on reception.  

It scans the MACPHY_DATA_REQ_TABLE whenever called (typically towards the first quarter of TTI \f$N\f$ for transmission 
during TTI \f$N\f$) in order to check if a pending request for CHBCH has be generated by the MAC.
*/
void phy_decode_chbch_top(void);

/*! \fn unsigned char phy_decode_chbch(unsigned char chbch_ind,unsigned char nb_antennas_tx,unsigned char nb_antennas_rx,unsigned char *chbch_mac_pdu,unsigned int chbch_pdu_length_bytes)
\brief This routine decodes the CHBCH.  
\param chbch_ind Index of the CHBCH to be processed
\param nb_antennas_rx Number of RX antennas at UE/MR
\param nb_antennas_tx Number of TX antennas at CH
\param chbch_mac_pdu Pointer to CHBCH_PDU (typecasted to char)
\param chbch_pdu_length_bytes Length of CHBCH_PDU + tail/CRC bits.

Operations

Let \f$\mathbf{r}_i\f$ be the \f$i^{\mathrm{th}}\f$ symbol of the CHBCH, \f$i=0,\cdots,N_{s,\mathrm{CHSCH}}-1 \f$

-# Loop over the symbols, skipping the extension if the hardware does not strip it out (always done if timing acquisition is not complete),and generate the frequency-domain symbols, \f$\mathbf{R}_i = \mathrm{DFT}(\mathbf{r}_i)\f$.  These are stored in Transport_data.rx_sig_f[aa] for antenna aa.
-# Compute carrier phase offset for each symbol, with respect to reference CHSCH (CHSCH_data.rx_sig_f), using the sparse pilots in each symbol as:

 */ 
int phy_decode_chbch(unsigned char chbch_ind,
		     unsigned char nb_antennas_rx,
		     unsigned char nb_antennas_tx,
		     unsigned char *chbch_mac_pdu,
		     unsigned int chbch_pdu_length_bytes
);


/*!\brief This routine decodes two CHBCHs comming from two different CHs using either MMSE receiver or SIC.  
\param chbch_ind[2] Indices of the CHBCHs to be processed
\param mode indicates if MMSE or SIC receiver schould be used (currently unused - only MMSE possible)
\param nb_antennas_rx Number of TX antennas at CH
\param nb_antennas_tx Number of RX antennas at UE/MR
\param chbch_mac_pdu[2] Pointers to CHBCH_PDU (typecasted to char)
\param ret[2] CRCs of both streams
\param chbch_pdu_length_bytes Length of the MAC layer pdu (to avoid buffer overrun)
Operations

Let \f$\mathbf{r}_{i,k}\f$ be the \f$i^{\mathrm{th}}\f$ symbol of the CHBCH with index chbch_ind[k]

for k=0,1 do
-# Loop over the symbols \f$\mathbf{r}_{i,k}\f$, skipping the extension if the hardware does not strip it out (always done if timing acquisition is not complete), and generate the frequency-domain symbols, \f$\mathbf{R}_{i,k} = \mathrm{DFT}(\mathbf{r}_{i,k})\f$.  These are stored in chbch_data[chbch_ind[k]].rx_sig_f[aa] for antenna aa.
-# Compute carrier phase offset for each symbol, with respect to reference CHSCH (CHSCH_data[chbch_ind[k]].rx_sig_f), using the sparse pilots in each symbol. Store it in CHBCH_data[chbch_ind[k]].perror[NB_ANTENNAS_RX][NUMBER_OF_CHBCH_SYMBOLS]
-# Derotate symbols and store result in CHBCH_data[chbch_ind[k]].rx_sig_f.
-# Apply MMSE filter to CHBCH_data[chbch_ind[k]].rx_sig_f store result in CHBCH_data[chbch_ind[k]].rx_sig_f2. The equalized stream from CH k is now in  CHBCH_data[chbch_ind[k]].rx_sig_f2[k]. (This procedure can be improved, because the full MMSE filter is applied unecessarily twice). The matrix-vector multiplication scales the result by the square root of the energy of the MMSE filter matrix minus 2 bits (log2_maxh). 
-# Clipping
-# Frequency Deinterleaving & Demodulation
-# Viterbi Decoding

end do
*/ 
void phy_decode_chbch_2streams(unsigned char chbch_ind[2],
			       int mode, 
			       unsigned char nb_antennas_rx,
			       unsigned char nb_antennas_tx,
			       unsigned char *chbch_mac_pdu[2],
			       int ret[2],
			       unsigned int chbch_pdu_length_bytes);
 

/*!\brief This routine decodes two CHBCHs comming from two different CHs using a non-linear BICM-MIMO LLR-based receiver.  
\param chbch_ind[2] Indices of the CHBCHs to be processed
\param mode indicates if MMSE or SIC receiver schould be used (currently unused - only MMSE possible)
\param nb_antennas_rx Number of TX antennas at CH
\param nb_antennas_tx Number of RX antennas at UE/MR
\param chbch_mac_pdu[2] Pointers to CHBCH_PDU (typecasted to char)
\param ret[2] CRCs of both streams
\param chbch_pdu_length_bytes Length of the MAC layer pdu (to avoid buffer overrun)
Operations

Let \f$\mathbf{r}_{i,k}\f$ be the \f$i^{\mathrm{th}}\f$ symbol of the CHBCH with index chbch_ind[k]

for k=0,1 do
-# Loop over the symbols \f$\mathbf{r}_{i,k}\f$, skipping the extension if the hardware does not strip it out (always done if timing acquisition is not complete), and generate the frequency-domain symbols, \f$\mathbf{R}_{i,k} = \mathrm{DFT}(\mathbf{r}_{i,k})\f$.  These are stored in chbch_data[chbch_ind[k]].rx_sig_f[aa] for antenna aa.
-# Compute carrier phase offset for each symbol, with respect to reference CHSCH (CHSCH_data[chbch_ind[k]].rx_sig_f), using the sparse pilots in each symbol. Store it in CHBCH_data[chbch_ind[k]].perror[NB_ANTENNAS_RX][NUMBER_OF_CHBCH_SYMBOLS]
-# Derotate symbols and store result in CHBCH_data[chbch_ind[k]].rx_sig_f.
-# Apply MMSE filter to CHBCH_data[chbch_ind[k]].rx_sig_f store result in CHBCH_data[chbch_ind[k]].rx_sig_f2. The equalized stream from CH k is now in  CHBCH_data[chbch_ind[k]].rx_sig_f2[k]. (This procedure can be improved, because the full MMSE filter is applied unecessarily twice). The matrix-vector multiplication scales the result by the square root of the energy of the MMSE filter matrix minus 2 bits (log2_maxh). 
-# Clipping
-# Frequency Deinterleaving & Demodulation
-# Viterbi Decoding

end do
*/ 

void phy_decode_chbch_2streams_ml(unsigned char chbch_ind[2],
				  int mode, 
				  unsigned char nb_antennas_rx,
				  unsigned char nb_antennas_tx,
				  unsigned char *chbch_mac_pdu[2],
				  int ret[2],
				  unsigned int chbch_pdu_length_bytes);
 
/*!\fn int phy_chbch_phase_comp(struct complex16 *Rchsch, struct complex16 *Rsymb, int chbch_ind, int nb_antennas_tx, struct complex16 *perror);
\brief Calculate the phase drift using the pilots in the CHBCH and derotates the symbol accordingly
@param Rchsch Pointer to the reference symbol. Usually the CHSCH 
@param Rsymb Pointer to symbol for the estimation
@param chbch_ind Index of the CHBCH
@param nb_antennas_tx No. TX antennas
@param perror Estimated phase offset
@param do_rotate Flag to indicate that rotation with phase offset is to be performed on Rsymb (1 means do it)
@returns Returns norm of the estimate if successfull and -1 if phase estimation is too weak (realtime mode only)
*/
int phy_chbch_phase_comp(struct complex16 *Rchsch, struct complex16 *Rsymb, int chbch_ind, int nb_antennas_tx, struct complex16 *perror,unsigned char do_rotate);

/*!\fn void phy_generate_mrbch_top(unsigned char sch_index)
\brief This routine implements the MAC interface for the MRBCH on transmission.  
@param sch_index Index for the chbch to be generated.  The index is in correspondance with an associated sch_index.

It scans the MACPHY_DATA_REQ_TABLE whenever called (typically
towards the middle of TTI \f$N\f$ for transmission during TTI \f$N\f$, if the MRBCH is located at the end of the TTI)
in order to check if a pending request for CHBCH has be generated by the MAC.
*/
void phy_generate_mrbch_top(unsigned char sch_index);


/*!\fn unsigned char phy_generate_mrbch(unsigned char sch_index,unsigned char extension_switch,unsigned char nb_antennas_tx,unsigned char *mrbch_pdu)
\brief This routine generates the MRBCH for an MR.  
@param sch_index Index for an associated SCH
@param extension_switch Flag for generation of cyclic extension
@param nb_antennas_tx Number of TX antennas to use
@param mrbch_pdu Pointer to memory containing PDU
*/ 
unsigned char phy_generate_mrbch(unsigned char sch_index,
				 unsigned char extension_switch,
				 unsigned char nb_antennas_tx,
				 unsigned char *mrbch_pdu);

/*! \brief This routine initializes PHY_vars->chbch_data[chbch_index].pilot indices and 
PHY_vars->chbch_data[chbch_index].pilots based on PHY_config->PHY_chbch[chbch_index].Npilot
and the pilot symbols. 
How many pilots do we put per NUMBER_OF_USEFUL_CARRIERS/NUMBER_OF_FREQUENCY_GROUPS 
scubcarriers. At which position do we put them. Where are those parameteres configured.
\param chbch_index Index of the CHBCH
\param nb_antennas_tx Number of TX antennas
\return 0 on succes, -1 otherwise
*/
int phy_chbch_pilot_init(unsigned char chbch_index, unsigned char nb_antennas_tx);

/*!\fn void phy_decode_mrbch_top(unsigned char sch_index)
\brief This routine implements the MAC interface for the MRBCH on reception at CH.  
@param sch_index Index for the chbch to be decoded in correspondance with an associated chsch_ind.

It scans the MACPHY_DATA_REQ_TABLE whenever called (typically towards the first quarter of TTI \f$N\f$ for transmission 
during TTI \f$N-1\f$, in the case where the MRBCH is at the end of the TTI) in order to check if a pending 
request for MRBCH has be generated by the CH MAC.
*/
void phy_decode_mrbch_top(unsigned char sch_index);


/*!\fn void phy_decode_mrbch(unsigned char sch_index,unsigned char nb_antennas_tx,unsigned char nb_antennas_rx,unsigned char *mrbch_mac_pdu,unsigned int mrbch_pdu_length_bytes)
\brief This routine implements the MAC interface for the MRBCH on reception at CH.  
@param sch_index Index for the mrbch to be decoded in correspondance with an associated sch_ind
@param nb_antennas_tx Number of transmit antennas
@param nb_antennas_rx Number of receive antennas
@param mrbch_mac_pdu Pointer to MRBCH_PDU structure (typcasted to char)
@param mrbch_pdu_length_bytes
*/
int phy_decode_mrbch(unsigned char sch_index,
		     unsigned char nb_antennas_tx,
		     unsigned char nb_antennas_rx,
		     unsigned char *mrbch_mac_pdu,
		     unsigned int mrbch_pdu_length_bytes);


/*!\fn unsigned char phy_generate_sch(unsigned int stream_index,unsigned int sch_index,unsigned int symbol,unsigned short freq_alloc,unsigned char extension,unsigned char nb_antennas_tx)
\brief This routine implements the generation of the sch for UE/MR.
@param stream_index Stream Index for the sch to be generated
@param sch_index Index for the sch to be generated
@param symbol Symbol position in TTI
@param freq_alloc Frequency group allocation bitmap
@param extension Flag for cyclic extension inclusion
@param nb_antennas_tx Number of TX antennas
*/
unsigned char phy_generate_sch(unsigned int stream_index,
			       unsigned int sch_index,
			       unsigned int symbol, 
			       unsigned short freq_alloc, 
			       unsigned char extension,
			       unsigned char nb_antennas_tx);

/*!\fn void phy_generate_sach_top(unsigned char last_slot,int time_in)
\brief This routine implements the MAC interface for sach/sacch for UE/MR/CH
@param last_slot Index for the last received slot used for selecting the SACH to be generated.
@param time_in Timing information at routine entry (for real-time performance statistics)
It scans the MACPHY_DATA_REQ_TABLE whenever called and treats the SACH which are pending for the slot (groups of symbols)
in question.
*/
void phy_generate_sach_top(unsigned char last_slot,int time_in);

/*!\brief This routine implements generation of signals for sach/sacch in UE/MR/CH with respect to coding/interleaving/modulation.
@param ch_index Clusterhead index
@param sacch_flag Flag to indicated generation of SACCH required
@param sch_type Type of pilot sequence associated to this SACH (SCH or CHSCH)
@param sch_index Index of pilot sequence associated to this SACH
@param sacch_flag Flag to indicated generation of SACCH required
@param sach_pdu Pointer to MAC SACH PDU
@param sacch_pdu Pointer to MAC SACCH PDU
@param time_alloc Time allocation vector indicating first symbol and length of SACH resource
@param freq_alloc Frequency group allocation vector
@param coding_fmt Channel Coding and Modulation format for SACH portion
@param nb_antennas_tx Number of TX antennas
@param tb_size_bytes Size of transport block in bytes
@param Active_process_map Bit map containing activation indicators of up to 16 HARQ processes
@param New_process_map Bit map containing newly created TBs (i.e. round 0 of HARQ protocol) for up to 16 HARQ processes
@param first_sach_flag Flag indicating that this is the first sach in the slot.  This is for FFT buffer initialization
@param total_groups Number of active groups in this TTI across all SACH.  This is to adjust the amplitude of the data carriers properly.
Note this function currently being updated for HARQ and multiple-transport block support.
*/
void phy_generate_sach1(unsigned char ch_index,
			unsigned int sacch_flag,
			unsigned char sch_type,
			unsigned char sch_index,
			unsigned char *sach_pdu,
			unsigned char *sacch_pdu,
			unsigned char time_alloc,
			unsigned short freq_alloc,
			unsigned char coding_fmt,
			unsigned char nb_antennas_tx,
                        unsigned short tb_size_bytes,
                        unsigned int Active_process_map,
                        unsigned int New_process_map,
			unsigned char first_sach_flag,
			unsigned char total_groups);

/*!\fn unsigned char phy_generate_sach2(unsigned char extension_switch,unsigned char first_symbol,unsigned char number_of_symbols,unsigned char nb_antennas_tx)
\brief This routine implements generation of signals for sach/sacch in UE/MR/CH with respect to OFDM (IDFT) and cyclic extension
@param extension_switch Flag to indicate that cyclic extension is required
@param first_symbol First symbol of the signal to be generated
@param number_of_symbols Number of symbols in the signal to be generated
@param nb_antennas_tx Number of TX antennas for which to generate the signals
\return Returns signal strength in dB on generated signal (averaged over the nb_antennas_tx TX antennas)
*/
unsigned char phy_generate_sach2(unsigned char extension_switch, 
			unsigned char first_symbol,
			unsigned char number_of_symbols,
			unsigned char nb_antennas_tx);

 
/*!\fn int phy_decode_sach_top(unsigned char last_slot)
\brief This routine implements the generation of signals for sach/sacch in UE/MR/CH with respect to OFDM (IDFT) and cyclic extension
@param last_slot Index for the last received slot used for selecting the SACH to be generated.
\return Return

It scans the MACPHY_DATA_REQ_TABLE whenever called and treats the SACH which are pending for the slot (groups of symbols)
in question. 

*/ 
int phy_decode_sach_top(unsigned char last_slot);

/*!\fn void phy_decode_sach_common(int first_symbol,int number_of_symbols,unsigned char nb_antennas_rx,unsigned int sach_index)
\brief This routine implements the frequency-transform portion of SACH detection
@param first_symbol First symbol of the signal to be generated
@param number_of_symbols Number of symbols in the signal to be generated
@param nb_antennas_rx Number of RX antennas for which to generate the RX signals
@param sach_index Index of SACH data for storage
*/
void phy_decode_sach_common(int first_symbol,
			    int number_of_symbols,
			    unsigned char nb_antennas_rx, 
			    unsigned int sach_index);
			    
/*!\brief This routine implements SACH Demodulation/Decoding
@param sacch_flag Flag to indicate SACCH detection
@param first_sach_flag Flag to indicate detection of first sach in TTI
@param Phy_Resources_ptr Pointer to a PHY_RESOURCES description of SACH
@param Sach_payload Pointer to SACH PDU buffer for storage
@param Sacch_payload Pointer to SACCH PDU buffer for storage
@param nb_antennas_rx Number of RX antennas
@param nb_antennas_tx Number of TX antennas
@param sach_index Index of SACH data structure where signal/data temporary variables are stored
@param sch_index Index of SACH data structure where signal/data temporary variables are stored
@param stream_index Index of SACH data structure where signal/data temporary variables are stored
@param num_tb Number of TBs in this generation
@param tb_size_bytes Size of TBs in bytes
@param active_processes bit map containing indication of active HARQ processes to be decoded
@param crc_status A pointer to an array of indications of the decoding status (SACH_OK, SACH_ERROR, SACCH_ERROR) for TBs on active proceses. SACCH ERROR is indicated on bit 0
@returns 0 on success, a negative status on error (-SACH_PARAM_ERROR,...)
*/
int phy_decode_sach(int sacch_flag,
		     unsigned int first_sach_flag,
		     PHY_RESOURCES *Phy_Resources_ptr,
		     unsigned char *Sach_payload,
		     unsigned char *Sacch_payload,
		     unsigned char nb_antennas_rx,
		     unsigned char nb_antennas_tx,
		     unsigned char sach_index,
		    unsigned char sch_index,
		    unsigned char stream_index,
		     unsigned char num_tb,
		     unsigned short tb_size_bytes,
		     unsigned int active_processes,
		     int *crc_status
		     );

int phy_decode_sach_2streams_ml(int sacch_flag,
				unsigned int first_sach_flag,
				PHY_RESOURCES *Phy_Resources_ptr,
				unsigned char *Sach_payload,
				unsigned char *Sacch_payload,
				unsigned char nb_antennas_rx,
				unsigned char nb_antennas_tx,
				unsigned char sach_index,
				unsigned char sch_index,
				unsigned char stream_index,
				unsigned char num_tb,
				unsigned short tb_size_bytes,
				unsigned int active_processes,
				int *crc_status);


void fill_chsch_measurement_info(unsigned char chbch_ind,
				 DL_MEAS *DL_meas);

void fill_sch_measurement_info(unsigned char sach_ind,
			       UL_MEAS *UL_meas,
			       unsigned short freq_alloc);

/** @}*/


void generate_sach_64qam_table(void);


int conv_alloc_to_tb2(unsigned char node_type,
		      unsigned char time_alloc,
		      unsigned short freq_alloc,
		      unsigned char target_spec_eff,
		      unsigned char dual_stream_flag,
		      unsigned char num_tb_max,
		      unsigned char *coding_fmt,
		      unsigned char *num_tb,
		      unsigned short tb_size_bytes);


#endif //__PHY_TRANSPORT_DEFS_H__
