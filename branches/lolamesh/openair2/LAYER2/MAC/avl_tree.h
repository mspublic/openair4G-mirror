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

/*! \file avl_tree.h
* \brief structure and data types for supporting avl_tree storage, search in the mec_buffer according to packets pdu_size and seq_num 
* \author apostolos apostolaras and navid nikaein 
* \date 2013
* \version 0.1
* @ingroup _mac
*/

#ifndef __AVL_TREE__
#define __AVL_TREE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "forwarding_buffer.h"


typedef struct avl_node_t{

 struct avl_node_t *left;
 struct avl_node_t *right;
 struct mem_element_t *packet;
 struct avl_node_t *next; //used only in the case of pdu_size_tree!
 
 int pdu_seq_num_tree; // binary variables 0 or 1 to classify whether a particular avl_node belongs to a particular tree
 int pdu_size_tree; 
 int pdu_size_tree_in_next;
 
 int toDelete; // is used to identify whether should delete a node in pdu_size_tree only 1 or 0
 
 int height;
 int key;           
 int second_key;    
 // in pdu_seq_num tree the key is pdu_seq_num and the second_key is the pdu_size
 // in pdu_size tree the key is pdu_size and the second_key is the pdu_seq_num

}avl_node_t;


// AVL TREE API FUNCTIONS

avl_node_t *avl_tree_make_empty(avl_node_t *tree_root); // Parameter the initial tree_root // Returns the new root
avl_node_t *avl_tree_find(avl_node_t *tree_root, int key); // Parameter the initial tree_root // Returns the new root
avl_node_t *avl_tree_find_min(avl_node_t *tree_root);// Parameter the initial tree_root // Returns the new root
avl_node_t *avl_tree_find_max(avl_node_t *tree_root);// Parameter the initial tree_root // Returns the new root
avl_node_t *avl_tree_find_less_or_equal(avl_node_t *t, int value); // Parameter the initial tree_root and a value // Returns the node on the tree with key==value or the node that is max(keys)<alues

avl_node_t *avl_tree_insert_node(avl_node_t *t, struct mem_element_t *elementP, int pdu_seq_num_tree, int pdu_size_tree, int pdu_size_tree_in_next);
avl_node_t *avl_tree_insert_node_pdu_size(avl_node_t *tree_root, struct mem_element_t *elem);
avl_node_t *avl_tree_delete_node(avl_node_t *tree_root, int key, int pdu_seq_num_tree, int pdu_size_tree, int pdu_size_tree_in_next);// Parameter the initial tree_root // Returns the new root and the deleted node ptr (pass by reference)
avl_node_t *avl_tree_delete_node_pdu_size_tree(avl_node_t *t, int key, int second_key);

// INTERNAL FUNCTIONS
int avl_tree_height(avl_node_t *tree_node);
int Max(int Lhs, int Rhs);
int avl_tree_get_balance(avl_node_t *tree_root);
avl_node_t *avl_tree_balance(avl_node_t *t);
avl_node_t *avl_tree_balance_2(avl_node_t *t, int key);
avl_node_t *avl_tree_single_rotate_with_left(avl_node_t *K2);
avl_node_t *avl_tree_single_rotate_with_right(avl_node_t *K1);
avl_node_t *avl_tree_double_rotate_with_left(avl_node_t *K3);
avl_node_t *avl_tree_double_rotate_with_right(avl_node_t *K1);


// HELP FUNCTIONS
int avl_tree_get_key(avl_node_t *tree_root); //Parameter the tree root
void avl_tree_pre_order_print(avl_node_t *tree_root); //Parameter the tree root
void avl_tree_pre_order_print_3(avl_node_t *t, int select, int *j); //select 0 for pdu_seq_num tree and 1 for pdu_size_tree
void avl_tree_level_order(avl_node_t *t);
//void avl_tree_level_order_print_pdu_size(avl_node_t *t, packet_list_t *q);
//void avl_tree_level_order_print_pdu_seqn(avl_node_t *t, packet_list_t *q);
 
#endif