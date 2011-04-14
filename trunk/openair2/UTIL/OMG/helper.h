/*
 * helper.h
 *
 *  Created on: Jan 28, 2011
 *      Author: jerome haerri
 */

#ifndef HELPER_H_
#define HELPER_H_
#include <stdlib.h>

#define frand() ((double) rand() / (RAND_MAX+1))

typedef struct pair_struct *Pair;

struct pair_struct {
	void *a;
	void *b;
};

inline float random(const float min, const float max) { return ( min + (max-min)*frand());}

inline int pair_cmp(const void *a, const void *b){
    const Pair *ap = (Pair *)a;
    const Pair *bp = (Pair *)b;
	if ((float)(ap->a) > (float)(bp->a))
    	return 1;
    else if ((float)(ap->a) < (float)(bp->b))
    	return -1;
    else
    	return 0;
};

inline void* head (const void *a, const a_length){
	/*
	 * TODO : resize the array
	 */
	return a[0];
};

inline void* peek (const void *a){
	return a[0];
};

#endif /* HELPER_H_ */
