#ifndef VANETMOBISIM_MOBILITY_H_
#define VANETMOBISIM_MOBILITY_H_
#include "global_parameters.h"
#include "common.h"
#include "job.h"

	

/* ======================================================================
    Function Prototypes
   ====================================================================== */

void usage();

void set_time(double time);

double get_time();

void initialize_node(NodePtr node) ;

void initialize_nodes(double cur_time);

Pair sleep_node(NodePtr node, m_global_param param_list, double cur_time);

Pair move_node(NodePtr node, m_global_param param_list, double cur_time) ;

void update_nodes(double cur_time) ;

Node_list get_current_positions();

NodePtr get_node_position(int nID);




/* ======================================================================
    Global variable
   ====================================================================== */

Node_list Node_Vector;

Job_list Job_Vector;

m_global_param param_list;

double m_time;

#endif /* VANETMOBISIM_MOBILITY_H_ */
