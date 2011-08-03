/***************************************************************************
                          nasmt_ascontrol.c  -  description
                             -------------------
    copyright            : (C) 2003 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#ifdef NODE_MT

#include "nasmt_variables.h"
#include "nasmt_proto.h"

//---------------------------------------------------------------------------
void nasmt_ASCTL_init(void){
//---------------------------------------------------------------------------
//	struct cx_entity *cx;
  int i;
  gpriv->next_sclassref = NASMT_DEFAULTRAB_CLASSREF;
  for (i = 0; i<NASMT_MBMS_SVCES_MAX; i++){
     gpriv->cx->joined_services[i]= -1;
   }
	printk("nasmt_ASCTL_init Complete\n");
}

//---------------------------------------------------------------------------
//For demo, add automatically a classifier
//Equivalent to class add send 0 -f qos <x> -cr 0
void nasmt_ASCTL_start_default_sclassifier(struct cx_entity *cx,struct rb_entity *rb){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("\nnasmt_ASCTL_start_default_sclass - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_start_default_sclass - input parameter cx is NULL \n");
    return;
  }
  if (rb==NULL){
 	  printk("nasmt_ASCTL_start_default_sclass - input parameter rb is NULL \n");
    return;
  }
// End debug information
//
  gc=nasmt_CLASS_add_sclassifier(cx, GRAAL_DSCP_DEFAULT, gpriv->next_sclassref);
//  gc=graal_CLASS_add_sclassifier(cx, 5, 0);
  if (gc==NULL){
 	  printk("nasmt_ASCTL_start_default_sclass - Error - Classifier not added \n");
    return;
  }
  gc->fct = nasmt_COMMON_QOS_send;
  gc->rab_id =rb->rab_id;
  gc->rb= rb;
  gc->version = NASMT_DEFAULTRAB_IPVERSION;
	gc->protocol= GRAAL_PROTOCOL_DEFAULT;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_ASCTL_start_default_sclass - end \n");
  nasmt_TOOL_print_classifier(gc);
#endif
}

//---------------------------------------------------------------------------
//For demo, add automatically a classifier
//Equivalent to class add send 0 -f qos <x> -cr 0
void nasmt_ASCTL_start_sclassifier(struct cx_entity *cx,struct rb_entity *rb){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("\nnasmt_ASCTL_start_sclass - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_start_sclass - input parameter cx is NULL \n");
    return;
  }
  if (rb==NULL){
 	  printk("nasmt_ASCTL_start_sclass - input parameter rb is NULL \n");
    return;
  }
// End debug information
//
  gc=nasmt_CLASS_add_sclassifier(cx, rb->dscp, gpriv->next_sclassref);
//  gc=graal_CLASS_add_sclassifier(cx, 5, 0);
  if (gc==NULL){
 	  printk("nasmt_ASCTL_start_sclass - Error - Classifier not added \n");
    return;
  }
  gc->fct = nasmt_COMMON_QOS_send;
  gc->rab_id =rb->rab_id;
  gc->rb= rb;
  gc->version = NASMT_DEFAULTRAB_IPVERSION;
	gc->protocol= GRAAL_PROTOCOL_DEFAULT;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_ASCTL_start_sclass - end \n");
  nasmt_TOOL_print_classifier(gc);
#endif
}

//---------------------------------------------------------------------------
void nasmt_ASCTL_timer(unsigned long data){
//---------------------------------------------------------------------------
	u8 cxi;
	struct cx_entity *cx;
	struct rb_entity *rb;
	spin_lock(&gpriv->lock);
#ifdef GRAAL_DEBUG_TIMER
	printk("nasmt_ASCTL_timer - begin \n");
#endif
	(gpriv->timer).function=nasmt_ASCTL_timer;
	(gpriv->timer).expires=jiffies+GRAAL_TIMER_TICK;
	(gpriv->timer).data=0L;
	for (cxi=0; cxi<GRAAL_CX_MAX;++cxi){
		cx=gpriv->cx+cxi;
    if (cx==NULL){
      #ifdef GRAAL_DEBUG_TIMER
      printk("nasmt_ASCTL_timer - No pointer for connection %d \n", cxi);
      #endif
      continue;
    }
		if (cx->countimer!=GRAAL_TIMER_IDLE){
#ifdef GRAAL_DEBUG_TIMER
  	printk("nasmt_ASCTL_timer: lcr %u, countimer %u\n", cx->lcr, cx->countimer);
#endif
			if (cx->countimer==0){
				switch (cx->state){
				case GRAAL_CX_CONNECTING:
				case GRAAL_CX_CONNECTING_FAILURE:
					if (cx->retry<gpriv->retry_limit)
						nasmt_ASCTL_DC_send_cx_establish_request(cx);
					else{
						printk("nasmt_ASCTL_timer: Establishment failure\n");
						cx->state=GRAAL_IDLE;
						cx->retry=0;
						cx->countimer=GRAAL_TIMER_IDLE;
					}
					break;
				case GRAAL_CX_RELEASING_FAILURE:
					nasmt_ASCTL_DC_send_cx_release_request(cx);
					break;
				default:
					printk("nasmt_ASCTL_timer: default value\n");
					cx->countimer=GRAAL_TIMER_IDLE;
				}
			}
			else
				--cx->countimer;
		}
		for (rb=cx->rb;rb!=NULL; rb=rb->next){
			if (rb->countimer!=GRAAL_TIMER_IDLE){
      #ifdef GRAAL_DEBUG_TIMER
      	printk("nasmt_ASCTL_timer : rb countimer %d, rb state %d\n", rb->countimer, rb->state);
      #endif
				if (rb->countimer==0){
					switch (rb->state){
            case GRAAL_RB_DCH:
              #ifdef DEMO_3GSM
              if (cx->num_rb == 1){
                 nasmt_ASCTL_start_default_sclassifier(cx, rb);
              }
              #endif
              nasmt_ASCTL_start_sclassifier(cx, rb);
  						rb->countimer=GRAAL_TIMER_IDLE;
              break;
  					default:
  						rb->countimer=GRAAL_TIMER_IDLE;
					}
				}else{
					--rb->countimer;
         	printk("nasmt_ASCTL_timer : rb countimer-- %d, rb state %d\n", rb->countimer, rb->state);
        }
			}
		}
	}
	add_timer(&gpriv->timer);
	spin_unlock(&gpriv->lock);
}


//---------------------------------------------------------------------------
// Request the establishment of a connexion (DC channel)
int nasmt_ASCTL_DC_send_cx_establish_request(struct cx_entity *cx){
//---------------------------------------------------------------------------
	struct nas_ue_dc_element *p;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_cx_establish - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_send_cx_establish - input parameter cx is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information

	switch (cx->state){
	case GRAAL_CX_CONNECTING:
	case GRAAL_CX_CONNECTING_FAILURE:
	case GRAAL_IDLE:
		p= (struct nas_ue_dc_element *)(gpriv->xbuffer);
		p->type = CONN_ESTABLISH_REQ;
		p->length =  NAS_TL_SIZE + sizeof(struct NASConnEstablishReq);
		p->nasUEDCPrimitive.conn_establish_req.localConnectionRef = cx->lcr;
		p->nasUEDCPrimitive.conn_establish_req.cellId = cx->cellid;
#ifdef GRAAL_DEBUG_DC
    printk ("\nCONN_ESTABLISH_REQ Buffer to Xmit: ");
    nasmt_TOOL_print_buffer((char *)p,p->length);
#endif
		++cx->retry;
		bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
		cx->countimer=gpriv->timer_establishment;
		if (bytes_wrote==p->length){
			cx->state=GRAAL_CX_CONNECTING;
#ifdef GRAAL_DEBUG_DC
			printk("nasmt_ASCTL_DC_send_cx_establish - Message sent successfully in DC-FIFO\n");
			printk(" Local Connection reference %u\n", p->nasUEDCPrimitive.conn_establish_req.localConnectionRef);
			printk(" Cell Identification %u\n", p->nasUEDCPrimitive.conn_establish_req.cellId);
			nasmt_TOOL_print_state(cx->state);
#endif
		}else{
			cx->state=GRAAL_CX_CONNECTING_FAILURE;
			printk("nasmt_ASCTL_DC_send_cx_establish - Message sent failure in DC-FIFO\n");
			nasmt_TOOL_print_state(cx->state);
		}
		return bytes_wrote;
	default:
		return -GRAAL_ERROR_NOTIDLE;
#ifdef GRAAL_DEBUG_DC
	  printk("nasmt_ASCTL_DC_send_cx_establish - GRAAL_ERROR_NOTIDLE \n");
#endif
	}
}

//---------------------------------------------------------------------------
// Request the release of a connexion (DC channel)
int nasmt_ASCTL_DC_send_cx_release_request(struct cx_entity *cx){
//---------------------------------------------------------------------------
	struct nas_ue_dc_element *p;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_cx_release - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_send_cx_release - input parameter cx is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
	switch (cx->state)
	{
	case GRAAL_CX_RELEASING_FAILURE:
	case GRAAL_CX_DCH:
		p= (struct nas_ue_dc_element *)(gpriv->xbuffer);
		p->type = CONN_RELEASE_REQ;
		p->length =  NAS_TL_SIZE + sizeof(struct NASConnReleaseReq);
		p->nasUEDCPrimitive.conn_release_req.localConnectionRef = cx->lcr;
		p->nasUEDCPrimitive.conn_release_req.releaseCause = GRAAL_CX_RELEASE_UNDEF_CAUSE;
		bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
		if (bytes_wrote==p->length)
		{
			cx->state=GRAAL_IDLE;
			cx->iid4=0;
//			graal_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)cx->iid6);
			nasmt_COMMON_flush_rb(cx);
      nasmt_CLASS_flush_sclassifier(cx);

#ifdef GRAAL_DEBUG_DC
			printk("nasmt_ASCTL_DC_send_cx_release - Message sent successfully in DC-FIFO\n");
			printk(" Local Connection Reference %u\n", p->nasUEDCPrimitive.conn_release_req.localConnectionRef);
			printk(" Release Cause %u\n", p->nasUEDCPrimitive.conn_release_req.releaseCause);
			nasmt_TOOL_print_state(cx->state);
#endif
		}
		else
		{
			++cx->retry;
			cx->countimer=gpriv->timer_release;
			cx->state=GRAAL_CX_RELEASING_FAILURE;
			printk("nasmt_ASCTL_DC_send_cx_release - Message sent failure in DC-FIFO\n");
			nasmt_TOOL_print_state(cx->state);
		}
		return bytes_wrote;
	default:
		return -GRAAL_ERROR_NOTCONNECTED;
#ifdef GRAAL_DEBUG_DC
	  printk("nasmt_ASCTL_DC_send_cx_release - GRAAL_ERROR_NOTCONNECTED \n");
#endif
	}
}

//---------------------------------------------------------------------------
// Request the transfer of data (DC SAP)
void nasmt_ASCTL_DC_send_sig_data_request(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc){
//---------------------------------------------------------------------------
	struct nas_ue_dc_element *p;
  char data_type = 'A';

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_sig_data - begin \n");
#endif
  if (skb==NULL){
 	  printk("nasmt_ASCTL_DC_send_sig_data - input parameter skb is NULL \n");
    return;
  }
  if (gc==NULL){
 	  printk("nasmt_ASCTL_DC_send_sig_data - input parameter gc is NULL \n");
    return;
  }
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_send_sig_data - input parameter cx is NULL \n");
    return;
  }
// End debug information
	if (cx->state!=GRAAL_CX_DCH){
		printk("nasmt_ASCTL_DC_send_sig_data - Not connected, so the message is dropped\n");
		++gpriv->stats.tx_dropped;
		return;
	}
	p = (struct nas_ue_dc_element *)(gpriv->xbuffer);
	p->type = DATA_TRANSFER_REQ;
	p->length =  NAS_TL_SIZE + sizeof(struct NASDataReq);
	p->nasUEDCPrimitive.data_transfer_req.localConnectionRef = cx->lcr;
	p->nasUEDCPrimitive.data_transfer_req.priority = 3;  // TBD
	p->nasUEDCPrimitive.data_transfer_req.nasDataLength = (skb->len)+1; //adds category character
	bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
	if (bytes_wrote!=p->length){
		printk("nasmt_ASCTL_DC_send_sig_data - Header sent failure in DC-FIFO\n");
		return;
	}
	bytes_wrote += rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], &data_type, 1);
	bytes_wrote += rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], skb->data, skb->len);
	if (bytes_wrote != p->length + skb->len + 1){
		printk("nasmt_ASCTL_DC_send_sig_data - Data sent failure in DC-FIFO\n");
		return;
	}
	gpriv->stats.tx_bytes   += skb->len;
	gpriv->stats.tx_packets ++;
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_sig_data - end \n");
#endif
}

//---------------------------------------------------------------------------
// Request the transfer of data (DC SAP)
void nasmt_ASCTL_DC_send_peer_sig_data_request(struct cx_entity *cx, u8 sig_category){
//---------------------------------------------------------------------------
	struct nas_ue_dc_element *p;
  u8 graal_data[10];
  unsigned int graal_length;
  char data_type = 'Z';

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_peer_sig_data - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_send_peer_sig_data - input parameter cx is NULL \n");
    return;
  }
// End debug information

	if (cx->state!=GRAAL_CX_DCH)
	{
		printk("nasmt_ASCTL_DC_send_peer_sig_data: Not connected, so the message is dropped\n");
		return;
	}
  // Initialize peer message
  graal_length = 10;
  memset (graal_data, 0, graal_length);
  graal_data[0]= sig_category;
  //
	p = (struct nas_ue_dc_element *)(gpriv->xbuffer);
	p->type = DATA_TRANSFER_REQ;
	p->length =  NAS_TL_SIZE + sizeof(struct NASDataReq);
	p->nasUEDCPrimitive.data_transfer_req.localConnectionRef = cx->lcr;
	p->nasUEDCPrimitive.data_transfer_req.priority = 3;  // TBD
	p->nasUEDCPrimitive.data_transfer_req.nasDataLength = (graal_length)+1; //adds category character
	bytes_wrote = rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], p, p->length);
	if (bytes_wrote!=p->length)
	{
		printk("nasmt_ASCTL_DC_send_peer_sig_data - Header sent failure in DC-FIFO\n");
		return;
	}
	bytes_wrote += rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], &data_type, 1);
	bytes_wrote += rtf_put(cx->sap[GRAAL_DC_INPUT_SAPI], (char *)graal_data, graal_length);
	if (bytes_wrote != p->length + graal_length + 1){
		printk("nasmt_ASCTL_DC_send_peer_sig_data - Data sent failure in DC-FIFO\n");
		return;
	}
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_send_peer_sig_data - end \n");
#endif
}

//---------------------------------------------------------------------------
// Decode CONN_ESTABLISH_RESP message from RRC
void nasmt_ASCTL_DC_decode_cx_establish_resp(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
  u8 sig_category;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_cx_establish - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_decode_cx_establish - input parameter cx is NULL \n");
    return;
  }
  if (p==NULL){
 	  printk("nasmt_ASCTL_DC_decode_cx_establish - input parameter p is NULL \n");
    return;
  }
// End debug information
    cx->retry=0;
    if (p->nasUEDCPrimitive.conn_establish_resp.status == TERMINATED){
    	cx->state=GRAAL_CX_DCH; //to be changed to GRAAL_CX_FACH
    	cx->iid4=1;
    	//graal_TOOL_imei2iid(GRAAL_RG_IMEI, (u8 *)cx->iid6);
      sig_category = GRAAL_CMD_OPEN_RB;
//For demo, add automatically a radio bearer
#ifdef DEMO_3GSM
 	    printk("nasmt_ASCTL_DC_decode_cx_establish - sig_category %u \n", sig_category);
      nasmt_ASCTL_DC_send_peer_sig_data_request(cx, sig_category);
#endif
    }else{
    	cx->state=GRAAL_IDLE;
    }
#ifdef GRAAL_DEBUG_DC
		printk(" nasmt_ASCTL_DC_decode_cx_establish: CONN_ESTABLISH_RESP\n");
		printk(" Local Connection reference %u\n",p->nasUEDCPrimitive.conn_establish_resp.localConnectionRef);
		printk(" Connection Establishment status %u\n",p->nasUEDCPrimitive.conn_establish_resp.status);
		nasmt_TOOL_print_state(cx->state);
#endif
}

//---------------------------------------------------------------------------
// Decode CONN_LOSS_IND message from RRC
void nasmt_ASCTL_DC_decode_cx_loss_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_cx_loss - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_decode_cx_loss - input parameter cx is NULL \n");
    return;
  }
  if (p==NULL){
 	  printk("nasmt_ASCTL_DC_decode_cx_loss - input parameter p is NULL \n");
    return;
  }
// End debug information
 		cx->state=GRAAL_IDLE;
 		cx->iid4=0;
 		//graal_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)cx->iid6);
 		nasmt_COMMON_flush_rb(cx);
#ifdef GRAAL_DEBUG_DC
		printk(" nasmt_ASCTL_DC_decode_cx_loss: CONN_LOSS_IND reception\n");
		printk(" Local Connection reference %u\n", p->nasUEDCPrimitive.conn_loss_ind.localConnectionRef);
		nasmt_TOOL_print_state(cx->state);
#endif
}

//---------------------------------------------------------------------------
// Decode CONN_RELEASE_IND message from RRC
//void nasmt_ASCTL_DC_decode_cx_release_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
//			printk("\t\tCONN_RELEASE_IND\n");
//			printk("\t\tLocal Connection reference %u\n", p->nasUEDCPrimitive.conn_release_ind.localConnectionRef);
//			printk("\t\tRelease cause %u\n", p->nasRGDCPrimitive.conn_release_ind.releaseCause);
//			if (gpriv->cx[cxi].state==GRAAL_CX_DCH)
//			{
//				gpriv->cx[cxi].state=GRAAL_IDLE;
//				printk("\t\tMobile no more connected\n");
//				return bytes_read;
//			}
//			printk("\t\tIncoherent state %u\n", gpriv->cx[cxi].state);
//			return bytes_read;
//}

//---------------------------------------------------------------------------
// Decode DATA_TRANSFER_IND message from RRC
void nasmt_ASCTL_DC_decode_sig_data_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_sig_data - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_decode_sig_data - input parameter cx is NULL \n");
    return;
  }
  if (p==NULL){
 	  printk("nasmt_ASCTL_DC_decode_sig_data - input parameter p is NULL \n");
    return;
  }
// End debug information

			nasmt_COMMON_receive(p->length, p->nasUEDCPrimitive.data_transfer_ind.nasDataLength, cx->sap[GRAAL_DC_OUTPUT_SAPI]);
#ifdef GRAAL_DEBUG_DC
			printk(" nasmt_ASCTL_DC_decode_sig_data: DATA_TRANSFER_IND reception\n");
			printk(" Local Connection reference %u\n",p->nasUEDCPrimitive.data_transfer_ind.localConnectionRef);
			printk(" Signaling Priority %u\n",p->nasUEDCPrimitive.data_transfer_ind.priority);
			printk(" NAS Data length %u\n",p->nasUEDCPrimitive.data_transfer_ind.nasDataLength);
			printk(" NAS Data string %s\n", (u8 *)p+p->length);
#endif

}
//---------------------------------------------------------------------------
// Decode RB_ESTABLISH_IND message from RRC
void nasmt_ASCTL_DC_decode_rb_establish_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
 	struct rb_entity *rb;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_rb_establish - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_decode_rb_establish - input parameter cx is NULL \n");
    return;
  }
  if (p==NULL){
 	  printk("nasmt_ASCTL_DC_decode_rb_establish - input parameter p is NULL \n");
    return;
  }
// End debug information
 	rb=nasmt_COMMON_search_rb(cx, p->nasUEDCPrimitive.rb_release_ind.rbId);
 		if (rb==NULL){
 			rb=nasmt_COMMON_add_rb(cx, p->nasUEDCPrimitive.rb_establish_ind.rbId, p->nasUEDCPrimitive.rb_establish_ind.QoSclass);
 			rb->state=GRAAL_RB_DCH;
    	cx->state=GRAAL_CX_DCH;
      rb->dscp = p->nasUEDCPrimitive.rb_establish_ind.dscp;
      rb->sapi = p->nasUEDCPrimitive.rb_establish_ind.sapId;
			rb->countimer=1;
   #ifdef GRAAL_DEBUG_DC
    	printk(" nasmt_ASCTL_DC_decode_rb_establish: RB_ESTABLISH_IND reception\n");
    	printk(" Local Connection reference %u\n",p->nasUEDCPrimitive.rb_establish_ind.localConnectionRef);
    	printk(" Radio Bearer Identity %u \n",p->nasUEDCPrimitive.rb_establish_ind.rbId);
    	printk(" QoS Traffic Class %u\n",p->nasUEDCPrimitive.rb_establish_ind.QoSclass);
    	printk(" DSCP Code %u\n",p->nasUEDCPrimitive.rb_establish_ind.dscp);
    	printk(" SAP Id %u\n",p->nasUEDCPrimitive.rb_establish_ind.sapId);
    	nasmt_TOOL_print_state(cx->state);
      nasmt_TOOL_print_rb_entity(rb);
   #endif
 		}else
 			printk("GRAAL_MT_DC_DECODE_RB_ESTABLISH_IND: RB_ESTABLISH_IND reception, Radio bearer already opened\n");
}

//---------------------------------------------------------------------------
// Decode RB_RELEASE_IND message from RRC
void nasmt_ASCTL_DC_decode_rb_release_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
  struct rb_entity *rb;
  u8 dscp;

// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_rb_release - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasmt_ASCTL_DC_decode_rb_release - input parameter is NULL \n");
    return;
  }
// End debug information

 	rb=nasmt_COMMON_search_rb(cx, p->nasUEDCPrimitive.rb_release_ind.rbId);
 	if (rb!=NULL){
#ifdef GRAAL_DEBUG_DC
		printk(" nasmt_ASCTL_DC_decode_rb_release : RB_RELEASE_IND reception\n");
		printk(" Local Connection reference %u\n",p->nasUEDCPrimitive.rb_release_ind.localConnectionRef);
		printk(" Radio Bearer Identity %u\n",p->nasUEDCPrimitive.rb_release_ind.rbId);
		nasmt_TOOL_print_state(cx->state);
#endif
    // rb->state=GRAAL_IDLE;
    dscp = rb->dscp;
    nasmt_COMMON_del_rb(cx, p->nasUEDCPrimitive.rb_release_ind.rbId, dscp);
  }else
 		printk("nasmt_ASCTL_DC_decode_rb_release: RB_RELEASE_IND reception, No corresponding radio bearer\n");

}
//---------------------------------------------------------------------------
// Decode MEASUREMENT_IND message from RRC
void nasmt_ASCTL_DC_decode_measurement_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
	u8 i;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_measurement - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_decode_measurement - input parameter cx is NULL \n");
    return;
  }
  if (p==NULL){
 	  printk("nasmt_ASCTL_DC_decode_measurement - input parameter p is NULL \n");
    return;
  }
// End debug information
#ifdef GRAAL_DEBUG_DC_MEASURE
 	printk(" nasmt_ASCTL_DC_decode_measurement : MEASUREMENT_IND reception\n");
 	printk(" Local Connection reference: %u\n", p->nasUEDCPrimitive.measurement_ind.localConnectionRef);
 	printk(" Number of RGs: %u\n", p->nasUEDCPrimitive.measurement_ind.nb_rg);
 	nasmt_TOOL_print_state(cx->state);
 	for (i=0; i<p->nasUEDCPrimitive.measurement_ind.nb_rg; ++i){
 		printk(" RG[%u]:  Cell_Id %u, Level: %u\n", i,
 				p->nasUEDCPrimitive.measurement_ind.measures[i].cell_id,
 				p->nasUEDCPrimitive.measurement_ind.measures[i].level);
  }
#endif
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_measurement - Local cell %d\n",p->nasUEDCPrimitive.measurement_ind.measures[0].cell_id);
#endif
  cx->num_measures = p->nasUEDCPrimitive.measurement_ind.nb_rg;
  for (i=0; i<cx->num_measures; i++){
      cx->meas_cell_id[i]= (int)(p->nasUEDCPrimitive.measurement_ind.measures[i].cell_id);
      cx->meas_level[i] = (int)(p->nasUEDCPrimitive.measurement_ind.measures[i].level);
      //npriv->provider_id[i]=;
  }
  cx->provider_id[0]=25;
  cx->provider_id[1]=1;
  cx->provider_id[2]=25;

}





//---------------------------------------------------------------------------
// Decode MBMS_UE_NOTIFY_IND message from RRC
void nasmt_ASCTL_DC_decode_mbms_ue_notify_ind(struct cx_entity *cx, struct nas_ue_dc_element *p){
//---------------------------------------------------------------------------
	u8 i, j, k;
// Start debug information
#ifdef GRAAL_DEBUG_DC
	printk("nasmt_ASCTL_DC_decode_mbms_ue_notify - begin \n");
#endif
  if (!cx || !p){
 	  printk("nasmt_ASCTL_DC_decode_mbms_ue_notify - input parameter is NULL \n");
    return;
  }

  for (i = 0; i<NASMT_MBMS_SVCES_MAX; i++){
    if (p->nasUEDCPrimitive.mbms_ue_notify_ind.joined_services[i].mbms_serviceId >=0){
      for (j = 0; j<NASMT_MBMS_SVCES_MAX; j++){
        if (cx->joined_services[j] ==-1){
          cx->joined_services[j]= p->nasUEDCPrimitive.mbms_ue_notify_ind.joined_services[i].mbms_serviceId;
          break;
        }
      }
    }
    if (p->nasUEDCPrimitive.mbms_ue_notify_ind.left_services[i].mbms_serviceId >=0){
      for (k = 0; k<NASMT_MBMS_SVCES_MAX; k++){
        if (cx->joined_services[k] == p->nasUEDCPrimitive.mbms_ue_notify_ind.left_services[i].mbms_serviceId){
          cx->joined_services[k]=-1;
          break;
        }
      }
    }
  }

// End debug information
#ifdef GRAAL_DEBUG_DC
 	printk(" nasmt_ASCTL_DC_decode_mbms_ue_notify : MBMS_UE_NOTIFY_IND reception\n");
 	printk(" Local Connection reference: %u\n", p->nasUEDCPrimitive.mbms_ue_notify_ind.localConnectionRef);
 	nasmt_TOOL_print_state(cx->state);
  printk("Joined services: ");
  for (i = 0; i<MAX_MBMS_SERVICES && (int) (p->nasUEDCPrimitive.mbms_ue_notify_ind.joined_services[i].mbms_serviceId) >= 0; i++)
  	printk("%d    ", (p->nasUEDCPrimitive.mbms_ue_notify_ind.joined_services[i].mbms_serviceId));
  printk("\n");
  printk("Left services: ");
  for (i = 0; i<MAX_MBMS_SERVICES && (int) (p->nasUEDCPrimitive.mbms_ue_notify_ind.left_services[i].mbms_serviceId) >= 0; i++)
  	printk("%d    ", (p->nasUEDCPrimitive.mbms_ue_notify_ind.left_services[i].mbms_serviceId));
  printk("\n");
#endif
}

//---------------------------------------------------------------------------
// Check if anything in DC FIFO and decode it (MT)
int nasmt_ASCTL_DC_receive(struct cx_entity *cx){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DC_DETAIL
	printk("nasmt_ASCTL_DC_receive - begin \n");
#endif
  if (cx==NULL){
 	  printk("nasmt_ASCTL_DC_receive - input parameter cx is NULL \n");
    return GRAAL_ERROR_NOTCORRECTVALUE;
  }
// End debug information
	bytes_read = rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI] , gpriv->rbuffer, NAS_TL_SIZE);
	if (bytes_read>0){
		struct nas_ue_dc_element *p;

		p= (struct nas_ue_dc_element *)(gpriv->rbuffer);
		//get the rest of the primitive
		bytes_read += rtf_get(cx->sap[GRAAL_DC_OUTPUT_SAPI], (u8 *)p+NAS_TL_SIZE, p->length-NAS_TL_SIZE);
		if (bytes_read!=p->length){
			printk("nasmt_ASCTL_DC_receive: Problem while reading primitive header\n");
			return bytes_read;
		}
		switch (p->type){
		case CONN_ESTABLISH_RESP :
			if (p->nasUEDCPrimitive.conn_establish_resp.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: CONN_ESTABLISH_RESP, Local connection reference not correct %u\n",p->nasUEDCPrimitive.conn_establish_resp.localConnectionRef);
			else{
  			switch (cx->state){
        case GRAAL_CX_CONNECTING:
        	nasmt_ASCTL_DC_decode_cx_establish_resp(cx,p);   // process message
          break;
  			default:
  				printk("nasmt_ASCTL_DC_receive: CONN_ESTABLISH_RESP reception, Invalid state %u\n", cx->state);
  			}
			}
			break;
		case CONN_LOSS_IND :
			if (p->nasUEDCPrimitive.conn_loss_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: CONN_LOSS_IND reception, Local connection reference not correct %u\n", p->nasUEDCPrimitive.conn_loss_ind.localConnectionRef);
			else{
   			switch (cx->state){
   		  	case GRAAL_CX_RELEASING_FAILURE:
   				  cx->retry=0;
   		  	case GRAAL_CX_DCH:
   					nasmt_ASCTL_DC_decode_cx_loss_ind(cx,p);   // process message
      			break;
   	  		default:
   			  	printk("nasmt_ASCTL_DC_receive: CONN_LOSS_IND reception, Invalid state %u", cx->state);
   			}
      }
			break;
//		case CONN_RELEASE_IND :
//			break;
		case DATA_TRANSFER_IND :
			if (p->nasUEDCPrimitive.data_transfer_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: DATA_TRANSFER_IND reception, Local connection reference not correct %u\n", p->nasUEDCPrimitive.conn_loss_ind.localConnectionRef);
			else{
   			switch (cx->state){
   		  	case GRAAL_CX_FACH:
   		  	case GRAAL_CX_DCH:
   					nasmt_ASCTL_DC_decode_sig_data_ind(cx,p);   // process message
      			break;
   	  		default:
   			  	printk("nasmt_ASCTL_DC_receive: DATA_TRANSFER_IND reception, Invalid state %u", cx->state);
   			}
      }
			break;
		case RB_ESTABLISH_IND :
			if (p->nasUEDCPrimitive.rb_establish_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: RB_ESTABLISH_IND reception, Local connexion reference not correct %u\n", p->nasUEDCPrimitive.rb_establish_ind.localConnectionRef);
			else{
   			switch (cx->state){
   		  	case GRAAL_CX_FACH:
   		  	case GRAAL_CX_DCH:
   					nasmt_ASCTL_DC_decode_rb_establish_ind(cx,p);   // process message
      			break;
   	  		default:
   			  	printk("nasmt_ASCTL_DC_receive: RB_ESTABLISH_IND reception, Invalid state %u", cx->state);
   			}
      }
			break;
		case RB_RELEASE_IND :
			if (p->nasUEDCPrimitive.rb_release_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: RB_RELEASE_IND reception, Local connection reference not correct %u\n", p->nasUEDCPrimitive.rb_release_ind.localConnectionRef);
			else{
   			switch (cx->state){
   		  	case GRAAL_CX_DCH:
   					nasmt_ASCTL_DC_decode_rb_release_ind(cx,p);   // process message
      			break;
   	  		default:
   			  	printk("nasmt_ASCTL_DC_receive: RB_RELEASE_IND reception, Invalid state %u", cx->state);
   			}
			}
			break;
		case MEASUREMENT_IND :
			if (p->nasUEDCPrimitive.measurement_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: MEASUREMENT_IND reception, Local connection reference not correct %u\n", p->nasUEDCPrimitive.measurement_ind.localConnectionRef);
			else{
				nasmt_ASCTL_DC_decode_measurement_ind(cx,p);
			}
			break;
		case MBMS_UE_NOTIFY_IND :
			if (p->nasUEDCPrimitive.rb_release_ind.localConnectionRef!=cx->lcr)
				printk("nasmt_ASCTL_DC_receive: MBMS_UE_NOTIFY_IND reception, Local connection reference not correct %u\n", p->nasUEDCPrimitive.rb_release_ind.localConnectionRef);
			else{
   			switch (cx->state){
   		  	case GRAAL_CX_DCH:
   					nasmt_ASCTL_DC_decode_mbms_ue_notify_ind(cx,p);   // process message
      			break;
   	  		default:
   			  	printk("nasmt_ASCTL_DC_receive: MBMS_UE_NOTIFY_IND reception, Invalid state %u", cx->state);
   			}
			}
			break;
		default :
			printk("nasmt_ASCTL_DC_receive: Invalid message received\n");
		}
	}
#ifdef GRAAL_DEBUG_DC_DETAIL
	printk("nasmt_ASCTL_DC_receive - end \n");
#endif
	return bytes_read;
}

//---------------------------------------------------------------------------
// Check if anything in GC FIFO and decode it (MT)
int nasmt_ASCTL_GC_receive(void){
//---------------------------------------------------------------------------
#ifdef GRAAL_DEBUG_GC
	printk("nasmt_ASCTL_GC_receive - begin \n");
#endif
	bytes_read = rtf_get(gpriv->sap[GRAAL_GC_SAPI], gpriv->rbuffer, NAS_TL_SIZE);
	if (bytes_read>0)
	{
		struct nas_ue_gc_element *p;
		p= (struct nas_ue_gc_element *)(gpriv->rbuffer);
		//get the rest of the primitive
		bytes_read += rtf_get(gpriv->sap[GRAAL_GC_SAPI], (u8 *)p+NAS_TL_SIZE, p->length-NAS_TL_SIZE);
		if (bytes_read!=p->length)
		{
			printk("nasmt_ASCTL_GC_receive: Problem while reading primitive's header\n");
			return bytes_read;
		}
		// start decoding message
		switch (p->type)
		{
		case INFO_BROADCAST_IND :
			bytes_read += rtf_get(gpriv->sap[GRAAL_GC_SAPI], (u8 *)p+p->length, p->nasUEGCPrimitive.broadcast_ind.nasDataLength);
			if (bytes_read!=p->length+p->nasUEGCPrimitive.broadcast_ind.nasDataLength)
			{
				printk("nasmt_ASCTL_GC_receive: INFO_BROADCAST_IND reception, Problem while reading primitive's data\n");
				return bytes_read;
			}
#ifdef GRAAL_DEBUG_GC
			printk(" nasmt_ASCTL_GC_receive : INFO_BROADCAST_IND reception\n");
			printk(" Primitive length %d \n", (int)(p->type));
			printk(" Data length %u\n", p->nasUEGCPrimitive.broadcast_ind.nasDataLength);
			printk(" Data string %s\n", (u8 *)p+p->length);
#endif
			return bytes_read;
		default :
			printk("nasmt_ASCTL_GC_receive: Invalid message received\n");
			return -1;
		}
	}
	 else
	 	return -1;
}

#endif
