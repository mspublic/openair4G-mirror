#include "defs.h"

/*==============================================================================
* dlsch_encoding.c
*
* Returns the decoded bit sequence.
*
* example: e = dlsch_encoding(llrs,struct1,struct2)
* 
* Author: Sebastian Wagner
* Date: 23-07-2012
*
===============================================================================*/

// #define DEBUG_DLSCH_DECODING

void mexFunction( int mlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]
                )
{
    
    /* Declare */
	short *dlsch_llr;
	unsigned char mcs;	
	unsigned int *ret;
	unsigned char mod_order;
	unsigned char num_pdcch_symbols;
	unsigned char harq_pid;
	unsigned char subframe;
	unsigned char Kmimo;
	unsigned char Mdlharq;
	unsigned char abstraction_flag;
	LTE_UE_DLSCH_t* dlsch;
	LTE_DL_FRAME_PARMS *frame_parms;
	PHY_VARS_UE *phy_vars_ue;   
	
	/* Allocate input */
	dlsch_llr = (short*) mxGetData(prhs[0]);
	
	/* Create new dlsch */
	Kmimo = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"Kmimo"));
	Mdlharq = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"Mdlharq"));
	abstraction_flag = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"abstraction_flag"));
	mcs = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"mcs"));
	
    #ifdef DEBUG_DLSCH_DECODING
    mexPrintf("Kmimo %d\n",Kmimo);
    mexPrintf("Mdlharq %d\n",Mdlharq);
    mexPrintf("abstraction_flag %d\n",abstraction_flag);
    mexPrintf("mcs %d\n",mcs);
    #endif
    
	/* Create new dlsch */
	dlsch = new_ue_dlsch(Kmimo,Mdlharq,abstraction_flag);
    
    // Init CRC tables
	crcTableInit();	
	init_td();
 	    
	harq_pid = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"harq_pid"));
	dlsch->current_harq_pid = harq_pid;
	dlsch->harq_processes[harq_pid]->rvidx = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"rvidx"));
	dlsch->harq_processes[harq_pid]->Nl = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"Nl"));
	dlsch->harq_processes[harq_pid]->Ndi = (unsigned char) mxGetScalar(mxGetField(prhs[2],0,"Ndi"));
	dlsch->harq_processes[harq_pid]->mcs = mcs;
	dlsch->harq_processes[harq_pid]->rb_alloc[0] = (unsigned int) mxGetScalar(mxGetField(prhs[1],0,"rb_alloc"));
	dlsch->harq_processes[harq_pid]->nb_rb = (unsigned short) mxGetScalar(mxGetField(prhs[1],0,"nb_rb"));
				
	dlsch->harq_processes[harq_pid]->TBS = dlsch_tbs25[get_I_TBS(mcs)][dlsch->harq_processes[harq_pid]->nb_rb-1];
				
	
	num_pdcch_symbols = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"num_pdcch_symbols"));
	subframe = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"subframe"));
	
	phy_vars_ue = malloc16(sizeof(PHY_VARS_UE));	
	
	// Create a LTE_DL_FRAME_PARMS structure and assign required params
	frame_parms = malloc16(sizeof(LTE_DL_FRAME_PARMS));	
	frame_parms->N_RB_DL = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"nb_rb"));
	frame_parms->frame_type = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"frame_type"));
	frame_parms->mode1_flag = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"mode1_flag"));
	frame_parms->Ncp = (unsigned char) mxGetScalar(mxGetField(prhs[1],0,"Ncp"));
	
    
 	mod_order = get_Qm(dlsch->harq_processes[harq_pid]->mcs);
	dlsch->harq_processes[harq_pid]->G = get_G(frame_parms,dlsch->harq_processes[harq_pid]->nb_rb,dlsch->harq_processes[harq_pid]->rb_alloc,mod_order,num_pdcch_symbols,0,subframe);
	
	if (dlsch->harq_processes[harq_pid]->G != mxGetM(prhs[0])) {
		free_ue_dlsch(dlsch);
		free(frame_parms);
        free(phy_vars_ue);
		mexErrMsgTxt("Length of the LLR vector is incorrect.");		
	}
	
	/* Allocate Output */	
	plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT32_CLASS, mxREAL);
	ret = (unsigned int*) mxGetPr(plhs[0]);         	
	
    /* Algo */	
	*ret = dlsch_decoding(phy_vars_ue, dlsch_llr, frame_parms, dlsch, dlsch->harq_processes[harq_pid], subframe, dlsch->current_harq_pid, 1);
					
	/* free dlsch */
	free_ue_dlsch(dlsch);
	free(frame_parms);
	free(phy_vars_ue);
    free_td();    
}
