/*________________________SIMULATION/PHY_EMULATION/CONTROL/defs.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/


#ifndef __PHY_EMULATION_H__
#define __PHY_EMULATION_H__
#include "SIMULATION/PHY_EMULATION/ABSTRACTION/defs.h"
#include "PHY_INTERFACE/defs.h"

#include "SIMULATION/PHY_EMULATION/impl_defs.h"

#ifdef USER_MODE
#include <math.h>
#endif




#define UINT_SIZE sizeof(unsigned int)
#define USHORT_SIZE sizeof(unsigned short)
#define RX_READY 5
//#define ONE 0x0001

#define  SYNC_WAIT 0
#define  SYNC_OK 1
#define  SYNC_NOK 2

#define  SYNC_OK_TRESHOLD 2
#define  SYNC_NOK_TRESHOLD 10
#define  PHY_CHBCH_SCH_WAIT_MAX  20


char is_node_local_neighbor(unsigned short Node_id);
void emul_check_out_in_traffic(void);

// Top-level emulation, invokes lower-level routines
void emulation_tx_rx(void);
void reset_rssi_sinr(void);
void reset_rssi_meas(void);
void clear_non_ack_req(void);
void emul_meas_ul_sch(void);
//void emul_rx_local_data(void);

/**@defgroup _phy_abstraction_ PHY Abstraction Implementation
 *@ingroup _ref_implementation_ 
 *@{
 */

/*!\fn void emul_rx_local_measurement(void)
  \brief Do measurement abstraction for local (same machine) measurements
*/

void emul_rx_local_measurement(void);


void phy_meas_ul_sch(unsigned char Mod_id , MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry);


//  
/*!\fn void emul_rx_local_ul_dl_data(void)
  \brief Do PDU error abstraction for local (same machine) CHBCH PDU error abstraction for local (same machine) SACH/SACCH
*/
void emul_rx_local_chbch_data(void);

//  

/*!\fn void emul_rx_local_ul_dl_data(void)
  \brief Do PDU error abstraction for local (same machine) SACH/SACCH
*/
void emul_rx_local_ul_dl_data(void);


unsigned int emul_tx_handler(u8 Mode,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows);


/*!\fn void emul_rx_data(void)
  \brief Do PDU error abstraction for external (via ethernet) SACH/SACCH
*/
unsigned int emul_rx_data(void);


unsigned int emul_rx_handler(unsigned char,char *rx_buffer,unsigned int R_bytes);

unsigned int emul_tx_handler(unsigned char,char *Tx_buffer,unsigned int* Nbytes,unsigned int *Nb_flows);

void serialize (MACPHY_DATA_REQ_TX *Tx_phy_pdu,char *Tx_buffer,unsigned int *Nbytes,unsigned char);

unsigned short deserialize_chbch(char *Phy_payload, CHBCH_PDU *CHBCH_pdu);

unsigned short deserialize_UL_sach(char *Phy_payload, UL_SACH_PDU *UL_SACH_pdu);

unsigned short deserialize_DL_sach(char *Phy_payload, DL_SACH_PDU *DL_SACH_pdu);

int bypass_mac_rx_handler(unsigned int fifo,int rw);

/*!\fn void get_transport_block_bler(double *signal_strength,
			      double *interference_strength,
			      unsigned char ntb,
			      unsigned char coding_fmt,
			      unsigned char spec_eff,
			      unsigned short tb_size_bits,
			      unsigned short sacch_size_bits,
			      unsigned int *sacch_carrier_alloc,
			      unsigned int *sach_chbch_carrier_alloc,
			      unsigned int *bler)
  \brief Compute a BLER vector for all TBs and SACCH in current allocation
  @param signal_strength Pointer to signal strength vector in current allocation
  @param interference_strength Pointer to interference strength vector in current allocation
  @param ntb Number of transport blocks in current allocation
  @param coding_fmt Indicator of code (rate 1/2 C.Code, rate 1/3 Turbo Code) and Modulation (QPSK,16-QAM,64-QAM)
  @param spec_eff Chosen spectral efficiency by MAC
  @param tb_size_bits Number of bits (including CRC+tail) in transport block
  @param sacch_size_bits Number of bits (including CRC+tail) in SACCH allocation (0 means no sacch)
  @param sacch_carrier_alloc Carrier allocation vector for sacch
  @param sach_chbch_carrier_alloc Carrier allocation vector for sach/chbch
  @param bler Vector of computed BLER (0,...,1e-6) for each TB and SACCH to be returned (BLER x 1e-6)
*/

void get_transport_block_bler(double *signal_strength,
			      double *interference_strength,
			      unsigned char ntb,
			      unsigned short tb_size_bits,
			      unsigned short sacch_size_bits,
			      unsigned int *sacch_carrier_alloc,
			      unsigned int *sach_chbch_carrier_alloc,
			      unsigned int *bler);


#define MAX_POSITIONS 16
#define NB_ANTENNAS_TX_MAX 4
#define NB_ANTENNAS_RX_MAX 4
#define NB_AMPS_MAX 8

#define MAX_NUM_CH 2// ?? H.A. 
#define NB_SUBCARRIERS_MAX NUMBER_OF_FREQUENCY_GROUPS //? H.A. 


/*! \brief position_t structure is used to represent the position of a node on the 2D-plane*/
typedef struct {
  double x,y;
}position_t;

/*! \brief PROPSIM_MOBILITY_DESC_UE structure is used to represent the static RF topology parameters*/
typedef struct {
  double BW; /*!< \brief This is the total system bandwidth for frequency correlation structure*/
  unsigned char nb_antennas_tx; /*!< \brief This is the total number of TX antennas for the node*/
  unsigned char nb_antennas_rx; /*!< \brief This is the total number of RX antennas for the node*/
  position_t positions[MAX_POSITIONS];   /*!< \brief This is a position vector for different points in space*/
  unsigned char speed[MAX_POSITIONS-1];/*!< \brief This is a speed vector for route between points*/
  double amps[NB_AMPS_MAX][MAX_POSITIONS][MAX_NUM_CH];/*!< \brief This is a multipath amplitude descriptor for a particular position relative to a given CH*/
  double delays[NB_AMPS_MAX][MAX_POSITIONS][MAX_NUM_CH];/*!< \brief This is a multipath delay descriptor for a particular position relative to a given CH*/
}PROPSIM_MOBILITY_DESC_UE;

/*! \brief PROPSIM_MOBILITY_DESC_CH structure is used to represent the static RF topology parameters (fixed for CH)*/
typedef struct {
  double BW; /*!< \brief This is the total system bandwidth for frequency correlation structure*/
  unsigned char nb_antennas_tx; /*!< \brief This is the total number of TX antennas for the node*/
  unsigned char nb_antennas_rx; /*!< \brief This is the total number of RX antennas for the node*/
  position_t positions[MAX_POSITIONS]; /*!< \brief This is a position vector for different points in space*/
}PROPSIM_MOBILITY_DESC_CH;



/*! \brief PROPSIM_P2P_CHANNEL_DESC structure is used to represent the instantaneous peer-to-peer RF topology parameters computed prior to PHY abstraction*/
typedef struct {
  struct complex16 freq_response_CH_to_UE[NB_SUBCARRIERS_MAX][NB_ANTENNAS_TX_MAX][NB_ANTENNAS_RX_MAX];
  struct complex16 freq_response_UE_to_CH[NB_SUBCARRIERS_MAX][NB_ANTENNAS_TX_MAX][NB_ANTENNAS_RX_MAX];
/*!< \brief Instantaneous spatial frequency response*/
  double TX_POWER_CH_dBm; /*!< \brief Instantaneous per-user DL transmit power (dBm)*/
  double TX_POWER_UE_dBm; /*!< \brief Instantaneous per-user UL transmit power (dBm)*/
  double PATH_LOSS_dB; /*!< \brief Instantaneous path-loss in dB*/
}PROPSIM_P2P_CHANNEL_DESC;


/*!\fn void phy_abstraction_ue(unsigned short Src_id, unsigned char Mod_id, MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry)
  \brief This routine computes the SINR between a source (Src_id) and the destination UE (Mod_id).  computes the subband channel responses between two nodes pointed to by Src_id and Dst_id
  @param Src_id    The MAC instance of the sending signal
  @param Dst_id    The MAC instance of the receiving UE
  @param Macphy_data_req_entry A pointer to the MAC RX request descriptor
*/
void phy_abstraction_ue(unsigned short Src_id, unsigned char Mod_id, MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry);

/*!\fn void phy_abstraction_ch(unsigned short Src_id, unsigned char Mod_id, MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry)
  \brief This routine computes the SINR between a source (Src_id) and the destination CH (Mod_id).  computes the subband channel responses between two nodes pointed to by Src_id and Dst_id
  @param Src_id    The MAC instance of the sending signal
  @param Dst_id    The MAC instance of the receiving UE
  @param Macphy_data_req_entry A pointer to the MAC RX request descriptor
*/
void phy_abstraction_ch(unsigned short Src_id, unsigned char Mod_id, MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_data_req_entry);

/*!\fn void propsim(unsigned short CH_id, unsigned char UE_id)
\brief This routine computes the subband channel responses between two nodes pointed to by CH_id and UE_id
@param CH_id    The MAC instance of the sending signal
@param UE_id    The MAC instance of the receiving signal
*/
void propsim(unsigned short CH_id, unsigned char UE_id);

//* @}
//*/


#endif
