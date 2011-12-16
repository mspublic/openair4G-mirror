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
#define RRC_IDLE 1
#define RRC_PRE_SYNCHRO 2
#define RRC_PRE_ASSOCIATED 3
#define RRC_ASSOCIATED 4
#define RRC_CONNECTED 5

#define NB_CNX_eNB MAX_MOBILES_PER_RG
#define RRM_FREE(p)       if ( (p) != NULL) { free(p) ; p=NULL ; }
#define RRM_MALLOC(t,n)   (t *) malloc16( sizeof(t) * n ) 
#define RRM_CALLOC(t,n)   (t *) malloc16( sizeof(t) * n) 
#define RRM_CALLOC2(t,s)  (t *) malloc16( s ) 


#define MSG_L2ID(p) msg("[INFO] L2ID=%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",\
	                (p).L2_id[0],(p).L2_id[1], (p).L2_id[2],(p).L2_id[3],\
	                (p).L2_id[4],(p).L2_id[5], (p).L2_id[6],(p).L2_id[7] )


#define PAYLOAD_SIZE_MAX 1024

#define NB_RAB_BRODCAST_MAX 2
#define MAX_ALLOWED_BCCH_MISS 10

typedef struct{
  u8 Status;
  u8 SIB1Status;
  u8 SIStatus;
  u8 SIwindowsize;
  u16 SIperiod;
  u8 CH_id;
  unsigned short UE_index;
  unsigned int Rach_tx_cnt;
  unsigned int Nb_bcch_wait;
  unsigned int Nb_bcch_miss;
  u8 Nb_rach_res; 
  u8 Rach_time_alloc;
  unsigned short Rach_freq_alloc;
  //  L2_ID CH_mac_id;
}UE_RRC_INFO;

typedef struct{
  u8 Status[NB_CNX_eNB];
  u8 Nb_ue;
  //unsigned short UE_index_list[NB_CNX_eNB];
  //L2_ID UE_list[NB_CNX_eNB];
  u8 UE_list[NB_CNX_eNB][5];
}__attribute__ ((__packed__)) eNB_RRC_INFO;

typedef struct{
  int Status;
union{
	UE_RRC_INFO UE_info;
	eNB_RRC_INFO CH_info;
 }Info;
}RRC_INFO;




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




typedef struct{
  eNB_RRC_INFO Info;
  SRB_INFO SI;
  SRB_INFO Srb0;
  SRB_INFO_TABLE_ENTRY Srb1[NB_CNX_eNB+1];
  SRB_INFO_TABLE_ENTRY Srb2[NB_CNX_eNB+1];
  u8 *SIB1;
  u8 sizeof_SIB1;
  u8 *SIB23;
  u8 sizeof_SIB23;
  SystemInformationBlockType1_t sib1;
  SystemInformation_t systemInformation;
  SystemInformationBlockType2_t *sib2;
  SystemInformationBlockType3_t *sib3;
  struct SRB_ToAddMod             *SRB1_config[NB_CNX_eNB];
  struct SRB_ToAddMod             *SRB2_config[NB_CNX_eNB];
  struct DRB_ToAddMod             *DRB_config[NB_CNX_eNB][8];
  u8                               DRB_active[NB_CNX_eNB][8];
  struct PhysicalConfigDedicated  *physicalConfigDedicated[NB_CNX_eNB];
  struct SPS_Config               *sps_Config[NB_CNX_eNB];
  MAC_MainConfig_t                *mac_MainConfig[NB_CNX_eNB];
}eNB_RRC_INST;


typedef struct{
  UE_RRC_INFO Info[NB_SIG_CNX_UE];
  SRB_INFO Srb0[NB_SIG_CNX_UE];
  SRB_INFO_TABLE_ENTRY Srb1[NB_CNX_UE];
  SRB_INFO_TABLE_ENTRY Srb2[NB_CNX_UE];
  u8 *SIB1[NB_CNX_UE];
  u8 sizeof_SIB1[NB_CNX_UE];
  u8 *SI[NB_CNX_UE];
  u8 sizeof_SI[NB_CNX_UE];
  u8 SIB1Status[NB_CNX_UE];
  u8 SIStatus[NB_CNX_UE];
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
  struct SRB_ToAddMod             *SRB1_config[NB_CNX_UE];
  struct SRB_ToAddMod             *SRB2_config[NB_CNX_UE];
  struct DRB_ToAddMod             *DRB_config[NB_CNX_UE][8];
  struct PhysicalConfigDedicated  *physicalConfigDedicated[NB_CNX_UE];
  struct SPS_Config               *sps_Config[NB_CNX_UE];
  MAC_MainConfig_t                *mac_MainConfig[NB_CNX_UE];
  MeasGapConfig_t                 *measGapConfig[NB_CNX_UE];
}UE_RRC_INST;

//main.c
int rrc_init_global_param(void);
int L3_xface_init(void);
void openair_rrc_top_init(void);
char openair_rrc_eNB_init(u8 Mod_id);
char openair_rrc_ue_init(u8 Mod_id,u8 CH_IDX);
void rrc_config_buffer(SRB_INFO *srb_info, u8 Lchan_type, u8 Role);
void openair_rrc_on(u8 Mod_id);


// UE RRC Procedures

/** \brief Decodes DL-CCCH message and invokes appropriate routine to handle the message
    \param Mod_id Instance ID of UE
    \param Srb_info Pointer to SRB_INFO structure (SRB0)
    \param CH_index Index of corresponding eNB/CH*/
int rrc_ue_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info,u8 CH_index);

/** \brief Decodes a DL-DCCH message and invokes appropriate routine to handle the message
    \param Mod_id Instance ID of UE
    \param Srb_id Index of Srb (1,2)
    \param Buffer Pointer to received SDU
    \param CH_index Index of corresponding CH/eNB*/
void rrc_ue_decode_dcch(u8 Mod_id, u8 Srb_id, u8* Buffer,u8 CH_index);

/** \brief Generate/Encodes RRCConnnectionRequest message at UE 
    \param Mod_id Instance ID of UE
    \param Srb_id Index of Srb (1,2)
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, u8 CH_index);

/** \brief Generates/Encodes RRCConnnectionSetupComplete message at UE 
    \param Mod_id Instance ID of UE
    \param CH_index Index of corresponding eNB/CH*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id,u8 CH_index);

/** \brief Establish SRB1 based on configuration in SRB_ToAddMod structure.  Configures RLC/PDCP accordingly
    \param Mod_id Instance ID of UE
    \param CH_index Index of corresponding eNB/CH
    \param SRB_config Pointer to SRB_ToAddMod IE from configuration
    @returns 0 on success*/
s32  rrc_ue_establish_srb1(u8 Mod_id,u8 CH_index,struct SRB_ToAddMod *SRB_config);

/** \brief Establish SRB2 based on configuration in SRB_ToAddMod structure.  Configures RLC/PDCP accordingly
    \param Mod_id Instance ID of UE
    \param CH_index Index of corresponding eNB/CH
    \param SRB_config Pointer to SRB_ToAddMod IE from configuration
    @returns 0 on success*/
s32  rrc_ue_establish_srb2(u8 Mod_id,u8 CH_index,struct SRB_ToAddMod *SRB_config);

/** \brief Establish a DRB according to DRB_ToAddMod structure
    \param Mod_id Instance ID of UE
    \param CH_index Index of corresponding CH/eNB
    \param DRB_config Pointer to DRB_ToAddMod IE from configuration
    @returns 0 on success */
s32  rrc_ue_establish_drb(u8 Mod_id,u8 CH_index,struct DRB_ToAddMod *DRB_config);

/** \brief Process a measConfig Message and configure PHY/MAC
    \param Mod_id Instance of UE on which to act
    \param CH_index Index of corresponding CH/eNB
    \param  measConfig Pointer to MeasConfig  IE from configuration*/
void	rrc_ue_process_measConfig(u8 Mod_id,u8 eNB_index,MeasConfig_t *measConfig);

/** \brief Process a RadioResourceConfigDedicated Message and configure PHY/MAC
    \param Mod_id Instance of UE on which to act
    \param CH_index Index of corresponding CH/eNB
    \param radioResourceConfigDedicated Pointer to RadioResourceConfigDedicated IE from configuration*/
void rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u8 CH_index,
						 RadioResourceConfigDedicated_t *radioResourceConfigDedicated);

// eNB/CH RRC Procedures

/**\brief Entry routine to decode a UL-CCCH-Message.  Invokes PER decoder and parses message.
   \param Mod_id Instance ID for CH/eNB
   \param Srb_info Pointer to SRB0 information structure (buffer, etc.)*/
int rrc_eNB_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info);

/**\brief Entry routine to decode a UL-DCCH-Message.  Invokes PER decoder and parses message.
   \param Mod_id Instance ID for CH/eNB
   \param UE_index Index of UE sending the message
   \param Rx_sdu Pointer Received Message
   \param sdu_size Size of incoming SDU*/
int rrc_eNB_decode_dcch(u8 Mod_id, u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size);  

/**\brief Generate the RRCConnectionSetup based on information coming from RRM
   \param Mod_id Instance ID for eNB/CH
   \param UE_index Index of UE receiving the message*/
void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u16 UE_index);

/**\brief Process the RRCConnectionSetupComplete based on information coming from UE
   \param Mod_id Instance ID for eNB/CH
   \param UE_index Index of UE transmitting the message
   \param rrcConnectionSetupComplete Pointer to RRCConnectionSetupComplete message*/
void rrc_eNB_process_RRCConnectionSetupComplete(u8 Mod_id, u8 UE_index, RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete);

/**\brief Process the RRCConnectionReconfigurationComplete based on information coming from UE
   \param Mod_id Instance ID for eNB/CH
   \param UE_index Index of UE transmitting the messages
   \param rrcConnectionReconfigurationComplete Pointer to RRCConnectionReconfigurationComplete message*/
void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete);


//L2_interface.c
u8 mac_rrc_lite_data_req( u8 Mod_id, unsigned short Srb_id, u8 Nb_tb,char *Buffer,u8 eNB_flag,u8 eNB_index);
u8 mac_rrc_lite_data_ind( u8 Mod_id,  unsigned short Srb_id, char *Sdu, unsigned short Sdu_len,u8 eNB_flag,u8 Mui);
void mac_sync_ind( u8 Mod_id, u8 status);
void rlcrrc_lite_data_ind( u8 Mod_id, u32 Rb_id, u32 sdu_size,u8 *Buffer);
void rrc_lite_out_of_sync_ind(u8 Mod_id, unsigned short eNB_index);

//MESSAGES/asn1_msg.c
/** 
\brief Generate a default configuration for SIB1 (eNB).
@param buffer Pointer to PER-encoded ASN.1 description of SIB1
@param sib1 Pointer to asn1c C representation of SIB1
@return size of encoded bit stream in bytes*/
uint8_t do_SIB1(uint8_t *buffer,
		SystemInformationBlockType1_t *sib1);
/** 
\brief Generate a default configuration for SIB2/SIB3 in one System Information PDU (eNB).
@param buffer Pointer to PER-encoded ASN.1 description of SI PDU
@param systemInformation Pointer to asn1c C representation of SI PDU
@param sib2 Pointer (returned) to sib2 component withing SI PDU
@param sib3 Pointer (returned) to sib3 component withing SI PDU
@return size of encoded bit stream in bytes*/
uint8_t do_SIB23(uint8_t *buffer,  
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3);

/** 
\brief Generate an RRCConnectionRequest UL-CCCH-Message (UE) based on random string or S-TMSI.  This 
routine only generates an mo-data establishment cause.
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@param rv 5 byte random string or S-TMSI
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionRequest(uint8_t *buffer,u8 *rv);

/** \brief Generate an RRCConnectionSetupComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer);

/** \brief Generate an RRCConnectionReconfigurationComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionReconfigurationComplete(uint8_t *buffer);

/** 
\brief Generate an RRCConnectionSetup DL-CCCH-Message (eNB).  This routine configures SRB_ToAddMod (SRB1/SRB2) and 
PhysicalConfigDedicated IEs.  The latter does not enable periodic CQI reporting (PUCCH format 2/2a/2b) or SRS.
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param SRB1_config Pointer (returned) to SRB1_config IE for this UE
@param physicalConfigDedicated Pointer (returned) to PhysicalConfigDedicated IE for this UE
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB1_config,
			      struct SRB_ToAddMod **SRB2_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);

/** 
\brief Generate an RRCConnectionReconfiguration DL-DCCH-Message (eNB).  This routine configures SRBToAddMod (SRB2) and one DRBToAddMod 
(DRB3).  PhysicalConfigDedicated is not updated.
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param SRB2_config Pointer (returned) to SRB_ToAddMod IE for this UE
@param DRB_config Pointer (returned) to DRB_ToAddMod IE for this UE
@param physicalConfigDedicated Pointer (returned void) to PhysicalConfigDedicated IE for this UE
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionReconfiguration(uint8_t *buffer,
					uint8_t UE_id,
					uint8_t Transaction_id,
					struct SRB_ToAddMod **SRB2_config,
					struct DRB_ToAddMod **DRB_config,
					struct PhysicalConfigDedicated  **physicalConfigDedicated);

int decode_SIB1(u8 Mod_id,u8 CH_index);

int decode_SI(u8 Mod_id,u8 CH_index,u8 si_window);

int get_rrc_status(u8 Mod_id,u8 eNB_flag,u8 eNB_index);

#endif


/** @ */
