/*________________________mac_defs.h________________________
  
 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
 ________________________________________________________________*/

/*! \file mac_defs.h 
* \brief Structures and definitions for generic mac interface
* \author R. Knopp
* \date March 2006
* \note
* \warning
* @ingroup macxface
*/


#ifndef __MAC_INTERFACE_DEFS_H__
#define __MAC_INTERFACE_DEFS_H__


#include "PHY/types.h"


/*! \fn void macphy_scheduler(u8 slot)
*  \brief TTI MAC entry point for TrCH channel scheduling
*  \param last_slot
* @ingroup  tch
*/ 
void macphy_scheduler(u8);

/*! \fn int mac_init(void)
* \brief 
* \return 0 on success, otherwise -1 
* @ingroup  tch
*/
int mac_init(void);

/*! \fn void MAC_cleanup(void)
*  \brief freeing the allocated memory 
* @ingroup  tch
*/
void mac_cleanup(void);

/*
  \fn void mac_resynch(void)
*  \brief Clean up MAC after resynchronization procedure.  Called by low-level scheduler during resynch.
*/
void mac_resynch(void);


#ifndef USER_MODE
#ifdef DEBUG_LEVEL
#define COMMENT //
#define msg  (((mac_xface->frame%5==0))? printk : COMMENT) 
#endif
#endif

#ifdef OPENAIR2
#include "LAYER2/MAC/defs.h"
#include "PHY_INTERFACE/defs.h"


#else


/*! * @ingroup  _PHY_MAC_INTERFACE_
 * @{
 */

/*! \brief MACPHY Interface */
typedef struct
  {

    void (*macphy_scheduler)(unsigned char); /*!<\brief Pointer to phy scheduling routine in MAC.  Used by the low-level hardware synchronized scheduler*/
    void (*macphy_setparams)(void *);     /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_init)(void);          /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_exit)(const char *);          /*  Pointer function that stops the low-level scheduler due an exit condition */
#ifdef OPENAIR2
    void (*macphy_data_ind)(unsigned char, unsigned char, unsigned short);
    void (*out_of_sync_ind)(unsigned char,unsigned short);

    void (*mrbch_phy_sync_failure) (unsigned char Mod_id, unsigned char Free_ch_index);
    void (*chbch_phy_sync_success) (unsigned char Mod_id, unsigned char CH_index);


#endif //OPENAIR2
    unsigned int frame;
    unsigned char slots_per_frame;
    unsigned char is_cluster_head;
    unsigned char is_primary_cluster_head;
    unsigned char is_secondary_cluster_head;
    unsigned char cluster_head_index;
  } MAC_xface;
  
/*@}*/



/*! \brief Uplink SACCH Feedback Information
 */
typedef struct {
  unsigned char  Pc:2 ;          /*!< \brief Power control bits (0,1,-1)*/
  unsigned char  Qdepth ;      /*!< \brief Backlog of corresponding UL Logical Channel RLC Queue;  1 bits for DTCH LCHAN index (NB DTCH MAX 8)*/
  unsigned short  Ack:14;           /*!< \brief HARQ acknowlegde receipt, bit-mapped for 8 HARQ processes*/
  unsigned int cqi;
}__attribute__ ((__packed__)) UL_SACCH_FB;

/*! \brief Uplink SACCH PDU Structure
 */
typedef struct {
#ifdef PHY_EMUL
  unsigned short Lchan_id ;          /*!< \brief MAC PDU Logical Channel ID */
  unsigned short Pdu_size ;
  // unsigned CH_id  __attribute__ ((packed));  //I need this for RAB Identification
#endif //PHY_EMUL
  unsigned char Coding_fmt  ;  /*!< \brief Coding format for this PDU*/
  //  unsigned int  Seq_index __attribute__ ((packed));     /*!< \brief MAC PDU sequence index (HARQ), 8 times 4 bits*/
  UL_SACCH_FB   UL_sacch_fb ;  /*!< \brief Feedback information (ACK,CSI,PC)*/
} __attribute__ ((__packed__)) UL_SACCH_PDU;


#define NUMBER_OF_MEASUREMENT_SUBBANDS 16//NUMBER_OF_FREQUENCY_GROUPS 

/*! \brief Downlink PHY measurement structure
 */
typedef struct {
  char Wideband_rssi_dBm;                          /*!< This is a wideband rssi (signal plus interference) measurement of Node-B signal strength summed over receive antennas.  This is derived from the downlink CHSCH pilot symbol */
  char Wideband_interference_level_dBm;            /*!< This is a wideband interference level measurement (common to all Node-B measurements!)*/
  //  char Subband_spatial_sinr[NUMBER_OF_MEASUREMENT_SUBBANDS]; /*!< This measures the downlink SINR on a sub-carrier basis for each transmit/receive antenna pair*/
  char Sub_band_sinr[NUMBER_OF_MEASUREMENT_SUBBANDS];          /*!< This measures the downlink aggregate SINR per frequency group*/ 
  char Wideband_sinr_dB;
} __attribute__ ((__packed__))DL_MEAS;

/*! \brief Uplink PHY measurement structure
 */ 
typedef struct {
  char Wideband_rssi_dBm;                          /*!< This is a wideband rssi (signal plus interference) measurement of UE signal strength summed over receive antennas.This is derived from the UPLINK SCH pilot symbol*/
  char Wideband_interference_level_dBm;           /*!< This is a wideband interference level measurement (common to all UE measurements!)*/
  char Sub_band_sinr[NUMBER_OF_MEASUREMENT_SUBBANDS];          /*!< This measures the uplink aggregate SINR per frequency group*/
} __attribute__ ((__packed__))UL_MEAS; 


#endif //OPENAIR2
#endif 

/** @} */
