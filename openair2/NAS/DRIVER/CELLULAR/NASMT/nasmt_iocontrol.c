/***************************************************************************
                          nasmt_iocontrol.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#include "nasmt_variables.h"
#include "nasmt_iocontrol.h"
#include "nasmt_proto.h"

//#include <linux/in.h>
#include <asm/uaccess.h>
#include <asm/checksum.h>
#include <asm/uaccess.h>


// Statistic
//---------------------------------------------------------------------------
void nasmt_set_msg_statistic_reply(struct graal_msg_statistic_reply *msgrep){
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
int nasmt_ioCTL_statistic_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_statistic_reply msgrep;
	printk("nasmt_ioCTL_statistic: stat requested\n");
	nasmt_set_msg_statistic_reply(&msgrep);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_statistic: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connections List
//---------------------------------------------------------------------------
void nasmt_set_msg_cx_list_reply(u8 *msgrep){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	nasLocalConnectionRef_t lcr;
	struct graal_msg_cx_list_reply *list;
	msgrep[0]=GRAAL_CX_MAX;
	list=(struct graal_msg_cx_list_reply *)(msgrep+1);
	for(lcr=0;lcr<GRAAL_CX_MAX;++lcr)
	{
		cx=nasmt_COMMON_search_cx(lcr);
		list[lcr].lcr=lcr;
		list[lcr].state=cx->state;
		list[lcr].cellid=cx->cellid;
		list[lcr].iid4=cx->iid4;
		list[lcr].iid6[0]=cx->iid6[0];
		list[lcr].iid6[1]=cx->iid6[1];
		list[lcr].num_rb=cx->num_rb;
		list[lcr].nsclassifier=cx->nsclassifier;
    printk("nasmt_set_msg_cx_list_reply: nsc=%u\n",cx->nsclassifier);
	}
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_cx_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_CX_MAX*sizeof(struct graal_msg_cx_list_reply)+1];
	printk("nasmt_ioCTL_cx_list: connection list requested\n");
	nasmt_set_msg_cx_list_reply(msgrep);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_CX_MAX*sizeof(struct graal_msg_cx_list_reply)+1))
	{
		printk("nasmt_ioCTL_cx_list: copy_to_user failure\n");
		return -EFAULT;
	}
	printk("nasmt_ioCTL_cx_list: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connection Establishment
//---------------------------------------------------------------------------
void nasmt_set_msg_cx_establishment_reply(struct graal_msg_cx_establishment_reply *msgrep, struct graal_msg_cx_establishment_request *msgreq){
//---------------------------------------------------------------------------
#ifdef NODE_RG
	msgrep->status=-GRAAL_ERROR_NOTMT;
#else
	struct cx_entity *cx;
	cx=nasmt_COMMON_search_cx(msgreq->lcr);
	if (cx!=NULL)
	{
		cx->cellid=msgreq->cellid;
		msgrep->status=nasmt_ASCTL_DC_send_cx_establish_request(cx);
	}
	else
		msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
#endif
}
//---------------------------------------------------------------------------
int nasmt_ioCTL_cx_establishment_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_cx_establishment_request msgreq;
	struct graal_msg_cx_establishment_reply msgrep;
	printk("nasmt_ioCTL_cx_establishment: connection establishment requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_cx_establishment: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_cx_establishment_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_cx_establishment: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connection Release
//---------------------------------------------------------------------------
void nasmt_set_msg_cx_release_reply(struct graal_msg_cx_release_reply *msgrep, struct graal_msg_cx_release_request *msgreq){
//---------------------------------------------------------------------------
#ifdef NODE_RG
	msgrep->status=-GRAAL_ERROR_NOTMT;
#else
	struct cx_entity *cx;
	cx=nasmt_COMMON_search_cx(msgreq->lcr);
	if (cx!=NULL)
		msgrep->status=nasmt_ASCTL_DC_send_cx_release_request(cx);
	else
		msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
#endif
}

//---------------------------------------------------------------------------
// Request the release of a connection
int nasmt_ioCTL_cx_release_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_cx_release_request msgreq;
	struct graal_msg_cx_release_reply msgrep;

	printk("nasmt_ioCTL_cx_release: connection release requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_cx_release: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_cx_release_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_cx_release: copy_to_user failure\n");
		return -EFAULT;
	}
  printk("nasmt_ioCTL_cx_release: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer List
//---------------------------------------------------------------------------
void nasmt_set_msg_rb_list_reply(u8 *msgrep, struct graal_msg_rb_list_request *msgreq){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	cx=nasmt_COMMON_search_cx(msgreq->lcr);
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
		for (rb=cx->rb, rbi=0; (rb!=NULL)&&(rbi<msgrep[0]); rb=rb->next, ++rbi)
		{
			list[rbi].state=rb->state;
			list[rbi].rab_id=rb->rab_id;
			list[rbi].sapi=rb->sapi;
			list[rbi].qos=rb->qos;
		}
	}
	else
		msgrep[0]=0;
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_rb_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_LIST_RB_MAX*sizeof(struct graal_msg_rb_list_reply)+1];
	struct graal_msg_rb_list_request msgreq;
	printk("nasmt_ioCTL_rb_list: Radio Bearer list requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_rb_list: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_rb_list_reply(msgrep, &msgreq);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_LIST_RB_MAX*sizeof(struct graal_msg_rb_list_reply)+1))
	{
		printk("nasmt_ioCTL_rb_list: copy_to_user failure\n");
		return -EFAULT;
	}
	printk("nasmt_ioCTL_rb_list: end\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Establishment
//---------------------------------------------------------------------------
void nasmt_set_msg_rb_establishment_reply(struct graal_msg_rb_establishment_reply *msgrep, struct graal_msg_rb_establishment_request *msgreq){
//---------------------------------------------------------------------------
	msgrep->status=-GRAAL_ERROR_NOTRG;
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_rb_establishment_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_rb_establishment_request msgreq;
	struct graal_msg_rb_establishment_reply msgrep;
	printk("nasmt_ioCTL_rb_establishment: Radio bearer establishment requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_rb_establishment: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_rb_establishment_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_rb_establishment: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Release
//---------------------------------------------------------------------------
void nasmt_set_msg_rb_release_reply(struct graal_msg_rb_release_reply *msgrep, struct graal_msg_rb_release_request *msgreq){
//---------------------------------------------------------------------------
	msgrep->status=-GRAAL_ERROR_NOTRG;
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_rb_release_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_rb_release_request msgreq;
	struct graal_msg_rb_release_reply msgrep;
	printk("nasmt_ioCTL_rb_release: Radio bearer release requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_rb_release: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_rb_release_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_rb_release: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Classifier List
//---------------------------------------------------------------------------
void nasmt_set_msg_class_list_reply(u8 *msgrep, struct graal_msg_class_list_request *msgreq){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
	struct classifier_entity *gc;
	struct graal_msg_class_list_reply *list;
	u8 cli;
	list=(struct graal_msg_class_list_reply *)(msgrep+1);
	switch(msgreq->dir)
	{
	case GRAAL_DIRECTION_SEND:
		cx=nasmt_COMMON_search_cx(msgreq->lcr);
		if (cx==NULL){
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
		switch(gc->version)
		{
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
		list[cli].fct=nasmt_TOOL_invfct(gc);
	}
	msgrep[0]=cli;
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_class_list_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	u8 msgrep[GRAAL_LIST_CLASS_MAX*sizeof(struct graal_msg_class_list_reply)+1];
	struct graal_msg_class_list_request msgreq;
	printk("nasmt_ioCTL_class_list: classifier list requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_class_list: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_class_list_reply(msgrep, &msgreq);
	if (copy_to_user(gifr->msg, msgrep, GRAAL_LIST_CLASS_MAX*sizeof(struct graal_msg_class_list_reply)+1))
	{
		printk("nasmt_ioCTL_class_list: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the addition of a classifier rule
//---------------------------------------------------------------------------
void nasmt_set_msg_class_add_reply(struct graal_msg_class_add_reply *msgrep, struct graal_msg_class_add_request *msgreq){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;
	if (msgreq->dscp>GRAAL_DSCP_DEFAULT){
		printk("nasmt_set_msg_class_add_reply: Incoherent parameter value\n");
		msgrep->status=-GRAAL_ERROR_NOTCORRECTDSCP;
		return;
	}
	if (msgreq->dir==GRAAL_DIRECTION_SEND){
		struct cx_entity *cx;
		cx=nasmt_COMMON_search_cx(msgreq->lcr);
		if (cx!=NULL){
	  	printk("nasmt_set_msg_class_add_reply: DSCP %d, Classref %d\n",msgreq->dscp, msgreq->classref );
			gc=nasmt_CLASS_add_sclassifier(cx, msgreq->dscp, msgreq->classref);
      printk("nasmt_set_msg_class_add_reply: %p %p\n" , msgreq, gc);
			if (gc==NULL){
				msgrep->status=-GRAAL_ERROR_NOMEMORY;
				return;
			}
		}else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
			return;
		}
		gc->rab_id=msgreq->rab_id;
		gc->rb=nasmt_COMMON_search_rb(cx, gc->rab_id);
	}else{
		if (msgreq->dir==GRAAL_DIRECTION_RECEIVE){
			gc=nasmt_CLASS_add_rclassifier(msgreq->dscp, msgreq->classref);
			if (gc==NULL){
				msgrep->status=-GRAAL_ERROR_NOMEMORY;
				return;
			}
		}else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTDIR;
			return;
		}
	}
	nasmt_TOOL_fct(gc, msgreq->fct);
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
int nasmt_ioCTL_class_add_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_class_add_request msgreq;
	struct graal_msg_class_add_reply msgrep;
	printk("nasmt_ioCTL_class_add: Add classifier components requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))){
		printk("nasmt_ioCTL_class_add: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_class_add_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep))){
		printk("nasmt_ioCTL_class_add: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the deletion of a classifier rule
//---------------------------------------------------------------------------
void nasmt_set_msg_class_del_reply(struct graal_msg_class_del_reply *msgrep, struct graal_msg_class_del_request *msgreq){
//---------------------------------------------------------------------------
	if (msgreq->dscp>GRAAL_DSCP_DEFAULT)
	{
		printk("nasmt_set_msg_class_del_reply: Incoherent parameter value\n");
		msgrep->status=-GRAAL_ERROR_NOTCORRECTDSCP;
		return;
	}
	if (msgreq->dir==GRAAL_DIRECTION_SEND){
		struct cx_entity *cx;
		cx=nasmt_COMMON_search_cx(msgreq->lcr);
		if (cx!=NULL)
			nasmt_CLASS_del_sclassifier(cx, msgreq->dscp, msgreq->classref);
		else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
			return;
		}
	}else{
		if (msgreq->dir==GRAAL_DIRECTION_RECEIVE)
			nasmt_CLASS_del_rclassifier(msgreq->dscp, msgreq->classref);
		else{
			msgrep->status=-GRAAL_ERROR_NOTCORRECTDIR;
			return;
		}
	}
	msgrep->status=0;
}

//---------------------------------------------------------------------------
int nasmt_ioCTL_class_del_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_class_del_request msgreq;
	struct graal_msg_class_del_reply msgrep;
	printk("nasmt_ioCTL_class_del: Del classifier components requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_class_del: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_class_del_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_class_del: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Measurement
// Messages for Measurement transfer

//---------------------------------------------------------------------------
void nasmt_set_msg_measure_reply(struct graal_msg_measure_reply *msgrep, struct graal_msg_measure_request *msgreq){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
  int lcr=0; // Temp lcr->mt =0
  int i;

	cx = nasmt_COMMON_search_cx(lcr);
	if (cx!=NULL){
		msgrep->num_cells = cx->num_measures;
    for (i=0; i<cx->num_measures; i++){
      msgrep-> measures[i].cell_id = cx->meas_cell_id[i];
      msgrep-> measures[i].level = cx->meas_level[i];
      msgrep-> measures[i].provider_id = cx->provider_id[i];
    }
    msgrep->signal_lost_flag = 0;
	}else{
//		msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
//		return;
	}
}
//---------------------------------------------------------------------------
int nasmt_ioCTL_measure_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_measure_request msgreq;
	struct graal_msg_measure_reply msgrep;
	printk("nasmt_ioCTL_measure: Measurement requested\n");
	if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
	{
		printk("nasmt_ioCTL_measure: copy_from_user failure\n");
		return -EFAULT;
	}
	nasmt_set_msg_measure_reply(&msgrep, &msgreq);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_measure: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// IMEI
// Messages for IMEI transfer
//---------------------------------------------------------------------------
void nasmt_set_msg_imei_reply(struct graal_msg_l2id_reply *msgrep){
//---------------------------------------------------------------------------
	struct cx_entity *cx;
  int lcr=0; // Temp lcr->mt =0
  int i;

	cx=nasmt_COMMON_search_cx(lcr);
	if (cx!=NULL){
		msgrep->l2id[0] = cx->iid6[0];
		msgrep->l2id[1] = cx->iid6[1];
	}else{
//		msgrep->status=-GRAAL_ERROR_NOTCORRECTLCR;
//		return;
	}
}
//---------------------------------------------------------------------------
int nasmt_ioCTL_imei_request(struct graal_ioctl *gifr){
//---------------------------------------------------------------------------
	struct graal_msg_l2id_reply msgrep;
	printk("nasmt_ioCTL_imei: IMEI requested\n");
	nasmt_set_msg_imei_reply(&msgrep);
	if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
	{
		printk("nasmt_ioCTL_imei: copy_to_user failure\n");
		return -EFAULT;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IOCTL command
//---------------------------------------------------------------------------
int nasmt_CTL_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd){
//---------------------------------------------------------------------------
	struct graal_ioctl *gifr;
	int r;
	printk("nasmt_CTL_ioctl: begin\n");
	spin_lock(&gpriv->lock);
	switch(cmd)
	{
	case NASMT_IOCTL_RAL:
		gifr=(struct graal_ioctl *)ifr;
		switch(gifr->type)
		{
		case GRAAL_MSG_STATISTIC_REQUEST:
			r=nasmt_ioCTL_statistic_request(gifr);
			break;
		case GRAAL_MSG_CX_ESTABLISHMENT_REQUEST:
			r=nasmt_ioCTL_cx_establishment_request(gifr);
			break;
		case GRAAL_MSG_CX_RELEASE_REQUEST:
			r=nasmt_ioCTL_cx_release_request(gifr);
			break;
		case GRAAL_MSG_CX_LIST_REQUEST:
			r=nasmt_ioCTL_cx_list_request(gifr);
			break;
		case GRAAL_MSG_RB_ESTABLISHMENT_REQUEST:
			r=nasmt_ioCTL_rb_establishment_request(gifr);
			break;
		case GRAAL_MSG_RB_RELEASE_REQUEST:
			r= nasmt_ioCTL_rb_release_request(gifr);
			break;
		case GRAAL_MSG_RB_LIST_REQUEST:
			r=nasmt_ioCTL_rb_list_request(gifr);
			break;
		case GRAAL_MSG_CLASS_ADD_REQUEST:
			r=nasmt_ioCTL_class_add_request(gifr);
			break;
		case GRAAL_MSG_CLASS_LIST_REQUEST:
			r=nasmt_ioCTL_class_list_request(gifr);
			break;
		case GRAAL_MSG_CLASS_DEL_REQUEST:
			r=nasmt_ioCTL_class_del_request(gifr);
			break;
		case GRAAL_MSG_MEAS_REQUEST:
			r=nasmt_ioCTL_measure_request(gifr);
			break;
		case GRAAL_MSG_IMEI_REQUEST:
			r=nasmt_ioCTL_imei_request(gifr);
			break;
		default:
			printk("nasmt_CTL_ioctl: unkwon request type, type=%x\n", gifr->type);
			r=-EFAULT;
		}
		break;
	default:
		printk("nasmt_CTL_ioctl: Unknown ioctl command, cmd=%x\n", cmd);
		r=-EFAULT;
	}
	spin_unlock(&gpriv->lock);
	printk("nasmt_CTL_ioctl: end\n");
	return r;
}

//---------------------------------------------------------------------------
void nasmt_CTL_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc){
//---------------------------------------------------------------------------
	printk("nasmt_CTL_send - void \n");
}

