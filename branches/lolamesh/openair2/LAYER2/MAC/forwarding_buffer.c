#define NULL 0

#include "forwarding_buffer.h"

// will be called during the init phase
int init_mac_buffer() {

  mac_list_buffer_g = malloc(NB_UE_INST*sizeof(mac_list_buffer*));
	for (UE_id=0; UE_id<NB_UE_INST;UE_id++){  
    mac_list_buffer_g[UE_id] = mac_list_buffer_init();

}


void packet_list_init(packet_list_t *listP, char *nameP)
{
  int  i = 0;
  
  if (nameP){
    while ((listP->name[i] = nameP[i]) && (i++ < LIST_NAME_MAX_CHAR));
  }
  listP->tail = NULL;
  listP->head = NULL;
  listP->nb_elements = 0;
  listP->total_size = 0;
}




void mac_list_buffer_init(mac_list_buffer *mac_buff, char *nameB, char *nameP)
{
  strcpy(mac_buff->name, nameB);
  mac_buff->my_p = (packet_list_t*)malloc(sizeof(packet_list_t));
  packet_list_init(mac_buff->my_p, nameP);
}


void  packet_list_free (packet_list_t* listP)
{
  mem_element_t      *le;
  while ((le = packet_list_remove_head (listP))) {
    //free_mem_block (le);
    free(le);
  }						
}



void  mac_buffer_list_free(mac_list_buffer * mac_buffer){
      
	packet_list_free(mac_buffer->my_p);
}


mem_element_t * packet_list_remove_head (packet_list_t * listP){
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



mem_element_t * mac_list_buffer_remove_head(mac_list_buffer *mac_buff){
  if(mac_buff->my_p->nb_elements!=0){
    return (mem_element_t *)( packet_list_remove_head(mac_buff->my_p));
  }else{
    return NULL;
  }
}


mem_element_t * packet_list_remove_tail (packet_list_t * listP)
{
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

mem_element_t * mac_list_buffer_remove_tail(mac_list_buffer *mac_buff){
  if(mac_buff->my_p->nb_elements!=0){
    return (mem_element_t *)( packet_list_remove_tail(mac_buff->my_p));
  }else{
    return NULL;
  }
}


mem_element_t * packet_list_get_head (packet_list_t * listP)
{
//-----------------------------------------------------------------------------

  // access optimisation
  mem_element_t  *head;
  head = listP->head;
  if (head == NULL) {
    //msg("[MEM_MGT][WARNING] return NULL head from empty list \n",listP->name);
  }
  return head;
}



mem_element_t * mac_list_buffer_get_head(mac_list_buffer *mac_buff)
{
  if(mac_buff->my_p->nb_elements!=0){
    return  (mem_element_t*)( packet_list_get_head(mac_buff->my_p),sizeof(struct mem_element_t) );
  }
  else{
   // msg("[MEM_MGT][WARNING] return NULL head from empty list \n");
  return NULL;
  }
}


mem_element_t * mac_buffer_data_req(mac_list_buffer *mac_buf,  int seq_num, int size, int HARQ_proccess_ID){

  mem_element_t *ptr_h;
  mem_element_t *help_head;
  mem_element_t *help_tail;
 
  help_head = mac_buf->my_p->head;
  help_tail = mac_buf->my_p->tail;
  
  ptr_h = mac_buf->my_p->head;
 
  

  if(!(seq_num<0) && !(HARQ_proccess_ID<0)){
    if(ptr_h==NULL){
      return NULL; // empty list no packet seq_num exists, it returns 0 !!!    
    }else{
      while(ptr_h!=NULL){
	if(ptr_h->seq_num == seq_num && ptr_h->HARQ_proccess_ID==HARQ_proccess_ID){
	  
	  if(ptr_h==help_head){
	    return  (mem_element_t*)(mac_list_buffer_remove_head(mac_buf));
	  }
	  else if(ptr_h==help_tail){
	  return  (mem_element_t*)(mac_list_buffer_remove_tail(mac_buf));
	  }
	  else{

	    ptr_h -> previous -> next = ptr_h -> next;
	    ptr_h -> next -> previous = ptr_h -> previous;
	    ptr_h -> next = NULL;
	    ptr_h -> next = NULL;
	
	    mac_buf->my_p->nb_elements = mac_buf->my_p->nb_elements - 1;
	    mac_buf->my_p->total_size = mac_buf->my_p->total_size - ptr_h->pdu_size;

	    return ptr_h;
	  }
	}
	//ptr_h_prev = ptr_h;
	ptr_h = ptr_h->next;
	//printf("Prev % d, Curr %d\n",ptr_h_prev->seq_num,ptr_h->seq_num);
      }
      // I didn't find anything so I return NULL;
      //      return  (mem_element_t*) (mac_list_buffer_remove_head(mac_buf));
      return  NULL;
    }
  } 
  else if (!(size<0)){
    
    
    if(ptr_h==NULL){
      return NULL; // empty list no packet seq_num exists, it returns 0 !!!    
    }else{
      while(ptr_h!=NULL){
	if(ptr_h-> pdu_size <= size ){ // the intellegent would be to find the 
	  if(ptr_h==help_head){
	    return  (mem_element_t*)(mac_list_buffer_remove_head(mac_buf));
	  }
	  else if(ptr_h==help_tail){
	   return  (mem_element_t*)(mac_list_buffer_remove_tail(mac_buf));
	  }
	  else{
	     ptr_h -> previous -> next = ptr_h -> next;
	    ptr_h -> next -> previous = ptr_h -> previous;
	    ptr_h -> next = NULL;
	    ptr_h -> next = NULL;
	
	    mac_buf->my_p->nb_elements = mac_buf->my_p->nb_elements - 1;
	    mac_buf->my_p->total_size = mac_buf->my_p->total_size - ptr_h->pdu_size;

	    return ptr_h;
	  }
	}
	ptr_h = ptr_h->next;
      }
      // I didn't find anything so I return NULL;
      //return (mem_element_t*)(mac_list_buffer_remove_head(mac_buf));
      return NULL; 
    }
   }
   else{ //return  NULL
     return NULL; // (mem_element_t *)(mac_list_buffer_remove_head(mac_buf));
  }
}



int packet_list_find_pdu_seq_num(packet_list_t *listP, int seq_num)
{
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



void packet_list_add_tail (mem_element_t * elementP, packet_list_t * listP )
{
  mem_element_t      *tail;
//-----------------------------------------------------------------------------

  if (elementP != NULL) {
    // access optimisation
    listP->nb_elements = listP->nb_elements + 1;
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

int mac_buffer_data_ind( mac_list_buffer *mac_buff, mem_element_t *elementP, int seq_num, int pdu_size, int HARQ_proccess_ID){
    if (elementP == NULL){
    return 0;
    }
    else{
      elementP->seq_num = seq_num;
      elementP->pdu_size = pdu_size;
      elementP->HARQ_proccess_ID = HARQ_proccess_ID;
      if (mac_list_buffer_add_tail( mac_buff, elementP) == 1)
	return 1;
      else
	return 0;
    }
}

int
mac_list_buffer_add_tail( mac_list_buffer *mac_buff, mem_element_t *elementP)
{
  if(packet_list_find_pdu_seq_num(mac_buff->my_p, elementP->seq_num) == 0 ){ // packet element with seq_num not found, then I add the element to the buffer
    packet_list_add_tail(elementP, mac_buff->my_p);
    return 1;
  }
  else{
    return 0;
  }
}


void mac_buff_print_lists(mac_list_buffer *mac_buff)
{
  mem_element_t *ptr_p;
  ptr_p = mac_buff->my_p->head;
  
  
  if(ptr_p==NULL){
    return; 
    printf("Empty help list!!!");
  }else{
    while(ptr_p!=NULL){
     // int seq_num, int pdu_size, int total_size, int HARQ_proccess_ID
      printf("Seg num: %d, Pdu size %d,  HARQ Process %d ||| Pool_id %d \n",ptr_p->seq_num,ptr_p->pdu_size,ptr_p->HARQ_proccess_ID,ptr_p->pool_id);
      
      ptr_p = ptr_p->next;
    }
  }
}

int  mac_list_buffer_size(mac_list_buffer *mac_buf){
  return mac_buf->my_p->total_size;
}
int  mac_list_buffer_nb_elements(mac_list_buffer *mac_buf){
  return mac_buf->my_p->nb_elements;
}
