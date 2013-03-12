
#ifndef __FORWARDING_BUFFER__
#define __FORWARDING_BUFFER__
 


#include "platform_types.h"
#include "platform_constants.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// remove after #include "UTIL/MEM/mem_block.h"
//#include "UTIL/LISTS/list.h"

#define LIST_NAME_MAX_CHAR 32


//-----------------------------------------------------------------------------

typedef struct mem_block_t{
  struct mem_element_t *next;
  struct mem_element_t *previous;
  int seq_num;
  int pdu_size;
  int HARQ_proccess_ID;
  
  unsigned char pool_id;
  char *data;
}mem_element_t;

typedef struct {
  struct mem_element_t *head;
  struct mem_element_t *tail;
  
  int total_size;
  int    nb_elements;
  char   name[LIST_NAME_MAX_CHAR];
  
} packet_list_t;

typedef struct {
 packet_list_t *my_p;
 char name[LIST_NAME_MAX_CHAR];
} mac_list_buffer;

mac_list_buffer **mac_list_buffer_g;

void packet_list_init (packet_list_t* , char *);
void mac_list_buffer_init(mac_list_buffer * mac_buff, char *, char *); // + flag for buffer type: fifo, sort

mem_element_t * packet_list_remove_head (packet_list_t * listP);
mem_element_t * mac_list_buffer_remove_head(mac_list_buffer *mac_buff); 

mem_element_t * packet_list_remove_tail(packet_list_t * listP); 
mem_element_t * mac_list_buffer_remove_tail(mac_list_buffer *mac_buff);

mem_element_t * packet_list_get_head (packet_list_t * listP); // returns the head of the list without removing it
mem_element_t * mac_list_buffer_get_head(mac_list_buffer *mac_buff); // returns the head of the list without removing it

int packet_list_find_pdu_seq_num(packet_list_t *listP, int seq_num);
void packet_list_add_tail (mem_element_t * elementP, packet_list_t * listP);
int mac_list_buffer_add_tail( mac_list_buffer *mac_buff, mem_element_t *elementP);

void mac_list_buffer_print(mac_list_buffer *mac_buf);

// MAC API
int  mac_list_buffer_total_size(mac_list_buffer *mac_buf); // should be Mod_id
int  mac_list_buffer_nb_elements(mac_list_buffer *mac_buf); // should be Mod_id

mem_element_t * mac_buffer_data_req( mac_list_buffer *mac_buf, int seq_num, int size, int HARQ_proccess_ID); // 
//mem_element_t * mac_buffer_data_req(int Mod_id, int seq_num, int size, int HARQ_proccess_ID);

int mac_buffer_data_ind( mac_list_buffer *mac_buf, mem_element_t *elementP, int seq_num, int pdu_size, int HARQ_proccess_ID ); // returns 1 for success otherwise 0; 
//int mac_buffer_data_ind( int Mod_id, mem_element_t *elementP, int seq_num, int pdu_size, int HARQ_proccess_ID ); // returns 1 for success otherwise 0; // mod_id, add and sort the pdu according to the flag
#endif
