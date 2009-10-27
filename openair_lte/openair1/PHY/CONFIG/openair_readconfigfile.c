/*! 
 *  \brief Read each section of config file associated to the given scenario
 *  \author Navid Nikaein, modified R. Knopp
 *  \version 1.1
 *  \date    2005, modified April 2006
 *  \bug 
 *  \warning
 */

#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "extern.h"
#include "defs.h"

void 
loadConfig (void)
{
	int SectionIndex;
	int cfgNumber;
	char SectionName[MAX_SECTION_NAME_SIZE];
	char LineBuffer[MAX_LINE_SIZE];
	
	
	while ( (fgets (LineBuffer, MAX_LINE_SIZE, config)) != NULL)
	{
		if (LineBuffer[0] != '[') 
			continue;
		
		if (sscanf(LineBuffer, "[%s %d]", SectionName, &cfgNumber) != 2)	
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
		  continue;
	  }
		for (SectionIndex=0; SectionIndex < 8; SectionIndex++) //sizeof(Section)/sizeof(cfg_Section) ; SectionIndex++)
		{
			if ( strcmp(SectionName,Section[SectionIndex].SectionName) == 0)
			{
				Section[SectionIndex].Func(config, cfgNumber);
				break;
			}
		}
		if (SectionIndex == 8)//(sizeof(Section)/sizeof(cfg_Section) == SectionIndex)
			msg("[CONFIG] Unknown <%s> section!\n", SectionName);
	}
	msg("[CONFIG] loadConfig done.\n");
}


void 
cfg_readPhyFraming (FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	
	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[CONFIG] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf(LineBuffer, "fc_khz: %lu", &phyFraming[cfgNumber].fc_khz ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].fc_khz);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "fs_khz: %lu", &phyFraming[cfgNumber].fs_khz ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].fs_khz);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nsymb: %hu", &phyFraming[cfgNumber].Nsymb ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].Nsymb);
		
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nd: %hu", &phyFraming[cfgNumber].Nd ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].Nd);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "log2Nd: %hu", &phyFraming[cfgNumber].log2Nd ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  } 
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].log2Nd);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nc: %hu", &phyFraming[cfgNumber].Nc ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].Nc);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nz: %hu", &phyFraming[cfgNumber].Nz ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }
	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].Nz);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nf: %hu", &phyFraming[cfgNumber].Nf ) !=1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }

	//msg("[CONFIG] %s (%d)\n",LineBuffer,phyFraming[cfgNumber].Nf);

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Extension_type: %hu", &phyFraming[cfgNumber].Extension_type ) !=1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }

	msg("[CONFIG FRAMING] : Done\n");
}


PHY_FRAMING *cfg_getPhyFraming(int cfgNumber)
{
	if (cfgNumber < MAX_CFG_SECTIONS)
		return &phyFraming[cfgNumber];
	else
	{
		msg("[CONFIG] Outrange config number <%d>.\n", cfgNumber);
		return NULL; //exit(1)??
	}
}

#ifndef OPENAIR_LTE
void 
cfg_readPhyCHBCH(FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	char temp_string[32];
	char i;
	
	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[CONFIG CHBCH] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf(LineBuffer, "symbol: %hu", &phyCHBCH[cfgNumber].symbol ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nsymb: %hu", &phyCHBCH[cfgNumber].Nsymb ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "IntDepth: %hu", &phyCHBCH[cfgNumber].IntDepth ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);	  
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "dd_offset: %hu", &phyCHBCH[cfgNumber].dd_offset ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }		
	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "Npilot: %hu", &phyCHBCH[cfgNumber].Npilot ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);	  
	  }
	for (i=0;i<8;i++) {
	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_re[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyCHBCH[cfgNumber].pilot_re[i] ) != 1)
	    {
	      msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_im[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyCHBCH[cfgNumber].pilot_im[i] ) != 1)
	    {
	      msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	}

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "FreqReuse: %hu", &phyCHBCH[cfgNumber].FreqReuse ) !=1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "FreqReuse_Ind: %hu", &phyCHBCH[cfgNumber].FreqReuse_ind ) !=1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "CHBCH_POWER_dBm: %d", &phyCHBCH[cfgNumber].CHBCH_POWER_dBm ) != 1)
		{
			msg("[CONFIG CHBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	msg("[CONFIG CHBCH]: Done\n");
}

 
PHY_CHBCH *cfg_getPhyCHBCH(int cfgNumber)
{
	if ( cfgNumber < MAX_CFG_SECTIONS )
		return &phyCHBCH[cfgNumber];
	else
	{
		msg("[CONFIG CHBCH] Outrange config number <%d>.\n", cfgNumber);
		return NULL; //exit(1)??
	}
}


void 
cfg_readPhyMRBCH(FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	char temp_string[32];
	char i;
	
	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[CONFIG MRBCH] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf(LineBuffer, "symbol: %hu", &phyMRBCH[cfgNumber].symbol ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nsymb: %hu", &phyMRBCH[cfgNumber].Nsymb ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "IntDepth: %hu", &phyMRBCH[cfgNumber].IntDepth ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);	  
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "dd_offset: %hu", &phyMRBCH[cfgNumber].dd_offset ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }		
	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "Npilot: %hu", &phyMRBCH[cfgNumber].Npilot ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);	  
	  }
	for (i=0;i<8;i++) {
	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_re[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyMRBCH[cfgNumber].pilot_re[i] ) != 1)
	    {
	      msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_im[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyMRBCH[cfgNumber].pilot_im[i] ) != 1)
	    {
	      msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	}

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "FreqReuse: %hu", &phyMRBCH[cfgNumber].FreqReuse ) !=1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "FreqReuse_Ind: %hu", &phyMRBCH[cfgNumber].FreqReuse_ind ) !=1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "MRBCH_POWER_dBm: %d", &phyMRBCH[cfgNumber].MRBCH_POWER_dBm ) != 1)
		{
			msg("[CONFIG MRBCH] Parse Error: \"%s\"\n", LineBuffer);
	  }
	msg("[CONFIG MRBCH]: Done\n");
}

 
PHY_MRBCH *cfg_getPhyMRBCH(int cfgNumber)
{
	if ( cfgNumber < MAX_CFG_SECTIONS )
		return &phyMRBCH[cfgNumber];
	else
	{
		msg("[CONFIG MRBCH] Outrange config number <%d>.\n", cfgNumber);
		return NULL; //exit(1)??
	}
}

void 
cfg_readPhyCHSCH(FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	char temp_string[32];
	int i;

	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[CHSCH CONFIG] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf(LineBuffer, "symbol: %hu", &phyCHSCH[cfgNumber].symbol ) != 1)
		{
			msg("[CHSCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nsymb: %hu", &phyCHSCH[cfgNumber].Nsymb ) != 1)
		{
			msg("[CHSCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "dd_offset: %lu", &phyCHSCH[cfgNumber].dd_offset ) != 1)
		{
			msg("[CHSCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	  }

	for (i=0;i<32;i++) {
	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"chsch_seq_re[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyCHSCH[cfgNumber].chsch_seq_re[i] ) != 1)
	    {
	      msg("[CHSCH CONFIG] %d: Parse Error: \"%s\"\n (%s)", cfgNumber,LineBuffer,temp_string);
	    }		

	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"chsch_seq_im[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phyCHSCH[cfgNumber].chsch_seq_im[i] ) != 1)
	    {
	      msg("[CHSCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	    }		

	}

	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "CHSCH_POWER_dBm: %d", &phyCHSCH[cfgNumber].CHSCH_POWER_dBm ) != 1)
		{
			msg("[CHSCH CONFIG] %d:Parse Error: \"%s\"\n", cfgNumber,LineBuffer);	  
	  }

	msg("[CONFIG CHSCH]: Done\n");			
}

 
PHY_CHSCH* cfg_getPhyCHSCH(int cfgNumber)
{
	if ( cfgNumber < MAX_CFG_SECTIONS )
		return &phyCHSCH[cfgNumber];
	else
	{
		msg("[CONFIG] Outrange config number <%d>.\n", cfgNumber);
		return NULL; //exit(1)??
	}
}

void 
cfg_readPhySCH(FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	char temp_string[32];
	int i;

	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[SCH CONFIG] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "Nsymb: %hu", &phySCH[cfgNumber].Nsymb ) != 1)
		{
			msg("[SCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	  }
	fgets(LineBuffer, MAX_LINE_SIZE, config);
	if( sscanf( LineBuffer, "dd_offset: %lu", &phySCH[cfgNumber].dd_offset ) != 1)
		{
			msg("[SCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	  }

	for (i=0;i<32;i++) {
	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"sch_seq_re[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phySCH[cfgNumber].sch_seq_re[i] ) != 1)
	    {
	      msg("[SCH CONFIG] %d: Parse Error: \"%s\"\n (%s)", cfgNumber,LineBuffer,temp_string);
	    }		

	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"sch_seq_im[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phySCH[cfgNumber].sch_seq_im[i] ) != 1)
	    {
	      msg("[SCH CONFIG] %d: Parse Error: \"%s\"\n", cfgNumber,LineBuffer);
	    }		

	}

	fgets( LineBuffer, MAX_LINE_SIZE, config );
	if( sscanf( LineBuffer, "SCH_POWER_dBm: %d", &phySCH[cfgNumber].SCH_POWER_dBm ) != 1)
		{
			msg("[SCH CONFIG] %d:Parse Error: \"%s\"\n", cfgNumber,LineBuffer);	  
	  }

	msg("[CONFIG SCH]: Done\n");			
}

 
PHY_SCH* cfg_getPhySCH(int cfgNumber)
{
	if ( cfgNumber < MAX_CFG_SECTIONS )
		return &phySCH[cfgNumber];
	else
	{
		msg("[CONFIG] Outrange config number <%d>.\n", cfgNumber);
		return NULL; //exit(1)??
	}
}


void 
cfg_readPhySACH(FILE* config, int cfgNumber)
{
	char LineBuffer[MAX_LINE_SIZE];
	char temp_string[32];
	int i;

	if( cfgNumber >= MAX_CFG_SECTIONS )
	{
		msg("[CONFIG] %d outrang number of sections\n", cfgNumber);	
	}
	
	fgets( LineBuffer, MAX_LINE_SIZE, config );

	
	if( sscanf( LineBuffer, "Npilot: %hu", &phySACH[cfgNumber].Npilot ) != 1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);	  
	  }

	printf("SACH cfgnum %d, pilots %d\n",phySACH[cfgNumber].Npilot);

	for (i=0;i<8;i++) {
	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_re[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phySACH[cfgNumber].pilot_re[i] ) != 1)
	    {
	      msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	  fgets(LineBuffer, MAX_LINE_SIZE, config);
	  sprintf(temp_string,"pilot_im[%d]: %%x",i);

	  if( sscanf( LineBuffer,temp_string, &phySACH[cfgNumber].pilot_im[i] ) != 1)
	    {
	      msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	    }		

	}

	fgets(LineBuffer, MAX_LINE_SIZE, config);
	//	msg("[CONFIG SACH] : %s\n",LineBuffer);
	if( sscanf( LineBuffer, "SACH_POWER_dBm: %d", &phySACH[cfgNumber].SACH_POWER_dBm ) !=1)
		{
			msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  }	

}

 
PHY_SACH *cfg_getPhySACH(int cfgNumber)
{
  if ( cfgNumber < MAX_CFG_SECTIONS ) 
    return &phySACH[cfgNumber];
  else
    {
      msg("[CONFIG ] Outrange config number <%d>.\n", cfgNumber);
      return NULL; //exit(1)??
    }
}

#endif //OPENAIR_LTE
