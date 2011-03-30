/*
 * global_parameters.h
 *
 *  Created on: Jan 28, 2011
 *      Author: jerome haerri
 */

#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_

/*int nodes = 10;
float min_X=0.0;
float max_X=100.0;
float min_Y=0;
float max_Y=100.0;
float min_speed=0.1; // must NOT be 0.0 to avoid instability
float max_speed=20.0;
float min_sleep=0.1; // must NOT be 0.0 to avoid instability
float max_sleep=5.0;
float max_time=1000;*/

struct global_parameters {
	int nodes = 10;
	float min_X=0.0;
	float max_X=100.0;
	float min_Y=0;
	float max_Y=100.0;
	float min_speed=0.1; // must NOT be 0.0 to avoid instability
	float max_speed=20.0;
	float min_pause=0.1; // must NOT be 0.0 to avoid instability
	float max_pause=5.0;
	float max_time=1000;
} m_global_param;

struct spatial_parameters {
	float min_X=0.0;
	float max_X=100.0;
	float min_Y=0;
	float max_Y=100.0;
	Graph *map;
} m_spatial_param;


#endif /* GLOBAL_PARAMETERS_H_ */
