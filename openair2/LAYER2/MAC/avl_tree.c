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

/*! \file avl_tree.c
* \brief primitives for supporting avl_tree 
* \author apostolos apostolaras and navid nikaein 
* \date 2013
* \version 0.1
* @ingroup _mac
*/



#include <stdlib.h>

#include "extern.h"
#include "defs.h"
#include "UTIL/LOG/log.h"

#include "avl_tree.h"
#include "forwarding_buffer.h"



avl_node_t * avl_tree_make_empty(avl_node_t *t){
 if(t != NULL){
	avl_tree_make_empty(t->left);
	avl_tree_make_empty(t->right);
	free(t);
 }
 return NULL;
}
 
avl_node_t *avl_tree_find(avl_node_t *t, int key){
 if(t==NULL)
	return NULL;
 if(key < t->key)
	return avl_tree_find(t->left, key);
 else
	if(key > t->key)
	 return avl_tree_find(t->right, key);
	else
	 return t;
}
 
avl_node_t *avl_tree_find_less_or_equal(avl_node_t *t, int value){
 avl_node_t *ptr = t;
 avl_node_t *ptr_ind=NULL;
 int ref=-1;
 if(ptr == NULL){
	return NULL;
 }
 else{
	while(ptr!=NULL){
	 if(ptr->key == value){
	 return ptr;
	 }
	 else if(value < ptr->key){
		ptr=ptr->left;
	 }
	 else if(value > ptr->key){
		if(ref < value && ref < ptr->key ){
		 ref=ptr->key;
		 ptr_ind=ptr;
		}
		ptr=ptr->right;
	 }
	}
	return ptr_ind;
 }
}

avl_node_t *avl_tree_find_min(avl_node_t *t){
 if(t == NULL)
	return NULL;
 else
	if(t->left == NULL)
	 return t;
	else
	 return avl_tree_find_min(t->left);
}
 
avl_node_t *avl_tree_find_max(avl_node_t *t){
 if(t != NULL)
	while( t->right != NULL )
	 t = t->right;
	return t;
}
 
int avl_tree_height(avl_node_t *tree_node){
 if(tree_node == NULL)
	return 0;
 else
	return tree_node->height;
}
 
int Max(int Lhs, int Rhs){
    return Lhs > Rhs ? Lhs : Rhs;
}

int avl_tree_get_balance(avl_node_t *t){
 if(t == NULL)
	return 0;
 return avl_tree_height(t->left) - avl_tree_height(t->right);
}
 
/* This function can be called only if K2 has a left child */
/* Perform a rotate between a node (K2) and its left child */
/* Update heights, then return new root */
 
avl_node_t *avl_tree_single_rotate_with_left(avl_node_t *K2){
 avl_node_t *K1;
 
 K1 = K2->left;
 K2->left = K1->right;
 K1->right = K2;
 
 K2->height = Max( avl_tree_height(K2->left), avl_tree_height(K2->right) ) + 1;
 K1->height = Max( avl_tree_height(K1->left), K2->height ) + 1;
 
 return K1;  /* New root */
}
 
/* This function can be called only if K1 has a right child */
/* Perform a rotate between a node (K1) and its right child */
/* Update heights, then return new root */
 
avl_node_t *avl_tree_single_rotate_with_right(avl_node_t *K1){
 avl_node_t *K2;
 
 K2 = K1->right;
 K1->right = K2->left;
 K2->left = K1;
 
 K1->height = Max( avl_tree_height(K1->left), avl_tree_height(K1->right) ) + 1;
 K2->height = Max( avl_tree_height(K2->right), K1->height ) + 1;
 
 return K2;  /* New root */
}
 
/* This function can be called only if K3 has a left */
/* child and K3's left child has a right child */
/* Do the left-right double rotation */
/* Update heights, then return new root */
 
avl_node_t *avl_tree_double_rotate_with_left(avl_node_t *K3){
 /* Rotate between K1 and K2 */
 K3->left = avl_tree_single_rotate_with_right(K3->left);
 
 /* Rotate between K3 and K2 */
 return avl_tree_single_rotate_with_left(K3);
}
 
/* This function can be called only if K1 has a right */
/* child and K1's right child has a left child */
/* Do the right-left double rotation */
/* Update heights, then return new root */
 
avl_node_t *avl_tree_double_rotate_with_right(avl_node_t *K1){
 /* Rotate between K3 and K2 */
 K1->right = avl_tree_single_rotate_with_left(K1->right);
 /* Rotate between K1 and K2 */
 return avl_tree_single_rotate_with_right(K1);
}
 
 
avl_node_t *avl_tree_balance(avl_node_t *t){
   // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
	 t->height = Max(avl_tree_height(t->left), avl_tree_height(t->right)) + 1;
	 // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to check whether //  this node became unbalanced)
	 int balance = avl_tree_get_balance(t);
	 
	 // If this node becomes unbalanced, then there are 4 cases
	 // Left Left Case
	 if (balance > 1 && avl_tree_get_balance(t->left) >= 0){
		return avl_tree_single_rotate_with_left(t);
	 }
	 // Left Right Case
	 if (balance > 1 && avl_tree_get_balance(t->left) < 0){
		return avl_tree_double_rotate_with_left(t);
	 }
	 // Right Right Case
	 if (balance < -1 && avl_tree_get_balance(t->right) <= 0){
		return avl_tree_single_rotate_with_right(t);
	 }
		// Right Left Case
	 if (balance < -1 && avl_tree_get_balance(t->right) > 0){
		 return avl_tree_double_rotate_with_right(t);
	 }
	 return t;
}

avl_node_t *avl_tree_balance_2(avl_node_t *t, int key){
  // 2. Update height of this ancestor node 
 t->height = Max(avl_tree_height(t->left), avl_tree_height(t->right)) + 1;
 
 // 3. Get the balance factor of this ancestor node to check whether
 //     this node became unbalanced 
 int balance = avl_tree_get_balance(t);
 
 // If this node becomes unbalanced, then there are 4 cases
 
 // Left Left Case
 if (balance > 1 && key < t->left->key){
	return avl_tree_single_rotate_with_left(t);
 }
 // Right Right Case
 if (balance < -1 && key > t->right->key){
	return avl_tree_single_rotate_with_right(t);
 }
 // Left Right Case
 if (balance > 1 && key > t->left->key){
    return avl_tree_double_rotate_with_left(t);
 }
 // Right Left Case
 if (balance < -1 && key < t->right->key){
    return avl_tree_double_rotate_with_right(t);
 }
 // return the (unchanged) node pointer 
 return t;
 
}

avl_node_t *avl_tree_insert_node(avl_node_t *t, struct mem_element_t *elementP, int pdu_seq_num_tree, int pdu_size_tree, int pdu_size_tree_in_next){
 
  if(elementP==NULL){
	 return NULL;
	}
  int key = elementP->seq_num;
	int second_key = elementP->pdu_size;
  // 1.  Perform the normal BST rotation 
 if(t == NULL){
	//	 Create and return a one-node tree 
	t = malloc(sizeof(struct avl_node_t));
	if(t == NULL){
	 //printf("Insufficient Memory - Allocation Error! avl_tree_insert_node()!\n");
	 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure  for avl_tree_insert_node()\n");
   mac_xface->macphy_exit("out of memory for for avl_tree_insert_node()");
	 return NULL;
	}
	else{
	 t->height = 1;
	 t->key = key; 
	 t->second_key= second_key;
	 t->left = t->right = NULL;
	 t->pdu_seq_num_tree = pdu_seq_num_tree;
	 t->pdu_size_tree = pdu_size_tree;
	 t->pdu_size_tree_in_next = pdu_size_tree_in_next;
   t->toDelete=0;
	 if(pdu_seq_num_tree == 1){
		elementP -> avl_node_pdu_seqn = t;
		t->next = NULL;
	 }
	 if(pdu_size_tree_in_next == 1){
		elementP -> avl_node_pdu_size = t; 
	 }
	 t->packet = elementP;
	 return t;
	}
 }
 else{
	if(key < t->key){
	 t->left  = avl_tree_insert_node(t->left, elementP, pdu_seq_num_tree, pdu_size_tree, pdu_size_tree_in_next);
	}
	else{
	 t->right = avl_tree_insert_node(t->right, elementP, pdu_seq_num_tree, pdu_size_tree, pdu_size_tree_in_next);
	}
 }
 t = avl_tree_balance_2(t, key);
 return t;
} 

avl_node_t *avl_tree_insert_node_pdu_size(avl_node_t *t, mem_element_t *elementP){
	if(elementP==NULL){
	 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure  in avl_tree_insert_node_pdu_size(), trying to add a NULL elementP\n");
   mac_xface->macphy_exit("elementP is NULL in avl_tree_insert_node_pdu_size()");
	 return NULL;
	}
	int flag_a = 1;
	int key = elementP->pdu_size;
	int second_key = elementP->seq_num;
 // 1.  Perform the normal BST rotation 
 if(t == NULL){
	//	 Create and return a one-node tree 
	t = malloc(sizeof(struct avl_node_t));
	if(t == NULL){
	 LOG_E(MAC,"[MEM_MGT][WARNING] Memory allocation failure  for avl_tree_insert_node_pdu_size()\n");
   mac_xface->macphy_exit("out of memory for for avl_tree_insert_node_pdu_size()");
	 return NULL;
	}
	else{
	 //printf("new node key, second_key (%d, %d)",key,second_key);
	 t->key = key;
	 t->second_key = second_key;
	 t->height = 1;
	 t->left = t->right = NULL;
	 t->pdu_seq_num_tree = 0;
	 t->pdu_size_tree= 1;
	 t->pdu_size_tree_in_next = 0;
	 t->next = avl_tree_insert_node(t->next, elementP, 0, 0, 1);
  // printf("-->%d %d\n",t->next->key,t->next->second_key);
	 }
 }
 else{
	if(key < t->key){
	 t->left  = avl_tree_insert_node_pdu_size(t->left, elementP);
	}
	else if(key > t->key){
	 t->right = avl_tree_insert_node_pdu_size(t->right, elementP);
	}
	else if(key == t->key){
	 t->next = avl_tree_insert_node(t->next, elementP, 0, 0, 1);
	 if(t->next->key != t->second_key){
		t->key = t->next->second_key;
		t->second_key = t->next->key;
	 }
	}
 }
 
 t = avl_tree_balance_2(t, key);
 return t;
}
 
avl_node_t *avl_tree_delete_node(avl_node_t *t, int key, int pdu_seq_num_tree, int pdu_size_tree, int pdu_size_tree_in_next){

 if (t == NULL){
	return t;
 }
 if (key < t->key) {
	t->left = avl_tree_delete_node(t->left, key, pdu_seq_num_tree, pdu_size_tree, pdu_size_tree_in_next);
 }
 else if ( key > t->key){
	t->right = avl_tree_delete_node(t->right, key, pdu_seq_num_tree, pdu_size_tree, pdu_size_tree_in_next);
 }
 else if(key == t->key){
  
	if ((t->left == NULL) && (t->right != NULL)){
   avl_node_t *remove_node;
	 remove_node = t->right;
	// *t = *remove_node;
   //memcpy(t,remove_node,sizeof(struct avl_node_t));
   t = remove_node;
   t->left=remove_node->left;
   t->right=remove_node->right;
   t->height=remove_node->height;
   t->key=remove_node->key; 
   t->second_key = remove_node->second_key; 
	 if(pdu_seq_num_tree==1){
			remove_node->packet->avl_node_pdu_seqn = t;
      t->next=NULL;
	 }
	 if(pdu_size_tree_in_next==1){
		remove_node->packet->avl_node_pdu_size=t;
		 t->next=remove_node->next;
	 }
	 t->packet = remove_node -> packet;
  // free(remove_node->next);
//   free(remove_node); // this is for free-ing the memory
	}
	else if((t->right == NULL)  && (t->left != NULL)){
   avl_node_t *remove_node;
	 remove_node = t->left;
	// *t = *remove_node;
  // memcpy(t,remove_node,sizeof(struct avl_node_t));
   t->left=remove_node->left;
   t->right=remove_node->right;
   t->height=remove_node->height;
   t->key=remove_node->key; 
   t->second_key = remove_node->second_key; 
	 if(pdu_seq_num_tree==1){
		 remove_node->packet->avl_node_pdu_seqn = t;
      t->next=NULL;
	 }
	 if(pdu_size_tree_in_next==1){
    remove_node->packet->avl_node_pdu_size=t; 
    t->next=remove_node->next;
	 }
	 t->packet = remove_node -> packet;
  // free(remove_node->next);
	// free(remove_node);
	}
	else if ((t->right == NULL)  && (t->left == NULL)){
   avl_node_t *remove_node;
	 remove_node = t;
	 t = NULL;
	}
	else{
    avl_node_t *remove_node;
	 remove_node =  avl_tree_find_min(t->right);
	 t->key = remove_node->key;
	 t->second_key = remove_node->second_key;

	 
	 if(pdu_seq_num_tree==1){
		 remove_node->packet->avl_node_pdu_seqn =t;  
	 }
	 if(pdu_size_tree_in_next==1){
		 remove_node->packet->avl_node_pdu_size=t; 
	 }
	 t->packet = remove_node -> packet; 
		t->next = remove_node -> next;
  	t->right = avl_tree_delete_node(t->right, remove_node->key, pdu_seq_num_tree, pdu_size_tree, pdu_size_tree_in_next); 

	}
 }
  if (t == NULL){
	return t;
 }
 t = avl_tree_balance(t);
 return t;
}

avl_node_t *avl_tree_delete_node_pdu_size_tree(avl_node_t *t, int key, int second_key){
 int flag =1;

 if (t == NULL){
	return t;
 }
 if (key < t->key) {
	t->left = avl_tree_delete_node_pdu_size_tree(t->left, key, second_key);
 }
 else if ( key > t->key){
	t->right = avl_tree_delete_node_pdu_size_tree(t->right, key, second_key);
 }
 else{
	 /*if(t->key==394){
		printf("t->next->key %d, t->next->second_key %d\n",t->next->key,t->next->second_key);
		printf("t->toDelete %d \n",t->toDelete);
	 }*/
   if(t->next!=NULL && t->pdu_size_tree==1 &&  t->toDelete==0){
   t->next = avl_tree_delete_node(t->next, second_key, 0,0,1);
   if(t->next!=NULL && t->next->key != t->second_key){
    t->key = t->next->second_key;
    t->second_key = t->next->key;
    t->packet=NULL;
    flag=0;
   }
   if(t->next==NULL){
    t->toDelete=1;
   }
  }
  if(t->pdu_size_tree==1 && t->toDelete==1  ){
   if ((t->left == NULL) && (t->right != NULL)){
    avl_node_t *remove_node;
    remove_node = t->right;
//    *t = *remove_node;
  //  memcpy(t,remove_node,sizeof(struct avl_node_t));
    if(t->pdu_size_tree==1){
     t->key=remove_node->key;
     t->second_key = remove_node->second_key;
     t->left=remove_node->left;
     t->right=remove_node->right;
     t->height=remove_node->height;
     t->next = remove_node->next;
		 t->toDelete = 1 - remove_node->toDelete;
     }
    free(remove_node);// this is for free-ing the memory
   }
   else if((t->right == NULL)  && (t->left != NULL)){
    avl_node_t *remove_node;
    remove_node = t->left;
 //   *t = *remove_node;
 //    memcpy(t,remove_node,sizeof(struct avl_node_t));
    if(t->pdu_size_tree==1){
     t->key=remove_node->key;
     t->second_key = remove_node->second_key;
     t->left=remove_node->left;
     t->right=remove_node->right;
     t->height=remove_node->height;
     t->next = remove_node->next;
		 t->toDelete = 1 - remove_node->toDelete;
     }
    free(remove_node);
   }
   else if ((t->right == NULL)  && (t->left == NULL)){
    avl_node_t *remove_node;
    remove_node = t;
    t = NULL;
   }
   else{
    avl_node_t *remove_node;
    remove_node =  avl_tree_find_min(t->right);
    t->key = remove_node->key;
    t->second_key = remove_node->second_key;
    t->next = remove_node -> next;
    remove_node->toDelete=1;
    t->toDelete = 1 - remove_node->toDelete;
		t->right = avl_tree_delete_node_pdu_size_tree(t->right, remove_node->key, remove_node->second_key);
   }
  }
 }
 
  if (t == NULL){
	return t;
 }
 if(flag==1){
	t = avl_tree_balance(t);
 }
	 return t;
 
}

int avl_tree_get_key(avl_node_t *t){
    return t->key;
}
// debugging functions
void avl_tree_pre_order_print(avl_node_t *t){
 if(t != NULL){
	printf("(%d |%d)", t->key,t->second_key);
	avl_tree_pre_order_print(t->left);
	avl_tree_pre_order_print(t->right);
 }
}

void avl_tree_pre_order_print_3(avl_node_t *t, int select, int *j){
 avl_node_t *ptr;
 if(t != NULL){
	if(select == 0){ //pdu_seq_num_tree
	 if(t->next==NULL){
		printf("(%d) ",t->key);
	 }
	 if(t->pdu_seq_num_tree == 1 && t->packet->avl_node_pdu_seqn!=t ){
		 printf("\n Error a: t->packet->avl_node_pdu_seqn!=t (t->packet->seq_num %d t->packet->avl_node_pdu_seqn->key %d)\n",t->packet->seq_num,t->packet->avl_node_pdu_seqn->key);
	 }
	 if(t->pdu_size_tree_in_next == 1 && t->packet->avl_node_pdu_size!=t ){
//		 printf("\n t->pdu_size_tree_in_next %d\n",t->pdu_size_tree_in_next);
 	 	 printf("\n Error b: t->packet->avl_node_pdu_size!=t \n");
//	 	 printf("\n1st level Error: t->packet->avl_node_pdu_size!=t (t->packet->seq_num %d t->packet->avl_node_pdu_size->key %d)\n",t->packet->seq_num,t->packet->avl_node_pdu_size->key);
	 }
	}else if(select==1){ //pdu_size_tree
	  printf("%d) [ (%d, %d) --> ",*j, t->key, t->second_key );
		avl_tree_pre_order_print_3(t->next,0, j);
		printf("] \n");
		(*j)++;
	}
	avl_tree_pre_order_print_3(t->left, select, j);
	avl_tree_pre_order_print_3(t->right,select, j);
 }
}


void avl_tree_level_order_print_pdu_seqn(avl_node_t *t, packet_list_t *q){
 mem_element_t tmp,temp[2];// = malloc(sizeof(struct mem_element_t));
 temp[0].avl_node_pdu_seqn = malloc(sizeof(struct avl_node_t));
 temp[1].avl_node_pdu_seqn = malloc(sizeof(struct avl_node_t));
 avl_node_t *ref;
 
 mem_element_t *packet;
 
 if(t != NULL){
	memcpy(&temp[0],t->packet,sizeof(struct mem_element_t));
  packet_list_add_tail_2(&temp[0], q);	
	int nodesInCurrentLevel = 1;
  int nodesInNextLevel = 0;
	
	while(q->nb_elements!=0){
		packet = packet_list_remove_head_2(q);
		memcpy(&temp[1],packet,sizeof(struct mem_element_t));		
    nodesInCurrentLevel--;
		printf("(%d) ",temp[1].avl_node_pdu_seqn->key);
		if(temp[1].avl_node_pdu_seqn->left!=NULL){
		 	packet_list_add_tail_2(temp[1].avl_node_pdu_seqn->left->packet, q);
		}
		if(temp[1].avl_node_pdu_seqn->right!=NULL){
		 packet_list_add_tail_2(temp[1].avl_node_pdu_seqn->right->packet, q);
		}
		nodesInNextLevel += 2;
		if (nodesInCurrentLevel == 0) {
      printf("\n");
      nodesInCurrentLevel = nodesInNextLevel;
      nodesInNextLevel = 0;
    }
	} 
 }
}

void avl_tree_level_order_print_pdu_size(avl_node_t *t, packet_list_t *q){
 mem_element_t tmp,temp[2];// = malloc(sizeof(struct mem_element_t));
 temp[0].avl_node_pdu_size = malloc(sizeof(struct avl_node_t));
 temp[1].avl_node_pdu_size = malloc(sizeof(struct avl_node_t));
 avl_node_t *ref;
 
 mem_element_t *packet;
 
 if(t != NULL){
	memcpy(&temp[0],t->packet,sizeof(struct mem_element_t));
  packet_list_add_tail_2(&temp[0], q);	
	int nodesInCurrentLevel = 1;
  int nodesInNextLevel = 0;
	
	while(q->nb_elements!=0){
		packet = packet_list_remove_head_2(q);
		memcpy(&temp[1],packet,sizeof(struct mem_element_t));		
    nodesInCurrentLevel--;
		printf("(%d, %d) ",temp[1].avl_node_pdu_size->key, temp[1].avl_node_pdu_size->second_key);
		if(temp[1].avl_node_pdu_size->left!=NULL){
		 	packet_list_add_tail_2(temp[1].avl_node_pdu_size->left->packet, q);
		}
		if(temp[1].avl_node_pdu_size->right!=NULL){
		 packet_list_add_tail_2(temp[1].avl_node_pdu_size->right->packet, q);
		}
		nodesInNextLevel += 2;
		if (nodesInCurrentLevel == 0) {
      printf("\n");
      nodesInCurrentLevel = nodesInNextLevel;
      nodesInNextLevel = 0;
    }
	} 
 }
}

void avl_tree_level_order(avl_node_t *t){
 MAC_BUFFER *mac_q;
 packet_list_t *q;
 mac_q = mac_buffer_init("q","q",1);
 q=mac_q->my_p;
 if(t->second_key!=-1){
	printf("LEVEL ORDER -- PDU SIZE\n");
	avl_tree_level_order_print_pdu_size(t, q);
 }
 else{
	printf("LEVEL ORDER -- PDU SEQN\n");
	avl_tree_level_order_print_pdu_seqn(t, q);
 }
	packet_list_free (q);
}