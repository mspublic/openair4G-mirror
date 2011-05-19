#ifndef __OMG_H_
#define __OMG_H_

#include "omg_constants.h"
#include "defs.h"
#include "omg_vars.h"
#include "UTIL/LOG/log.h"

/* ==========================================================
    Function Prototypes
   ====================================================================== */

void usage();

void init_omg_global_params(void);

void set_time(double time);

double get_time();

void update_nodes(int mobility_type, double cur_time) ;

Node_list get_current_positions(int mobility_type, int nodes_type,double cur_time);

NodePtr get_node_position(int mobility_type, int nodes_type,int nID);

Node_list add_entry(NodePtr node, Node_list Node_Vector);

#endif /* __OMG_H_ */
