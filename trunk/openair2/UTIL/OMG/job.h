/*
 * job.h
 *
 *  Created on: Feb 3, 2011
 *      Author: jerome haerri
 */

#ifndef JOB_H_
#define JOB_H_

typedef struct job_list *Jobs;

struct job_list {
	Pair *pair;
	struct job_list *next;
	Node *node;
};

inline Jobs* push_job(Pair *job) {
	Jobs *entry = malloc(sizeof(Jobs));
	entry->pair = job;
	entry->next = NULL;
	jobs_len++;
	return entry;
}

inline Pair* peek_job() {
	return jobs->pair;
}

inline Pair* head_job() {
	Jobs *tmp_job = jobs;
	Pair *tmp_pair = jobs->pair;

	jobs = jobs->next;
	delete(tmp_job);
	jobs_len--;
	if(jobs==NULL)
		printf("jobs::head() - job_list now empty \n");

	return tmp_pair;
}

inline void init_job(){
	if (jobs == NULL) {
		job = malloc(sizeof(Jobs));
		job->next = NULL;
		jobs_len = 0;
	}
	else {
		printf("job.h - trying to initialize the job_list while job_list not empty \n");
	}
}

static int jobs_len;
static Jobs *jobs;

#endif /* JOB_H_ */
