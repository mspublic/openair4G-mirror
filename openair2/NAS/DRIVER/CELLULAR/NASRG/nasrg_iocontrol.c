/***************************************************************************
                          nasrg_iocontrol.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#include "nasrg_variables.h"
#include "nasrg_iocontrol.h"
#include "nasrg_proto.h"

//#include <linux/in.h>
#include <asm/uaccess.h>
#include <asm/checksum.h>
#include <asm/uaccess.h>


// Statistic
///////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
void nasrg_set_msg_statistic_reply(struct graal_msg_statistic_reply *msgrep){
//---------------------------------------------------------------------------
	msgrep->rx_packets=gpriv->stats.rx_packets;
	msgrep->tx_packets=gpriv->stats.tx_packets;
	msgrep->rx_bytes=gpriv->stats.rx_bytes;
	msgrep->tx_bytes=gpriv->stats.tx_bytes;
	msgrep->rx_errors=gpriv->stats.rx_errors;
	msgrep->tx_errors=gpriv->stats.tx_errors;
	msgrep->rx_dropped=gpriv->stats.rx_dropped;
	msgrep->tx_dropped=gpriv->stats.tx_dropped;
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_statistic_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_statistic_reply msgrep;
	printk("nasrg_ioCTL_statistic: stat requested\n");
	nasrg_set_msg_statistic_reply(&msgrep);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_ioCTL_statistic: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Connections List
//---------------------------------------------------------------------------
void nasrg_set_msg_cx_list_reply(u8 *msgrep){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	nasLocalConnectionRef_t lcr;
	struct graal_msg_cx_list_reply *list;
	msgrep[0]=GRAAL_CX_MAX;
	list=(struct graal_msg_cx_list_reply *)(msgrep+1);
	for(lcr=0;lcr<GRAAL_CX_MAX;++lcr)
	{
		cx=nasrg_COMMON_search_cx(lcr);
		list[lcr].lcr=lcr;
		list[lcr].state=cx->state;
		list[lcr].cellid=cx->cellid;
		list[lcr].iid4=cx->iid4;
		list[lcr].iid6[0]=cx->iid6[0];
		list[lcr].iid6[1]=cx->iid6[1];
		list[lcr].num_rb=cx->num_rb;
		list[lcr].nsclassifier=cx->nsclassifier;
    printk("nasrg_set_msg_cx_list: nsc=%u\n",cx->nsclassifier);
	}
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_cx_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_CX_MAX*sizeof(struct graal_msg_cx_list_reply)+1];
	printk("nasrg_ioCTL_cx_list: connection list requested\n");
	nasrg_set_msg_cx_list_reply(msgrep);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_CX_MAX*sizeof(struct graal_msg_cx_list_reply)+1))
	{
		printk("nasrg_ioCTL_cx_list: copy_to_user failure\n");
		return -EFAULT;
	}
	printk("nasrg_ioCTL_cx_list: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connection Establishment
//---------------------------------------------------------------------------
void nasrg_set_msg_cx_establishment_reply(struct graal_msg_cx_establishment_reply *msgrep, struct graal_msg_cx_establishment_request *msgreq){
//---------------------------------------------------------------------------
	msgrep->status=-GRAAL_ERROR_NOTMT;
}
//---------------------------------------------------------------------------
int nasrg_ioCTL_cx_establishment_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_cx_establishment_request msgreq;
	struct graal_msg_cx_establishment_reply msgrep;
	printk("nasrg_ioCTL_cx_establishment: connection establishment requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_cx_establishment: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_cx_establishment_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_ioCTL_cx_establishment: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connection Release
//---------------------------------------------------------------------------
void nasrg_set_msg_cx_release_reply(struct graal_msg_cx_release_reply *msgrep, struct graal_msg_cx_release_request *msgreq){
//---------------------------------------------------------------------------
	msgrep->status=-GRAAL_ERROR_NOTMT;
}

//---------------------------------------------------------------------------
// Request the release of a connection
int nasrg_ioCTL_cx_release_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_cx_release_request msgreq;
	struct graal_msg_cx_release_reply msgrep;

	printk("nasrg_set_msg_cx_release: connection release requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_set_msg_cx_release: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_cx_release_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_set_msg_cx_release: copy_to_user failure\n");
		return -EFAULT;
	}
  printk("nasrg_set_msg_cx_release: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer List
//---------------------------------------------------------------------------
void nasrg_set_msg_rb_list_reply(u8 *msgrep, struct graal_msg_rb_list_request *msgreq){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	cx=nasrg_COMMON_search_cx(msgreq->lcr);
	if (cx!=NULL)
	{
		u8 rbi;
		struct rb_entity *rb;
		struct graal_msg_rb_list_reply *list;
		if (cx->num_rb > GRAAL_LIST_RB_MAX)
			msgrep[0] = GRAAL_LIST_RB_MAX;
		else
			msgrep[0] = cx->num_rb;
		list=(struct graal_msg_rb_list_reply *)(msgrep+1);
    // list all radio bearers
		for (rb=cx->rb, rbi=0; (rb!=NULL)&&(rbi<msgrep[0]); rb=rb->next, ++rbi){
			list[rbi].state=rb->state;
			list[rbi].rab_id=rb->rab_id;
			list[rbi].sapi=rb->sapi;
			list[rbi].qos=rb->qos;
			list[rbi].cnxid=rb->cnxid;
		}
    // check if mt0 requested and multicast bearer started, then add it to the list
    if ((msgreq->lcr==0)&&(gpriv->mbms_rb[0].mbms_rbId!=0)){
      printk("MT %d, MBMS bearer rb_id %d\n", msgreq->lcr, gpriv->mbms_rb[0].mbms_rbId);
      //++rbi;
			++msgrep[0];
			list[rbi].state=gpriv->mbms_rb[0].state;
			list[rbi].rab_id=gpriv->mbms_rb[0].mbms_rbId;
			list[rbi].sapi=gpriv->mbms_rb[0].sapi;
			list[rbi].qos=gpriv->mbms_rb[0].qos;
			list[rbi].cnxid=gpriv->mbms_rb[0].cnxid;
      printk("rab_id  cnxid  Sapi  QoS  State\n");
      printk("%u  %u  %u  %u  ", list[rbi].rab_id,list[rbi].cnxid, list[rbi].sapi, list[rbi].qos);
      nasrg_TOOL_print_state(list[rbi].state);
      printk("\n");
    }
	}
	else
		msgrep[0]=0;
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_rb_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_LIST_RB_MAX*sizeof(struct graal_msg_rb_list_reply)+1];
	struct graal_msg_rb_list_request msgreq;
	printk("nasrg_ioCTL_rb_list: Radio Bearer list requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_rb_list: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_rb_list_reply(msgrep, &msgreq);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_LIST_RB_MAX*sizeof(struct graal_msg_rb_list_reply)+1))
	{
		printk("nasrg_ioCTL_rb_list: copy_to_user failure\n");
		return -EFAULT;
	}
	printk("nasrg_ioCTL_rb_list: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Establishment
//---------------------------------------------------------------------------
void nasrg_set_msg_rb_establishment_reply(struct graal_msg_rb_establishment_reply *msgrep, struct graal_msg_rb_establishment_request *msgreq){
//---------------------------------------------------------------------------
  if ((msgreq->rab_id<5)||(msgreq->rab_id>(GRAAL_RB_MAX_NUM-1)))
    msgrep->status=-GRAAL_ERROR_NOTCORRECTRABI;
  else{
    struct cx_entity *cx;
    cx=nasrg_COMMON_search_cx(msgreq->lcr);
    if (cx!=NULL){ //not multicast
      struct rb_entity *rb;
      rb=nasrg_COMMON_add_rb(cx, msgreq->rab_id, msgreq->qos);
      if (rb!=NULL){
        rb->cnxid = msgreq->cnxid;
        rb->dscp = msgreq->dscp_dl;
        rb->dscp_ul = msgreq->dscp_ul;
        msgrep->status=nasrg_ASCTL_DC_send_rb_establish_request(cx, rb);
      }else{
        msgrep->status=-GRAAL_ERROR_NOMEMORY;
      }
    }else{  //no MT found
      if (msgreq->mcast_flag){ //multicast
        int mbms_ix=0; // should allocate index based on Service_id /cnxid / MC IP address
        //
        gpriv->mbms_rb[mbms_ix].cnxid = msgreq->cnxid;
        gpriv->mbms_rb[mbms_ix].serviceId = msgreq->cnxid;
        gpriv->mbms_rb[mbms_ix].sessionId = NASRG_TEMP_MBMS_SESSION_ID; //Temp hard coded
        gpriv->mbms_rb[mbms_ix].mbms_rbId = msgreq->rab_id;
        gpriv->mbms_rb[mbms_ix].sapi = GRAAL_IN_INPUT_SAPI;
        #ifdef GRAAL_DEBUG_MBMS_PROT
        gpriv->mbms_rb[mbms_ix].sapi = GRAAL_CO_INPUT_SAPI; //Only one RT-FIFO is used
        #endif
        gpriv->mbms_rb[mbms_ix].qos = msgreq->qos;
        gpriv->mbms_rb[mbms_ix].dscp = msgreq->dscp_dl;
        gpriv->mbms_rb[mbms_ix].duration = NASRG_TEMP_MBMS_DURATION; //Temp hard coded
        memcpy ((char *)&(gpriv->mbms_rb[mbms_ix].mcast_address),(char *)&(msgreq->mcast_group), 16);
        msgrep->status=nasrg_ASCTL_GC_send_mbms_bearer_establish_req(mbms_ix);
      }else{
        msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
      }
      msgrep->cnxid  = msgreq->cnxid;
    }
  }
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_rb_establishment_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_rb_establishment_request msgreq;
	struct graal_msg_rb_establishment_reply msgrep;
	printk("nasrg_ioCTL_rb_establishment: Radio bearer establishment requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_rb_establishment: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_rb_establishment_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_ioCTL_rb_establishment: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Release
//---------------------------------------------------------------------------
void nasrg_set_msg_rb_release_reply(struct graal_msg_rb_release_reply *msgrep, struct graal_msg_rb_release_request *msgreq){
//---------------------------------------------------------------------------
  if (msgreq->lcr<GRAAL_CX_MAX){
    if (msgreq->rab_id!=NASRG_DEFAULTRAB_RBID){
       struct rb_entity *rb;
       struct cx_entity *cx;
       cx=nasrg_COMMON_search_cx(msgreq->lcr);
       rb=nasrg_COMMON_search_rb(cx, msgreq->rab_id);
       if ((rb!=NULL)&&(cx!=NULL)){
         u8 dscp;
         msgrep->status=nasrg_ASCTL_DC_send_rb_release_request(cx, rb);
         dscp=rb->dscp;
         nasrg_COMMON_del_rb(cx, msgreq->rab_id, dscp);
       }else
         msgrep->status=-GRAAL_ERROR_NOTCONNECTED;
       msgrep->cnxid  = msgreq->cnxid;
    }else{
       msgrep->status=-GRAAL_ERROR_NOTCORRECTRABI;
    }
  }else{
    if (msgreq->mcast_flag){ // multicast
      int mbms_ix=0;  // should search mbms_ix based on cnxid
      msgrep->status=nasrg_ASCTL_GC_send_mbms_bearer_release_req(mbms_ix);
      msgrep->cnxid  = msgreq->cnxid;
    }else{
      msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
    }
  }
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_rb_release_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_rb_release_request msgreq;
	struct graal_msg_rb_release_reply msgrep;
	printk("nasrg_ioCTL_rb_release: Radio bearer release requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_rb_release: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_rb_release_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_ioCTL_rb_release: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Classifier List
//---------------------------------------------------------------------------
void nasrg_set_msg_class_list_reply(u8 *msgrep, struct graal_msg_class_list_request *msgreq){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	struct classifier_entity *gc;
	struct graal_msg_class_list_reply *list;
	u8 cli;
	list=(struct graal_msg_class_list_reply *)(msgrep+1);
	switch(msgreq->dir)
	{
	case GRAAL_DIRECTION_SEND:
		cx=nasrg_COMMON_search_cx(msgreq->lcr);
		if (cx==NULL)
		{
			msgrep[0]=0;
			return;
		}
		gc=cx->sclassifier[msgreq->dscp];
		break;
	case GRAAL_DIRECTION_RECEIVE:
		cx=NULL;
		gc=gpriv->rclassifier[msgreq->dscp];
		break;
	default:
		cx=NULL;
		msgrep[0]=0;
		return;
	}
	for (cli=0; (gc!=NULL)&&(cli<GRAAL_LIST_CLASS_MAX); gc=gc->next, ++cli)
	{
		list[cli].classref=gc->classref;
		list[cli].lcr=msgreq->lcr;
		list[cli].dir=msgreq->dir;
		list[cli].dscp=msgreq->dscp;
		list[cli].rab_id=gc->rab_id;
		list[cli].version=gc->version;
		switch(gc->version){
  		case 4:
  			list[cli].saddr.ipv4 = gc->saddr.ipv4;
  			list[cli].daddr.ipv4 = gc->daddr.ipv4;
  			break;
  		case 6:
  			list[cli].saddr.ipv6 = gc->saddr.ipv6;
  			list[cli].daddr.ipv6 = gc->daddr.ipv6;
  			break;
		}
		list[cli].protocol=gc->protocol;
		list[cli].sport=ntohs(gc->sport);
		list[cli].dport=ntohs(gc->dport);
		list[cli].splen=gc->splen;
		list[cli].dplen=gc->dplen;
		list[cli].fct=nasrg_TOOL_invfct(gc);
	}
	msgrep[0]=cli;
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_class_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_LIST_CLASS_MAX*sizeof(struct graal_msg_class_list_reply)+1];
	struct graal_msg_class_list_request msgreq;
	printk("nasrg_ioCTL_class_list: classifier list requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_class_list: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_class_list_reply(msgrep, &msgreq);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_LIST_CLASS_MAX*sizeof(struct graal_msg_class_list_reply)+1))
	{
		printk("nasrg_ioCTL_class_list: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the addition of a classifier rule
//---------------------------------------------------------------------------
void nasrg_set_msg_class_add_reply(struct graal_msg_class_add_reply *msgrep, struct graal_msg_class_add_request *msgreq){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;
	if (msgreq->dscp>GRAAL_DSCP_DEFAULT){
		printk("nasrg_set_msg_class_add: Incoherent parameter value\n");
		msgrep->status=-GRAAL_ERROR_NOTCORRECTDSCP;
		return;
	}
	if (msgreq->dir==GRAAL_DIRECTION_SEND){
		struct cx_entity *cx;
		cx=nasrg_COMMON_search_cx(msgreq->lcr);
		if (cx!=NULL){
	  	printk("nasrg_set_msg_class_add: DSCP %d, Classref %d\n",msgreq->dscp, msgreq->classref );
			gc=nasrg_CLASS_add_sclassifier(cx, msgreq->dscp, msgreq->classref);
      printk("nasrg_set_msg_class_add: %p %p\n" , msgreq, gc);
			if (gc==NULL){
				msgrep->status=-GRAAL_ERROR_NOMEMORY;
				return;
			}
		}else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
			return;
		}
		gc->rab_id=msgreq->rab_id;
		gc->rb=nasrg_COMMON_search_rb(cx, gc->rab_id);
	}else{
		if (msgreq->dir==GRAAL_DIRECTION_RECEIVE){
			gc=nasrg_CLASS_add_rclassifier(msgreq->dscp, msgreq->classref);
			if (gc==NULL){
				msgrep->status=-GRAAL_ERROR_NOMEMORY;
				return;
			}
		}else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTDIR;
			return;
		}
	}
	nasrg_TOOL_fct(gc, msgreq->fct);
	gc->version=msgreq->version;
	switch(gc->version){
  	case 4:
  		gc->saddr.ipv4=msgreq->saddr.ipv4;
  		gc->daddr.ipv4=msgreq->daddr.ipv4;
  		gc->splen=msgreq->splen;
  		gc->dplen=msgreq->dplen;
  		break;
  	case 6:
  		gc->saddr.ipv6=msgreq->saddr.ipv6;
  		gc->daddr.ipv6=msgreq->daddr.ipv6;
  		gc->splen=msgreq->splen;
  		gc->dplen=msgreq->dplen;
  		break;
  	case 0:
  		gc->saddr.ipv6.s6_addr32[0]=0;
  		gc->daddr.ipv6.s6_addr32[1]=0;
  		gc->saddr.ipv6.s6_addr32[2]=0;
  		gc->daddr.ipv6.s6_addr32[3]=0;
  		gc->splen=0;
  		gc->dplen=0;
  		break;
  	default:
  		msgrep->status=-GRAAL_ERROR_NOTCORRECTVERSION;
  		kfree(gc);
  		return;
	}
	gc->protocol=msgreq->protocol;
	gc->sport=htons(msgreq->sport);
	gc->dport=htons(msgreq->dport);
	msgrep->status=0;
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_class_add_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_class_add_request msgreq;
	struct graal_msg_class_add_reply msgrep;
	printk("nasrg_ioCTL_class_add: Add classifier components requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))){
		printk("nasrg_ioCTL_class_add: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_class_add_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep))){
		printk("nasrg_ioCTL_class_add: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the deletion of a classifier rule
//---------------------------------------------------------------------------
void nasrg_set_msg_class_del_reply(struct graal_msg_class_del_reply *msgrep, struct graal_msg_class_del_request *msgreq){
//---------------------------------------------------------------------------
	if (msgreq->dscp>GRAAL_DSCP_DEFAULT)
	{
		printk("nasrg_set_msg_class_del: Incoherent parameter value\n");
		msgrep->status=-GRAAL_ERROR_NOTCORRECTDSCP;
		return;
	}
	if (msgreq->dir==GRAAL_DIRECTION_SEND)
	{
		struct cx_entity *cx;
		cx=nasrg_COMMON_search_cx(msgreq->lcr);
		if (cx!=NULL)
			nasrg_CLASS_del_sclassifier(cx, msgreq->dscp, msgreq->classref);
		else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
			return;
		}
	}else{
		if (msgreq->dir==GRAAL_DIRECTION_RECEIVE)
			nasrg_CLASS_del_rclassifier(msgreq->dscp, msgreq->classref);
		else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTDIR;
			return;
		}
	}
	msgrep->status=0;
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_class_del_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_class_del_request msgreq;
	struct graal_msg_class_del_reply msgrep;
	printk("nasrg_ioCTL_class_del: Del classifier components requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasrg_ioCTL_class_del: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_msg_class_del_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasrg_ioCTL_class_del: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// UE Multicast Join
//---------------------------------------------------------------------------
void nasrg_set_ue_multicast_join_reply(struct graal_msg_mt_mcast_reply *msgrep, struct graal_msg_mt_mcast_join *msgreq){
//---------------------------------------------------------------------------
	if (msgreq->ue_id<GRAAL_CX_MAX){
		struct cx_entity *cx;
		cx=nasrg_COMMON_search_cx(msgreq->ue_id);
    cx->requested_joined_services[0] = msgreq->cnxid;
    msgrep->ue_id  = msgreq->ue_id;
		msgrep->result = nasrg_ASCTL_DC_send_mbms_ue_notify_req(cx);
    msgrep->cnxid  = msgreq->cnxid;
  }else{
	  msgrep->result=-GRAAL_ERROR_NOTCORRECTLCR;
  }
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_ue_multicast_join_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_mt_mcast_join msgreq;
	struct graal_msg_mt_mcast_reply msgrep;
	printk("nasrg_ioCTL_ue_multicast_join: UE multicast join requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))){
		printk("nasrg_ioCTL_ue_multicast_join: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_ue_multicast_join_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep))){
		printk("nasrg_ioCTL_ue_multicast_join: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// UE Multicast Leave
//---------------------------------------------------------------------------
void nasrg_set_ue_multicast_leave_reply(struct graal_msg_mt_mcast_reply *msgrep, struct graal_msg_mt_mcast_leave *msgreq){
//---------------------------------------------------------------------------
	if (msgreq->ue_id<GRAAL_CX_MAX){
		struct cx_entity *cx;
		cx=nasrg_COMMON_search_cx(msgreq->ue_id);
    cx->requested_left_services[0] = msgreq->cnxid;
    msgrep->ue_id  = msgreq->ue_id;
		msgrep->result = nasrg_ASCTL_DC_send_mbms_ue_notify_req(cx);
    msgrep->cnxid  = msgreq->cnxid;
  }else{
	  msgrep->result=-GRAAL_ERROR_NOTCORRECTLCR;
  }
}

//---------------------------------------------------------------------------
int nasrg_ioCTL_ue_multicast_leave_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_mt_mcast_leave msgreq;
	struct graal_msg_mt_mcast_reply msgrep;
	printk("nasrg_ioCTL_ue_multicast_leave: UE multicast leave requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))){
		printk("nasrg_ioCTL_ue_multicast_leave: copy_from_user failure\n");
		return -EFAULT;
	}
	nasrg_set_ue_multicast_leave_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep))){
		printk("nasrg_ioCTL_ue_multicast_leave: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IOCTL command
//---------------------------------------------------------------------------
int nasrg_CTL_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd){
//---------------------------------------------------------------------------
	struct graal_ioctl *gifr;
	int r;
	printk("nasrg_CTL_ioctl: begin\n");
	spin_lock(&gpriv->lock);
	switch(cmd)
	{
	case NASRG_IOCTL_RAL:
		gifr=(struct graal_ioctl *)ifr;
		switch(gifr->type)
		{
		case GRAAL_MSG_STATISTIC_REQUEST:
			r=nasrg_ioCTL_statistic_request(gifr);
			break;
		case GRAAL_MSG_CX_ESTABLISHMENT_REQUEST:
			r=nasrg_ioCTL_cx_establishment_request(gifr);
			break;
		case GRAAL_MSG_CX_RELEASE_REQUEST:
			r=nasrg_ioCTL_cx_release_request(gifr);
			break;
		case GRAAL_MSG_CX_LIST_REQUEST:
			r=nasrg_ioCTL_cx_list_request(gifr);
			break;
		case GRAAL_MSG_RB_ESTABLISHMENT_REQUEST:
			r=nasrg_ioCTL_rb_establishment_request(gifr);
			break;
		case GRAAL_MSG_RB_RELEASE_REQUEST:
			r=nasrg_ioCTL_rb_release_request(gifr);
			break;
		case GRAAL_MSG_RB_LIST_REQUEST:
			r=nasrg_ioCTL_rb_list_request(gifr);
			break;
		case GRAAL_MSG_CLASS_ADD_REQUEST:
			r=nasrg_ioCTL_class_add_request(gifr);
			break;
		case GRAAL_MSG_CLASS_LIST_REQUEST:
			r=nasrg_ioCTL_class_list_request(gifr);
			break;
		case GRAAL_MSG_CLASS_DEL_REQUEST:
			r=nasrg_ioCTL_class_del_request(gifr);
			break;
		case NAS_RG_MSG_MT_MCAST_JOIN:
			r=nasrg_ioCTL_ue_multicast_join_request(gifr);
			break;
		case NAS_RG_MSG_MT_MCAST_LEAVE:
			r=nasrg_ioCTL_ue_multicast_leave_request(gifr);
			break;
		default:
			printk("nasrg_CTL_ioctl: unkwon request type, type=%x\n", gifr->type);
			r=-EFAULT;
		}
		break;
	default:
		printk("nasrg_CTL_ioctl: Unknown ioctl command, cmd=%x\n", cmd);
		r=-EFAULT;
	}
	spin_unlock(&gpriv->lock);
	printk("nasrg_CTL_ioctl: end\n");
	return r;
}

//---------------------------------------------------------------------------
void nasrg_CTL_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc){
//---------------------------------------------------------------------------
	printk("nasrg_CTL_send - void \n");
}

