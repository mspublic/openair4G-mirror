/** @addtogroup _PHY_STRUCTURES_
 @{

\fn int phy_init(unsigned char nb_antennas_tx)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param nb_antennas_tx Number of TX antennas
*/
#ifndef OPENAIR_LTE
int phy_init(unsigned char nb_antennas_tx);
#endif

/*
\fn int phy_init_top(unsigned char nb_antennas_tx)
\brief Allocate and Initialize the PHY variables after receiving static configuration
@param nb_antennas_tx Number of TX antennas
*/
#ifdef OPENAIR_LTE
int phy_init_top(unsigned char nb_antennas_tx);
#endif


/*!\fn void phy_cleanup(void)
\brief Cleanup the PHY variables*/ 
void phy_cleanup(void);

#ifdef OPENAIR_LTE
int init_frame_parms(LTE_DL_FRAME_PARMS *frame_parms);
#endif

/** @} */
