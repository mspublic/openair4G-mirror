/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file forwarding_buffer.h
* \brief structures and data types for supporting packet mac_buffer storage
* \author apostolos apostolaras and navid nikaein 
* \date 2013
* \version 0.1
* @ingroup _mac
*/

#ifndef __FORWARDING_BUFFER__
#define __FORWARDING_BUFFER__

//#include "platform_types.h"
//#include "platform_constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl_tree.h" 

#define LIST_NAME_MAX_CHAR 32
#define MAC_BUFFER_MAXIMUM_CAPACITY 40
#include <stdint.h>
//typedef uint8_t u8;
//#define NB_UE_INST 4

//-----------------------------------------------------------------------------

enum operation_mode{
 ONE_BUF_PER_CH = 0,
 ONE_BUF_PER_CORNTI = 1,
};

typedef struct mem_element_t{
  struct mem_element_t *next;
  struct mem_element_t *previous;
	
	struct avl_node_t *avl_node_pdu_seqn;
	struct avl_node_t *avl_node_pdu_size;
	
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
 struct avl_node_t *tree_pdu_seqn;
 struct avl_node_t *tree_pdu_size;
 
// u8 sorting_flag;

 u8 eNB_index;
 u16 cornti;
 int maximum_capacity;
 char name[LIST_NAME_MAX_CHAR];
}MAC_BUFFER;


typedef struct{
 MAC_BUFFER **mac_buffer_g;
 
 u8 mode; //0:  1x buffer per CH, 1:  1x buffer per cornti
 int total_number_of_buffers_allocated;
}MAC_BUFFER_UE;

//MAC_BUFFER **mac_buffer_g;
MAC_BUFFER_UE *mac_buffer_u;


void mac_buffer_top_init();
int mac_buffer_instantiate (u8 Mod_id, u8 eNB_index, u16 cornti);
MAC_BUFFER *mac_buffer_init(char *nameB, char *nameP, u8 eNB_index, u16 cornti); 
void packet_list_init (packet_list_t*, char *nameP);
void packet_list_free (packet_list_t* listP);
void mac_buffer_free(u8 Mod_id, u8 b_index);

mem_element_t *packet_list_remove_head (packet_list_t *listP); // makes NULL internal pointers to nodes tress before returning the head
mem_element_t *packet_list_remove_head_2(packet_list_t * listP); // it returns the head as a whole
mem_element_t *mac_buffer_remove_head(u8 Mod_id, u8 b_index, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size);

mem_element_t *packet_list_remove_tail(packet_list_t *listP);
mem_element_t *mac_buffer_remove_tail(u8 Mod_id, u8 b_index, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size);

mem_element_t *packet_list_remove_middle(packet_list_t *listP, mem_element_t *ptr);
mem_element_t *mac_buffer_remove_middle(u8 Mod_id, u8 b_index, mem_element_t *ptr, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size);


mem_element_t *packet_list_get_head (packet_list_t *listP); // returns the head of the list without removing it!
mem_element_t *mac_buffer_get_head(u8 Mod_id, u8 b_index); // returns the head of the list without removing it!

void packet_list_get_info_from_the_first_elements(packet_list_t * listP, u16 number_of_packets_asked, u16 **seq_num, u16 **size);


int packet_list_find_pdu_seq_num(packet_list_t *listP, int seq_num);
int packet_list_find_pdu_seq_num2(packet_list_t *listP, int seq_num);
void packet_list_print(packet_list_t *listP);

void packet_list_add_tail(mem_element_t *elementP, packet_list_t *listP); // pointers of elementP to packet trees are inserted with NULL, and are updated outside this func
void packet_list_add_tail_2(mem_element_t *elementP, packet_list_t *listP); //pointers of elementP to packet trees are assigned already.

void packet_list_add_head(mem_element_t *elementP, packet_list_t *listP);

void packet_list_add_after_ref(mem_element_t *new_elementP, mem_element_t *elementP_ref, packet_list_t *listP);

int mac_buffer_add_tail(u8 Mod_id, u8 b_index, mem_element_t *elementP);
int mac_buffer_add_after(u8 Mod_id, u8 b_index, mem_element_t *elementP); 
int mac_buffer_return_b_index(u8 Mod_id, u8 eNB_index, u16 cornti);

void mac_buffer_print(u8 Mod_id, u8 eNB_index, u16 cornti); 
void mac_buffer_print_2(u8 Mod_id, u8 eNB_index, u16 cornti); // used also for debugging
void mac_buffer_print_3(u8 Mod_id, u8 eNB_index, u16 cornti);  // used also for debugging
void mac_buffer_print_4(u8 Mod_id, u8 eNB_index, u16 cornti); // used also for debugging
void mac_buffer_print_reverse(u8 Mod_id, u8 eNB_index, u16 cornti);  // used also for debugging
void mac_buffer_print_all_per_MR(u8 Mod_id); // used also for debugging

mem_element_t *packet_list_find_pivot_seq_num(int seq_num, packet_list_t *listP, int *after);
mem_element_t *packet_list_find_pivot_pdu_size(int pdu_size, packet_list_t *listP, int *after);

// MAC API
int  mac_buffer_total_size(u8 Mod_id, u8 eNB_index, u16 cornti);
int  mac_buffer_nb_elements(u8 Mod_id, u8 eNB_index, u16 cornti); 

// just return the pointer to the element for consulting, do not remove from the buffer
//mem_element_t *mac_buffer_stat_ind(u8 Mod_id, u8 eNB_index, u16 cornti, u16 eid);
void mac_buffer_stat_ind(u8 Mod_id, u8 eNB_index, u16 cornti, u16 *number_of_packets_asked, u16 **seq_num, u16 **size);

mem_element_t *mac_buffer_data_req(u8 Mod_id, u8 eNB_index, u16 cornti, int seq_num, int requested_size, int HARQ_proccess_ID); 
int mac_buffer_data_ind(u8 Mod_id, u8 eNB_index, u16 cornti, char *data, int seq_num, int pdu_size, int HARQ_proccess_ID);// returns 1 for success otherwise 0 //
#endif
