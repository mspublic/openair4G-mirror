#ifndef __INIT_DEFS__H__
#define __INIT_DEFS__H__

#include "PHY/defs.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "TDD-Config.h"

#ifndef OPENAIR_LTE
/**
\fn int phy_init(unsigned char nb_antennas_tx)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param nb_antennas_tx Number of TX antennas
*/
int phy_init(unsigned char nb_antennas_tx);
#endif

#ifdef OPENAIR_LTE

/** @addtogroup _PHY_STRUCTURES_
 * @{
 */

/*!
\fn int phy_init_top(LTE_DL_FRAME_PARMS *frame_parms)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param frame_parms Pointer to LTE_DL_FRAME_PARMS (common configuration)
@returns 0 on success
*/
int phy_init_top(LTE_DL_FRAME_PARMS *frame_parms);


/*!
\brief Allocate and Initialize the PHY variables relevant to the LTE implementation
@param phy_vars_ue Pointer to UE Variables
@param abstraction_flag 1 indicates memory should be allocated for abstracted MODEM
@returns 0 on success
*/
int phy_init_lte_ue(PHY_VARS_UE *phy_vars_ue,
		    u8 abstraction_flag);

/*!
\brief Allocate and Initialize the PHY variables relevant to the LTE implementation (eNB)
@param phy_vars_eNb Pointer to eNB Variables
@param is_secondary_eNb Flag to indicate this eNB gets synch from another
@param cooperation_flag
@param abstraction_flag 1 indicates memory should be allocated for abstracted MODEM
@returns 0 on success
*/
int phy_init_lte_eNB(PHY_VARS_eNB *phy_vars_eNb,
		     unsigned char is_secondary_eNb,
		     unsigned char cooperation_flag,
		     unsigned char abstraction_flag);

/** \brief Configure LTE_DL_FRAME_PARMS with components derived after initial synchronization (MIB decoding + primary/secondary synch).  The basically allows configuration of \f$N_{\mathrm{RB}}^{\mathrm{DL}}\f$, the cell id  \f$N_{\mathrm{ID}}^{\mathrm{cell}}\f$, the normal/extended prefix mode, the frame type (FDD/TDD), \f$N_{\mathrm{cp}}\f$, the number of TX antennas at eNB (\f$p\f$) and the number of PHICH groups, \f$N_{\mathrm{group}}^{\mathrm{PHICH}}\f$ 
@param lte_frame_parms pointer to LTE parameter structure
@param N_RB_DL Number of DL resource blocks
@param Nid_cell Cell ID
@param Ncp Normal/Extended Prefix flag
@param frame_type FDD/TDD framing
@param p_eNB Number of eNB TX antennas
@param phich_config Pointer to PHICH_CONFIG_COMMON
*/
void phy_config_mib(LTE_DL_FRAME_PARMS *lte_frame_parms,
		    u8 N_RB_DL,
		    u8 Nid_cell,
		    u8 Ncp,
		    u8 frame_type,
		    u8 p_eNB,
		    PHICH_CONFIG_COMMON *phich_config);


/** \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB1.  From a PHY perspective this allows configuration of TDD framing parameters and SI reception.
@param Mod_id Instance ID of eNB
@param tdd_Config TDD UL/DL and S-subframe configurations
@param SIwindowsize Size of a SI window in frames where repetitions of a unique System Information message block is repeated 
@param SIperiod Periodicity of System Information Messages (in multiples of a frame)*/
void phy_config_sib1_eNB(u8 Mod_id,
			 u8 CC_id,
			 TDD_Config_t *tdd_Config,
			 u8 SIwindowsize,
			 u16 SIperiod);

/** \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB1.  From a PHY perspective this allows configuration of TDD framing parameters and SI reception.
@param Mod_id Instance ID of UE
@param CH_index Index of eNB for this configuration
@param tdd_Config TDD UL/DL and S-subframe configurations
@param SIwindowsize Size of a SI window in frames where repetitions of a unique System Information message block is repeated 
@param SIperiod Periodicity of System Information Messages (in multiples of a frame)*/
void phy_config_sib1_ue(u8 Mod_id, u8 CC_id, u8 CH_index,
			TDD_Config_t *tdd_Config,
			u8 SIwindowsize,
			u16 SIperiod);


/*!
  \fn void phy_config_sib2_ue(u8 Mod_id,u8 CH_index,
			RadioResourceConfigCommonSIB_t *radioResourceConfigCommon)
  \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB2 (at UE).  
  @param Mod_id Instance id
  @param CH_index Index of CH to which UE is connected
  @param radioResourceConfigCommon Radio Configuration from SIB2
*/
void phy_config_sib2_ue(u8 Mod_id, u8 CC_id, u8 CH_index,
			RadioResourceConfigCommonSIB_t *radioResourceConfigCommon);

/*!
  \fn void phy_config_sib2_eNB(u8 Mod_id,
			RadioResourceConfigCommonSIB_t *radioResourceConfigCommon)
  \brief Configure LTE_DL_FRAME_PARMS with components of SIB2 (at eNB).  
  @param Mod_id Instance id
  @param radioResourceConfigCommon Radio Configuration from SIB2
*/
void phy_config_sib2_eNB(u8 Mod_id,
			 u8 CC_id,
			 RadioResourceConfigCommonSIB_t *radioResourceConfigCommon);


/*!
\fn void phy_config_dedicated_ue(u8 Mod_id,u8 CH_index,
 			         struct PhysicalConfigDedicated *physicalConfigDedicated)
\brief Configure UE dedicated parameters. Invoked upon reception of RRCConnectionSetup or RRCConnectionReconfiguration from eNB.
@param Mod_id Instance ID for eNB
@param CC_id Compinent carrier ID
@param CH_index Index of eNB for this configuration
@param physicalConfigDedicated PHY Configuration information

*/
void phy_config_dedicated_ue(u8 Mod_id,u8 CC_id, u8 CH_index,
			    struct PhysicalConfigDedicated *physicalConfigDedicated);

/*!
\fn void phy_config_dedicated_eNB(u8 Mod_id,u16 rnti,
                                  struct PhysicalConfigDedicated *physicalConfigDedicated)
\brief Prepare for configuration of PHY with dedicated parameters. Invoked just prior to transmission of RRCConnectionSetup or RRCConnectionReconfiguration at eNB.
@param Mod_id Instance ID for eNB
@param rnti rnti for UE context
@param physicalConfigDedicated PHY Configuration information
*/
void phy_config_dedicated_eNB(u8 Mod_id,u8 CC_id, u16 rnti,
			      struct PhysicalConfigDedicated *physicalConfigDedicated);

void phy_config_dedicated_scell_eNB(u8 Mod_id,u16 rnti,
			      struct PhysicalConfigDedicatedSCell_r10 *physicalConfigDedicatedSCell_r10, u8 CC_id);

void phy_config_dedicated_scell_ue(u8 Mod_id,u16 rnti,
			      struct PhysicalConfigDedicatedSCell_r10 *physicalConfigDedicatedSCell_r10, u8 CC_id);

/*!
\fn void phy_config_dedicated_eNB_step2(PHY_VARS_eNB *phy_vars_eNB)
\brief Configure PHY with dedicated parameters between configuration of DLSCH (n) and ULSCH (n+4) in current subframe (n).
@param phy_vars_eNB Pointer to PHY_VARS_eNB structure
*/
void phy_config_dedicated_eNB_step2(PHY_VARS_eNB *phy_vars_eNB);

/*
  \fn int phy_init_secsys_eNB(PHY_VARS_eNB *phy_vars_eNb)
\brief Allocate and Initialize the PHY variables relevant to the LTE implementation. 
@param phy_vars_eNb pointer to LTE parameter structure for the eNb
*/
int phy_init_secsys_eNB(PHY_VARS_eNB *phy_vars_eNb);


void phy_init_lte_top(LTE_DL_FRAME_PARMS *lte_frame_parms);

void copy_lte_parms_to_phy_framing(LTE_DL_FRAME_PARMS *frame_parm, PHY_FRAMING *phy_framing);

#endif


/*! !\fn void phy_cleanup(void)
\brief Cleanup the PHY variables*/ 
void phy_cleanup(void);

#ifdef OPENAIR_LTE
int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms,u8 osf);
void dump_frame_parms(LTE_DL_FRAME_PARMS *frame_parms);
#endif

/** @} */
#endif

