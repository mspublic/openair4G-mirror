/*! \file rtai_mem.c
* \brief a wrapper for Unified RTAI real-time memory management.
* \author Florian Kaltenberger
* \date 2011-04-06
* \version 0.1 
* \company Eurecom
* \email: florian.kaltenberger@eurecom.fr
* \note 
* \bug  
* \warning  
*/ 

#include <rtai.h>
#include <rtai_shm.h>


void* rt_alloc_wrapper(int size) {

  unsigned long* tmp_ptr;
  static unsigned long name = 0;

  tmp_ptr = (unsigned long*) rt_shm_alloc(name,size+sizeof(unsigned long),GFP_KERNEL);
	
  tmp_ptr[0] = name;

  return (void*) tmp_ptr[1];

}

int rt_free_wrapper(void* ptr) {

  unsigned long name = *(((unsigned long*) ptr)-1);

  return rt_shm_free(name);

}

void* rt_realloc_wrapper(void* oldptr, int size) {

  rt_free_wrapper(oldptr);

  return rt_alloc_wrapper(size);

}
