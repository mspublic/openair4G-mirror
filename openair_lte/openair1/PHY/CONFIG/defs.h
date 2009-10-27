#ifndef __CONFIG_DEFS_H__
#define __CONFIG_DEFS_H__

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#include "PHY/types.h"

/** @addtogroup _PHY_CONFIG_BLOCKS_
 * @{
 */
#define MAX_ACTION_NAME_SIZE 100
#define MAX_SECTION_NAME_SIZE 100
#define MAX_LINE_SIZE 200
#define MAX_CFG_SECTIONS 10
 

typedef struct
{
	char ActionName[MAX_ACTION_NAME_SIZE];
	int	(*Func) (int);
} cfg_Action;


typedef struct
{
	char SectionName[MAX_SECTION_NAME_SIZE];
	void	(*Func) (FILE*, int);
} cfg_Section;



/*! \fn int reconfigure_MACPHY(FILE* infile);
 *  \brief reconfigure MAC and PHY layer according to a given scenario file
 *  \param infile scenario.scn
 *  \exception
 *  \return 1 if the scenario and config files are readable.
 */
int reconfigure_MACPHY(FILE *);

/*! \fn void loadConfig ()
 *  \brief load the config.cfg file from the current directory. The list of sections are formated as follows:
 */
void loadConfig();

/*! \fn int phyFraming_ProcessInitReq (int cfgNumber)
 *  \brief get the PHY_FRAMING config
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the cfgNumber
 */
int phyFraming_ProcessInitReq(int cfgNumber);

/*! \fn int cfg_readPhyFraming (FILE *inconfig, int cfgNumber)
 *  \param inconfig an input config file
 *  \param cfgNumber Configuration Number
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyFraming(FILE *inconfig, int cfgNumber);

#ifndef OPENAIR_LTE
 int phyCHSCH_ProcessInitReq(int);

 /*! \fn void cfg_readPhyCHSCH (FILE *inconfig, int cfgNumber)
 *  \brief Read the PHY_CHSCH section: 
 *  \param inconfig an input config file
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyCHSCH(FILE*, int);

int phySCH_ProcessInitReq(int);
void cfg_readPhySCH(FILE*, int);

/*! \fn int phyCHBCH_ProcessInitReq (int cfgNumber)
 *  \brief get the PHY_CHBCH config
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the cfgNumber
 */
int phyCHBCH_ProcessInitReq(int cfgNumber);

 /*! \fn int cfg_readPhyCHBCH (FILE *inconfig, int cfgNumber)
 *  \brief Read the PHY_CHBCH section: 
 *  \param inconfig Pointer to Config file
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyCHBCH(FILE*, int);

int phyMRBCH_ProcessInitReq(int);

 /*! \fn void cfg_readPhyMRBCH (FILE *inconfig, int cfgNumber)
 *  \brief Read the PHY_MRBCH section: 
 *  \param inconfig an input config file 
 *  \param cfgNumber in integer
 */
void cfg_readPhyMRBCH(FILE *inconfig, int cfgNumber);

int phySACH_ProcessInitReq(int);

 /*! \fn int cfg_readPhySACH(FILE *inconfig, int cfgNumber)
 *  \brief Read the PHY_SACH section: 
 *  \param inconfig an input config file
 *  \param cfgNumber in integer
 */
void cfg_readPhySACH(FILE *inconfig, int cfgNumber);

/*! \fn PHY_FRAMING *cfg_getPhyFraming (int cfgNumber)
 *  \brief Get the PHY_FRAMING section
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the PHY_FRAMING structure
 */
PHY_FRAMING* cfg_getPhyFraming(int cfgNumber);

/*! \fn Framing *cfg_getPhyMRBCH (int cfgNumber)
 *  \brief Get the PHY_MRBCH section
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the PHY_MRBCH structure
 */
PHY_MRBCH* cfg_getPhyMRBCH(int cfgNumber);

/*! \fn Framing *cfg_getPhyCHSCH (int cfgNumber)
 *  \brief Get the PHY_CHSCH section
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the sscanf status
 */
PHY_CHSCH* cfg_getPhyCHSCH(int cfgNumber);

/*! \fn Framing *cfg_getPhySCH (int cfgNumber)
 *  \brief Get the PHY_SCH section
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the PHY_SCH structure
 */
PHY_SCH* cfg_getPhySCH(int cfgNumber);

/*! \fn Framing *cfg_getPhySACH (int cfgNumber)
 *  \brief Get the PHY_SACH section
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the PHY_SACH structure
 */
PHY_SACH* cfg_getPhySACH(int cfgNumber);

/** @}*/
#endif //OPENAIR_LTE
#endif /*__CONFIG_DEFS_H__*/












