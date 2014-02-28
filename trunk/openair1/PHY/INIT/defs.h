#ifndef __INIT_DEFS__H__
#define __INIT_DEFS__H__

#include "PHY/defs.h"

#ifdef OPENAIR_LTE
#include "SystemInformationBlockType2.h"
//#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "TDD-Config.h"
#include "MBSFN-SubframeConfigList.h"
#include "MobilityControlInfo.h"
#else

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
@param nb_connected_eNB Number of eNB that UE can process in one PDSCH demodulation subframe
@param abstraction_flag 1 indicates memory should be allocated for abstracted MODEM
@returns 0 on success
 */
int phy_init_lte_ue(PHY_VARS_UE *phy_vars_ue,
                    int          nb_connected_eNB,
                    uint8_t         abstraction_flag);

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
                    uint8_t                N_RB_DL,
                    uint8_t                Nid_cell,
                    uint8_t                Ncp,
                    uint8_t                frame_type,
                    uint8_t                p_eNB,
                    PHICH_CONFIG_COMMON *phich_config);


/** \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB1.  From a PHY perspective this allows configuration of TDD framing parameters and SI reception.
@param Mod_id Instance ID of eNB
@param tdd_Config TDD UL/DL and S-subframe configurations
@param SIwindowsize Size of a SI window in frames where repetitions of a unique System Information message block is repeated 
@param SIperiod Periodicity of System Information Messages (in multiples of a frame)*/
void phy_config_sib1_eNB(module_id_t    Mod_id,
                         TDD_Config_t  *tdd_Config,
                         uint8_t           SIwindowsize,
                         uint16_t            SIperiod);

/** \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB1.  From a PHY perspective this allows configuration of TDD framing parameters and SI reception.
@param Mod_id Instance ID of UE
@param CH_index Index of eNB for this configuration
@param tdd_Config TDD UL/DL and S-subframe configurations
@param SIwindowsize Size of a SI window in frames where repetitions of a unique System Information message block is repeated 
@param SIperiod Periodicity of System Information Messages (in multiples of a frame)*/
void phy_config_sib1_ue(module_id_t   Mod_id,
                        uint8_t          CH_index,
                        TDD_Config_t *tdd_Config,
                        uint8_t          SIwindowsize,
                        uint16_t           SIperiod);


/*!
  \fn void phy_config_sib2_ue(module_id_t Mod_id,uint8_t CH_index,
			RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
			ARFCN_ValueEUTRA_t *ul_CArrierFreq,
		        long *ul_Bandwidth,
			AdditionalSpectrumEmission_t additionalSpectrumEmission,
			struct MBSFN_SubframeConfigList	*mbsfn_SubframeConfigList)
  \brief Configure LTE_DL_FRAME_PARMS with components derived after reception of SIB2 (at UE).  
  @param Mod_id Instance id
  @param CH_index Index of CH to which UE is connected
  @param radioResourceConfigCommon Radio Configuration from SIB2
  @param ul_CarrierFreq UL carrier ARFCN, null if optional (i.e. implicit from DL)
  @param ul_Bandwidth UL bandwidth, null if optional (i.e. same as DL)
  @param additionalSpectrumEmission UL parameter (see 36.101)
  @param mbsfn_SubframeConfigList MBSFN subframe configuration
 */
void phy_config_sib2_ue(module_id_t                      Mod_id,
                        uint8_t                             CH_index,
                        RadioResourceConfigCommonSIB_t  *radioResourceConfigCommon,
                        ARFCN_ValueEUTRA_t              *ul_CArrierFreq,
                        long                            *ul_Bandwidth,
                        AdditionalSpectrumEmission_t    *additionalSpectrumEmission,
                        struct MBSFN_SubframeConfigList *mbsfn_SubframeConfigList);


/*! 
  \fn phy_config_afterHO_ue
  \brief Configure Common PHY parameters from mobilityControlInfo
  @param Mod_id 
  @param eNB_index
  @param mobilityControlInfo pointer to the mobility control information for handover
  @param ho_failed flag to indicated whether the ho was successful or not
 */
void phy_config_afterHO_ue(module_id_t Mod_id,
                           uint8_t eNB_index,
                           MobilityControlInfo_t *mobilityControlInfo,
                           uint8_t ho_failed);
/*!
  \fn void phy_config_sib2_eNB(module_id_t Mod_id,
                               RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
			       ARFCN_ValueEUTRA_t *ul_CArrierFreq,
			       long *ul_Bandwidth,
			       AdditionalSpectrumEmission_t additionalSpectrumEmission,
			       struct MBSFN_SubframeConfigList	*mbsfn_SubframeConfigList)
  \brief Configure LTE_DL_FRAME_PARMS with components of SIB2 (at eNB).  
  @param Mod_id Instance id
  @param radioResourceConfigCommon Radio Configuration from SIB2
  @param ul_CarrierFreq UL carrier ARFCN, null if optional (i.e. implicit from DL)
  @param ul_Bandwidth UL bandwidth, null if optional (i.e. same as DL)
  @param additionalSpectrumEmission UL parameter (see 36.101)
  @param mbsfn_SubframeConfigList MBSFN subframe configuration
 */
void phy_config_sib2_eNB(module_id_t                             Mod_id,
                         RadioResourceConfigCommonSIB_t         *radioResourceConfigCommon,
                         ARFCN_ValueEUTRA_t                     *ul_CArrierFreq,
                         long                                   *ul_Bandwidth,
                         AdditionalSpectrumEmission_t           *additionalSpectrumEmission,
                         struct MBSFN_SubframeConfigList        *mbsfn_SubframeConfigList);


/*!
\fn void phy_config_dedicated_ue(module_id_t Mod_id,uint8_t CH_index,
 			         struct PhysicalConfigDedicated *physicalConfigDedicated)
\brief Configure UE dedicated parameters. Invoked upon reception of RRCConnectionSetup or RRCConnectionReconfiguration from eNB.
@param Mod_id Instance ID for eNB
@param CH_index Index of eNB for this configuration
@param physicalConfigDedicated PHY Configuration information

 */

/**
\brief Configure UE MBSFN common parameters. Invoked upon reception of SIB13 from eNB.
@param Mod_id Instance ID for UE
@param CH_index eNB id (for multiple eNB reception)
@param mbsfn_Area_idx Index of MBSFN-Area for which this command operates
@param mbsfn_AreaId_r9 MBSFN-Area Id
 */

void phy_config_sib13_ue(module_id_t Mod_id,
                         uint8_t CH_index,int mbsfn_Area_idx,
                         long mbsfn_AreaId_r9);

/**
\brief Configure eNB MBSFN common parameters. Invoked upon transmission of SIB13 from eNB.
@param Mod_id Instance ID for eNB
@param mbsfn_Area_idx Index of MBSFN-Area for which this command operates
@param mbsfn_AreaId_r9 MBSFN-Area Id
 */
void phy_config_sib13_eNB(module_id_t Mod_id,
                          int mbsfn_Area_idx,
                          long mbsfn_AreaId_r9);

/**
\brief Configure cba rnti for .
@param Mod_id Instance ID for eNB
@param eNB_flag flag indicating whether the nodeis eNB (1) or UE (0)
@param index index of the node 
@param cba_rnti rnti for the cba transmission
@param num_active_cba_groups num active cba group
 */
void  phy_config_cba_rnti (module_id_t Mod_id,eNB_flag_t eNB_flag, uint8_t index, rnti_t cba_rnti, uint8_t cba_group_id, uint8_t num_active_cba_groups);

void phy_config_dedicated_ue(module_id_t Mod_id,
                             uint8_t CH_index,
                             struct PhysicalConfigDedicated *physicalConfigDedicated);

/** \brief Configure RRC inter-cell measurements procedures
@param Mod_id Index of UE
@param eNB_index Index of corresponding eNB
@param n_adj_cells Number of adjacent cells on which to perform the measuremnts
@param adj_cell_id Array of cell ids of adjacent cells
 */
void phy_config_meas_ue(module_id_t Mod_id,
                        uint8_t eNB_index,
                        uint8_t n_adj_cells,
                        uint32_t *adj_cell_id);

/*!
\fn void phy_config_dedicated_eNB(module_id_t Mod_id,uint16_t rnti,
                                  struct PhysicalConfigDedicated *physicalConfigDedicated)
\brief Prepare for configuration of PHY with dedicated parameters. Invoked just prior to transmission of RRCConnectionSetup or RRCConnectionReconfiguration at eNB.
@param Mod_id Instance ID for eNB
@param rnti rnti for UE context
@param physicalConfigDedicated PHY Configuration information
 */
void phy_config_dedicated_eNB(module_id_t Mod_id,
                              rnti_t rnti,
                              struct PhysicalConfigDedicated *physicalConfigDedicated);

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

//void copy_lte_parms_to_phy_framing(LTE_DL_FRAME_PARMS *frame_parm, PHY_FRAMING *phy_framing);

#endif


/*! !\fn void phy_cleanup(void)
\brief Cleanup the PHY variables*/ 
void phy_cleanup(void);

#ifdef OPENAIR_LTE
int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms,uint8_t osf);
void dump_frame_parms(LTE_DL_FRAME_PARMS *frame_parms);
#endif

/** @} */
#endif

