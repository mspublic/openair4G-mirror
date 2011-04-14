/*
 * vanetmobisim_mobility.c
 *
 *  Created on: Jan 27, 2011
 *      Author: jerome haerri
 */

#include "vanetmobisim_mobility.h"
#include "global_parameters.h"
#include "rwp.h"
#include "static.h"
#include "job.h"
#include "log.h"

#include "vanetmobisim_constants.h"

#include "helper.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include "EMU_time.h"



/*
 * EMU call
 */
void mobility_generator(unsigned char mobility_model, m_global_param param_list) {
	switch (mobility_model) {
	    case RWP: {
	    	start_RWP_Generator(param_list);
		break;
	    }
	    case STATIC: {
	    	start_Static_Generator(param_list);
	    break;
	    }
	    case RWALK: {
	       	start_RWALK_Generator(param_list);
	        break;
	    }
	    case UNDEF:
	    default:
	    	printf("Unsupported generator %c \n", mobility_model);
	}
}


/*
 * EMU call:
 */
void update_node_position(Node *node) {

	switch(node->generator) {
		case RWP:
			update_rwp_node_position(node);
			break;
		case RWALK:
			update_rwalk_node_position(node);
			break;
		case STATIC:
		default:
			printf("vanetmobisim - updateNodePosition - node static or unsupported generator");
	}
}

/*
 * EMU call:
 */
void update_node_position(int node_id) {
	for (ptr = nodes; ptr != NULL; ptr = ptr->next ) {
		Node *node = &ptr;
		if(node->ID = node_id) {
			update_node_position(node);
		}
	}
}

/*
 * EMU call:
 */
void update_node_position() {
	for (ptr = nodes; ptr != NULL; ptr = ptr->next ) {
		update_node_position(ptr);
	}
}

/*
 * EMU call:
 */
Node* get_node(int node_id) {
	Node *m_node;
	for (ptr = nodes; ptr != NULL; ptr = ptr->next ) {
		Node *node = ptr;
		if(node->ID = node_id) {
			m_node = node;
		}
	}
	return m_node;
}

/*
 * EMU call:
 */
Node_Vector* get_nodes() {
	return nodes;
}

/*
 * EMU call: function to call at every time step from the EMULATOR
 */
void update() {
	/*
	 * Pair <float, *Node> m_pair = job_list->peek();
	 */
	Pair *m_pair = (Pair *)peek_job();

	if((m_pair !=NULL) && ((float)m_pair->a == now())) {
		m_pair = head_job(job_list);
		Node* m_node= (Node *)m_pair.b;
		if(m_node->mobile == 1) {
			// stop node and let it sleep
			m_node->mobile = 0;
			node_sleep(m_node);
		}
		else if (m_node->mobile ==0) {
			// node slept enough...let's move again
			m_node->mobile = 1;
			node_move(m_node);
		}
		else
			LOG_E("update_generator: unsupported node state - mobile : %d \n", m_node->mobile);

		// sorting the new entries
		qsort(jobs, jobs_len, sizeof(Pair *), pair_cmp);
	}
	else
		return;
}

void set_time(int time) {
	m_time = time;
}

int get_time() {
	return time;
}

void usage(char *argv[]) {
	fprintf(stderr,"\n usage: %s\t-n <nodes> -pmin <min pause time> -pmin <max pause time> -smin <min speed> -smax <max speed>\n",argv[0]);
    fprintf(stderr,"\t\t-t <simulation time> -x <max X> -y <max Y> -gen <generator type> \n\n");
}

void main (int argc, char *argv[]) {
	/*
	 * TODO add required methods to generate the parameterList and retrieve the generator type
	 */

	int nn = 1;
	int min_speed = 1;
	int max_speed = 10;
	int min_pause = 1;
	int max_pause = 5;
	int duration = 100;
	int max_X = 50;
	int max_Y = 50;
	char *mobility_model = "RWP";

	if(max_X == 0.0 || max_Y == 0.0 || nodes == 0 || max_time == 0.0) {
		usage(argv);
		exit(1);
	}

	printf("Input Received: nodes: %d, max pause: %.2f, max speed: %.2f  max x = %.2f, max y: %.2f, gen = %s\n",
			nodes , max_pause, max_speed, max_X, max_Y, gen);

	mobility_generator(mobility_model, m_global_param);
}


