//#define NULL 0

//#include "PHY/types.h"
//#include "PHY/defs.h"
//#include "PHY/vars.h"
#include <stdlib.h>

#include "extern.h"
#include "defs.h"

#include "forwarding_buffer.h"

void mac_buffer_top_init(){
  u8 UE_id;
	
	int sorting_flag = SORT_PDU_SEQN;//SORT_FIFO; //SORT_PDU_SEQN or SORT_PDU_SIZE!
	
	char *string=malloc(sizeof(char)*3);
	char *string1=malloc(sizeof(char)*23);
	char *string2=malloc(sizeof(char)*23);
	
  mac_buffer_g = malloc(NB_UE_INST*sizeof(MAC_BUFFER));
	if(mac_buffer_g==NULL)
	{
	 //msg("[MEM_MGT][WARNING] Memory allocation failure for mac_buffer/mac_buffer_top_init");
	}
	for (UE_id=0; UE_id<NB_UE_INST;UE_id++){  
	  sprintf(string,"%d",UE_id);
		strcpy(string1,"mac_buffer id: ");
		strcpy(string2,"packet_list id: ");
		strcat(string1,string);
		strcat(string2,string);
    mac_buffer_g[UE_id] = mac_buffer_init(string1,string2,UE_id,sorting_flag);
		string[0]='\0'; // flush string buffer
		string1[0]='\0';
		string2[0]='\0';
 }
}

MAC_BUFFER *mac_buffer_init(char *nameB, char *nameP, u8 Mod_id, u8 sorting_flag){
 
 MAC_BUFFER * mac_buff;
 mac_buff = malloc(sizeof(MAC_BUFFER));
 if(mac_buff==NULL){
	//msg("[MEM_MGT][WARNING] Memory allocation failure for mac_buff for Mod_id %d \n",Mod_id);
	return NULL;
 }
 strcpy(mac_buff->name, nameB);
 mac_buff->sorting_flag=sorting_flag;
 mac_buff->maximum_capacity = MAC_BUFFER_MAXIMUM_CAPACITY;
 
 mac_buff->my_p = (packet_list_t*)malloc(sizeof(packet_list_t));
 if(mac_buff->my_p==NULL){
	//msg("[MEM_MGT][WARNING] Memory allocation failure for packet_list in the mac_buff for Mod_id %d \n",Mod_id);
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

void packet_list_free (packet_list_t* listP){
 mem_element_t	*le;
	while ((le = packet_list_remove_head (listP))){
	 free(le);
	}
}

void mac_buffer_free(u8 Mod_id){
	packet_list_free(mac_buffer_g[Mod_id]->my_p);
}

mem_element_t *packet_list_remove_head (packet_list_t * listP){
//-----------------------------------------------------------------------------
  // access optimisation
  mem_element_t      *head;
  head = listP->head;
  // almost one element
  if (head != NULL) {
    listP->head = head->next;
    listP->nb_elements = listP->nb_elements - 1;
    listP->total_size = listP->total_size - head->pdu_size;
    // if only one element, update tail
    if (listP->head == NULL) {
      listP->tail = NULL;
    } else {
      listP->head->previous = NULL;
      head->next = NULL;
    }
  } else {
    //msg("[MEM_MGT][WARNING] remove_head_from_list(%s) no elements\n",listP->name);
  }
  return head;
}

mem_element_t *mac_buffer_remove_head(u8 Mod_id){
  if(mac_buffer_g[Mod_id]->my_p->nb_elements!=0){
    return (mem_element_t*)( packet_list_remove_head(mac_buffer_g[Mod_id]->my_p));
  }else{
    return NULL;
  }
}

mem_element_t *packet_list_remove_tail (packet_list_t * listP){
//-----------------------------------------------------------------------------
  // access optimisation;
  mem_element_t      *tail;
  tail = listP->tail;
  // almost one element;
  if (tail != NULL) {
    listP->nb_elements = listP->nb_elements - 1;
    listP->total_size = listP->total_size - tail->pdu_size;
    // if only one element, update head, tail;
    if (listP->head == tail) {
      listP->head = NULL;
      listP->tail = NULL;
    } else {
      listP->tail = tail->previous;
      tail->previous->next = NULL;
    }
    tail->previous = NULL;
  } else {
    //msg("[MEM_MGT][WARNING] remove_head_from_list(%s) no elements\n",listP->name);
  }
  return tail;
}

mem_element_t *mac_buffer_remove_tail(u8 Mod_id){
  if(mac_buffer_g[Mod_id]->my_p->nb_elements!=0){
    return (mem_element_t *)( packet_list_remove_tail(mac_buffer_g[Mod_id]->my_p));
  }else{
    return NULL;
  }
}

mem_element_t *packet_list_get_head(packet_list_t * listP){
//-----------------------------------------------------------------------------
  // access optimisation
  mem_element_t  *head;
  head = listP->head;
  if (head == NULL) {
    //msg("[MEM_MGT][WARNING] return NULL head from empty list \n",listP->name);
    return NULL;
  }
  return head;
}

mem_element_t *mac_buffer_get_head(u8 Mod_id){
  if(mac_buffer_g[Mod_id]->my_p->nb_elements!=0){
    return  (mem_element_t*)( packet_list_get_head(mac_buffer_g[Mod_id]->my_p) );
  }
  else{
   // msg("[MEM_MGT][WARNING] return NULL head from empty list \n");
  return NULL;
  }
}

mem_element_t *mac_buffer_data_req(u8 Mod_id, int seq_num, int size, int HARQ_proccess_ID){

 mem_element_t *ptr_h;
 mem_element_t *help_head;
 mem_element_t *help_tail;
 
 help_head = mac_buffer_g[Mod_id]->my_p->head;
 help_tail = mac_buffer_g[Mod_id]->my_p->tail;
 ptr_h = mac_buffer_g[Mod_id]->my_p->head;
 
 if(!(seq_num<0) && !(HARQ_proccess_ID<0)){
	if(ptr_h==NULL){
	 return NULL; // empty list no packet seq_num exists, it returns 0 !!!    
	}else{
	 while(ptr_h!=NULL){
		if(ptr_h->seq_num == seq_num && ptr_h->HARQ_proccess_ID==HARQ_proccess_ID){
		 if(ptr_h==help_head){
			return  (mem_element_t*)(mac_buffer_remove_head(Mod_id));
		 }
		 else if(ptr_h==help_tail){
			return  (mem_element_t*)(mac_buffer_remove_tail(Mod_id));
		 }
		 else{
			ptr_h -> previous -> next = ptr_h -> next;
			ptr_h -> next -> previous = ptr_h -> previous;
			ptr_h -> next = NULL;
			ptr_h -> next = NULL;
			mac_buffer_g[Mod_id]->my_p->nb_elements = mac_buffer_g[Mod_id]->my_p->nb_elements - 1;
			mac_buffer_g[Mod_id]->my_p->total_size = mac_buffer_g[Mod_id]->my_p->total_size - ptr_h->pdu_size;
			return ptr_h;
		 }
		}
		ptr_h = ptr_h->next;//printf("Prev % d, Curr %d\n",ptr_h_prev->seq_num,ptr_h->seq_num);
	 }
	 // I didn't find anything so I return NULL;
	 return  NULL;
	 }
  }
  else if (!(size<0)){
	 if(ptr_h==NULL){
		return NULL; // empty list no packet seq_num exists, it returns 0 !!!
	 }else{
		while(ptr_h!=NULL){
		 if(ptr_h-> pdu_size <= size ){
			if(ptr_h==help_head){
			 return  (mem_element_t*)(mac_buffer_remove_head(Mod_id));
			}
			else if(ptr_h==help_tail){
			 return  (mem_element_t*)(mac_buffer_remove_tail(Mod_id));
			}
			else{
			 ptr_h -> previous -> next = ptr_h -> next;
			 ptr_h -> next -> previous = ptr_h -> previous;
			 ptr_h -> next = NULL;
			 ptr_h -> next = NULL;
			 mac_buffer_g[Mod_id]->my_p->nb_elements = mac_buffer_g[Mod_id]->my_p->nb_elements - 1;
			 mac_buffer_g[Mod_id]->my_p->total_size = mac_buffer_g[Mod_id]->my_p->total_size - ptr_h->pdu_size;
			 return ptr_h;
			}
		 }
		 ptr_h = ptr_h->next;
		}
		// I didn't find anything so I return NULL;
		return NULL; 
		}
	}
	else{ //return  NULL
	 return NULL;
	}
}

int packet_list_find_pdu_seq_num(packet_list_t *listP, int seq_num){
 mem_element_t *ptr;
 ptr = listP->head;
 if(ptr==NULL){
	return 0; // empty list no packet seq_num exists, it returns 0 !!!    
	}else{
	 while(ptr!=NULL){
		if(ptr->seq_num == seq_num ){
			return 1; // seq_num found it returns 1
		}
    ptr = ptr->next;
	}
 return 0; // finished all elements in list but no seq_num found, so it returns 0;
 }
}

void packet_list_add_tail (mem_element_t * elementP, packet_list_t * listP ){
  mem_element_t      *tail;
//-----------------------------------------------------------------------------
  if (elementP != NULL) {
    // access optimisation
    elementP->next = NULL;
   
    tail = listP->tail;
    // almost one element
    if (tail == NULL) {
      elementP->previous = NULL;
      listP->head = elementP;
    } else {
      tail->next = elementP;
      elementP->previous = tail;
    }
    listP->tail = elementP;
    listP->total_size = listP->total_size + elementP->pdu_size;
    listP->nb_elements = listP->nb_elements + 1;
  } else {
    //msg("[CNT_LIST][ERROR] add_cnt_tail() element NULL\n");
  }
}

void packet_list_add_head(mem_element_t *elementP, packet_list_t * listP){
//-----------------------------------------------------------------------------
  // access optimisation;
  mem_element_t  *head;

  if (elementP != NULL) {
    head = listP->head;

    // almost one element
    if (head == NULL) {
		  elementP->next = NULL;
			elementP->previous = NULL;
      listP->head = elementP; 
      listP->tail = elementP;
    }else{
      elementP->next = head;
      head->previous = elementP;
      elementP->previous = NULL;
      listP->head = elementP;
    }
    listP->nb_elements = listP->nb_elements + 1;
		listP->total_size = listP->total_size + elementP->pdu_size;
  }
}

void packet_list_add_after_ref(mem_element_t * new_elementP, mem_element_t *elementP_ref, packet_list_t * listP){
  mem_element_t *tail;
//-----------------------------------------------------------------------------
  if (new_elementP != NULL && elementP_ref!=NULL) {
    // access optimisation
    tail = listP->tail;
    // almost one element
    if (tail == NULL){
      new_elementP->previous = NULL;
      listP->head = new_elementP;
    }else{
		 new_elementP->next = elementP_ref->next;
		 new_elementP->previous = elementP_ref;
		 if(elementP_ref->next == NULL){
			listP->tail = new_elementP;
		 }else{
		  elementP_ref->next->previous = new_elementP;
		 }
      elementP_ref->next = new_elementP;
    }
    listP->total_size = listP->total_size + new_elementP->pdu_size;
    listP->nb_elements = listP->nb_elements + 1;
  }else{
    //msg("[CNT_LIST][ERROR] add_cnt_tail() element NULL\n");
  }
}

int mac_buffer_data_ind(u8 Mod_id, mem_element_t *elementP, int seq_num, int pdu_size, int HARQ_proccess_ID){
 if (elementP == NULL || mac_buffer_nb_elements(Mod_id)==MAC_BUFFER_MAXIMUM_CAPACITY){
	return 0;
 }
 else{
	elementP->seq_num = seq_num;
	elementP->pdu_size = pdu_size;
	elementP->HARQ_proccess_ID = HARQ_proccess_ID;
	// ask Navid about the pool_id and data, if this values should be assigned here or outside this function!
	//elementP->pool_id = pool_id;
	//elementP->data = data;
	if (mac_buffer_g[Mod_id]->sorting_flag == SORT_FIFO){ // FIFO Queue-list, PDUs are inserted on the tail as they arrive!
	 if (mac_buffer_add_tail(Mod_id, elementP) == 1)
		return 1;
	 else
		return 0;
	}else if(mac_buffer_g[Mod_id]->sorting_flag == SORT_PDU_SEQN || mac_buffer_g[Mod_id]->sorting_flag == SORT_PDU_SIZE){
	 if (mac_buffer_add_sorted( Mod_id, elementP) == 1)
		return 1;
	 else
		return 0;
	}else{
	 //msg("[MAC] UNSPECIFIED SORTING in PDU Insertion, Packet not inserted!\n");
	 return 0;
	}
 }
}

int mac_buffer_add_tail(u8 Mod_id, mem_element_t *elementP){
  if(packet_list_find_pdu_seq_num(mac_buffer_g[Mod_id]->my_p, elementP->seq_num) == 0 ){ // packet element with seq_num not found, then I add the element to the buffer
    packet_list_add_tail(elementP, mac_buffer_g[Mod_id]->my_p);
    return 1;
  }
  else{
    return 0;
  }
}

int mac_buffer_add_sorted(u8 Mod_id, mem_element_t *elementP){
 mem_element_t *pivot;
 int after=-1;
 if(packet_list_find_pdu_seq_num(mac_buffer_g[Mod_id]->my_p, elementP->seq_num) == 0 ){ // packet element with seq_num not found, then I add the element to the buffer
	if (mac_buffer_g[Mod_id]->sorting_flag == SORT_PDU_SEQN){ 
	 pivot=packet_list_find_pivot_seq_num(elementP->seq_num, mac_buffer_g[Mod_id]->my_p, &after);
	}else if(mac_buffer_g[Mod_id]->sorting_flag == SORT_PDU_SIZE){
	 pivot=packet_list_find_pivot_pdu_size(elementP->pdu_size, mac_buffer_g[Mod_id]->my_p, &after);
	}
	if(pivot==NULL){
	 packet_list_add_tail(elementP, mac_buffer_g[Mod_id]->my_p);
	 return 1;
	}else if(pivot==mac_buffer_g[Mod_id]->my_p->head && after == 1){
	 packet_list_add_after_ref(elementP, pivot, mac_buffer_g[Mod_id]->my_p);
	 return 1; 
	}else if(pivot==mac_buffer_g[Mod_id]->my_p->head && after == 0){
	 packet_list_add_head(elementP, mac_buffer_g[Mod_id]->my_p);
	 return 1; 
	}else{ // if(pivot!=NULL){
	 packet_list_add_after_ref(elementP, pivot, mac_buffer_g[Mod_id]->my_p);
	 return 1;
	}
 }
 else{
	return 0;
 }
}


mem_element_t *packet_list_find_pivot_seq_num(int seq_num, packet_list_t *listP, int *after){
 mem_element_t *ptr_p;
 ptr_p = listP->head;
 if(ptr_p==NULL){
	// printf("Empty help list!!!");
	return NULL; 
 }
 else{
	while(ptr_p!=NULL){
	 if(ptr_p->seq_num > seq_num){
		if (ptr_p==listP->head){
		 *after=0;
		 return ptr_p;
		}
		else if(ptr_p->previous==listP->head){
		 *after=1;
		 return (ptr_p->previous);
		}
		else{
		 return (ptr_p->previous);
		}
	 }
	 ptr_p = ptr_p->next;
	}
	return ptr_p;
 }
}

mem_element_t *packet_list_find_pivot_pdu_size(int pdu_size, packet_list_t *listP, int *after){
 mem_element_t *ptr_p;
 ptr_p = listP->head;
 if(ptr_p==NULL){
	  printf("Empty help list!!!");
    return NULL; 
  }else{
    while(ptr_p!=NULL){
     if(ptr_p->pdu_size > pdu_size){
			if (ptr_p==listP->head){
			 *after=0;
			 return ptr_p;
			}else if(ptr_p->previous==listP->head)
			{
			 *after=1;
			 return (ptr_p->previous);
			}
			else{
			 return (ptr_p->previous);
			}
		 }
     ptr_p = ptr_p->next;
		 }
		 return ptr_p;
   }
}

void mac_buffer_print(u8 Mod_id){
  mem_element_t *ptr_p;
  ptr_p = mac_buffer_g[Mod_id]->my_p->head;
	printf("\n\n%s | %s \n",mac_buffer_g[Mod_id]->name,mac_buffer_g[Mod_id]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id), mac_buffer_nb_elements(Mod_id));
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

void mac_buffer_print_reverse(u8 Mod_id){
  mem_element_t *ptr_p;
  ptr_p = mac_buffer_g[Mod_id]->my_p->tail;
	printf("\n\nREVERSE| %s | %s \n",mac_buffer_g[Mod_id]->name,mac_buffer_g[Mod_id]->my_p->name);
	printf("Total_size %d, nb elements %d \n",mac_buffer_total_size(Mod_id), mac_buffer_nb_elements(Mod_id));
  if(ptr_p==NULL){
	  printf("Empty help list!!!");
    return; 
  }else{
    while(ptr_p!=NULL){
     // int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id); 
      ptr_p = ptr_p->previous;
    }
  }
}

int  mac_buffer_total_size(u8 Mod_id){
  return mac_buffer_g[Mod_id]->my_p->total_size;
}

int  mac_buffer_nb_elements(u8 Mod_id){
  return mac_buffer_g[Mod_id]->my_p->nb_elements;
}

