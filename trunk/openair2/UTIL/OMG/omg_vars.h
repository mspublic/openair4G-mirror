#ifndef __OMG_VARS_H__
#define __OMG_VARS_H__

#include "omg.h"

//Node_list Node_Vector;
Node_list Node_Vector_Static;
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
