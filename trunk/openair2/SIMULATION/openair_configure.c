/*! 
 *  \brief Reconfigure the MAC/PHY parameters according to the scenario file from "scenario.scn"
 *  \author Navid Nikaein
 *  \version 1.0
 *  \date    1st of March 2005
 *  \bug nothing known to date!
 *  \warning The config file has to respect the order in which they defined in openair_readconfigfile
  */
 
 
#include "COMMON/openair_types.h"
#include "config_defs.h"
#include "config_extern.h"
#include "phy_extern.h"
//#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/extern.h"
//#include "PHY_INTERFACE/extern.h"


#define TRACE 1

void 
dump_config() {

  int i,n;
  FILE *fid;

  msg("[openair][CONFIG] Dumping Config\n\n");
  msg("[openair][CONFIG] PHY_FRAMING\n");
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] fc = %d kHz\n",PHY_config->PHY_framing.fc_khz);
  msg("[openair][CONFIG] fs = %d kHz\n",PHY_config->PHY_framing.fs_khz);
  msg("[openair][CONFIG] Nsymb= %d\n",PHY_config->PHY_framing.Nsymb);
  msg("[openair][CONFIG] Nd= %d\n",PHY_config->PHY_framing.Nd);
  msg("[openair][CONFIG] log2Nd= %d\n",PHY_config->PHY_framing.log2Nd);
  msg("[openair][CONFIG] Nc= %d\n",PHY_config->PHY_framing.Nc);
  msg("[openair][CONFIG] Nz= %d\n",PHY_config->PHY_framing.Nz);
  msg("[openair][CONFIG] Nf= %d\n",PHY_config->PHY_framing.Nf);
  switch (PHY_config->PHY_framing.Extension_type) {
    
  case CYCLIC_SUFFIX:
    msg("[openair][CONFIG] Extension= CYCLIC_SUFFIX \n");
    break;
  case CYCLIC_PREFIX:
    msg("[op`enair][CONFIG] Extension= CYCLIC_PREFIX \n");
    break;
  case ZEROS:
    msg("[openair][CONFIG] Extension= ZEROS\n");
    break;
  default:
    msg("[openair][CONFIG] Extension= CYCLIC_SUFFIX\n");
    break;
  }	

	for (n=0;n<PHY_config->total_no_chsch;n++)
	{
  msg("[openair][CONFIG]\n");
  msg("[openair][CONFIG] PHY_CHSCH %d\n",n);
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] symbol = %d\n",PHY_config->PHY_chsch[n].symbol);
  msg("[openair][CONFIG] Nsymb= %d\n",PHY_config->PHY_chsch[n].Nsymb);
  msg("[openair][CONFIG] dd_offset= %d\n",PHY_config->PHY_chsch[n].dd_offset);
  for (i=0;i<32;i++) {
    msg("[openair][CONFIG] chsch_seq_re[%d]= %x\n",i,PHY_config->PHY_chsch[n].chsch_seq_re[i]);
    msg("[openair][CONFIG] chsch_seq_im[%d]= %x\n",i,PHY_config->PHY_chsch[n].chsch_seq_im[i]);
  }
  msg("[openair][CONFIG] CHSCH_POWER= %d dBm\n",PHY_config->PHY_chsch[n].CHSCH_POWER_dBm);
	}
	
	for (n=0;n<PHY_config->total_no_sch;n++)
	{
	msg("[openair][CONFIG]\n");
  msg("[openair][CONFIG] PHY_SCH %d\n",n);
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] Nsymb= %d\n",PHY_config->PHY_sch[n].Nsymb);
  msg("[openair][CONFIG] dd_offset= %d\n",PHY_config->PHY_sch[n].dd_offset);
  for (i=0;i<32;i++) {
    msg("[openair][CONFIG] sch_seq_re[%d]= %x\n",i,PHY_config->PHY_sch[n].sch_seq_re[i]);
    msg("[openair][CONFIG] sch_seq_im[%d]= %x\n",i,PHY_config->PHY_sch[n].sch_seq_im[i]);
  }
  msg("[openair][CONFIG] SCH_POWER= %d dBm\n",PHY_config->PHY_sch[n].SCH_POWER_dBm);
	}
	
	msg("[openair][CONFIG]\n");
  msg("[openair][CONFIG] PHY_CHBCH\n");
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] symbol = %d\n",PHY_config->PHY_chbch.symbol);
  msg("[openair][CONFIG] Nsymb= %d\n",PHY_config->PHY_chbch.Nsymb);
  msg("[openair][CONFIG] IntDepth= %d\n",PHY_config->PHY_chbch.IntDepth);
  msg("[openair][CONFIG] dd_offset= %d\n",PHY_config->PHY_chbch.dd_offset);
  msg("[openair][CONFIG] Npilot= %d\n",PHY_config->PHY_chbch.Npilot);
  for (i=0;i<8;i++) {
    msg("[openair][CONFIG] pilot_re[%d]= %x\n",i,PHY_config->PHY_chbch.pilot_re[i]);
    msg("[openair][CONFIG] pilot_im[%d]= %x\n",i,PHY_config->PHY_chbch.pilot_im[i]);
  }
  msg("[openair][CONFIG] FreqReuse= %d\n",PHY_config->PHY_chbch.FreqReuse);
  msg("[openair][CONFIG] FreqReuse_ind= %d\n",PHY_config->PHY_chbch.FreqReuse_ind);
  msg("[openair][CONFIG] CHBCH_POWER= %d dBm\n",PHY_config->PHY_chbch.CHBCH_POWER_dBm);

  msg("[openair][CONFIG]\n");
  msg("[openair][CONFIG] PHY_SACH\n");
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] Npilot= %d\n",PHY_config->PHY_sach.Npilot);
	msg("[openair][CONFIG]\n");
  msg("[openair][CONFIG] PHY_MRBCH\n");
  msg("[openair][CONFIG] -----------\n");
  msg("[openair][CONFIG] symbol = %d\n",PHY_config->PHY_mrbch.symbol);
  msg("[openair][CONFIG] Nsymb= %d\n",PHY_config->PHY_mrbch.Nsymb);
  msg("[openair][CONFIG] IntDepth= %d\n",PHY_config->PHY_mrbch.IntDepth);
  msg("[openair][CONFIG] dd_offset= %d\n",PHY_config->PHY_mrbch.dd_offset);
  msg("[openair][CONFIG] Npilot= %d\n",PHY_config->PHY_mrbch.Npilot);
  for (i=0;i<8;i++) {
    msg("[openair][CONFIG] pilot_re[%d]= %x\n",i,PHY_config->PHY_mrbch.pilot_re[i]);
    msg("[openair][CONFIG] pilot_im[%d]= %x\n",i,PHY_config->PHY_mrbch.pilot_im[i]);
  }
  msg("[openair][CONFIG] FreqReuse= %d\n",PHY_config->PHY_mrbch.FreqReuse);
  msg("[openair][CONFIG] FreqReuse_ind= %d\n",PHY_config->PHY_mrbch.FreqReuse_ind);
  msg("[openair][CONFIG] MRBCH_POWER= %d dBm\n",PHY_config->PHY_mrbch.MRBCH_POWER_dBm);

  msg("[openair][CONFIG]\n");

}


int
reconfigure_MACPHY(FILE* scenario)
 {
 
 FILE *inscenario=scenario;
 int cfgNumber, ActionIndex;
 char LineBuffer[MAX_LINE_SIZE];
 char ActionName[MAX_ACTION_NAME_SIZE];
 
 /*!< \brief Reading or opening the scenario*/
 

 loadConfig();
 
 while ( fgets(LineBuffer, MAX_LINE_SIZE, inscenario) != NULL )
   {
     printf("[CONFIG] : %s\n",LineBuffer);
     if ( sscanf(LineBuffer, "%s %d", ActionName, &cfgNumber) != 2)
       {
	 msg("[CONFIG] Parse Error: \"%s\" \n", LineBuffer);	
	 continue;
       }
     for (ActionIndex=0; ActionIndex <6;ActionIndex++) //sizeof(Action)/sizeof(cfg_Action); ActionIndex++)
       {

	 if (strcmp(ActionName, Action[ActionIndex].ActionName) == 0)
	   {
	     Action[ActionIndex].Func(cfgNumber);
	     break;
	   }
       }
     if (ActionIndex == 6)//(sizeof(Action)/sizeof(cfg_Action) == ActionIndex)
       msg("[CONFIG] Unknown <%s> Action! \n", ActionName);
   }
 
 fclose(inscenario);
 
 return 1;
 }


int 
phyFraming_ProcessInitReq (int cfgNumber)
 {
	 PHY_FRAMING *phyframing_processinitreq;
	 phyframing_processinitreq = cfg_getPhyFraming(cfgNumber);

	 if(phyframing_processinitreq != NULL)
	 {
	   memcpy((void *)&PHY_config->PHY_framing,phyframing_processinitreq,sizeof(PHY_FRAMING));
	 #ifdef TRACE
           msg("[CONFIG] PHY_FRAMING configuration completed!\n");
	 #endif TRACE
	 }
	 else
           msg("[CONFIG FRAMING] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 }
 
 
 
int 
phyCHBCH_ProcessInitReq (int cfgNumber)
 {
	 PHY_CHBCH *phychbch_processinitreq;
	 
	 phychbch_processinitreq = cfg_getPhyCHBCH(cfgNumber);
	 if (phychbch_processinitreq != NULL)
	 { 
	   memcpy((void *)&PHY_config->PHY_chbch,phychbch_processinitreq,sizeof(PHY_CHBCH));
	#ifdef TRACE			 
	   msg("[CONFIG] PHY_CHBCH configuration completed!\n");
	#endif TRACE   
	 }
	 else
	    msg("[CONFIG CHBCH ] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 }

int 
phyMRBCH_ProcessInitReq (int cfgNumber)
 {
	 PHY_MRBCH *phymrbch_processinitreq;
	 
	 phymrbch_processinitreq = cfg_getPhyMRBCH(cfgNumber);
	 if (phymrbch_processinitreq != NULL)
	 { 
	   memcpy((void *)&PHY_config->PHY_mrbch,phymrbch_processinitreq,sizeof(PHY_MRBCH));
	#ifdef TRACE			 
	   msg("[CONFIG] PHY_MRBCH configuration completed!\n");
	#endif TRACE   
	 }
	 else
	    msg("[CONFIG MRBCH ] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 }


int 
phyCHSCH_ProcessInitReq (int cfgNumber)
{
	static int total_no_chsch = 0;
	 PHY_CHSCH *phychsch_processinitreq;
	 phychsch_processinitreq = cfg_getPhyCHSCH(cfgNumber);
	 if (phychsch_processinitreq != NULL)
	 { 
	   memcpy(&PHY_config->PHY_chsch[cfgNumber],phychsch_processinitreq,sizeof(PHY_CHSCH));
		 total_no_chsch++;
		 PHY_config->total_no_chsch = total_no_chsch;

#ifdef TRACE			 
	   printf("[CONFIG] PHY_CHSCH configuration completed!\n");
#endif TRACE   
	 }
	 else
	   msg("[CONFIG CHSCH] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 }

int 
phySCH_ProcessInitReq (int cfgNumber)
{
	static int total_no_sch = 0;
	PHY_SCH *physch_processinitreq;
	 physch_processinitreq = cfg_getPhySCH(cfgNumber);
	 if (physch_processinitreq != NULL)
	 { 
	   memcpy(&PHY_config->PHY_sch[cfgNumber],physch_processinitreq,sizeof(PHY_SCH));
		 total_no_sch++;
		 PHY_config->total_no_sch = total_no_sch;

#ifdef TRACE			 
	   printf("[CONFIG] PHY_SCH configuration completed!\n");
#endif TRACE   
	 }
	 else
	   msg("[CONFIG SCH] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 }

int 
phySACH_ProcessInitReq (int cfgNumber)
{
	 PHY_SACH *physach_processinitreq;
	 physach_processinitreq = cfg_getPhySACH(cfgNumber);
	 if (physach_processinitreq != NULL)
	 { 
	   memcpy((void *)&PHY_config->PHY_sach,physach_processinitreq,sizeof(PHY_SACH));

#ifdef TRACE			 
	   msg("[CONFIG] PHY_SACH configuration completed!\n");
#endif TRACE   
	 }
	 else
		 msg("[CONFIG SACH] The config number <%d> is not loaded.\n", cfgNumber);
	 return cfgNumber;
 } 
 
 

/* 
int
main(int argc, char* argv[])
{
 FILE *inscenario;

 if (( inscenario=fopen("scenario.scn","r")) == NULL) 
 {
   if (argc==1 || (inscenario=fopen(argv[1],"r")) == NULL) 
	{
		printf("The scenario file <scenario.scn> could not be found!\n");
		printf("Usage : %s scenario.scn \n", argv[0]);
		exit(1);
	}		
 }
	
reconfigure_MACPHY(inscenario);

}
*/ 
