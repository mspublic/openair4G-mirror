

#ifndef __CONFIG_PROTO_H__
#define __CONFIG_PROTO_H__

#include "SIMULATION/PHY_EMULATION/spec_defs.h"
//#include "openair_types.h"

/*! \fn int reconfigure_MACPHY(FILE*);
 *  \brief reconfigure MAC and PHY layer according to a given scenario file
 *		[<section name> <number>]
 *			# <comment>
 *			<field>: <value>
 *  \var cfgNumber in integer
 *	\var ActionName, LineBuffer, and LinePattern in character
 *  \param FILE scenario.scn
 *  \exception
 *  \return 1 if the scenario and config files are readable.
 */

int reconfigure_MACPHY(FILE *);

/*! \fn void loadConfig ()
 *  \brief load the config.txt file from the current directior. The list of sections are formated as follows:
 *		[<section name> <number>]
 *			# <comment>
 *			<field>: <value>
 *  \var inconfig in FILE
 *  \var SectionIndex in integer
 *	\var SectionName, LineBuffer, and LinePattern in character 
 *  \param void
 *  \exception 
 *  \return a character pointer.
 */
void loadConfig();

/*! \fn int phyFraming_ProcessInitReq (int cfgNumber)
 *  \brief get the PHY_FRAMING config
 *  \var processInitReq in pointer
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the cfgNumber
 */
 
int phyFraming_ProcessInitReq(int);

/*! \fn int cfg_readPhyFraming (inconfig, cfgNumber)
 *  \brief Read the PHY_FRAMING section: 
 *  \var LineBuffer in character
 *	\var cfumber in integer
 *  \param inconfig an input config file
 *  \exception 
 *  \return the sscanf status
 */
 
void cfg_readPhyFraming(FILE*, int);


 int phyCHSCH_ProcessInitReq(int);

 /*! \fn int cfg_readPhyCHSCH (inconfig, cfgNumber)
 *  \brief Read the PHY_CHSCH section: 
 *  \var LineBuffer in character
 *	\var cfgNumber in integer
 *  \param inconfig an input config file
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyCHSCH(FILE*, int);

int phySCH_ProcessInitReq(int);
void cfg_readPhySCH(FILE*, int);

/*! \fn int phySACH_ProcessInitReq (int cfgNumber)
 *  \brief get the PHY_MCH config
 *	\var 
 *  \param cfgNumber in integer
 *  \exception 
 *  \return the cfgNumber
 */
int phyCHBCH_ProcessInitReq(int);

 /*! \fn int cfg_readPhyCHBCH (inconfig, cfgNumber)
 *  \brief Read the PHY_CHBCH section: 
 *  \var LineBuffer in character
 *	\var cfgNumber in integer
 *  \param inconfig an input config file
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyCHBCH(FILE*, int);

int phyMRBCH_ProcessInitReq(int);

 /*! \fn int cfg_readPhyMRBCH (inconfig, cfgNumber)
 *  \brief Read the PHY_MRBCH section: 
 *  \var LineBuffer in character
 *	\var cfgNumber in integer
 *  \param inconfig an input config file
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhyMRBCH(FILE*, int);

int phySACH_ProcessInitReq(int);

 /*! \fn int cfg_readPhySACH(inconfig, cfgNumber)
 *  \brief Read the PHY_MCH section: 
 *  \var LineBuffer in character
 *	\var cfgNumber in integer
 *  \param inconfig an input config file
 *  \exception 
 *  \return the sscanf status
 */
void cfg_readPhySACH(FILE*, int);

/*! \fn Framing *cfg_getPhyFraming (int cfgNumber)
 *  \brief Get the PHY_FRAMING section
 *  \param cfgNumber in interger
 *  \exception 
 *  \return the sscanf status
 */
 
PHY_FRAMING* cfg_getPhyFraming(int cfgNumber);

/*! \fn Framing *cfg_getPhyMCH (int cfgNumber)
 *  \brief Get the PHY_MCH section
 *  \param cfgNumber in interger
 *  \exception 
 *  \return the sscanf status
 */

PHY_MRBCH* cfg_getPhyMRBCH(int cfgNumber);

PHY_CHSCH* cfg_getPhyCHSCH(int cfgNumber);

PHY_SCH* cfg_getPhySCH(int cfgNumber);

PHY_SACH* cfg_getPhySACH(int cfgNumber);

void config_topology(FILE*);

#endif /*__CONFIG_PORTO_H__*/ 
