#ifndef __OMG_VARS_H__
#define __OMG_VARS_H__

#include "omg.h"

#define MAX_NODE_TYPES 2 
//Node_list Node_Vector;
Node_list Node_Vector_Static_eNB;
Node_list Node_Vector_Static_UE;
Node_list Node_Vector_Rwalk;
Node_list Node_Vector_Rwp;

//Job_list Job_Vector;
Job_list Job_Vector_Rwp;
Job_list Job_Vector_Rwalk;

int Job_Vector_Rwp_len;
int Job_Vector_Rwalk_len;

omg_global_param omg_param_list;

double m_time;

#endif /*  __OMG_VARS_H__ */
