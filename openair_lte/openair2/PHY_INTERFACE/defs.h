/*________________________mac_phy_primitives.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __MAC_PHY_PRIMITIVES_H__
#    define __MAC_PHY_PRIMITIVES_H__


#include "LAYER2/MAC/defs.h"



#define MAX_NUMBER_OF_MAC_INSTANCES 16

#define NULL_PDU 255
#define DCI 0
#define DLSCH 1
#define ULSCH 2




/*! \brief MACPHY Interface */
typedef struct
  {
    /// Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC
    void (*macphy_init)(void);

    /// Pointer function that stops the low-level scheduler due an exit condition        
    void (*macphy_exit)(const char *);          
    
    /// Frame counter
    unsigned int frame;

    // eNB functions
    /// Invoke dlsch/ulsch scheduling procedure for new subframe
    void (*eNB_dlsch_ulsch_scheduler)(u8 Mod_id, u8 subframe);

    /// Fill random access response sdu, passing timing advance
    u16 (*fill_rar)(u8 Mod_id,u8 *dlsch_buffer,u16 N_RB_UL, u8 input_buffer_length);

    /// Terminate the RA procedure upon reception of l3msg on ulsch
    void (*terminate_ra_proc)(u8 Mod_id,u16 UE_id, u8 *l3msg);

    /// Initiate the RA procedure upon reception (hypothetical) of a valid preamble
    void (*initiate_ra_proc)(u8 Mod_id,u16 preamble,s16 timing_offset,u8 sect_id);

    /// Get DCI for current subframe from MAC
    DCI_PDU* (*get_dci_sdu)(u8 Mod_id,u8 subframe);

    /// Get DLSCH sdu for particular RNTI and Transport block index
    u8* (*get_dlsch_sdu)(u8 Mod_id,u16 rnti,u8 TB_index);

    /// Send ULSCH sdu to MAC for given rnti
    void (*rx_sdu)(u8 Mod_id,u16 rnti, u8 *sdu);

    /// Indicate failure to synch to external source
    void (*mrbch_phy_sync_failure) (u8 Mod_id,u8 Free_ch_index);


    // UE functions

    /// Indicate loss of synchronization of PBCH
    void (*out_of_sync_ind)(u8 Mod_id,u16);

    ///  Send a received SI sdu
    void (*ue_decode_si)(u8 Mod_id, u8 CH_index, void *pdu, u16 len);

    /// Send a received DLSCH sdu to MAC
    void (*ue_send_sdu)(u8 Mod_id,u8 *sdu,u8 CH_index);

    /// Retrieve ULSCH sdu from MAC
    void (*ue_get_sdu)(u8 Mod_id,u8 CH_index,u8 *ulsch_buffer,u16 buflen);

    /// Retrieve RRCConnectionReq from MAC
    u8* (*ue_get_rach)(u8 Mod_id,u8 CH_index);

    /// Process Random-Access Response
    u16 (*ue_process_rar)(u8 Mod_id,u8 *dlsch_buffer,u16 *t_crnti);

    /// Indicate synchronization with valid PBCH
    void (*chbch_phy_sync_success) (u8 Mod_id, u8 CH_index);

    // PHY Helper Functions

    /// RIV computation from PHY
    u16 (*computeRIV)(u16 N_RB_DL,u16 RBstart,u16 Lcrbs);

    /// TBS table lookup from PHY
    u16 (*get_TBS)(u8 mcs, u16 nb_rb);

    /// Function to retrieve the HARQ round index for a particular UE/DLSCH and harq_pid
    void (*get_ue_active_harq_pid)(u8 Mod_id, u16 rnti, u8 subframe, u8 *harq_pid, u8 *round);



    //    unsigned char is_cluster_head;
    //    unsigned char is_primary_cluster_head;
    //    unsigned char is_secondary_cluster_head;
    //    unsigned char cluster_head_index;

    // PHY variables/functions
    
    /// PHY Measurements
    LTE_eNB_UE_stats **eNB_UE_stats;

    /// PHY Frame Configuration
    LTE_DL_FRAME_PARMS *lte_frame_parms;

    
  } MAC_xface;


#endif


