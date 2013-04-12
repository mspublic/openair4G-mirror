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

/*! \file forwarding_buffer.c
* \brief primitives for supporting packet mac_buffer storage
* \author apostolos apostolaras and navid nikaein 
* \date 2013
* \version 0.1
* @ingroup _mac
*/

#include <stdlib.h>

#include "extern.h"
#include "defs.h"
#include "UTIL/LOG/log.h"

#include "forwarding_buffer.h"
#include "avl_tree.h"

int mac_buffer_instantiate(u8 Mod_id, u8 eNB_index, u16 cornti){
  int b_index,i; 
	int flag = 1;
	char *string=malloc(sizeof(char)*20);
  char *string1=malloc(sizeof(char)*40);
  char *string2=malloc(sizeof(char)*40);
	u8 mode = mac_buffer_u[Mod_id].mode;

  LOG_I(MAC, "[UE %d] instantiate the buffer for eNB %d and cornti %x \n", Mod_id, eNB_index, cornti);
	
	

	 if(mode == ONE_BUF_PER_CH){
		if(mac_buffer_u[Mod_id].total_number_of_buffers_allocated < NUMBER_OF_CONNECTED_eNB_MAX){
		 for(i=0;i<mac_buffer_u[Mod_id].total_number_of_buffers_allocated;i++){
			if(mac_buffer_u[Mod_id].mac_buffer_g[i]->eNB_index==eNB_index){
			 flag = 0;
			 break;
			}
		 }
		}
		else{
		 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure for mac_buffer_instantiate, CANNOT Allocate MORE than NUMBER_OF_CONNECTED_eNB_MAX=%d buffers per MR in mode ONE_BUF_PER_CH\n",NUMBER_OF_CONNECTED_eNB_MAX);
		 mac_xface->macphy_exit("out of memory for MAC buffer init mac_buffer_instantiate");
		 return 0;
		}
	 }else if(mode==ONE_BUF_PER_CORNTI){
		if(mac_buffer_u[Mod_id].total_number_of_buffers_allocated < 2*NUMBER_OF_CONNECTED_eNB_MAX){
		 for(i=0;i<mac_buffer_u[Mod_id].total_number_of_buffers_allocated;i++){
			if(mac_buffer_u[Mod_id].mac_buffer_g[i]->cornti==cornti){
			 flag = 0;
			 break;
			}
		 }
		}
		else{
		 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure for mac_buffer_instantiate, CANNOT Allocate MORE than 2*NUMBER_OF_CONNECTED_eNB_MAX=%d buffers per MR in mode ONE_BUF_PER_CORNTI\n",2*NUMBER_OF_CONNECTED_eNB_MAX);
		 mac_xface->macphy_exit("out of memory for MAC buffer init mac_buffer_instantiate");
		 return 0; 
		}
	 }
	 if(flag == 1){
		sprintf(string,"MR %d, (eNB_index %d, cornti %d)",Mod_id,eNB_index,cornti);
		strcpy(string1,"mac_buffer: ");
		strcpy(string2,"packet_list: ");
		strcat(string1,string);
		strcat(string2,string);
		b_index = mac_buffer_u[Mod_id].total_number_of_buffers_allocated;
		mac_buffer_u[Mod_id].mac_buffer_g[b_index] =  mac_buffer_init(string1, string2, eNB_index, cornti);
		mac_buffer_u[Mod_id].total_number_of_buffers_allocated +=1;
		string[0]='\0'; // flush string buffer
		string1[0]='\0';
		string2[0]='\0';
		return 1;
	 }
	 else{
		//  function returns -1, in the case where buffer must not be instantiated because it already exists (either in case 1 buf per CH or per cornti)
		return -1;
	 }


}


void mac_buffer_top_init(){
  u8 UE_id;
	u8 mode = ONE_BUF_PER_CH;
	
	LOG_E(MAC,"[MAC] initializing the MAC buffer for vlink support mode %d",(mode == 0 ) ? "1xbuffer:1xeNB" : "1xbuffer:1xCORNTI");
  
  mac_buffer_u = malloc(NB_UE_INST*sizeof(MAC_BUFFER_UE));
  if(mac_buffer_u==NULL){
    LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure for mac_buffer/mac_buffer_top_init (mac_buffer_u)\n");
    mac_xface->macphy_exit("out of memory for MAC buffer init (mac_buffer_u)");
  }
	for(UE_id=0; UE_id<NB_UE_INST;UE_id++){ 
	  mac_buffer_u[UE_id].mode=mode;
	  if(mac_buffer_u[UE_id].mode == ONE_BUF_PER_CH){
			 mac_buffer_u[UE_id].mac_buffer_g = malloc(NUMBER_OF_CONNECTED_eNB_MAX*sizeof(MAC_BUFFER));
		}else if(mac_buffer_u[UE_id].mode==ONE_BUF_PER_CORNTI){// This is the case of corntis so each link between CH has 2 conrntis so the MAX number of buffers is 2*NUMBER_OF_CONNECTED_eNB_MAX
			 mac_buffer_u[UE_id].mac_buffer_g = malloc(2*NUMBER_OF_CONNECTED_eNB_MAX*sizeof(MAC_BUFFER));		 
		}
		if(mac_buffer_u[UE_id].mac_buffer_g==NULL){
    LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure for mac_buffer/mac_buffer_top_init (mac_buffer_u[%d].mac_buffer_g)\n",UE_id);
    mac_xface->macphy_exit("out of memory for MAC buffer init (mac_buffer_u[-].mac_buffer_g)");
	  }
	  mac_buffer_u[UE_id].total_number_of_buffers_allocated=0;
		
	}
}

MAC_BUFFER *mac_buffer_init(char *nameB, char *nameP, u8 eNB_index, u16 cornti){
 
 MAC_BUFFER * mac_buff;
 mac_buff = malloc(sizeof(MAC_BUFFER));
 if(mac_buff==NULL){
	 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure in mac_buffer_init\n");
	 mac_xface->macphy_exit("out of memory for MAC buffer init");
	return NULL;
 }
 strcpy(mac_buff->name, nameB);

 mac_buff->maximum_capacity = MAC_BUFFER_MAXIMUM_CAPACITY;
 mac_buff->cornti=cornti;
 mac_buff->eNB_index=eNB_index;
 
 mac_buff->my_p = (packet_list_t*)malloc(sizeof(packet_list_t));
 if(mac_buff->my_p==NULL){
	LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure in mac_buffer_init for packet_list\n");
	mac_xface->macphy_exit("out of memory for MAC buffer init");
	return NULL;
 }
  packet_list_init(mac_buff->my_p, nameP);
	return mac_buff;
}

void packet_list_init(packet_list_t *listP, char *nameP){
  int  i = 0;
  
  if (nameP){
    while ((listP->name[i] = nameP[i]) && (i++ < LIST_NAME_MAX_CHAR));
  }
  listP->tail = NULL;
  listP->head = NULL;
  listP->nb_elements = 0;
  listP->total_size = 0;
}

void packet_list_free(packet_list_t* listP){
 mem_element_t	*le;
	while ((le = packet_list_remove_head (listP))){
	 free(le);
	}
}

void mac_buffer_free(u8 Mod_id, u8 b_index){
	packet_list_free(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p);
}

mem_element_t *packet_list_remove_head(packet_list_t * listP){
//-----------------------------------------------------------------------------
// access optimisation
 mem_element_t	*head;
 head = listP->head;
 // almost one element
 if(head != NULL){
	
	head->avl_node_pdu_seqn = NULL;
	head->avl_node_pdu_size = NULL;
	
	listP->head = head->next;
	listP->nb_elements = listP->nb_elements - 1;
	listP->total_size = listP->total_size - head->pdu_size;
    // if only one element, update tail
	if (listP->head == NULL){
	 listP->tail = NULL;
	}
	else{
	 listP->head->previous = NULL;
	 head->next = NULL;
	}
 }
 else{
    LOG_E(MAC,"[MEM_MGT][WARNING]  packet_list_remove_head (%s) \n",listP->name);
 }
 return head;
}

mem_element_t *packet_list_remove_head_2(packet_list_t * listP){
//-----------------------------------------------------------------------------
// access optimisation
 mem_element_t	*head;
 head = listP->head;
 // almost one element
 if(head != NULL){
	listP->head = head->next;
	listP->nb_elements = listP->nb_elements - 1;
	listP->total_size = listP->total_size - head->pdu_size;
    // if only one element, update tail
	if (listP->head == NULL){
	 listP->tail = NULL;
	}
	else{
	 listP->head->previous = NULL;
	 head->next = NULL;
	}
 }
 else{
   LOG_E(MAC,"[MEM_MGT][WARNING]  packet_list_remove_head_2 (%s) \n",listP->name);
 }
 return head;
}

mem_element_t *mac_buffer_remove_head(u8 Mod_id, u8 b_index, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size){
 if(avl_node_pdu_seqn==NULL || avl_node_pdu_size == NULL ){
	LOG_E(MAC,"[MEM_MGT][WARNING]  mac_buffer_remove_head() avl_node_pdu_seqn or avl_node_pdu_size is NULL \n");
	return NULL;
 }
 if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0){
 int d1 = avl_node_pdu_seqn->pdu_seq_num_tree;
 int d2 = avl_node_pdu_seqn->pdu_size_tree;
 int d3 = avl_node_pdu_seqn->pdu_size_tree_in_next;
 int seq_num = avl_node_pdu_seqn->key;
 int pdu_size = avl_node_pdu_seqn->second_key;

 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn = avl_tree_delete_node(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, seq_num, d1,d2,d3);
 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size = avl_tree_delete_node_pdu_size_tree(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size, pdu_size, seq_num);
 return (mem_element_t*)(packet_list_remove_head(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p));
 }else{
	return NULL;
 }
}

mem_element_t *packet_list_remove_tail(packet_list_t * listP){
//-----------------------------------------------------------------------------
// access optimisation;
 mem_element_t      *tail;
 tail = listP->tail;
 // almost one element;
 if(tail != NULL){
	tail->avl_node_pdu_seqn = NULL;
	tail->avl_node_pdu_size = NULL;
	
	listP->nb_elements = listP->nb_elements - 1;
	listP->total_size = listP->total_size - tail->pdu_size;
	
	// if only one element, update head, tail;
	if(listP->head == tail){
	 listP->head = NULL;
	 listP->tail = NULL;
	}
	else{
	 listP->tail = tail->previous;
	 tail->previous->next = NULL;
	}
	tail->previous = NULL;
 }
 else{
    //msg("[MEM_MGT][WARNING] packet_list_remove_tail (%s) \n",listP->name);
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_remove_tail (%s) \n",listP->name);
  }
  return tail;
}


mem_element_t *packet_list_remove_middle(packet_list_t * listP, mem_element_t *ptr){
//-----------------------------------------------------------------------------
// access optimisation;
 if(ptr != NULL){
	listP->nb_elements = listP->nb_elements - 1;
	listP->total_size = listP->total_size - ptr->pdu_size;
	
	ptr -> avl_node_pdu_seqn = NULL;
	ptr -> avl_node_pdu_size = NULL;
	
	ptr -> previous -> next = ptr -> next;
	ptr -> next -> previous = ptr -> previous;
	ptr -> next = NULL;
	ptr -> previous = NULL;
	return ptr;
  }
  else{
    //msg("[MEM_MGT][WARNING] packet_list_remove_middle (%s) \n",listP->name);
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_remove_middle (%s) \n",listP->name);
  return NULL;
	}
  
}

mem_element_t *mac_buffer_remove_middle(u8 Mod_id, u8 b_index, mem_element_t *packet, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size){
 if(avl_node_pdu_seqn==NULL || avl_node_pdu_size == NULL ){
	//printf("Error: in mac_buffer_remove_middle() avl_node_pdu_seqn or avl_node_pdu_size is NULL\n");
	LOG_E(MAC,"[MEM_MGT][WARNING]  in mac_buffer_remove_middle() avl_node_pdu_seqn or avl_node_pdu_size is NULL\n");
	return NULL;
 }
 if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0){
 int d1 = avl_node_pdu_seqn->pdu_seq_num_tree;
 int d2 = avl_node_pdu_seqn->pdu_size_tree;
 int d3 = avl_node_pdu_seqn->pdu_size_tree_in_next;
 int seq_num = avl_node_pdu_seqn->key;
 int pdu_size = avl_node_pdu_seqn->second_key;
 
 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn = avl_tree_delete_node(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, seq_num, d1,d2,d3);
 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size = avl_tree_delete_node_pdu_size_tree(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size, pdu_size, seq_num);
 return (mem_element_t *)( packet_list_remove_middle(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p, packet));
 }else{
	return NULL;
 }
}

mem_element_t *mac_buffer_remove_tail(u8 Mod_id, u8 b_index, struct avl_node_t *avl_node_pdu_seqn, struct avl_node_t *avl_node_pdu_size){
if(avl_node_pdu_seqn==NULL || avl_node_pdu_size == NULL ){
	//printf("Error: in mac_buffer_remove_tail() avl_node_pdu_seqn or avl_node_pdu_size is NULL\n");
	LOG_E(MAC,"[MEM_MGT][WARNING] in mac_buffer_remove_tail() avl_node_pdu_seqn or avl_node_pdu_size is NULL\n");
	return NULL;
 }
 if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0){
 int d1 = avl_node_pdu_seqn->pdu_seq_num_tree;
 int d2 = avl_node_pdu_seqn->pdu_size_tree;
 int d3 = avl_node_pdu_seqn->pdu_size_tree_in_next;
 int seq_num = avl_node_pdu_seqn->key;
 int pdu_size = avl_node_pdu_seqn->second_key;

 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn = avl_tree_delete_node(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, seq_num, d1,d2,d3);
 mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size = avl_tree_delete_node_pdu_size_tree(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size, pdu_size, seq_num);

 return (mem_element_t *)( packet_list_remove_tail(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p));
 }else{
	return NULL;
  }
}

void packet_list_get_info_from_the_first_elements(packet_list_t * listP, u16 number_of_packets_asked, u16 **seq_num, u16 **size){
//-----------------------------------------------------------------------------
  // access optimisation
  mem_element_t  *head,*ptr;
	int i;
  head = listP->head;
	ptr = listP->head;
  if (head == NULL) {
    //msg("[MEM_MGT][WARNING] packet_list_get_head() return NULL head from empty list %s \n",listP->name);
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_get_head() return NULL head from empty list %s \n",listP->name);
    //return NULL;
  }
  else{
	 for(i=0;i<number_of_packets_asked;i++){
		*seq_num[i]= ptr->seq_num;
    *size[i] = ptr->pdu_size;
		ptr=ptr->next;
	 }
	}
}


void mac_buffer_stat_ind(u8 Mod_id, u8 eNB_index, u16 cornti, u16 *number_of_packets_asked, u16 **seq_num, u16 **size){
 u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
 if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0 && mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements >= *number_of_packets_asked ){
	packet_list_get_info_from_the_first_elements(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p, *number_of_packets_asked, seq_num, size );
 }
 else if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0 && mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements < *number_of_packets_asked ){
	*number_of_packets_asked = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements;
	packet_list_get_info_from_the_first_elements(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p, *number_of_packets_asked, seq_num, size);
 }
}


mem_element_t *packet_list_get_head(packet_list_t * listP){
//-----------------------------------------------------------------------------
  // access optimisation
  mem_element_t  *head;
  head = listP->head;
  if (head == NULL) {
    //msg("[MEM_MGT][WARNING] packet_list_get_head() return NULL head from empty list %s \n",listP->name);
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_get_head() return NULL head from empty list %s \n",listP->name);
    return NULL;
  }
  return head;
}

mem_element_t *mac_buffer_get_head(u8 Mod_id, u8 b_index){
  if(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements!=0){
    return  (mem_element_t*)( packet_list_get_head(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p) );
  }
  else{
   // msg("[MEM_MGT][WARNING] mac_buffer_get_head() return NULL head from empty list\n");
   LOG_E(MAC,"[MEM_MGT][WARNING] mac_buffer_get_head() return NULL head from empty list\n");
  return NULL;
  }
}



mem_element_t *mac_buffer_data_req(u8 Mod_id, u8 eNB_index, u16 cornti, int seq_num, int requested_size, int HARQ_proccess_ID){
 
 u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);

 mem_element_t *help_head = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
 mem_element_t *help_tail = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->tail;
 avl_node_t *ptr_t,*ptr_k;
  
 if(help_head==NULL){
	 return NULL; // empty list no packet seq_num exists, it returns 0 !!!    
	}
	else{
	 if(!(seq_num<0) && !(HARQ_proccess_ID<0)){
		ptr_t = avl_tree_find(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, seq_num);
		if(ptr_t!=NULL){
		 	ptr_k = ptr_t->packet->avl_node_pdu_size;
		 if( ptr_k!=NULL && ptr_t->key == ptr_t->packet->seq_num && ptr_t->packet->HARQ_proccess_ID==HARQ_proccess_ID){
			if(ptr_k->key!=ptr_t->key){
			 //printf("Error data_req ptr_k->key!=ptr_t->key (%d, %d)\n",ptr_k->key,ptr_t->key);
			 LOG_E(MAC,"[MEM_MGT][WARNING] mac_buffer_data_req() mismatch/incompatible keys in the avl_trees\n");
			 mac_xface->macphy_exit("mac_buffer_data_req() mismatch/incompatible keys in the avl_trees");
			}
			if(ptr_k->packet!=ptr_t->packet){
			 //printf("Error data_req ptr_k->packet!=ptr_t->packet\n");
			 LOG_E(MAC,"[MEM_MGT][WARNING] mac_buffer_data_req() mismatch/incompatible packet in the avl_trees\n");
			 mac_xface->macphy_exit("mac_buffer_data_req() mismatch/incompatible packet in the avl_trees");
			}
			if(ptr_t->packet==help_head){
			 return  (mem_element_t*)(mac_buffer_remove_head(Mod_id, b_index, ptr_t, ptr_k));
			}
			else if(ptr_t->packet==help_tail){
			 return  (mem_element_t*)(mac_buffer_remove_tail(Mod_id, b_index, ptr_t, ptr_k));
			}
			else{			 
			 return (mem_element_t*) (mac_buffer_remove_middle(Mod_id, b_index, ptr_t->packet, ptr_t, ptr_k));
			}
		 }
		}else{
		 // I didn't find anything so I return NULL;
		 //printf("I did not find a packet anything\n");
		 LOG_E(MAC,"[WARNING] mac_buffer_data_req() packet not found for given seq_num %d and HARQ_proccess_ID %d \n",seq_num,HARQ_proccess_ID);
		 return  NULL;
		}
	 }
	 else if(!(requested_size<0)){
		 ptr_k = avl_tree_find_less_or_equal(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size, requested_size);
		 if(ptr_k!=NULL ){
			 ptr_t = ptr_k->next->packet->avl_node_pdu_seqn;
			 if(ptr_t!=NULL){
				if(ptr_k->next->packet==help_head){
				 return  (mem_element_t*)(mac_buffer_remove_head(Mod_id, b_index, ptr_t, ptr_k));
				}
			  else if(ptr_k->next->packet==help_tail){
				 return  (mem_element_t*)(mac_buffer_remove_tail(Mod_id, b_index, ptr_t, ptr_k));
				}
			  else{
				 return (mem_element_t*) (mac_buffer_remove_middle(Mod_id, b_index, ptr_k->next->packet, ptr_t, ptr_k));
				}
			 }
		 }
		 else{
			// I didn't find anything so I return NULL;
		 //printf("I did not find a packet anything\n");
		 LOG_E(MAC,"[WARNING] mac_buffer_data_req() packet not found for given requested_size %d\n",requested_size);
		 return  NULL;
		 }
	 }//end if requested_size<0 
	}//end else
}


void packet_list_add_tail_2(mem_element_t * elementP, packet_list_t * listP){
 mem_element_t *tail;
//-----------------------------------------------------------------------------
	if(elementP != NULL){
	 // access optimisation
	 elementP->next = NULL;
	 tail = listP->tail;
	 // almost one element
	 if(tail == NULL){
		elementP->previous = NULL;
		listP->head = elementP;
    }
    else{
		 tail->next = elementP;
		 elementP->previous = tail;
		}
    listP->tail = elementP;
		
		//elementP ->avl_node_pdu_seqn = NULL;
		//elementP ->avl_node_pdu_size = NULL;
		//printf("elementP->avl_node_pdu_size->key %d\n",elementP->avl_node_pdu_size->key);
    //printf("elementP->avl_node_pdu_size->second_key %d\n",elementP->avl_node_pdu_size->second_key);

		listP->total_size = listP->total_size + elementP->pdu_size;
    listP->nb_elements = listP->nb_elements + 1;
  }
  else{
    //msg("[CNT_LIST][ERROR] add_cnt_tail() element NULL\n");
     LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_add_tail_2() trying to add an uninitialized elementP in packet_list %s\n",listP->name);
  }
}


void packet_list_add_tail(mem_element_t * elementP, packet_list_t * listP){
 mem_element_t *tail;
//-----------------------------------------------------------------------------
	if(elementP != NULL){
	 // access optimisation
	 elementP->next = NULL;
	 tail = listP->tail;
	 // almost one element
	 if(tail == NULL){
		elementP->previous = NULL;
		listP->head = elementP;
    }
    else{
		 tail->next = elementP;
		 elementP->previous = tail;
		}
    listP->tail = elementP;
		
		elementP ->avl_node_pdu_seqn = NULL;
		elementP ->avl_node_pdu_size = NULL;
    
		listP->total_size = listP->total_size + elementP->pdu_size;
    listP->nb_elements = listP->nb_elements + 1;
  }
  else{
    //msg("[CNT_LIST][ERROR] add_cnt_tail() element NULL\n");
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_add_tail() trying to add an uninitialized elementP in packet_list %s\n",listP->name);
  }
}

void packet_list_add_head(mem_element_t *elementP, packet_list_t * listP){
//-----------------------------------------------------------------------------
// access optimisation;
 mem_element_t  *head;
 
 if (elementP != NULL){
	head = listP->head;
	// almost one element
	if(head == NULL){
	 elementP->next = NULL;
	 elementP->previous = NULL;
	 listP->head = elementP; 
	 listP->tail = elementP;
	}
	else{
	 elementP->next = head;
	 head->previous = elementP;
	 elementP->previous = NULL;
	 listP->head = elementP;
	}

	elementP->avl_node_pdu_seqn = NULL;
	elementP->avl_node_pdu_size = NULL;
	
	listP->nb_elements = listP->nb_elements + 1;
	listP->total_size = listP->total_size + elementP->pdu_size;
 }
}

void packet_list_add_after_ref(mem_element_t * new_elementP, mem_element_t *elementP_ref, packet_list_t * listP){
 mem_element_t *tail;
 //-----------------------------------------------------------------------------
 if(new_elementP != NULL && elementP_ref!=NULL){
	// access optimisation
	tail = listP->tail;
	// almost one element
	if(tail == NULL){
	 new_elementP->previous = NULL;
	 listP->head = new_elementP;
	}
	else{
	 new_elementP->next = elementP_ref->next;
	 new_elementP->previous = elementP_ref;
	 if(elementP_ref->next == NULL){
		listP->tail = new_elementP;
	 }
	 else{
		elementP_ref->next->previous = new_elementP;
	 }
	 elementP_ref->next = new_elementP;

	 new_elementP -> avl_node_pdu_seqn = NULL;
	 new_elementP -> avl_node_pdu_size = NULL;
	}
	listP->total_size = listP->total_size + new_elementP->pdu_size;
	listP->nb_elements = listP->nb_elements + 1;
 }
 else{
    //msg("[CNT_LIST][ERROR] add_cnt_tail() element NULL\n");
    LOG_E(MAC,"[MEM_MGT][WARNING] packet_list_add_after_ref() trying to add an uninitialized elementP and elementP_ref in packet_list %s\n",listP->name);
  }
}



int mac_buffer_return_b_index(u8 Mod_id, u8 eNB_index, u16 cornti){
 int i;
 u8 mode = mac_buffer_u[Mod_id].mode;
 
 if(mode == ONE_BUF_PER_CH){
	for(i=0;i<mac_buffer_u[Mod_id].total_number_of_buffers_allocated;i++){
	 if(mac_buffer_u[Mod_id].mac_buffer_g[i]->eNB_index == eNB_index){
		return i;
	 }
	}
 }
 else if (mode == ONE_BUF_PER_CORNTI){
	for(i=0;i<mac_buffer_u[Mod_id].total_number_of_buffers_allocated;i++){
	 if(mac_buffer_u[Mod_id].mac_buffer_g[i]->cornti == cornti){
		return i;
	 }
	}	
 }
 return -1; // did not find something
}


int mac_buffer_data_ind(u8 Mod_id, u8 eNB_index, u16 cornti, char *data, int seq_num, int pdu_size, int HARQ_proccess_ID){
 mem_element_t *elementP;
 
 u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
 elementP = malloc(sizeof(struct mem_element_t));
 
 if (elementP == NULL || mac_buffer_nb_elements(Mod_id, eNB_index, cornti)==MAC_BUFFER_MAXIMUM_CAPACITY){
	LOG_E(MAC," failed to allocate the memory or buffer overflow (maximum capacity is reached\n)");	
	return 0;
 }
 else{
	elementP->seq_num = seq_num;
	elementP->pdu_size = pdu_size;
	elementP->HARQ_proccess_ID = HARQ_proccess_ID;
	// ask Navid about the pool_id and data, if this values should be assigned here or outside this function!
	//elementP->pool_id = pool_id;
	//elementP->data = data;
	 if(mac_buffer_add_tail(Mod_id, b_index, elementP) == 1)
		return 1;
	 else
		return 0;
 }
}

int mac_buffer_add_tail(u8 Mod_id, u8 b_index, mem_element_t *elementP){
 avl_node_t *ptr_t;
 ptr_t=avl_tree_find(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, elementP->seq_num);
 if(ptr_t == NULL){ // pdu with the seq_num does not exist in the tree so I can add it into the list and then to the tree!
	packet_list_add_tail(elementP, mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p);
	mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn = avl_tree_insert_node(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_seqn, elementP, 1, 0, 0);
	mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size = avl_tree_insert_node_pdu_size(mac_buffer_u[Mod_id].mac_buffer_g[b_index]->tree_pdu_size, elementP);	
	return 1;
 }
 else{
	return 0;
 }
}


int  mac_buffer_total_size(u8 Mod_id, u8 eNB_index, u16 cornti){
 u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
 return mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->total_size;
}

int  mac_buffer_nb_elements(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
  return mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->nb_elements;
}


//DEBUGGING functions
void packet_list_print(packet_list_t *listP){
 mem_element_t *ptr_p;
 ptr_p = listP->head;
 if(ptr_p==NULL){
    printf("Empty help list!!!");
	 return; 
  }else{
    while(ptr_p!=NULL){
     // int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
     // printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      printf("ptr_p->avl_node_pdu_size->key ptr_p->avl_node_pdu_size->second_key %d %d \n",ptr_p->avl_node_pdu_size->key,ptr_p->avl_node_pdu_size->second_key);
			ptr_p = ptr_p->next;
    }
  }
}

void mac_buffer_print(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
  mem_element_t *ptr_p;
  ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
	printf("\n\n%s | %s \n",mac_buffer_u[Mod_id].mac_buffer_g[b_index]->name,mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id, eNB_index, cornti), mac_buffer_nb_elements(Mod_id, eNB_index, cornti));
  if(ptr_p==NULL){
    printf("Empty help list!!!");
	 return; 
  }else{
    while(ptr_p!=NULL){
     // int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      ptr_p = ptr_p->next;
    }
  }
}

void mac_buffer_print_reverse(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti); 
  mem_element_t *ptr_p, *head, *tail;
	tail = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->tail;
  ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->tail;
	head = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;

	printf("\n\nREVERSE| %s | %s \n",mac_buffer_u[Mod_id].mac_buffer_g[b_index]->name, mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id, eNB_index, cornti), mac_buffer_nb_elements(Mod_id, eNB_index, cornti));
  if(ptr_p==NULL){
	  printf("Empty help list!!!");
    return; 
  }else{
    while(ptr_p!=head){
     // int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      ptr_p = ptr_p->previous;
    }
    printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
  }
  if(head->next!=NULL){
	 if(tail->previous->next!=tail){printf("provlima\n");}
	}
}

void mac_buffer_print_2(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti); 
  mem_element_t *ptr_p;
  ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
	printf("\n\n%s | %s \n",mac_buffer_u[Mod_id].mac_buffer_g[b_index]->name, mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id, eNB_index, cornti), mac_buffer_nb_elements(Mod_id, eNB_index, cornti));
  if(ptr_p==NULL){
    printf("Empty list!!!");
	 return; 
  }else{
    while(ptr_p!=NULL){
      //int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      if(ptr_p->avl_node_pdu_seqn == NULL){
			 printf("Error 1a: ptr_p->avl_node_pdu_seqn == NULL\n");
			}
			if(ptr_p->avl_node_pdu_size == NULL){
			 printf("Error 1a: ptr_p->avl_node_pdu_seqn == NULL\n");
			}
			if(ptr_p->avl_node_pdu_seqn->packet!=ptr_p){
			 printf("Error 2a: ptr_p->avl_node_pdu_seqn->packet!=ptr_p \n");
			}
			if(ptr_p->avl_node_pdu_size->packet!=ptr_p){
			 printf("Error 2b: ptr_p->avl_node_pdu_size->packet!=ptr_p \n");
			}
			if(ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num){
			 printf("Error 3a: ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_seqn->key,ptr_p->seq_num);
			}
			if(ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num){
			  printf("Error 3b: ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_size->second_key, ptr_p->seq_num);
			}
			ptr_p = ptr_p->next;
    }
  }
}

void mac_buffer_print_3(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti);
  mem_element_t *ptr_p;
  ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
	int flag=0;
	printf("\n\n%s | %s \n", mac_buffer_u[Mod_id].mac_buffer_g[b_index]->name, mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id, eNB_index, cornti), mac_buffer_nb_elements(Mod_id, eNB_index, cornti));
  if(ptr_p==NULL){
    printf("Empty list!!!");
	 return; 
  }else{
    while(ptr_p!=NULL){
      //int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      //printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      if(ptr_p->avl_node_pdu_seqn == NULL){
			 flag=1;
			 //printf("Error a: ptr_p->avl_node_pdu_seqn == NULL\n");
			 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
			}
			
			if(ptr_p->avl_node_pdu_seqn->pdu_seq_num_tree == 1){
			 flag=1;
				if(ptr_p->avl_node_pdu_seqn->packet!=ptr_p){
				 flag=1;
				 //printf("Error a1: ptr_p->avl_node_pdu_seqn->packet!=ptr_p \n");
				 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
				}
				if(ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num){
				 flag=1;
				 //printf("Error a2: ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_seqn->key,ptr_p->seq_num);
				 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 			 
			 }
			}
			
			if(ptr_p->avl_node_pdu_size == NULL){
			 flag=1;
			 //printf("Error b: ptr_p->avl_node_pdu_seqn == NULL\n");
			 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
			 if(ptr_p->avl_node_pdu_size->pdu_size_tree == 1){
				flag=1;
				if(ptr_p->avl_node_pdu_size->packet!=ptr_p){flag=1;
				 //printf("Error b1: ptr_p->avl_node_pdu_size->packet!=ptr_p \n");
				 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
				}
				if(ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num){flag=1;
				 //printf("Error b2: ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_size->second_key,ptr_p->seq_num);
				 //printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				}
			 }
			 if(ptr_p->avl_node_pdu_size->pdu_size_tree_in_next == 1){
				 flag=1;
				 if(ptr_p->avl_node_pdu_size->packet!=ptr_p){
					flag=1;
					//printf("Error c1: ptr_p->avl_node_pdu_size->packet!=ptr_p \n");
					//printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				 }
				 if(ptr_p->avl_node_pdu_size->key!=ptr_p->seq_num){
					flag=1;
					//printf("Error c2: ptr_p->avl_node_pdu_size->key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_size->key,ptr_p->seq_num);
					//printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				 }
				}
			 }
			 
			ptr_p = ptr_p->next;
    }
  }
  if(flag==1){
	 ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
	 //printf("\n\n%s | %s \n",mac_buffer_g[Mod_id]->name,mac_buffer_g[Mod_id]->my_p->name);
	 //printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id), mac_buffer_nb_elements(Mod_id));
	  while(ptr_p!=NULL){
      //int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      //printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      if(ptr_p->avl_node_pdu_seqn == NULL){
			 flag=1;
			 printf("Error a: ptr_p->avl_node_pdu_seqn == NULL\n");
			 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
			}
			
			if(ptr_p->avl_node_pdu_seqn->pdu_seq_num_tree == 1){
			 flag=1;
				if(ptr_p->avl_node_pdu_seqn->packet!=ptr_p){
				 flag=1;
				 printf("Error a1: ptr_p->avl_node_pdu_seqn->packet!=ptr_p \n");
				 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
				}
				if(ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num){
				 flag=1;
				 printf("Error a2: ptr_p->avl_node_pdu_seqn->key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_seqn->key,ptr_p->seq_num);
				 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 			 
			 }
			}
			
			if(ptr_p->avl_node_pdu_size == NULL){
			 flag=1;
			 printf("Error b: ptr_p->avl_node_pdu_seqn == NULL\n");
			 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
			 if(ptr_p->avl_node_pdu_size->pdu_size_tree == 1){
				flag=1;
				if(ptr_p->avl_node_pdu_size->packet!=ptr_p){flag=1;
				 printf("Error b1: ptr_p->avl_node_pdu_size->packet!=ptr_p \n");
				 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
				}
				if(ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num){flag=1;
				 printf("Error b2: ptr_p->avl_node_pdu_size->second_key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_size->second_key,ptr_p->seq_num);
				 printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				}
			 }
			 if(ptr_p->avl_node_pdu_size->pdu_size_tree_in_next == 1){
				 flag=1;
				 if(ptr_p->avl_node_pdu_size->packet!=ptr_p){
					flag=1;
					printf("Error c1: ptr_p->avl_node_pdu_size->packet!=ptr_p \n");
					printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				 }
				 if(ptr_p->avl_node_pdu_size->key!=ptr_p->seq_num){
					flag=1;
					printf("Error c2: ptr_p->avl_node_pdu_size->key!=ptr_p->seq_num %d, %d \n",ptr_p->avl_node_pdu_size->key,ptr_p->seq_num);
					printf("Error --> Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
				 }
				}
			 }
			ptr_p = ptr_p->next;
    }
 }
}


void mac_buffer_print_4(u8 Mod_id, u8 eNB_index, u16 cornti){
  u8 b_index=mac_buffer_return_b_index(Mod_id, eNB_index, cornti); 
  mem_element_t *ptr_p, *ptr_h1,* ptr_h2;
	avl_node_t *ptr_r1,* ptr_r2;
  ptr_p = mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->head;
	
	int flag=0;
	printf("\n\n%s | %s \n",  mac_buffer_u[Mod_id].mac_buffer_g[b_index]->name, mac_buffer_u[Mod_id].mac_buffer_g[b_index]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id, eNB_index, cornti), mac_buffer_nb_elements(Mod_id, eNB_index, cornti));
  if(ptr_p==NULL){
    printf("Empty list!!!");
	 return; 
  }else{
    while(ptr_p!=NULL){
			ptr_h1 = ptr_p->avl_node_pdu_seqn->packet;
			ptr_h2 = ptr_p->avl_node_pdu_size->packet;
			ptr_r1 = ptr_p->avl_node_pdu_seqn;
			ptr_r2 = ptr_p->avl_node_pdu_size;
			if(ptr_p!=ptr_h1){
			  printf("ptr_p->seq_num %d\n",ptr_p->seq_num);
			 printf("Error 1: ptr_p != ptr_p->avl_node_pdu_seqn->packet\n");
			}
			if(ptr_p!=ptr_h2){
			 printf("ptr_p->seq_num %d\n",ptr_p->seq_num);
			 printf("Error 2: ptr_p != ptr_p->avl_node_pdu_size->packet\n");
			}
			if(ptr_p->seq_num!=ptr_r1->key || ptr_p->seq_num!=ptr_r2->key){
			 printf("ptr_p->seq_num %d\n",ptr_p->seq_num);
			 printf("Error 3: ptr_p->seq_num!=ptr_r1->key (%d, %d)|| ptr_p->seq_num!=ptr_r2->key (%d, %d)\n",ptr_p->seq_num, ptr_r1->key, ptr_p->seq_num, ptr_r2->key);
			}
			if(ptr_p->pdu_size!=ptr_r1->second_key || ptr_p->pdu_size!=ptr_r2->second_key){
			 printf("ptr_p->seq_num %d\n",ptr_p->seq_num);
			 printf("Error 4: ptr_p->pdu_size!=ptr_r1->second_key (%d, %d)|| ptr_p->pdu_size!=ptr_r2->second_key (%d, %d)\n",ptr_p->pdu_size, ptr_r1->second_key, ptr_p->pdu_size, ptr_r2->second_key);
			}
			ptr_p = ptr_p->next;
    }
  }
 
}





