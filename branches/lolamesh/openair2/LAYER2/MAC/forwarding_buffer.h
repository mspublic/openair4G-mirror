#ifndef __FORWARDING_BUFFER__
#define __FORWARDING_BUFFER__

//#include "platform_types.h"
//#include "platform_constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_NAME_MAX_CHAR 32
#define SORT_FIFO 1
#define SORT_PDU_SEQN 2
#define SORT_PDU_SIZE 3
#define MAC_BUFFER_MAXIMUM_CAPACITY 10

//-----------------------------------------------------------------------------

typedef struct mem_element_t{
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
  
  int	total_size;
  int	nb_elements;
  char	name[LIST_NAME_MAX_CHAR];
  
}packet_list_t;

typedef struct {
 packet_list_t *my_p;
 u8 sorting_flag;
 int maximum_capacity;
 char name[LIST_NAME_MAX_CHAR];
}MAC_BUFFER;

MAC_BUFFER **mac_buffer_g;

void mac_buffer_top_init();
MAC_BUFFER *mac_buffer_init(char *nameB, char *nameP, u8 Mod_id, u8 sorting_flag); 
// sorting_flag denotes the way that packets are sorted when inserted into the mac buffer's packet list (i.e FIFO, pdu_size, seq_num)
void packet_list_init (packet_list_t*, char *nameP);
void packet_list_free (packet_list_t* listP);
void mac_buffer_free(u8 Mod_id);

mem_element_t *packet_list_remove_head (packet_list_t *listP);
mem_element_t *mac_buffer_remove_head(u8 Mod_id); 

mem_element_t *packet_list_remove_tail(packet_list_t *listP); 
mem_element_t *mac_buffer_remove_tail(u8 Mod_id);

mem_element_t *packet_list_get_head (packet_list_t * listP); // returns the head of the list without removing it!
mem_element_t *mac_buffer_get_head(u8 Mod_id); // returns the head of the list without removing it!

int packet_list_find_pdu_seq_num(packet_list_t *listP, int seq_num);
int packet_list_find_pdu_seq_num2(packet_list_t *listP, int seq_num);


void packet_list_add_tail(mem_element_t * elementP, packet_list_t * listP);
void packet_list_add_head(mem_element_t * elementP, packet_list_t * listP);

void packet_list_add_after_ref(mem_element_t * new_elementP, mem_element_t *elementP_ref, packet_list_t * listP);

int mac_buffer_add_tail(u8 Mod_id, mem_element_t *elementP);
int mac_buffer_add_after(u8 Mod_id, mem_element_t *elementP); 
int mac_buffer_add_sorted(u8 Mod_id, mem_element_t *elementP);

void mac_buffer_print(u8 Mod_id);
void mac_buffer_print_reverse(u8 Mod_id);
int  mac_buffer_sort(u8 Mod_id);

mem_element_t *packet_list_find_pivot_seq_num(int seq_num, packet_list_t *listP, int *after);
mem_element_t *packet_list_find_pivot_pdu_size(int pdu_size, packet_list_t *listP, int *after);

// MAC API
int mac_buffer_get_sdu_size(u8 Mod_id, u16 seq_num, u8 eid);

int  mac_buffer_total_size(u8 Mod_id);
int  mac_buffer_nb_elements(u8 Mod_id);
mem_element_t * mac_buffer_data_req( u8 Mod_id, u8 eNB_index, int seq_num, int size, int HARQ_proccess_ID); 
int mac_buffer_data_ind(u8 Mod_id, u8 eNB_index, char *data, int seq_num, int pdu_size, int HARQ_proccess_ID);
//int mac_buffer_data_ind( u8 Mod_id, mem_element_t *elementP, int seq_num, int pdu_size, int HARQ_proccess_ID); // returns 1 for success otherwise 0 // returns 1 for success otherwise 0; // mod_id, add and sort the pdu according to the flag
#endif
