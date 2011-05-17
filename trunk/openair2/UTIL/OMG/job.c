#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "omg.h"

Job_list add(Pair job, Job_list Job_Vector, int mobility_type){
    Job_list entry = malloc(sizeof(Job_list));
    entry->pair = job;
    switch (mobility_type){
    case RWP:
      Job_Vector_Rwp_len++;
      break;
    case RWALK:
      Job_Vector_Rwalk_len++;
      break;
    }
    //LOG_D("\n  Job_Vector_len %d", Job_Vector_len); 
    entry->next = NULL;

    if (Job_Vector == NULL) {
      LOG_D(OMG, "\nempty Job_Vector");
      LOG_D(OMG,"\nadded elmt ID %d\n", entry->pair->b->ID);
        return entry;
    }
    else {
        Job_list tmp = Job_Vector;
        while (tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = entry;
        LOG_D(OMG,"\nnon empty Job_Vector");
	LOG_D(OMG,"\nadded elmt ID %d\n", entry->pair->b->ID);

        return Job_Vector;
    }
}
// display list of jobs
void display_job_list(Job_list Job_Vector){

    	Job_list tmp = Job_Vector;

   while (tmp != NULL){
     LOG_D(OMG,"\nnode %d \ntime %.3f\n", tmp->pair->b->ID, tmp->pair->a);
        tmp = tmp->next;
	//i++;

    }
}

// quick sort of the linked list
Job_list job_list_sort (Job_list list, Job_list end){

    Job_list pivot, tmp, next, before, after;
    if ( list != end && list->next != end ){

        pivot = list;
	before = pivot;
	after = end;
        for ( tmp=list->next; tmp != end; tmp=next )
        {
            next = tmp->next;
            if (tmp->pair->a > pivot->pair->a)
                tmp->next = after, after = tmp;
            else
                tmp->next = before, before = tmp; 
        }
        
        before = job_list_sort (before, pivot);
        after = job_list_sort (after, end);
        
        pivot->next = after;
        return before;
    }
    return list;
}

Job_list quick_sort (Job_list list)
{
    return job_list_sort(list, NULL);
}






