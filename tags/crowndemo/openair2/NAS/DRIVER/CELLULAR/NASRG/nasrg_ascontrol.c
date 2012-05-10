/***************************************************************************
                          nasrg_ascontrol.c  -  description
                             -------------------
    copyright            : (C) 2003 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/
#ifdef NODE_RG
#include "nasrg_variables.h"
#include "nasrg_proto.h"

//---------------------------------------------------------------------------
void nasrg_ASCTL_init(void){
//---------------------------------------------------------------------------
  int cxi, i;

  gpriv->next_sclassref = NASRG_DEFAULTRAB_CLASSREF;
  // Initialize MBMS services list in MT
  for (cxi=0; cxi<GRAAL_CX_MAX ; ++cxi){
    for (i = 0; i<NASRG_MBMS_SVCES_MAX; i++){
       gpriv->cx[cxi].requested_joined_services[i]= -1;
       gpriv->cx[cxi].requested_left_services[i]= -1;
       gpriv->cx[cxi].joined_services[i]= -1;
     }
  }
  printk("nasrg_ASCTL_init Complete\n");
}

//---------------------------------------------------------------------------
//For demo, add automatically a radio bearer
//Equivalent to rb add send 0 5 2
void nasrg_ASCTL_start_default_rb(struct cx_entity *cx){
//---------------------------------------------------------------------------
#ifdef DEMO_3GSM
	struct rb_entity *rb;
  int status;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_start_default_rb - begin \n");
#endif
  if (!cx){
 	  printk("nasrg_ASCTL_start_default_rb - input parameter cx is NULL \n");
    return;
  }
// End debug information

//	rb=graal_COMMON_add_rb(cx, 5, 2);
	rb = nasrg_COMMON_add_rb(cx, NASRG_DEFAULTRAB_RBID, NASRG_DEFAULTRAB_QoS);
	if (rb!=NULL){
    rb->cnxid = (GRAAL_RB_MAX_NUM * cx->lcr)+1;
    //rb->default_rab =1;
    rb->dscp = NASRG_DEFAULTRAB_DSCP;
		status=nasrg_ASCTL_DC_send_rb_establish_request(cx, rb);
  }
	else
		status=-GRAAL_ERROR_NOMEMORY;
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_start_default_rb - end %d \n",status);
#endif
}
#endif

//---------------------------------------------------------------------------
//For demo, add automatically a classifier
//Equivalent to class add send 0 -f qos <x> -cr 0
void nasrg_ASCTL_start_default_sclassifier(struct cx_entity *cx,struct rb_entity *rb){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_default_sclassifier - begin \n");
#endif
  if (!cx || !rb){
 	  printk("nasrg_ASCTL_start_default_sclassifier - input parameter is NULL \n");
    return;
  }
// End debug information
  gc=nasrg_CLASS_add_sclassifier(cx, GRAAL_DSCP_DEFAULT, gpriv->next_sclassref);
//  gc=graal_CLASS_add_sclassifier(cx, 5, 0);
  if (gc==NULL){
 	  printk("nasrg_ASCTL_start_default_sclassifier - Classifier pointer is null : not added \n");
    return;
  }
  gc->fct = nasrg_COMMON_QOS_send;
  gc->rab_id =rb->rab_id;
  gc->rb= rb;
  gc->version = NASRG_DEFAULTRAB_IPVERSION;
	gc->protocol= GRAAL_PROTOCOL_DEFAULT;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_default_sclassifier - end \n");
  nasrg_TOOL_print_classifier(gc);
#endif
}

//---------------------------------------------------------------------------
//Add automatically a classifier on DSCP
//Equivalent to class add send 0 -f qos <x> -cr 0
void nasrg_ASCTL_start_sclassifier(struct cx_entity *cx,struct rb_entity *rb){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_sclassifier - begin \n");
#endif
  if (!cx || !rb){
 	  printk("nasrg_ASCTL_start_sclassifier - input parameter is NULL \n");
    return;
  }
// End debug information
  gc=nasrg_CLASS_add_sclassifier(cx, rb->dscp, gpriv->next_sclassref);
//  gc=graal_CLASS_add_sclassifier(cx, 5, 0);
  if (gc==NULL){
 	  printk("nasrg_ASCTL_start_sclassifier - Classifier pointer is null : not added \n");
    return;
  }
  gc->fct = nasrg_COMMON_QOS_send;
  gc->rab_id =rb->rab_id;
  gc->rb= rb;
  gc->version = NASRG_DEFAULTRAB_IPVERSION;
	gc->protocol= GRAAL_PROTOCOL_DEFAULT;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_sclassifier - end \n");
  nasrg_TOOL_print_classifier(gc);
#endif
}

//---------------------------------------------------------------------------
//Add automatically a classifier for mbms
void nasrg_ASCTL_start_mbmsclassifier(int mbms_ix,struct rb_entity *mbms_rb){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_mbmsclassifier - begin \n");
#endif
  if (!mbms_rb){
 	  printk("nasrg_ASCTL_start_mbmsclassifier - input parameter is NULL \n");
    return;
  }
// End debug information
//
//  gc=nasrg_CLASS_add_mbmsclassifier(mbms_ix, gpriv->next_mbmsclassref);
  gc=nasrg_CLASS_add_mbmsclassifier(mbms_ix, gpriv->next_sclassref++);
  if (gc==NULL){
 	  printk("nasrg_ASCTL_start_mbmsclassifier - Classifier pointer is null : not added \n");
    return;
  }
  gc->fct = nasrg_COMMON_QOS_send;
  gc->rab_id =mbms_rb->mbms_rbId;
  gc->rb= mbms_rb;
  gc->version = NASRG_DEFAULTRAB_IPVERSION;
	gc->protocol= GRAAL_PROTOCOL_DEFAULT;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasrg_ASCTL_start_mbmsclassifier - end \n");
  nasrg_TOOL_print_classifier(gc);
#endif
}

//---------------------------------------------------------------------------
void nasrg_ASCTL_timer(unsigned long data){
//---------------------------------------------------------------------------
  u8 cxi;
  struct cx_entity *cx;
  struct rb_entity *rb;
  spin_lock(&gpriv->lock);
#ifdef GRAAL_DEBUG_TIMER
  printk("nasrg_ASCTL_timer - begin \n");
#endif
  (gpriv->timer).function = nasrg_ASCTL_timer;
  (gpriv->timer).expires=jiffies+GRAAL_TIMER_TICK;
  (gpriv->timer).data=0L;
  for (cxi=0; cxi<GRAAL_CX_MAX ; ++cxi){
    cx=gpriv->cx+cxi;
    if (!cx){
      printk("nasrg_ASCTL_timer - No pointer for connection %d \n", cxi);
      continue;
    }
    for (rb=cx->rb;rb!=NULL; rb=rb->next){
      if (rb->countimer!=GRAAL_TIMER_IDLE){
      #ifdef GRAAL_DEBUG_TIMER
        printk("nasrg_ASCTL_timer : rb countimer %d, rb state %d\n", rb->countimer, rb->state);
      #endif
        if (rb->countimer==0){
          switch (rb->state){
            case GRAAL_CX_CONNECTING:
            case GRAAL_CX_CONNECTING_FAILURE:  // MW - 15/01/07 Useless, currently no retry if failure
              if (rb->retry<gpriv->retry_limit){
                printk("nasrg_ASCTL_timer: Retry RB establishment %d\n", rb->retry);
                nasrg_ASCTL_DC_send_rb_establish_request(cx, rb);
              }else{
                printk("nasrg_ASCTL_timer: RB Establishment failure\n");
                rb->state=GRAAL_IDLE;
                rb->countimer=GRAAL_TIMER_IDLE;
              }
              break;
            case GRAAL_CX_DCH:
              #ifdef DEMO_3GSM
              if (cx->num_rb == 1){
                 nasrg_ASCTL_start_default_sclassifier(cx, rb);
              }
              #endif
              nasrg_ASCTL_start_sclassifier(cx, rb);
              rb->countimer=GRAAL_TIMER_IDLE;
              break;
            case GRAAL_CX_RELEASING_FAILURE:
              nasrg_ASCTL_DC_send_rb_release_request(cx, rb);
              break;
            default:
              rb->countimer=GRAAL_TIMER_IDLE;
          }
        }else{
          --rb->countimer;
          printk("nasrg_ASCTL_timer : rb countimer-- %d, rb state %d\n", rb->countimer, rb->state);
        }
      }
    }
  }
  add_timer(&gpriv->timer);
#ifdef GRAAL_DEBUG_TIMER
  printk("nasrg_ASCTL_timer - end \n");
#endif
  spin_unlock(&gpriv->lock);
}


/***************************************************************************
     Transmission side
 ***************************************************************************/
//---------------------------------------------------------------------------
// Encode INFO_BROADCAST_REQ message
int nasrg_ASCTL_GC_send_broadcast_request(u8 category){
//---------------------------------------------------------------------------
	char *xmit_data = "TESTING BROADCASTING ROUTER ADVERTISEMENT. TESTING BROADCASTING ROUTER ADVERTISEMENT. BROADCASTING ROUTER.\0";

	struct nas_rg_gc_element *p;
	p= (struct nas_rg_gc_element *)(gpriv->xbuffer);
	p->type = INFO_BROADCAST_REQ;
	p->length =  NAS_TL_SIZE + sizeof(struct NASInfoBroadcastReq);
//
	p->nasRGGCPrimitive.broadcast_req.period = 0;
	p->nasRGGCPrimitive.broadcast_req.category = category;
	p->nasRGGCPrimitive.broadcast_req.nasDataLength = strlen(xmit_data)+1;  // TBD
	bytes_wrote = rtf_put(gpriv->sap[GRAAL_GC_SAPI], p, p->length);
	bytes_wrote += rtf_put(gpriv->sap[GRAAL_GC_SAPI], xmit_data, p->nasRGGCPrimitive.broadcast_req.nasDataLength);
	if (bytes_wrote==p->length+p->nasRGGCPrimitive.broadcast_req.nasDataLength){
#ifdef GRAAL_DEBUG_GC
		printk("nasrg_ASCTL_GC_send_broadcast: INFO_BROADCAST_REQ primitive sent successfully in GC-FIFO\n");
#endif
	}else{
		printk("nasrg_ASCTL_GC_send_broadcast: Message sent failure in GC-FIFO\n");
  }
	return bytes_wrote;
}


//---------------------------------------------------------------------------
// Encode INFO_BROADCAST_REQ message for RRC SIB1
int nasrg_ASCTL_GC_send_SIB1_broadcast_request(struct sk_buff *skb){
//---------------------------------------------------------------------------
	struct nas_rg_gc_element *p;
  char sib1_flag; // will be used for reception in nas_ue

// Start debug information
#ifdef GRAAL_DEBUG_GC
	printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request - begin \n");
#endif
  if (!skb){
 	  printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request - input parameter is NULL \n");
    return 0;
  }
// End debug information
	p= (struct nas_rg_gc_element *)(gpriv->xbuffer);
	p->type = INFO_BROADCAST_REQ;
	p->length =  NAS_TL_SIZE + sizeof(struct NASInfoBroadcastReq);
//
	p->nasRGGCPrimitive.broadcast_req.period = 10; // to be checked
	p->nasRGGCPrimitive.broadcast_req.category = 1;
	p->nasRGGCPrimitive.broadcast_req.nasDataLength = skb->len+1;  // TBD
  sib1_flag = 1;
// send header
	bytes_wrote = rtf_put(gpriv->sap[GRAAL_GC_SAPI], p, p->length);
	if (bytes_wrote!=p->length){
		printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request: Header send failure in GC-FIFO\n");
		return bytes_wrote;
	}
// send sib1_flag
	bytes_wrote +=  rtf_put(gpriv->sap[GRAAL_GC_SAPI], &sib1_flag, 1);
	if (bytes_wrote!=p->length+1){
		printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request: sib1_flag send failure in GC-FIFO\n");
		return bytes_wrote;
	}
// send data
	bytes_wrote += rtf_put(gpriv->sap[GRAAL_GC_SAPI], skb->data, skb->len);
	if (bytes_wrote!=p->length+skb->len+1){
		printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request: Data send failure in GC-FIFO\n");
		return bytes_wrote;
	}
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_GC_send_SIB1_broadcast_request - end \n");
#endif
	return bytes_wrote;
}

//-----------------------------------------------------------------------------
// Encode MBMS_BEARER_ESTABLISH_REQ message
int nasrg_ASCTL_GC_send_mbms_bearer_establish_req(int mbms_ix ){
//-----------------------------------------------------------------------------
   struct nas_rg_gc_element *p;
   p= (struct nas_rg_gc_element *)(gpriv->xbuffer);
   p->type = MBMS_BEARER_ESTABLISH_REQ;
   p->length =  NAS_TL_SIZE + sizeof(struct NASMBMSBearerEstablishReq);
//
   p->nasRGGCPrimitive.mbms_establish_req.mbms_serviceId = gpriv->mbms_rb[mbms_ix].serviceId;
   p->nasRGGCPrimitive.mbms_establish_req.mbms_sessionId = gpriv->mbms_rb[mbms_ix].sessionId;
   p->nasRGGCPrimitive.mbms_establish_req.mbms_rbId = gpriv->mbms_rb[mbms_ix].mbms_rbId;
   p->nasRGGCPrimitive.mbms_establish_req.mbms_sapId = gpriv->mbms_rb[mbms_ix].sapi;
   p->nasRGGCPrimitive.mbms_establish_req.mbms_QoSclass = gpriv->mbms_rb[mbms_ix].qos;
   p->nasRGGCPrimitive.mbms_establish_req.mbms_duration = gpriv->mbms_rb[mbms_ix].duration;
   gpriv->mbms_rb[mbms_ix].state = GRAAL_RB_ESTABLISHING;
//
	bytes_wrote = rtf_put(gpriv->sap[GRAAL_GC_SAPI], p, p->length);
	if (bytes_wrote==p->length){
#ifdef GRAAL_DEBUG_GC
		printk(" nasrg_ASCTL_GC_send_mbms_bearer_establish: MBMS_BEARER_ESTABLISH_REQ primitive sent successfully in GC-FIFO\n");
    printk(" ServiceId %d, RB_Id %d , qos class %d \n",
              p->nasRGGCPrimitive.mbms_establish_req.mbms_serviceId,
              p->nasRGGCPrimitive.mbms_establish_req.mbms_rbId,
              p->nasRGGCPrimitive.mbms_establish_req.mbms_QoSclass);
#endif
	}else{
		printk("nasrg_ASCTL_GC_send_mbms_bearer_establish: Message sent failure in GC-FIFO\n");
  }
	return bytes_wrote;
}

//-----------------------------------------------------------------------------
// HNN - Encode MBMS_BEARER_RELEASE_REQ message
int nasrg_ASCTL_GC_send_mbms_bearer_release_req(int mbms_ix){
//-----------------------------------------------------------------------------
  struct nas_rg_gc_element *p;
  u16 classref=0;

   p= (struct nas_rg_gc_element *)(gpriv->xbuffer);

   p->type = MBMS_BEARER_RELEASE_REQ;
   p->length =  NAS_TL_SIZE + sizeof(struct NASMBMSBearerReleaseReq);
//
   p->nasRGGCPrimitive.mbms_release_req.mbms_serviceId = gpriv->mbms_rb[mbms_ix].serviceId;
   p->nasRGGCPrimitive.mbms_release_req.mbms_sessionId = gpriv->mbms_rb[mbms_ix].sessionId;
   p->nasRGGCPrimitive.mbms_release_req.mbms_rbId = gpriv->mbms_rb[mbms_ix].mbms_rbId;
   //gpriv->mbms_rb[mbms_ix].state = GRAAL_RB_RELEASING;
//
   bytes_wrote = rtf_put(gpriv->sap[GRAAL_GC_SAPI], p, p->length);
   if (bytes_wrote==p->length){
#ifdef GRAAL_DEBUG_GC
     printk(" nasrg_ASCTL_GC_send_mbms_bearer_release: MBMS_BEARER_RELEASE_REQ primitive sent successfully in GC-FIFO\n");
     printk(" ServiceId %d, RB_Id %d \n", p->nasRGGCPrimitive.mbms_establish_req.mbms_serviceId, p->nasRGGCPrimitive.mbms_establish_req.mbms_rbId);
#endif
     // clean NASRG private structures
     classref =  (gpriv->mbmsclassifier[mbms_ix])->classref;
     nasrg_CLASS_del_mbmsclassifier(mbms_ix, classref);
     //nasrg_CLASS_flush_mbmsclassifier();
     //gpriv->mbms_rb[mbms_ix].state = GRAAL_IDLE;
     memset (&(gpriv->mbms_rb[mbms_ix]),0,sizeof (struct rb_entity));
	}else{
		printk("nasrg_ASCTL_GC_send_mbms_bearer_release: Message sent failure in GC-FIFO\n");
  }
	return bytes_wrote;
}

//---------------------------------------------------------------------------
// Confirm the establishment of a connection (DC channel)
int nasrg_ASCTL_DC_send_cx_establish_confirm(struct cx_entity *cx, u8 response){
//---------------------------------------------------------------------------
	struct nas_rg_dc_element *p;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_send_cx_establish - begin \n");
#endif
  if (!cx){
 	  printk("nasrg_ASCTL_DC_send_cx_establish - input parameter cx is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
	p= (struct nas_rg_dc_element *)(gpriv->xbuffer);
	p->type = CONN_ESTABLISH_CNF;
	p->length =  NAS_TL_SIZE + sizeof(struct NASConnEstablishConf);
	p->nasRGDCPrimitive.conn_establish_conf.localConnectionRef = cx->lcr;
	p->nasRGDCPrimitive.conn_establish_conf.status = response;  // can be ACCEPTED  or FAILURE
	p->nasRGDCPrimitive.conn_establish_conf.num_RBs = 0; // Hard coded in first step
//
	bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
	if (bytes_wrote==p->length){
#ifdef GRAAL_DEBUG_DC
		printk("nasrg_ASCTL_DC_send_cx_establish: CONN_ESTABLISH_CNF primitive sent successfully in DC-FIFO\n");
		printk(" lcr (Mobile_id) %u\n",p->nasRGDCPrimitive.conn_establish_conf.localConnectionRef);
		printk(" Status %u\n",p->nasRGDCPrimitive.conn_establish_conf.status);
#endif
	}
	else
		printk("nasrg_ASCTL_DC_send_cx_establish: Message transmission failure to DC-FIFO\n");
	return bytes_wrote;
}

//---------------------------------------------------------------------------
// Request the establishment of a radio bearer
int nasrg_ASCTL_DC_send_rb_establish_request(struct cx_entity *cx, struct rb_entity *rb){
//---------------------------------------------------------------------------
  struct nas_rg_dc_element *p;
// Start debug information
#ifdef GRAAL_DEBUG_DC
  printk("nasrg_ASCTL_DC_send_rb_establish - begin \n");
#endif
  if (!cx || !rb){
    printk("nasrg_ASCTL_DC_send_rb_establish - input parameter is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
  switch(rb->state){
  case GRAAL_CX_CONNECTING:
  case GRAAL_CX_CONNECTING_FAILURE:
  case GRAAL_IDLE:
    ++rb->retry;
    rb->countimer=gpriv->timer_establishment;
    if (cx->state==GRAAL_CX_DCH){
      p= (struct nas_rg_dc_element *)(gpriv->xbuffer);
      p->type = RB_ESTABLISH_REQ;
      p->length =  NAS_TL_SIZE + sizeof(struct NASrbEstablishReq);
      p->nasRGDCPrimitive.rb_establish_req.localConnectionRef = cx->lcr;
      p->nasRGDCPrimitive.rb_establish_req.rbId = rb->rab_id + (GRAAL_RB_MAX_NUM * cx->lcr);
      p->nasRGDCPrimitive.rb_establish_req.QoSclass = rb->qos;
      p->nasRGDCPrimitive.rb_establish_req.dscp = rb->dscp_ul;
      //
      bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
      if (bytes_wrote==p->length){
        rb->state=GRAAL_CX_CONNECTING;
#ifdef GRAAL_DEBUG_DC
        printk(" nasrg_ASCTL_DC_send_rb_establish: RB_ESTABLISH_REQ primitive sent successfully in DC-FIFO\n");
        printk(" lcr (Mobile_id) %u\n",p->nasRGDCPrimitive.rb_establish_req.localConnectionRef);
        printk(" Radio Bearer identification %u\n",p->nasRGDCPrimitive.rb_establish_req.rbId);
        printk(" QoS %u\n",p->nasRGDCPrimitive.rb_establish_req.QoSclass);
#endif
      }else{
        rb->state=GRAAL_CX_CONNECTING_FAILURE;
        printk("nasrg_ASCTL_DC_send_rb_establish: Message sent failure in DC-FIFO\n");
      }
      return bytes_wrote;
    }else{
      rb->state=GRAAL_CX_CONNECTING_FAILURE;
      printk("nasrg_ASCTL_DC_send_rb_establish: Failure \n");
      return 0;
    }
  default:
    return -GRAAL_ERROR_NOTIDLE;
  }
}

//---------------------------------------------------------------------------
// Request the release of a radio bearer
int nasrg_ASCTL_DC_send_rb_release_request(struct cx_entity *cx, struct rb_entity *rb){
//---------------------------------------------------------------------------
	struct nas_rg_dc_element *p;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_send_rb_release - begin \n");
#endif
  if (!cx || !rb){
 	  printk("nasrg_ASCTL_DC_send_rb_release - input parameter is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
	switch (rb->state){
	case GRAAL_CX_RELEASING_FAILURE:
	case GRAAL_CX_DCH:
		p= (struct nas_rg_dc_element *)(gpriv->xbuffer);
		p->type = RB_RELEASE_REQ;
		p->length =  NAS_TL_SIZE + sizeof(struct NASrbReleaseReq);
		p->nasRGDCPrimitive.rb_release_req.localConnectionRef = cx->lcr;
		p->nasRGDCPrimitive.rb_release_req.rbId = rb->rab_id + (GRAAL_RB_MAX_NUM * cx->lcr);
		//
		bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
		if (bytes_wrote){
			rb->state=GRAAL_IDLE;
#ifdef GRAAL_DEBUG_DC
			printk("nasrg_ASCTL_DC_send_rb_release: RB_RELEASE_REQ primitive sent successfully in DC-FIFO\n");
#endif
		}else{
			++rb->retry;
			rb->countimer=gpriv->timer_release;
			rb->state=GRAAL_CX_RELEASING_FAILURE;
			printk("nasrg_ASCTL_DC_send_rb_release: Message sent failure in DC-FIFO\n");
		}
		return bytes_wrote;
	default:
		return -GRAAL_ERROR_NOTCONNECTED;
	}
}

//-----------------------------------------------------------------------------
// Request the notification of a UE to joined or left services
int nasrg_ASCTL_DC_send_mbms_ue_notify_req(struct cx_entity *cx) {	
//-----------------------------------------------------------------------------
   struct nas_rg_dc_element *p;
   int i;

#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_send_mbms_ue_notify_req - begin \n");
#endif
  if (!cx){
 	  printk("nasrg_ASCTL_DC_send_mbms_ue_notify_req - input parameter is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
  p= (struct nas_rg_dc_element *)(gpriv->xbuffer);
  p->type = MBMS_UE_NOTIFY_REQ;
  p->length =  NAS_TL_SIZE + sizeof(struct NASMBMSUENotifyReq);
//
  p->nasRGDCPrimitive.mbms_ue_notify_req.localConnectionRef = cx->lcr;
  // joined/left services are lists of MAX_MBMS_SERVICES
  // -1 means the end of the list
  for (i = 0; i < MAX_MBMS_SERVICES; i++){
    p->nasRGDCPrimitive.mbms_ue_notify_req.joined_services[i].mbms_serviceId = (nasMBMSServiceId_t)cx->requested_joined_services[i];
    p->nasRGDCPrimitive.mbms_ue_notify_req.left_services[i].mbms_serviceId = (nasMBMSServiceId_t)cx->requested_left_services[i];
  }
//
	bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
	if (bytes_wrote==p->length){
#ifdef GRAAL_DEBUG_DC
		printk("nasrg_ASCTL_DC_send_mbms_ue_notify_req: MBMS_UE_NOTIFY_REQ primitive sent successfully in DC-FIFO\n");
		printk(" lcr (Mobile_id) %u\n",p->nasRGDCPrimitive.mbms_ue_notify_req.localConnectionRef);
		printk(" joined service %d, left service %d\n",cx->requested_joined_services[0], cx->requested_left_services[0] );
#endif
	}
	else
		printk("nasrg_ASCTL_DC_send_mbms_ue_notify_req: Message transmission failure to DC-FIFO\n");
	return bytes_wrote;
}

//---------------------------------------------------------------------------
// Request the transfer of data (DC SAP)
void nasrg_ASCTL_DC_send_sig_data_request(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc){
//---------------------------------------------------------------------------
	struct nas_rg_dc_element *p;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_send_sig_data - begin \n");
#endif
  if (!skb || !gc || !cx){
 	  printk("nasrg_ASCTL_DC_send_sig_data - input parameter is NULL \n");
    return;
  }
// End debug information
	if (cx->state!=GRAAL_CX_DCH)
	{
		printk("nasrg_ASCTL_DC_send_sig_data: Not connected, so the message is dropped\n");
		++gpriv->stats.tx_dropped;
		return;
	}
	p = (struct nas_rg_dc_element *)(gpriv->xbuffer);
	p->type = DATA_TRANSFER_REQ;
	p->length =  NAS_TL_SIZE + sizeof(struct NASDataReq);
	p->nasRGDCPrimitive.data_transfer_req.localConnectionRef = cx->lcr;
	p->nasRGDCPrimitive.data_transfer_req.priority = GRAAL_SIG_SRB3;
	p->nasRGDCPrimitive.data_transfer_req.nasDataLength = skb->len;
  //
	bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
	if (bytes_wrote!=p->length){
		printk("nasrg_ASCTL_DC_send_sig_data: Header sent failure in DC-FIFO\n");
		return;
	}
	bytes_wrote += rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], skb->data, skb->len);
	if (bytes_wrote!=p->length+skb->len){
		printk("nasrg_ASCTL_DC_send_sig_data: Data sent failure in DC-FIFO\n");
		return;
	}
	gpriv->stats.tx_bytes   += skb->len;
	gpriv->stats.tx_packets ++;
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_send_sig_data - end \n");
#endif
}




/***************************************************************************
     Reception side
 ***************************************************************************/

//---------------------------------------------------------------------------
// Decode CONN_ESTABLISH_IND message from RRC
void nasrg_ASCTL_DC_decode_cx_establish_ind(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
  int i;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_decode_cx_establish - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasrg_ASCTL_DC_decode_cx_establish - input parameter is NULL \n");
    return;
  }
// End debug information
  if (nasrg_ASCTL_DC_send_cx_establish_confirm(cx, ACCEPTED)>0){
  	nasrg_TOOL_imei2iid(p->nasRGDCPrimitive.conn_establish_ind.InterfaceIMEI, (u8 *)cx->iid6);
  	cx->iid4=97;  // A AUTOMATISER
  	cx->lcr = p->nasRGDCPrimitive.conn_establish_ind.localConnectionRef;
  	cx->state=GRAAL_CX_DCH;
#ifdef GRAAL_DEBUG_DC
 	  printk("nasrg_ASCTL_DC_decode_cx_establish: CONN_ESTABLISH_IND reception\n");
		printk(" primitive length %d\n",p->length);
 		printk(" Local Connection reference %d\n",p->nasRGDCPrimitive.conn_establish_ind.localConnectionRef);
 		printk(" IMEI ");
 		for (i=0; i<14; ++i)
 			printk("%u",p->nasRGDCPrimitive.conn_establish_ind.InterfaceIMEI[i]);
  		printk(" state ");
		nasrg_TOOL_print_state(cx->state);
#endif
	}
}

//---------------------------------------------------------------------------
// Decode CONN_RELEASE_IND message from RRC
void nasrg_ASCTL_DC_decode_cx_release_ind(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC
  printk("nasrg_ASCTL_DC_decode_cx_release - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasrg_ASCTL_DC_decode_cx_release - input parameter is NULL \n");
    return;
  }
// End debug information
 	cx->state=GRAAL_IDLE;
 	cx->iid4=0;
 	nasrg_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)cx->iid6);
 	nasrg_COMMON_flush_rb(cx);
  nasrg_CLASS_flush_sclassifier(cx);
#ifdef GRAAL_DEBUG_DC
 	printk("nasrg_ASCTL_DC_decode_cx_release: CONN_RELEASE_IND reception\n");
 	printk(" Primitive length %u\n",p->length);
 	printk(" Local Connection reference %u\n",p->nasRGDCPrimitive.conn_release_ind.localConnectionRef);
 	printk(" Release cause %u\n",p->nasRGDCPrimitive.conn_release_ind.releaseCause);
 	nasrg_TOOL_print_state(cx->state);
#endif
}

//---------------------------------------------------------------------------
// Decode CONN_LOSS_IND message from RRC
void nasrg_ASCTL_DC_decode_cx_loss_ind(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_decode_cx_loss - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasrg_ASCTL_DC_decode_cx_loss - input parameter is NULL \n");
    return;
  }
// End debug information
 	cx->state = GRAAL_IDLE;
 	cx->iid4=0;
 	nasrg_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)cx->iid6);
 	nasrg_COMMON_flush_rb(cx);
#ifdef GRAAL_DEBUG_DC
 	printk("nasrg_ASCTL_DC_decode_cx_loss: CONN_LOSS_IND reception\n");
 	printk(" Primitive length %u\n",(int)(p->length));
 	printk(" Local Connection reference %u\n",p->nasRGDCPrimitive.conn_loss_ind.localConnectionRef);
 	nasrg_TOOL_print_state(cx->state);
#endif
}
//---------------------------------------------------------------------------
// Decode RB_ESTABLISH_CNF message from RRC
void nasrg_ASCTL_DC_decode_rb_establish_cnf(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
  struct rb_entity *rb;
  int rb_id;

// Start debug information
#ifdef GRAAL_DEBUG_DC
  printk("nasrg_ASCTL_DC_decode_rb_establish - begin \n");
#endif
  if (!cx || !p){
    printk("nasrg_ASCTL_DC_decode_rb_establish - input parameter is NULL \n");
    return;
  }
// End debug information
  rb_id = p->nasRGDCPrimitive.rb_establish_conf.rbId;
//  rb=graal_COMMON_search_rb(cx, rb_id);  // original version
  rb=nasrg_COMMON_search_rb(cx, rb_id - (GRAAL_RB_MAX_NUM * cx->lcr));
  //
  if (rb!=NULL){
    if (rb->state==GRAAL_CX_CONNECTING){
#ifdef GRAAL_DEBUG_DC
      printk("nasrg_ASCTL_DC_decode_rb_establish: RB_ESTABLISH_CNF received\n");
      printk(" Primitive length %u\n", p->length);
      printk(" Local Connection reference %u\n",p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef);
      printk(" RB Id %u\n",p->nasRGDCPrimitive.rb_establish_conf.rbId);
      printk(" SAP Id %u\n",p->nasRGDCPrimitive.rb_establish_conf.sapId);
      printk(" Status %u, Failure code %d, Cx state, RB state\n",p->nasRGDCPrimitive.rb_establish_conf.status, p->nasRGDCPrimitive.rb_establish_conf.fail_code);
      nasrg_TOOL_print_state(cx->state);
      nasrg_TOOL_print_state(rb->state);
#endif
      switch (p->nasRGDCPrimitive.rb_establish_conf.status){
        case ACCEPTED:
          rb->state = GRAAL_CX_DCH;
          rb->countimer=1;
          break;
        case FAILURE:
          printk("nasrg_ASCTL_DC_decode_rb_establish: RB_ESTABLISH_CNF rejected\n");
          rb->state = GRAAL_CX_CONNECTING_FAILURE;
          //delete rb
          break;
        default:
          printk("nasrg_ASCTL_DC_decode_rb_establish: RB_ESTABLISH_CNF reception, invalid status\n");
      }
    }
    else
      printk("nasrg_ASCTL_DC_decode_rb_establish: invalid state %u\n", cx->state);
  }
  else
    printk("nasrg_ASCTL_DC_decode_rb_establish: RB_ESTABLISH_CNF, No corresponding radio bearer\n");
}

//---------------------------------------------------------------------------
// Decode DATA_TRANSFER_IND message from RRC
void nasrg_ASCTL_DC_decode_data_transfer_ind(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
//  u8 graal_data[10];
  unsigned int graal_length;
  char data_type;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_decode_data_transfer - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasrg_ASCTL_DC_decode_data_transfer - input parameter is NULL \n");
    return;
  }
// End debug information
// Get first character
    graal_length = (p->nasRGDCPrimitive.data_transfer_ind.nasDataLength) -1;
		bytes_read += rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI], &data_type, 1);
//check if peer message
    if (data_type =='A'){
      // receive in a skbuff
			nasrg_COMMON_receive((p->length) + 1, graal_length, cx->sap[GRAAL_DC_OUTPUT_SAPI]);
    }else{
      // empty remaining data
      bytes_read += rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI], (gpriv->rbuffer)+ (p->length), graal_length);
      if (data_type=='Z'){
      // open radio bearer
         printk("nasrg_ASCTL_DC_decode_data_transfer: Opening Default Radio Bearer\n");
         nasrg_ASCTL_start_default_rb(cx);
      }else
         printk("nasrg_ASCTL_DC_decode_data_transfer: Error during reception of the message - Dropped\n");
    }
#ifdef GRAAL_DEBUG_DC
			printk("nasrg_ASCTL_DC_decode_data_transfer: DATA_TRANSFER_IND reception\n");
			printk(" Primitive length %u\n", p->length);
			printk(" Local Connection reference %u\n",p->nasRGDCPrimitive.data_transfer_ind.localConnectionRef);
			printk(" Data Length %u\n", p->nasRGDCPrimitive.data_transfer_ind.nasDataLength);
    	nasrg_TOOL_print_state(cx->state);
#endif
}

//---------------------------------------------------------------------------
// Decode MBMS_BEARER_ESTABLISH_CNF message from RRC
void nasrg_ASCTL_DC_decode_mbms_bearer_establish_cnf(struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
  int mbms_ix;
  int rb_id;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_decode_mbms_bearer_establish - begin \n");
#endif
  if (!p){
 	  printk("nasrg_ASCTL_DC_decode_mbms_bearer_establish - input parameter is NULL \n");
    return;
  }
// End debug information
  rb_id = p->nasRGDCPrimitive.mbms_establish_cnf.rbId;
  mbms_ix = 0;  // A revoir - find using cnxid...
  if (rb_id == gpriv->mbms_rb[mbms_ix].mbms_rbId){
    switch (p->nasRGDCPrimitive.rb_establish_conf.status){
      case ACCEPTED:
        gpriv->mbms_rb[mbms_ix].state = GRAAL_CX_DCH;
        gpriv->mbms_rb[mbms_ix].rab_id = gpriv->mbms_rb[mbms_ix].mbms_rbId;
        nasrg_ASCTL_start_mbmsclassifier(mbms_ix,&(gpriv->mbms_rb[mbms_ix]));
        break;
      case FAILURE:
        printk("nasrg_ASCTL_DC_decode_mbms_bearer_establish: RB_ESTABLISH_CNF rejected\n");
        gpriv->mbms_rb[mbms_ix].state = GRAAL_CX_CONNECTING_FAILURE; //supprimer l'entree
        break;
      default:
        printk("nasrg_ASCTL_DC_decode_mbms_bearer_establish: RB_ESTABLISH_CNF reception, invalid status\n");
    }
  }else
     printk(" nasrg_ASCTL_DC_decode_mbms_bearer_establish: invalid RB_Id %d\n", rb_id);

 #ifdef GRAAL_DEBUG_DC
 	printk(" nasrg_ASCTL_DC_decode_mbms_bearer_establish: MBMS_BEARER_ESTABLISH_CNF reception\n");
 	printk(" Primitive length %u\n",p->length);
 	printk(" rb_id %d, status %d\n",p->nasRGDCPrimitive.mbms_establish_cnf.rbId, p->nasRGDCPrimitive.mbms_establish_cnf.status);
  nasrg_TOOL_print_state(gpriv->mbms_rb[mbms_ix].state);
#endif
}

//---------------------------------------------------------------------------
// Decode MBMS_UE_NOTIFY_CNF message from RRC
void nasrg_ASCTL_DC_decode_mbms_ue_notify_cnf(struct cx_entity *cx, struct nas_rg_dc_element *p){
//---------------------------------------------------------------------------
  int i, j , k;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasrg_ASCTL_DC_decode_mbms_ue_notify - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasrg_ASCTL_DC_decode_mbms_ue_notify - input parameter is NULL \n");
    return;
  }
// End debug information
  if (p->nasRGDCPrimitive.mbms_ue_notify_cnf.mbmsStatus == ACCEPTED){
    for (i = 0; i<NASRG_MBMS_SVCES_MAX; i++){
      if (cx->requested_joined_services[i] >=0){
        for (j = 0; j<NASRG_MBMS_SVCES_MAX; j++){
          if (cx->joined_services[j] ==-1){
            cx->joined_services[j]= cx->requested_joined_services[i];
            cx->requested_joined_services[i]=-1;
            break;
          }
        }
      }
      if (cx->requested_left_services[i] >=0){
        for (k = 0; k<NASRG_MBMS_SVCES_MAX; k++){
          if (cx->joined_services[k] == cx->requested_left_services[i]){
            cx->joined_services[k]=-1;
            cx->requested_left_services[i]=-1;
            break;
          }
        }
      }
    }
  }

#ifdef GRAAL_DEBUG_DC
 	printk(" nasrg_ASCTL_DC_decode_mbms_ue_notify: MBMS_UE_NOTIFY_CNF reception\n");
 	printk(" Primitive length %u\n",p->length);
 	printk(" Local Connection reference %u\n",p->nasRGDCPrimitive.mbms_ue_notify_cnf.localConnectionRef);
  printk(" MBMS Status: %d\n", p->nasRGDCPrimitive.mbms_ue_notify_cnf.mbmsStatus);
  printk(" UE services currently joined \n");
  for (i = 0; i<NASRG_MBMS_SVCES_MAX; i++)
    printk ("%d * ", cx->joined_services[i]);
 	nasrg_TOOL_print_state(cx->state);
#endif
}

//---------------------------------------------------------------------------
// Check if anything in DC FIFO and process it (RG Finite State Machine)
int nasrg_ASCTL_DC_receive(struct cx_entity *cx){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC_DETAIL
  printk("nasrg_ASCTL_DC_receive - begin \n");
#endif
  if (!cx){
    printk("nasrg_ASCTL_DC_receive - input parameter cx is NULL \n");
    return 0;
  }
// End debug information

  bytes_read = rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI], gpriv->rbuffer, NAS_TL_SIZE);
  if (bytes_read>0){
    struct nas_rg_dc_element *p;
    p= (struct nas_rg_dc_element *)(gpriv->rbuffer);
    //get the rest of the primitive
    bytes_read += rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI], (u8 *)p+NAS_TL_SIZE, p->length-NAS_TL_SIZE);
    if (bytes_read!=p->length){
      printk("nasrg_ASCTL_DC_receive: Problem while reading primitive's header\n");
      return bytes_read;
    }
    switch (p->type){
    case CONN_ESTABLISH_IND :
      if (p->nasRGDCPrimitive.conn_establish_ind.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: CONN_ESTABLISH_IND reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.conn_establish_ind.localConnectionRef);
      else {
        switch(cx->state){
          case GRAAL_IDLE:
            nasrg_ASCTL_DC_decode_cx_establish_ind(cx,p);
            break;
          default:
            printk("nasrg_ASCTL_DC_receive: CONN_ESTABLISH_IND reception, invalid state %u\n", cx->state);
        }
      }
      break;
    case CONN_RELEASE_IND :
      if (p->nasRGDCPrimitive.conn_release_ind.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: CONN_RELEASE_IND reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.conn_release_ind.localConnectionRef);
      else{
        switch(cx->state){
          case GRAAL_CX_DCH:
            nasrg_ASCTL_DC_decode_cx_release_ind(cx,p);
            break;
          default:
            printk("nasrg_ASCTL_DC_receive: CONN_RELEASE_IND reception, invalid state %u\n", cx->state);
        }
      }
      break;
    case CONN_LOSS_IND:
      if (p->nasRGDCPrimitive.conn_loss_ind.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: CONN_LOSS_IND reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.conn_loss_ind.localConnectionRef);
      else{
        switch(cx->state){
          case GRAAL_CX_DCH:
            nasrg_ASCTL_DC_decode_cx_loss_ind(cx,p);
            break;
          default:
            printk("nasrg_ASCTL_DC_receive: CONN_LOSS_IND reception, invalid state %u\n", cx->state);
        }
      }
      break;
    case RB_ESTABLISH_CNF:
      if (p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: RB_ESTABLISH_CNF reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef);
      else{
        switch(cx->state){
          case GRAAL_CX_DCH:
            nasrg_ASCTL_DC_decode_rb_establish_cnf(cx,p);
            break;
          default:
            printk("nasrg_ASCTL_DC_receive: RB_ESTABLISH_CNF reception, invalid state %u\n", cx->state);
        }
      }
      break;
    case DATA_TRANSFER_IND:
      if (p->nasRGDCPrimitive.data_transfer_ind.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: DATA_TRANSFER_IND reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef);
      else{
        switch(cx->state){
          case GRAAL_CX_DCH:
            nasrg_ASCTL_DC_decode_data_transfer_ind(cx,p);
      	    break;
          default:
            printk("nasrg_ASCTL_DC_receive: DATA_TRANSFER_IND reception, invalid state %u\n", cx->state);
        }
      }
      break;
    // Temp - Should be in uplink GC-SAP
    case MBMS_BEARER_ESTABLISH_CNF:
//      if (p->nasRGDCPrimitive.mbms_ue_notify_cnf.localConnectionRef!=cx->lcr)
//        printk("nasrg_ASCTL_DC_receive: MBMS_BEARER_ESTABLISH_CNF reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef);
//      else
        nasrg_ASCTL_DC_decode_mbms_bearer_establish_cnf(p);
      break;
    case MBMS_UE_NOTIFY_CNF:
      if (p->nasRGDCPrimitive.mbms_ue_notify_cnf.localConnectionRef!=cx->lcr)
        printk("nasrg_ASCTL_DC_receive: MBMS_UE_NOTIFY_CNF reception, Local connection reference not correct %u\n", p->nasRGDCPrimitive.rb_establish_conf.localConnectionRef);
      else{
        switch(cx->state){
          case GRAAL_CX_DCH:
            nasrg_ASCTL_DC_decode_mbms_ue_notify_cnf(cx,p);
      	    break;
          default:
            printk("nasrg_ASCTL_DC_receive: MBMS_UE_NOTIFY_CNF reception, invalid state %u\n", cx->state);
        }
      }
      break;
    default :
      printk("nasrg_ASCTL_DC_receive: Invalid message received\n");
    }
  }
  return bytes_read;
}

#endif
