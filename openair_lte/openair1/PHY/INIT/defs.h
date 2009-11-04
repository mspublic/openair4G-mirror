#ifndef __INIT_DEFS__H__
#define __INIT_DEFS__H__
/** @addtogroup _PHY_STRUCTURES_
 @{
*/

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
  \fn int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
  LTE_UE_COMMON *lte_ue_common_var, LTE_UE_DLSCH *lte_ue_dlsch_var)
\brief Allocate and Initialize the PHY variables relevant to the LTE implementation
@param frame_parms pointer to LTE parameter structure
@param lte_ue_common_var pointer to structure to be initialized
@param lte_ue_dlsch_var pointer to structure to be initialized
*/

int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *lte_ue_common_vars,
		    LTE_UE_DLSCH *lte_ue_dlsch_vars);

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

