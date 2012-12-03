/*________________________defs.h________________________

 Authors : Raymond Knopp
 Company : EURECOM
 Emails  : knopp@eurecom.fr
________________________________________________________________*/


#ifndef __OPENAIR_RRC_DEFS_H__
#define __OPENAIR_RRC_DEFS_H__

#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "PHY/defs.h"
#include "COMMON/platform_constants.h"

#include "COMMON/mac_rrc_primitives.h"
#include "LAYER2/MAC/defs.h"

//#include "COMMON/openair_defs.h"
#ifndef USER_MODE
#include <rtai.h>
#endif

#include "SystemInformationBlockType1.h"
#include "SystemInformation.h"
#include "RRCConnectionReconfiguration.h"
#include "RRCConnectionReconfigurationComplete.h"
#include "RRCConnectionSetup.h"
#include "RRCConnectionSetupComplete.h"
#include "RRCConnectionRequest.h"
#include "BCCH-DL-SCH-Message.h"
#include "BCCH-BCH-Message.h"
#include "AS-Config.h"
#include "AS-Context.h"

//#include "L3_rrc_defs.h"
#ifndef NO_RRM
#include "L3_rrc_interface.h"
#include "rrc_rrm_msg.h"
#include "rrc_rrm_interface.h"
#endif

/** @defgroup _rrc_impl_ RRC Layer Reference Implementation
 * @ingroup _ref_implementation_
 * @{
 */

//#define NUM_PRECONFIGURED_LCHAN (NB_CH_CX*2)  //BCCH, CCCH

#define CH_READY 0

typedef enum  {
  RRC_IDLE=0,
  RRC_SI_RECEIVED,
  RRC_CONNECTED
} UE_STATE_t;



//#define NUMBER_OF_UE_MAX MAX_MOBILES_PER_RG
#define RRM_FREE(p)       if ( (p) != NULL) { free(p) ; p=NULL ; }
#define RRM_MALLOC(t,n)   (t *) malloc16( sizeof(t) * n ) 
#define RRM_CALLOC(t,n)   (t *) malloc16( sizeof(t) * n) 
#define RRM_CALLOC2(t,s)  (t *) malloc16( s ) 

#define MAX_MEAS_OBJ 6
#define MAX_MEAS_CONFIG 6
#define MAX_MEAS_ID 6

#define PAYLOAD_SIZE_MAX 1024
#define RRC_BUF_SIZE 140



typedef struct{
  UE_STATE_t State;
  u8 SIB1Status;
  u8 SIStatus;
  u8 SIwindowsize;
  u8 handoverTarget;
  u16 SIperiod;
  unsigned short UE_index;
  u32 T300_active;
  u32 T300_cnt;
  u32 T304_active;
  u32 T304_cnt;
  u32 T310_active;
  u32 T310_cnt;
  u32 N310_cnt;
  u32 N311_cnt;
}UE_RRC_INFO;

typedef struct{
  u8 Status[NUMBER_OF_UE_MAX];
  u8 Nb_ue;
  //unsigned short UE_index_list[NUMBER_OF_UE_MAX];
  //L2_ID UE_list[NUMBER_OF_UE_MAX];
  u8 UE_list[NUMBER_OF_UE_MAX][5];
}__attribute__ ((__packed__)) eNB_RRC_INFO;

typedef struct{
  int Status;
union{
	UE_RRC_INFO UE_info;
	eNB_RRC_INFO CH_info;
 }Info;
}RRC_INFO;

/* Intermediate structure for Hanodver management. Associated per-UE in RRC */
typedef struct{
	u8 ho_prepare;
	u8 ho_complete;
	u8 modid_s; //Mod_id of serving cell
	u8 modid_t; //Mod_id of target cell
	u8 ueid_s; //UE index in serving cell
	u8 ueid_t; //UE index in target cell
	AS_Config_t as_config; /* these two parameters are taken from 36.331 section 10.2.2: HandoverPreparationInformation-r8-IEs */
	AS_Context_t as_context; /* They are mandatory for HO */
	uint8_t buf[RRC_BUF_SIZE];	/* ASN.1 encoded handoverCommandMessage */
	int size;		/* size of above message in bytes(I think..) */
}HANDOVER_INFO;

#define RRC_HEADER_SIZE_MAX 64
#define RRC_BUFFER_SIZE_MAX 1024
typedef struct{
  char Payload[RRC_BUFFER_SIZE_MAX];
  char Header[RRC_HEADER_SIZE_MAX];  
  char payload_size;
}RRC_BUFFER; 
#define RRC_BUFFER_SIZE sizeof(RRC_BUFFER)

typedef struct{
  u16 Rb_id;  //=Lchan_id
  LCHAN_DESC Lchan_desc[2];
  MAC_MEAS_REQ_ENTRY *Meas_entry;
}RB_INFO;

typedef struct SRB_INFO{
  u16 Srb_id;  //=Lchan_id
  RRC_BUFFER Rx_buffer; 
  RRC_BUFFER Tx_buffer; 
  LCHAN_DESC Lchan_desc[2];
  unsigned int Trans_id;
  u8 Active;
}SRB_INFO;

typedef struct{
  RB_INFO Rb_info;
  u8 Active;
  u32 Next_check_frame;
  u8 Status;
}RB_INFO_TABLE_ENTRY;

typedef struct{
  SRB_INFO Srb_info;
  u8 Active;
  u8 Status;
u32 Next_check_frame;
}SRB_INFO_TABLE_ENTRY;

typedef struct {
	MeasId_t	 measId;
	//CellsTriggeredList	cellsTriggeredList;//OPTIONAL
	u32			 numberOfReportsSent;
} MEAS_REPORT_LIST;

typedef struct {
	u8 measFlag; //temporary flag to control frequency of MeasReport msgs
	PhysCellId_t targetCellId;
}HANDOVER_INFO_UE;

typedef struct{
  uint8_t                           *SIB1;
  uint8_t                           sizeof_SIB1;
  uint8_t                           *SIB23;
  uint8_t                           sizeof_SIB23;
  uint16_t                          physCellId;
  BCCH_BCH_Message_t                mib;
  BCCH_DL_SCH_Message_t             siblock1;
  SystemInformation_t               systemInformation;
  SystemInformationBlockType1_t     *sib1;
  SystemInformationBlockType2_t     *sib2;
  SystemInformationBlockType3_t     *sib3;
#ifdef Rel10
  SystemInformationBlockType13_r9_t *sib13;
  uint8_t                           MBMS_flag;
#endif
  struct SRB_ToAddMod               *SRB1_config[NUMBER_OF_UE_MAX];
  struct SRB_ToAddMod               *SRB2_config[NUMBER_OF_UE_MAX];
  struct DRB_ToAddMod               *DRB_config[NUMBER_OF_UE_MAX][8];
  uint8_t                           DRB_active[NUMBER_OF_UE_MAX][8];
  struct PhysicalConfigDedicated    *physicalConfigDedicated[NUMBER_OF_UE_MAX];
  struct SPS_Config                 *sps_Config[NUMBER_OF_UE_MAX];
  MeasObjectToAddMod_t              *MeasObj[NUMBER_OF_UE_MAX][MAX_MEAS_OBJ];
  struct ReportConfigToAddMod       *ReportConfig[NUMBER_OF_UE_MAX][MAX_MEAS_CONFIG];
  struct QuantityConfig             *QuantityConfig[NUMBER_OF_UE_MAX];
  struct MeasIdToAddMod             *MeasId[NUMBER_OF_UE_MAX][MAX_MEAS_ID];
  MAC_MainConfig_t                  *mac_MainConfig[NUMBER_OF_UE_MAX];
  MeasGapConfig_t                   *measGapConfig[NUMBER_OF_UE_MAX];
  eNB_RRC_INFO                      Info;
  SRB_INFO                          SI;
  SRB_INFO                          Srb0;
  SRB_INFO_TABLE_ENTRY              Srb1[NUMBER_OF_UE_MAX+1];
  SRB_INFO_TABLE_ENTRY              Srb2[NUMBER_OF_UE_MAX+1];
  MeasConfig_t						*measConfig[NUMBER_OF_UE_MAX];
//#ifdef X2_SIM
  HANDOVER_INFO						*handover_info[NUMBER_OF_UE_MAX];
//#endif
} eNB_RRC_INST;


typedef struct{
  UE_RRC_INFO Info[NB_SIG_CNX_UE];
  SRB_INFO Srb0[NB_SIG_CNX_UE];
  SRB_INFO_TABLE_ENTRY Srb1[NB_CNX_UE];
  SRB_INFO_TABLE_ENTRY Srb2[NB_CNX_UE];
  HANDOVER_INFO_UE HandoverInfoUe;
  u8 *SIB1[NB_CNX_UE];
  u8 sizeof_SIB1[NB_CNX_UE];
  u8 *SI[NB_CNX_UE];
  u8 sizeof_SI[NB_CNX_UE];
  u8 SIB1Status[NB_CNX_UE];
  u8 SIStatus[NB_CNX_UE];
  double  filter_coeff_rsrp;
  double filter_coeff_rsrq;
  SystemInformationBlockType1_t *sib1[NB_CNX_UE];
  SystemInformation_t *si[NB_CNX_UE][8];
  SystemInformationBlockType2_t *sib2[NB_CNX_UE];
  SystemInformationBlockType3_t *sib3[NB_CNX_UE];
  SystemInformationBlockType4_t *sib4[NB_CNX_UE];
  SystemInformationBlockType5_t *sib5[NB_CNX_UE];
  SystemInformationBlockType6_t *sib6[NB_CNX_UE];
  SystemInformationBlockType7_t *sib7[NB_CNX_UE];
  SystemInformationBlockType8_t *sib8[NB_CNX_UE];
  SystemInformationBlockType9_t *sib9[NB_CNX_UE];
  SystemInformationBlockType10_t *sib10[NB_CNX_UE];
  SystemInformationBlockType11_t *sib11[NB_CNX_UE];
#ifdef Rel10
  SystemInformationBlockType12_r9_t *sib12[NB_CNX_UE];
  SystemInformationBlockType13_r9_t *sib13[NB_CNX_UE];
#endif
  struct SRB_ToAddMod             *SRB1_config[NB_CNX_UE];
  struct SRB_ToAddMod             *SRB2_config[NB_CNX_UE];
  struct DRB_ToAddMod             *DRB_config[NB_CNX_UE][8];
  MeasObjectToAddMod_t            *MeasObj[NB_CNX_UE][MAX_MEAS_OBJ];
  struct ReportConfigToAddMod     *ReportConfig[NB_CNX_UE][MAX_MEAS_CONFIG];
  struct QuantityConfig           *QuantityConfig[NB_CNX_UE];
  struct MeasIdToAddMod           *MeasId[NB_CNX_UE][MAX_MEAS_ID];
  MEAS_REPORT_LIST				  *measReportList[NB_CNX_UE][MAX_MEAS_ID];
  u32							  measTimer[NB_CNX_UE][MAX_MEAS_ID][6]; // 6 is for hexagonal config
  RSRP_Range_t                    s_measure;
  struct MeasConfig__speedStatePars	  *speedStatePars;
  struct PhysicalConfigDedicated  *physicalConfigDedicated[NB_CNX_UE];
  struct SPS_Config               *sps_Config[NB_CNX_UE];
  MAC_MainConfig_t                *mac_MainConfig[NB_CNX_UE];
  MeasGapConfig_t                 *measGapConfig[NB_CNX_UE];
}UE_RRC_INST;

//main.c
int rrc_init_global_param(void);
int L3_xface_init(void);
void openair_rrc_top_init(void);
char openair_rrc_lite_eNB_init(u8 Mod_id);
char openair_rrc_lite_ue_init(u8 Mod_id,u8 CH_IDX);
void rrc_config_buffer(SRB_INFO *srb_info, u8 Lchan_type, u8 Role);
void openair_rrc_on(u8 Mod_id,u8 eNB_flag);

/** \brief Function to update timers every subframe.  For UE it updates T300,T304 and T310.
@param Mod_id Instance of UE/eNB
@param frame Frame index
@param eNB_flag Flag to indicate if this instance is and eNB or UE
@param index Index of corresponding eNB (for UE)
*/
RRC_status_t rrc_rx_tx(u8 Mod_id,u32 frame, u8 eNB_flag,u8 index);

// UE RRC Procedures

/** \brief Decodes DL-CCCH message and invokes appropriate routine to handle the message
    \param Mod_id Instance ID of UE
    \param Srb_info Pointer to SRB_INFO structure (SRB0)
    \param CH_index Index of corresponding eNB/CH*/
int rrc_ue_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info,u8 CH_index);

/** \brief Decodes a DL-DCCH message and invokes appropriate routine to handle the message
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param Srb_id Index of Srb (1,2)
    \param Buffer Pointer to received SDU
    \param CH_index Index of corresponding CH/eNB*/
void rrc_ue_decode_dcch(u8 Mod_id, u32 frame, u8 Srb_id, u8* Buffer,u8 CH_index);

/** \brief Generate/Encodes RRCConnnectionRequest message at UE 
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param Srb_id Index of Srb (1,2)
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, u32 frame, u8 CH_index);

/** \brief Generates/Encodes RRCConnnectionSetupComplete message at UE 
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id,u32 frame,u8 CH_index);\

/** \brief process the received rrcConnectionReconfiguration message at UE 
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param *rrcConnectionReconfiguration pointer to the sturcture
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_process_rrcConnectionReconfiguration(u8 Mod_id, u32 frame,RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,u8 eNB_index);

/** \brief Generates/Encodes RRCConnectionReconfigurationComplete  message at UE 
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id, u32 frame, u8 eNB_index);

/** \brief Establish SRB1 based on configuration in SRB_ToAddMod structure.  Configures RLC/PDCP accordingly
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param eNB_index Index of corresponding eNB/CH
    \param SRB_config Pointer to SRB_ToAddMod IE from configuration
    @returns 0 on success*/
s32  rrc_ue_establish_srb1(u8 Mod_id,u32 frame,u8 eNB_index,struct SRB_ToAddMod *SRB_config);

/** \brief Establish SRB2 based on configuration in SRB_ToAddMod structure.  Configures RLC/PDCP accordingly
    \param Mod_id Instance ID of UE
    \param frame Frame index
    \param eNB_index Index of corresponding eNB/CH
    \param SRB_config Pointer to SRB_ToAddMod IE from configuration
    @returns 0 on success*/
s32  rrc_ue_establish_srb2(u8 Mod_id,u32 frame, u8 eNB_index,struct SRB_ToAddMod *SRB_config);

/** \brief Establish a DRB according to DRB_ToAddMod structure
    \param Mod_id Instance ID of UE
    \param CH_index Index of corresponding CH/eNB
    \param DRB_config Pointer to DRB_ToAddMod IE from configuration
    @returns 0 on success */
s32  rrc_ue_establish_drb(u8 Mod_id,u32 frame,u8 CH_index,struct DRB_ToAddMod *DRB_config);

/** \brief Process a measConfig Message and configure PHY/MAC
    \param Mod_id Instance of UE on which to act
    \param CH_index Index of corresponding CH/eNB
    \param  measConfig Pointer to MeasConfig  IE from configuration*/
void	rrc_ue_process_measConfig(u8 Mod_id,u8 eNB_index,MeasConfig_t *measConfig);

/** \brief Process a RadioResourceConfigDedicated Message and configure PHY/MAC
    \param Mod_id Instance of UE on which to act
    \param CH_index Index of corresponding CH/eNB
    \param radioResourceConfigDedicated Pointer to RadioResourceConfigDedicated IE from configuration*/
void rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u32 frame, u8 CH_index,
						 RadioResourceConfigDedicated_t *radioResourceConfigDedicated);

// eNB/CH RRC Procedures

/**\brief Entry routine to decode a UL-CCCH-Message.  Invokes PER decoder and parses message.
   \param Mod_id Instance ID for CH/eNB
   \param frame  Frame index
   \param Srb_info Pointer to SRB0 information structure (buffer, etc.)*/
int rrc_eNB_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info);

/**\brief Entry routine to decode a UL-DCCH-Message.  Invokes PER decoder and parses message.
   \param Mod_id Instance ID for CH/eNB
   \param frame Frame index
   \param UE_index Index of UE sending the message
   \param Rx_sdu Pointer Received Message
   \param sdu_size Size of incoming SDU*/
int rrc_eNB_decode_dcch(u8 Mod_id, u32 frame, u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size);  

/**\brief Generate the RRCConnectionSetup based on information coming from RRM
   \param Mod_id Instance ID for eNB/CH
   \param frame Frame index
   \param UE_index Index of UE receiving the message*/
void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u32 frame, u16 UE_index);

/**\brief Process the RRCConnectionSetupComplete based on information coming from UE
   \param Mod_id Instance ID for eNB/CH
   \param frame Frame index
   \param UE_index Index of UE transmitting the message
   \param rrcConnectionSetupComplete Pointer to RRCConnectionSetupComplete message*/
void rrc_eNB_process_RRCConnectionSetupComplete(u8 Mod_id, u32 frame, u8 UE_index, 
						RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete);

/**\brief Process the RRCConnectionReconfigurationComplete based on information coming from UE
   \param Mod_id Instance ID for eNB/CH
   \param UE_index Index of UE transmitting the messages
   \param rrcConnectionReconfigurationComplete Pointer to RRCConnectionReconfigurationComplete message*/
void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u32 frame,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete);

/**\brief Generate/decode the RRCConnectionReconfiguration at eNB
   \param Mod_id Instance ID for eNB/CH
   \param frame Frame index
   \param UE_index Index of UE transmitting the messages*/
void rrc_eNB_generate_RRCConnectionReconfiguration(u8 Mod_id,u32 frame,u16 UE_index);


//L2_interface.c
s8 mac_rrc_lite_data_req( u8 Mod_id, u32 frame, unsigned short Srb_id, u8 Nb_tb,char *Buffer,u8 eNB_flag,u8 eNB_index);
s8 mac_rrc_lite_data_ind( u8 Mod_id,  u32 frame, unsigned short Srb_id, char *Sdu, unsigned short Sdu_len,u8 eNB_flag,u8 Mui);
void mac_sync_ind( u8 Mod_id, u8 status);
void rrc_lite_data_ind( u8 Mod_id, u32 frame, u8 eNB_flag, u32 Rb_id, u32 sdu_size,u8 *Buffer);
void rrc_lite_out_of_sync_ind(u8 Mod_id, u32 frame, unsigned short eNB_index);

/* 
uint8_t do_SIB1(LTE_DL_FRAME_PARMS            *frame_parms, 
		uint8_t                       *buffer,
		BCCH_DL_SCH_Message_t         *bcch_message,
		SystemInformationBlockType1_t **sib1);
*/
/** 
\brief Generate a default configuration for SIB2/SIB3 in one System Information PDU (eNB).
@param Mod_id Index of eNB (used to derive some parameters)
@param buffer Pointer to PER-encoded ASN.1 description of SI PDU
@param systemInformation Pointer to asn1c C representation of SI PDU
@param sib2 Pointer (returned) to sib2 component withing SI PDU
@param sib3 Pointer (returned) to sib3 component withing SI PDU
@return size of encoded bit stream in bytes*/
/*
uint8_t do_SIB23(uint8_t Mod_id,
		 uint8_t *buffer,  
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3
#ifdef Rel10
		 ,
                 SystemInformationBlockType13_r9_t **sib13,
		 uint8_t MBMS_flag
#endif
);
*/
/** 
\brief Generate an RRCConnectionRequest UL-CCCH-Message (UE) based on random string or S-TMSI.  This 
routine only generates an mo-data establishment cause.
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@param rv 5 byte random string or S-TMSI
@returns Size of encoded bit stream in bytes*/
//uint8_t do_RRCConnectionRequest(uint8_t *buffer,u8 *rv);

/** \brief Generate an RRCConnectionSetupComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
//uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer);

/** \brief Generate an RRCConnectionReconfigurationComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
//uint8_t do_RRCConnectionReconfigurationComplete(uint8_t *buffer);

/** 
\brief Generate an RRCConnectionSetup DL-CCCH-Message (eNB).  This routine configures SRB_ToAddMod (SRB1/SRB2) and 
PhysicalConfigDedicated IEs.  The latter does not enable periodic CQI reporting (PUCCH format 2/2a/2b) or SRS.
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param transmission_mode Transmission mode for UE (1-9)
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param frame_parms Pointer to DL Frame Configuration parameters for physicalConfigDedicated
@param SRB1_config Pointer (returned) to SRB1_config IE for this UE
@param physicalConfigDedicated Pointer (returned) to PhysicalConfigDedicated IE for this UE
@returns Size of encoded bit stream in bytes*/
/*uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      uint8_t transmission_mode,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      struct SRB_ToAddMod **SRB1_config,
			      struct SRB_ToAddMod **SRB2_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);
*/
/** 
\brief Generate an RRCConnectionReconfiguration DL-DCCH-Message (eNB).  This routine configures SRBToAddMod (SRB2) and one DRBToAddMod 
(DRB3).  PhysicalConfigDedicated is not updated.
@param Mod_id Module ID of this eNB Instance
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param SRB_list Pointer to SRB List to be added/modified (NULL if no additions/modifications)
@param DRB_list Pointer to DRB List to be added/modified (NULL if no additions/modifications)
@param DRB_list2 Pointer to DRB List to be released      (NULL if none to be released)
@param sps_Config Pointer to sps_Config to be modified (NULL if no modifications, or default if initial configuration)
@param physicalConfigDedicated Pointer to PhysicalConfigDedicated to be modified (NULL if no modifications)
@param MeasObj_list Pointer to MeasObj List to be added/modified (NULL if no additions/modifications)
@param ReportConfig_list Pointer to ReportConfig List (NULL if no additions/modifications)
@param QuantityConfig Pointer to QuantityConfig to be modified (NULL if no modifications)
@param MeasId_list Pointer to MeasID List (NULL if no additions/modifications)
@param mac_MainConfig Pointer to Mac_MainConfig(NULL if no modifications)
@param measGapConfig Pointer to MeasGapConfig (NULL if no modifications)
@returns Size of encoded bit stream in bytes*/
/*
uint8_t do_RRCConnectionReconfiguration(uint8_t                           Mod_id,
					uint8_t                           *buffer,
					uint8_t                           UE_id,
					uint8_t                           Transaction_id,
					SRB_ToAddModList_t                *SRB_list,
					DRB_ToAddModList_t                *DRB_list,
					DRB_ToReleaseList_t               *DRB_list2,
					struct SPS_Config                 *sps_Config,
					struct PhysicalConfigDedicated    *physicalConfigDedicated,
					MeasObjectToAddModList_t          *MeasObj_list,
					ReportConfigToAddModList_t        *ReportConfig_list.
					QuantityConfig                    *QuantityConfig,
					MeasIdToAddModList_t              *MeasId_list,
					MAC_MainConfig_t                  *mac_MainConfig,
					MeasGapConfig_t                   *measGapConfig)
*/


/**
\brief Generate an MCCH-Message (eNB). This routine configures MBSFNAreaConfiguration (PMCH-InfoList and Subframe Allocation for MBMS data)
@param buffer Pointer to PER-encoded ASN.1 description of MCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
//uint8_t do_MCCHMessage(uint8_t *buffer);


int decode_SIB1(u8 Mod_id,u8 CH_index);

int decode_SI(u8 Mod_id,u32 frame,u8 CH_index,u8 si_window);

int mac_get_rrc_lite_status(u8 Mod_id,u8 eNB_flag,u8 eNB_index);

#endif


/** @ */
