/*________________________mac_phy_primitives.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __MAC_PHY_PRIMITIVES_H__
#    define __MAC_PHY_PRIMITIVES_H__

#include "LAYER2/MAC/defs.h"


/** @defgroup _mac_phy_primitives_ MAC Layer Primitives for Communications with PHY 
* @ingroup _openair_mac_layer_specs_
* @{

This subclause describes the primitives for communications between the MAC and PHY sub-layers.

The primitives for dynamic MAC-PHY PDU exchange (Transport channel interface) are:

- MACPHY_DATA_REQ: transfers or requests a PDU from PHY.  The data is passed along with the dynamic PHY transmission 
format (coding and modulation, time/freq/space resource allocation)

- MACPHY_DATA_IND: Function call (by PHY) to deliver a new PDU and corresponding measurements to MAC.  This implicitly confirms the MACPHY_DATA_REQ by
filling the fields of the request (TX or RX) with the data and measurements.

One primitive is used for semi-static configuration (during logical channel establishment) 
relaying the puncturing/repetition patterns for HARQ:

- MACPHY_CONFIG_SACH_HARQ_REQ (still to be defined...)
 
The primitive for static (re)configuration is:
- MACPHY_CONFIG_REQ : This primitive transports the initial configuration during the setup phase of equipment, both for CH and UE.
Static configuration is used during the initialization phase of the equipment.  For a CH, it is done prior to any communication.  For a UE, some
structures may be set after receiving configuration information from the network via the BCCH/CCCH.
*/




/*! \brief MACPHY-DATA-REQ_RX structure is used to request transfer a new PDU from PHY corresponding to a particular transport channel*/
typedef struct { 
  int crc_status[MAX_NUMBER_TB_PER_LCHAN];                   /*!< This field indicates the CRC status of the PDU upon reception from PHY*/
#ifdef PHY_EMUL
  unsigned short Src_id;  
#endif 
  unsigned short Active_process_map;   /*!< HARQ indicator for active processes*/
  union {
    CHBCH_PDU   *Chbch_pdu;        /*!< This is a pointer to CHBCH data*/
    DL_SACH_PDU DL_sach_pdu;      /*!< This is a pointer to DL_SACH data*/
    UL_SACH_PDU  UL_sach_pdu;      /*!< This is a pointer to UL_SACH data*/
    RACH_PDU    *Rach_pdu;         /*!< This is a pointer to RACH data*/
    MRBCH_PDU   *Mrbch_pdu;        /*!< This is a pointer to MRBCH data*/
   UL_SACCH_PDU *UL_sacch_pdu;
  } Pdu;
  union {
    DL_MEAS *DL_meas;   /*!< This is an array of pointers to the current measurements of DL quality at UE (indexed by CH_id) */
    UL_MEAS *UL_meas;   /*!< This is an array of pointers to the current measurements of UL quality at Node-B (indexed by user_id) */
  } Meas;
} MACPHY_DATA_REQ_RX;


/*! \brief MACPHY-DATA-REQ_TX structure is used to transfer a new PDU to PHY corresponding to a particular transport channel*/
typedef struct { 
  unsigned short Active_process_map;   /*!< HARQ indicator for active processes*/
  unsigned short New_process_map;      /*!< HARQ indicator for new processes*/
  //  unsigned char round_indices_tx;     
  union {
    CHBCH_PDU   *Chbch_pdu;      /*!< pointer to CHBCH data */
    DL_SACH_PDU DL_sach_pdu;    /*!< pointer to DL_SACH data*/
    UL_SACH_PDU UL_sach_pdu;    /*!< pointer to UL_SACH data*/
    RACH_PDU Rach_pdu;  //H.A   /*!< pointer to RACH data */
    MRBCH_PDU   *Mrbch_pdu;     /*!< pointer to MRBCH data */
  } Pdu;
}MACPHY_DATA_REQ_TX;

/*! \brief MACPHY-DATA-REQ primitive is used to transfer a new PDU to PHY corresponding to a particular transport channel*/
typedef struct {
  unsigned char num_tb;                /*!< This field indicates the number of transport blocks to be received*/
  unsigned short tb_size_bytes;        /*!< This field indicates the number of bytes per transpor block*/
  unsigned char CH_index;

#ifdef PHY_EMUL
  unsigned short Src_id;
#endif
  unsigned char Direction;
  unsigned char Pdu_type;                 /*!< This field indicates the type of PDU requested */
  LCHAN_ID      Lchan_id;                 /*!< This field indicates the flow id of the PDU */
  PHY_RESOURCES *Phy_resources;           /*!< This field indicates to PHY the physical resources */
  unsigned int format_flag;               /*!< This field indicates to PHY something about a SACH, e.g. presense of SACCH*/
  union {
    MACPHY_DATA_REQ_RX Req_rx;            /*!< This field contains the request corresponding to an RX resource*/
    MACPHY_DATA_REQ_TX Req_tx;            /*!< This field contains the request corresponding to a TX resource*/
  } Dir;
}MACPHY_DATA_REQ;

/*!\fn void macphy_data_ind(unsigned char Mod_id,MACPHY_DATA_REQ_RX *Req_rx,unsigned char Pdu_type,unsigned short Index);
\brief MACPHY_DATA_IND function call.  Called by PHY to upload PDU and measurements in response to a MACPHY_DATA_REQ_RX.
@param Mod_id MAC instance ID (only useful if multiple MAC instances run in the same machine)
@param Req_rx Pointer to MACPHY_DTA_REQ_RX received previously
@param Pdu_type Type of PDU (redundant!)
@param Index CH Index for CH, UEid for UE
*/
void macphy_data_ind(unsigned char Mod_id,
		     MACPHY_DATA_REQ_RX *Req_rx,
		     unsigned char Pdu_type,
		     unsigned short Index);

/*! \brief MACPHY-CONFIG-REQ primitive is used to configure a new instance of OpenAirInterface (static configuration) during initialization*/
typedef struct {
  PHY_FRAMING Phy_framing;   /*!< Framing Configuration*/
  PHY_CHSCH Phy_chsch[8];    /*!< CHSCH Static Configuration*/
  PHY_CHBCH Phy_chbch;       /*!< CHBCH Static Configuration*/
  PHY_SCH   Phy_sch[8];      /*!< SCH Static Configuration*/
  PHY_SACH  Phy_sach;        /*!< SACH Statuc Configuration*/
} MACPHY_CONFIG_REQ;

/*! \brief MACPHY-CONFIG-SACH-HARQ-REQ primitive is used to configure a new SACH transport channel (dynamic configuration) during logical channel establishment*/
//typedef struct {
//  LCHAN_ID Lchan_id;             /*!< This is the identifier of the SACH, which should simply be the logical channel id*/
//  HARQ_PARAMS Harq_params;           /*!< This is the set of HARQ parameters corresponding to the QoS description of the logical channel*/
//} MACPHY_CONFIG_SACH_HARQ_REQ;

/** @} */

#define MAX_NUMBER_OF_MAC_INSTANCES 16

#define NULL_PDU 255
#define CHBCH 0
#define DL_SACH 1
#define UL_SACH 2
#define UL_SACCH_SACH 3
#define RACH 4
#define MRBCH 5
#define CHBCH_SCH 6
#define DL_SCH 7
#define UL_SCH 8


#define NUMBER_OF_SUBBANDS 64
#define LCHAN_KEY 0
#define PDU_TYPE_KEY 1
#define PHY_RESOURCES_KEY 2

typedef struct Macphy_req_entry_key{
  unsigned char Key_type;
  union{
    LCHAN_ID *Lchan_id;  //SACH, EMULATION
    unsigned char Pdu_type;//CHBCH, RACH, EMULATION
    PHY_RESOURCES Phy_resources;//REAL PHY
  }Key;
    unsigned short CH_index;
}MACPHY_REQ_ENTRY_KEY;

/** @ingroup _PHY_TRANSPORT_CHANNEL_PROCEDURES_
 * @{
\brief An entry in the MACPHY_DATA_REQ Table.
*/
typedef struct Macphy_data_req_table_entry {
  /// The MACPHY_DATA_REQ Structure itself
  MACPHY_DATA_REQ Macphy_data_req;
  /// Active flag.  Active=1 means that the REQ is pending.
  unsigned char Active;
} MACPHY_DATA_REQ_TABLE_ENTRY;

/*
\brief The MACPHY_DATA_REQ interface between MAC and PHY.  This table stores the pending requests from MAC which are serviced by PHY.  The pointer Macphy_req_table_entry points
to an array of idle reqests allocated during initialization of the MAC-layer.
*/
typedef struct  {
  /// Pointer to a MACPHY_DATA_REQ
  MACPHY_DATA_REQ_TABLE_ENTRY *Macphy_req_table_entry;
  /// Number of active requests
  unsigned int Macphy_req_cnt;
} MACPHY_DATA_REQ_TABLE;

/** @} */

/*typedef struct Tx_Phy_Pdu{                              //H.A
  PHY_RESOURCES *Phy_resources;
  MACPHY_DATA_IND *Macphy_data_ind;
}T_PHY_PDU;

typedef struct Rx_Phy_Pdu{                              //H.A
  PHY_RESOURCES *Phy_resources;
  char *Phy_payload;
  }RX_PHY_PDU;*/


typedef struct GRANTED_LCHAN_TABLE_ENTRY{
  PHY_RESOURCES *Phy_resources;
  LCHAN_ID Lchan_id;
}GRANTED_LCHAN_TABLE_ENTRY;

void clear_macphy_data_req(u8);
//void clean_macphy_interface(void);
unsigned char phy_resources_compare(PHY_RESOURCES *,PHY_RESOURCES*);
MACPHY_DATA_REQ_TABLE_ENTRY* find_data_req_entry(u8,MACPHY_REQ_ENTRY_KEY*);
void print_active_requests(u8);
void mac_process_meas_ul(u8 Mod_id,UL_MEAS *UL_meas, u16 Index);
void mac_process_meas_dl(u8 Mod_id,DL_MEAS *DL_meas, u16 Index);




MACPHY_DATA_REQ *new_macphy_data_req(u8);
//PHY_RESOURCES_TABLE_ENTRY *new_phy_resources(void);
//MACPHY_DATA_IND *new_macphy_data_ind(void);



/*! \brief MACPHY Interface */
typedef struct
  {

    void (*macphy_scheduler)(unsigned char); /*!<\brief Pointer to phy scheduling routine in MAC.  Used by the low-level hardware synchronized scheduler*/
    void (*macphy_setparams)(void *);     /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_init)(void);          /*  Pointer function that reads params for the MAC interface - this function is called when an IOCTL passes parameters to the MAC */
    void (*macphy_exit)(const char *);          /*  Pointer function that stops the low-level scheduler due an exit condition */
    void (*macphy_data_ind)(unsigned char, MACPHY_DATA_REQ_RX *,unsigned char, unsigned short);
    //    PHY_CONFIG *PHY_cfg;
    //#ifdef PHY_EMUL
    void (*out_of_sync_ind)(unsigned char,unsigned short);
    //#endif

  void (*mrbch_phy_sync_failure) (unsigned char Mod_id, unsigned char Free_ch_index);
  void (*chbch_phy_sync_success) (unsigned char Mod_id, unsigned char CH_index);

    unsigned int frame;
    unsigned char slots_per_frame;
    unsigned char is_cluster_head;
    unsigned char is_primary_cluster_head;
    unsigned char is_secondary_cluster_head;
    unsigned char cluster_head_index;
  } MAC_xface;
  

#endif


