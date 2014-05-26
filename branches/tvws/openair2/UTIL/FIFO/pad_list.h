/*
                                 list.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#ifndef __list_H__
#define __list_H__

#include "types.h"
//-----------------------------------------------------------------------------
#    ifdef list_C
#        define private_list(x) x
#        define public_list(x) x
#    else
#        define private_list(x)
#        define public_list(x) extern x
#    endif
#    ifdef LIST2_C
#        define private_list2(x) x
#        define public_list2(x) x
#    else
#        define private_list2(x)
#        define public_list2(x) extern x
#    endif
//-----------------------------------------------------------------------------

typedef struct Job_List_s {
  Job_elt_t *head;
  Job_elt_t *tail;
  int        nb_elements;
} Job_List_t;

typedef struct Event_List_s{
  Event_elt_t *head;
  Event_elt_t *tail;
  int          nb_elements;
} Event_List_t;

typedef struct Packet_OTG_List_s {
  Packet_otg_elt_t *head;
  Packet_otg_elt_t *tail;
  int               nb_elements;
} Packet_OTG_List_t;

//-----------------------------------------------------------------------------
public_list( void         job_list_init (Job_List_t*);)
public_list( void         job_list_free (Job_List_t* listP);)
public_list( Job_elt_t* job_list_get_head (Job_List_t*);)
public_list( Job_elt_t* job_list_remove_head (Job_List_t* );)
public_list( Job_elt_t* job_list_remove_element (Job_elt_t*, Job_List_t*);)
public_list( void         job_list_add_head (Job_elt_t* , Job_List_t* );)
public_list( void         job_list_add_tail_eurecom (Job_elt_t* , Job_List_t* );)
public_list( void         job_list_add_list (Job_List_t* , Job_List_t* );)
public_list( void         job_list_display (Job_List_t* );)

public_list( void         event_list_init (Event_List_t*);)
public_list( void         event_list_free (Event_List_t* listP);)
public_list( Event_elt_t* event_list_get_head (Event_List_t*);)
public_list( Event_elt_t* event_list_remove_head (Event_List_t* );)
public_list( Event_elt_t* event_list_remove_element (Event_elt_t*, Event_List_t*);)
public_list( void         event_list_add_head (Event_elt_t* , Event_List_t* );)
public_list( void         event_list_add_element (Event_elt_t* , Event_elt_t* , Event_List_t* );)
public_list( void         event_list_add_tail_eurecom (Event_elt_t* , Event_List_t* );)
public_list( void         event_list_add_list (Event_List_t* , Event_List_t* );)
public_list( void         event_list_display (Event_List_t* );)

public_list( void         pkt_list_init (Packet_OTG_List_t*);)
public_list( void         pkt_list_free (Packet_OTG_List_t* listP);)
public_list( Packet_otg_elt_t* pkt_list_get_head (Packet_OTG_List_t*);)
public_list( Packet_otg_elt_t* pkt_list_remove_head (Packet_OTG_List_t* );)
public_list( Packet_otg_elt_t* pkt_list_remove_element (Packet_otg_elt_t*, Packet_OTG_List_t*);)
public_list( void         pkt_list_add_head (Packet_otg_elt_t* , Packet_OTG_List_t* );)
public_list( void         pkt_list_add_element (Packet_otg_elt_t* , Packet_otg_elt_t* , Packet_OTG_List_t* );)
public_list( void         pkt_list_add_tail_eurecom (Packet_otg_elt_t* , Packet_OTG_List_t* );)
public_list( void         pkt_list_add_list (Packet_OTG_List_t* , Packet_OTG_List_t* );)
public_list( void         pkt_list_display (Packet_OTG_List_t* );)

#endif
