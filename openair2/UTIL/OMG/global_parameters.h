#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_

typedef struct {
	int nodes;
	double min_X;
	double max_X;
	double min_Y;
	double max_Y;
	double min_speed; // must NOT be 0.0 to avoid instability
	double max_speed;
	double min_journey_time; // must NOT be 0.0 to avoid instability
	double max_journey_time; 
	double min_azimuth; // RWALK direction
	double max_azimuth; 
	double min_sleep; // must NOT be 0.0 to avoid instability
	double max_sleep;
	int mobility_type;
}m_global_param;


#endif /* GLOBAL_PARAMETERS_H_ */
