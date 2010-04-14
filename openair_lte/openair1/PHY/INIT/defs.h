#ifndef __INIT_DEFS__H__
#define __INIT_DEFS__H__
/** @addtogroup _PHY_STRUCTURES_
 @{
*/

#include "PHY/defs.h"

#ifndef OPENAIR_LTE
/**
\fn int phy_init(unsigned char nb_antennas_tx)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param nb_antennas_tx Number of TX antennas
*/
int phy_init(unsigned char nb_antennas_tx);
#endif

#ifdef OPENAIR_LTE
/*
\fn int phy_init_top(unsigned char nb_antennas_tx)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param nb_antennas_tx Number of TX antennas
*/
int phy_init_top(unsigned char nb_antennas_tx);


/*
  \fn int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,LTE_UE_COMMON *ue_common_vars, LTE_UE_DLSCH **ue_dlsch_vars,LTE_UE_DLSCH **ue_dlsch_vars_cntl,LTE_UE_PBCH **ue_pbch_vars,LTE_UE_PDCCH **ue_pdcch_vars)
\brief Allocate and Initialize the PHY variables relevant to the LTE implementation
@param frame_parms pointer to LTE parameter structure
@param ue_common_vars pointer to structure to be initialized
@param ue_dlsch_vars pointer to structure to be initialized
@param ue_dlsch_vars_cntl pointer to structure to be initialized
@param ue_dlsch_vars_ra pointer to structure to be initialized
@param ue_dlsch_vars_ra pointer to structure to be initialized
@param ue_pbch_vars_cntl pointer to structure to be initialized
@param ue_pdcch_vars_cntl pointer to structure to be initialized
*/

int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *lte_ue_common_vars,
		    LTE_UE_DLSCH **lte_ue_dlsch_vars,
		    LTE_UE_DLSCH **lte_ue_dlsch_vars_cntl,
		    LTE_UE_DLSCH **lte_ue_dlsch_vars_ra,
		    LTE_UE_DLSCH **lte_ue_dlsch_vars_1A,
		    LTE_UE_PBCH **lte_ue_pbch_vars,
		    LTE_UE_PDCCH **ue_pdcch_vars);

int phy_init_lte_eNB(LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNB_COMMON *eNB_common,
		     LTE_eNB_ULSCH **eNB_ulsch);


void copy_lte_parms_to_phy_framing(LTE_DL_FRAME_PARMS *frame_parm, PHY_FRAMING *phy_framing);

#endif


/*!\fn void phy_cleanup(void)
\brief Cleanup the PHY variables*/ 
void phy_cleanup(void);

#ifdef OPENAIR_LTE
int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms);
#endif

/** @} */
#endif

