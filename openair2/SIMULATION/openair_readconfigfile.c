/*! 
 *  \brief Read each section of config file associated to the given scenario
 *  \author Navid Nikaein, modified R. Knopp
 *  \version 1.1
 *  \date    2005, modified April 2006
 *  \bug 
 *  \warning
 */

#include "COMMON/openair_types.h"
#include "config_defs.h"
#include "config_extern.h"
#include "phy_extern.h"
#include "SIMULATION/simulation_defs.h"

#define NB_CH_MAX 8
extern EMULATION_VARS *Emul_vars;

//#include "mac_extern.h"


#include "SIMULATION/PHY_EMULATION/ABSTRACTION/complex.h" 

#define GET_NB_NODE     0
#define GET_NODE_LIST   1
#define GET_RSSI_MATRIX 2
#define GET_NB_MACHINE  3
#define GET_MACHINE_ID  4

void config_topology(FILE *config){
  
  char SectionName[MAX_SECTION_NAME_SIZE];
  char LineBuffer[MAX_LINE_SIZE],car;
  int Next_action=GET_NB_NODE;
  int ret,i,j,k;
  unsigned int Machine_id;
  NB_UE_INST=0;NB_CH_INST=0;
  
  while ( (fgets (LineBuffer, MAX_LINE_SIZE, config)) != NULL){
    switch(Next_action){
    case GET_NB_NODE: 
      ret=sscanf(LineBuffer, "%s%c%d", SectionName,&car,&NB_NODE);	
      if (ret !=3 || strcmp(SectionName,"Nb_nodes") != 0){
	msg("[CONFIG] Parse Error: ret=%d,  \"%s\"\n", ret,LineBuffer);
	exit(0);
      }
      msg("[TOPOLOGY_CONFIG] GOT NB_NODE = %d\n",NB_NODE);
      Next_action=GET_NODE_LIST;
      break;
    case GET_NODE_LIST: 
      ret=sscanf(LineBuffer, "%s%c", SectionName,&car);	
      if (ret !=2 || strcmp(SectionName,"Nodes_list") != 0){
	msg("[CONFIG] Parse Error: ret=%d, \"%s\"\n", ret,LineBuffer);
	exit(0);
      }
      if(fgets (LineBuffer, MAX_LINE_SIZE, config) != NULL){
	for(i=0;i<NB_NODE;i++){
	  if(sscanf(&LineBuffer[3*i], "%d%c", &NODE_LIST[i],&car)!=2) {
	    msg("[CONFIG] Parse Error:i=%d, ret=%d, \"%s\"\n",i,ret, LineBuffer);
	    exit(0);
	  }
	  msg("[TOPOLOGY_CONFIG] GOT NODE_ID[%d] = %d\n",i,NODE_LIST[i]);
	}
	Next_action=GET_RSSI_MATRIX;
      }
      break;
    case GET_RSSI_MATRIX: 
      ret=sscanf(LineBuffer, "%s", SectionName);	
      if (ret !=1 || strcmp(SectionName,"Rssi_matrix_entries") != 0){
	msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	exit(0);
      }
      for(i=0;i<NB_NODE;i++){
	if(fgets (LineBuffer, MAX_LINE_SIZE, config) != NULL){
	  for(j=0;j<NB_NODE;j++){
	    if(sscanf(&LineBuffer[5*j], "%d%c",(int *) &RSSI[i][j],&car)!=2) {
	      msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	      exit(0);
	    }
	    // RSSI[i][j]=-RSSI[i][j];
	    msg("[TOPOLOGY_CONFIG] GOT RSSI MATRIX entry [%d][%d]= %d\n",i,j,(int)RSSI[i][j]);
	  }
	}
      }

      Next_action=GET_NB_MACHINE;
      break;
    case GET_NB_MACHINE: 
      ret=sscanf(LineBuffer, "%s%c%d", SectionName,&car,&NB_MASTER);	
      if (ret !=3 || strcmp(SectionName,"Nb_machines") != 0){
	msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	exit(0);
      }
      msg("[TOPOLOGY_CONFIG] GOT NB_MASTER = %d\n",NB_MASTER);
      Next_action=GET_MACHINE_ID;
      break;
    case GET_MACHINE_ID: 
      ret=sscanf(LineBuffer, "%s%c%d", SectionName,&car,&Machine_id);	
      if (ret !=3 || strcmp(SectionName,"Machine_id") != 0){
	msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	exit(0);
      }
      if(Machine_id == Master_id){
	msg("[TOPOLOGY_CONFIG] GOT MASTER_ID = %d\n",Master_id);
	if(fgets (LineBuffer, MAX_LINE_SIZE, config) != NULL){
	  ret=sscanf(LineBuffer, "%s%c%d", SectionName,&car,&NB_INST);
	  if (ret !=3 || strcmp(SectionName,"Nb_inst") != 0){
	    msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	    exit(0);
	  }
	  msg("[TOPOLOGY_CONFIG] GOT NB_INST = %d\n",NB_INST);
	  if(fgets (LineBuffer, MAX_LINE_SIZE, config) != NULL){
	    ret=sscanf(LineBuffer, "%s", SectionName);
	    if (ret !=1 || strcmp(SectionName,"Inst_list") != 0){
	      msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	      exit(0);
	    }
	    if(fgets (LineBuffer, MAX_LINE_SIZE, config) != NULL){
	      for(i=0;i<NB_INST;i++){
		if(sscanf(&LineBuffer[3*i], "%d%c", &NODE_ID[i],&car)!=2) {
		  msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
		  exit(0);
		}
		if(NODE_ID[i]<NB_CH_MAX) 
			NB_CH_INST++;
		else
			NB_UE_INST++;
		for(k=0;k<NB_NODE;k++)
		  if(NODE_ID[i]==NODE_LIST[k]) 
		    Emul_idx[i]=k;
		msg("[TOPOLOGY_CONFIG] GOT INST_ID[%d]=%d, Emul_idx = %d\n",i,NODE_ID[i],Emul_idx[i]);
	      }
	    }
	  }
	}
	Next_action=5;
      }
      else{
	fgets (LineBuffer, MAX_LINE_SIZE, config);
	fgets (LineBuffer, MAX_LINE_SIZE, config);
	fgets (LineBuffer, MAX_LINE_SIZE, config);
      }
      break;
    }
  }
  
}

void radio_emulation_load_KH(FILE *config){

  char LineBuffer[MAX_LINE_SIZE],car;
  unsigned char i,j=0;

  printf("before KH: Emul_idx[0] = %d\n",Emul_idx[0]);

  while ( j < NUMBER_OF_FREQUENCY_GROUPS){
    i=0;
    while ( i < NUMBER_OF_FREQUENCY_GROUPS){
      if((fgets (LineBuffer, MAX_LINE_SIZE, config)) != NULL){
	if(sscanf(LineBuffer, "%hd\t%hd",&KH[j][i].r,&KH[j][i].i)!=2) {
	  msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	  exit(0);
	}
	msg("[TOPOLOGY_CONFIG] GOT KH MATRIX entry [%d][%d]= %d,%d\n",j,i,KH[j][i].r,KH[j][i].i);
	i++;
      }
    }
    //    exit(0);
    j++;  
    
  }
  /*  j=0;
  while ( (fgets (LineBuffer, MAX_LINE_SIZE, config)) != NULL && j < NUMBER_OF_FREQUENCY_GROUPS){
    for(i=0;i<NUMBER_OF_FREQUENCY_GROUPS;i++){
      if(sscanf(&LineBuffer[6*i], "0x%X%c", (int *)&KH[j][i].i,&car)!=2) {
	msg("[CONFIG] Parse Error: \"%s\"\n", LineBuffer);
	exit(0);
      }
      msg("[TOPOLOGY_CONFIG] GOT KH IMAGINARY MATRIX entry [%d][%d]= %x\n",j,i,KH[j][i]);
      exit(0);
    }
   
    j++;  
  }
  */
}


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
