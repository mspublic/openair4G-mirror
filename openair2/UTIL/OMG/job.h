
#ifndef JOB_H_
#define JOB_H_
#include "common.h"

struct job_list_struct {
	Pair pair;
	struct job_list_struct *next;
}job_list_structure;
typedef struct job_list_struct* Job_list;
int Job_Vector_len;

Job_list add(Pair job, Job_list Job_Vector);
void display_job_list(Job_list Job_Vector);

Job_list job_list_sort (Job_list list, Job_list end);
Job_list quick_sort (Job_list list);
#endif /* JOB_H_ */
