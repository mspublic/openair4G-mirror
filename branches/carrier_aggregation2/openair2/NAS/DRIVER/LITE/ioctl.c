/*******************************************************************************

  Eurecom OpenAirInterface 2
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

#include "local.h"
#include "ioctl.h"
#include "proto_extern.h"

#include <asm/uaccess.h>
#include <asm/checksum.h>
#include <asm/uaccess.h>

#define NIP6ADDR(addr) \
        ntohs((addr)->s6_addr16[0]), \
        ntohs((addr)->s6_addr16[1]), \
        ntohs((addr)->s6_addr16[2]), \
        ntohs((addr)->s6_addr16[3]), \
        ntohs((addr)->s6_addr16[4]), \
        ntohs((addr)->s6_addr16[5]), \
        ntohs((addr)->s6_addr16[6]), \
        ntohs((addr)->s6_addr16[7])

u8 g_msgrep[OAI_NW_DRV_LIST_CLASS_MAX*sizeof(struct oai_nw_drv_msg_class_list_reply)+1];

// Statistic
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_statistic_reply(struct oai_nw_drv_msg_statistic_reply *msgrep,
                 struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  msgrep->rx_packets=priv->stats.rx_packets;
  msgrep->tx_packets=priv->stats.tx_packets;
  msgrep->rx_bytes=priv->stats.rx_bytes;
  msgrep->tx_bytes=priv->stats.tx_bytes;
  msgrep->rx_errors=priv->stats.rx_errors;
  msgrep->tx_errors=priv->stats.tx_errors;
  msgrep->rx_dropped=priv->stats.rx_dropped;
  msgrep->tx_dropped=priv->stats.tx_dropped;
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_statistic_request(struct oai_nw_drv_ioctl *gifr,
                struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_statistic_reply msgrep;
  printk("NAS_IOCTL_STATISTIC: stat requested\n");
  oai_nw_drv_set_msg_statistic_reply(&msgrep,priv);
  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
    {
      printk("NAS_IOCTL_STATISTIC: copy_to_user failure\n");
      return -EFAULT;
    }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Connections List
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_cx_list_reply(u8 *msgrep,
                   struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct cx_entity *cx;
  OaiNwDrvLocalConnectionRef_t lcr;
  struct oai_nw_drv_msg_cx_list_reply *list;
  msgrep[0]=OAI_NW_DRV_CX_MAX;
  list=(struct oai_nw_drv_msg_cx_list_reply *)(msgrep+1);
  for(lcr=0;lcr<OAI_NW_DRV_CX_MAX;++lcr)
    {
      cx=oai_nw_drv_common_search_cx(lcr,priv);
      list[lcr].lcr=lcr;
      list[lcr].state=cx->state;
      list[lcr].cellid=cx->cellid;
      list[lcr].iid4=cx->iid4;
      list[lcr].iid6[0]=cx->iid6[0];
      list[lcr].iid6[1]=cx->iid6[1];
      list[lcr].num_rb=cx->num_rb;
      list[lcr].nsclassifier=cx->nsclassifier;
      printk("NAS_SET_MSG_CX_LIST_REPLY: nsc=%u\n",cx->nsclassifier);
    }
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_cx_list_request(struct oai_nw_drv_ioctl *gifr,
                  struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  u8 msgrep[OAI_NW_DRV_CX_MAX*sizeof(struct oai_nw_drv_msg_cx_list_reply)+1];
  printk("NAS_IOCTL_CX_LIST: connection list requested\n");
  oai_nw_drv_set_msg_cx_list_reply(msgrep,priv);
  if (copy_to_user(gifr->msg, msgrep, OAI_NW_DRV_CX_MAX*sizeof(struct oai_nw_drv_msg_cx_list_reply)+1))
    {
      printk("NAS_IOCTL_CX_LIST: copy_to_user failure\n");
      return -EFAULT;
    }
  printk("NAS_IOCTL_CX_LIST: end\n");
  return 0;
}




///////////////////////////////////////////////////////////////////////////////
// Radio Bearer List
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_rb_list_reply(u8 *msgrep,
                   struct oai_nw_drv_msg_rb_list_request *msgreq,
                   struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct cx_entity *cx;
  cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);
  if (cx!=NULL)
    {
      u8 rbi;
      struct rb_entity *rb;
      struct oai_nw_drv_msg_rb_list_reply *list;
      if (cx->num_rb > OAI_NW_DRV_LIST_RB_MAX)
    msgrep[0] = OAI_NW_DRV_LIST_RB_MAX;
      else
    msgrep[0] = cx->num_rb;
      list=(struct oai_nw_drv_msg_rb_list_reply *)(msgrep+1);
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
int oai_nw_drv_ioCTL_rb_list_request(struct oai_nw_drv_ioctl *gifr,
                  struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  u8 msgrep[OAI_NW_DRV_LIST_RB_MAX*sizeof(struct oai_nw_drv_msg_rb_list_reply)+1];
  struct oai_nw_drv_msg_rb_list_request msgreq;
  printk("NAS_IOCTL_RB_LIST: Radio Bearer list requested\n");
  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
    {
      printk("NAS_IOCTL_RB_LIST: copy_from_user failure\n");
      return -EFAULT;
    }
  oai_nw_drv_set_msg_rb_list_reply(msgrep, &msgreq,priv);
  if (copy_to_user(gifr->msg, msgrep, OAI_NW_DRV_LIST_RB_MAX*sizeof(struct oai_nw_drv_msg_rb_list_reply)+1))
    {
      printk("NAS_IOCTL_RB_LIST: copy_to_user failure\n");
      return -EFAULT;
    }
  printk("NAS_IOCTL_CX_LIST: end\n");
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Establishment
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_rb_establishment_reply(struct oai_nw_drv_msg_rb_establishment_reply *msgrep,
                    struct oai_nw_drv_msg_rb_establishment_request *msgreq,
                    struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  if ((msgreq->rab_id<3)||(msgreq->rab_id>OAI_NW_DRV_MAX_RABS)) { // navid : increase the number
    msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTRABI;
  } else {
      struct cx_entity *cx;
      cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);
      if (cx==NULL) {
          msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTLCR;
      } else  {
          struct rb_entity *rb;
          rb=oai_nw_drv_common_add_rb(priv, cx, msgreq->rab_id, msgreq->qos);
          if (rb!=NULL){
              //rb->cnxid = msgreq->cnxid;
              //msgrep->status=oai_nw_drv_rg_DC_send_rb_establish_request(cx, rb);
          } else {
              msgrep->status=-OAI_NW_DRV_ERROR_NOMEMORY;
              //msgrep->cnxid  = msgreq->cnxid;
          }
      }
  }
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_rb_establishment_request(struct oai_nw_drv_ioctl *gifr,
                       struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_rb_establishment_request msgreq;
  struct oai_nw_drv_msg_rb_establishment_reply msgrep;
  printk("NAS_IOCTL_RB_ESTABLISHMENT: Radio bearer establishment requested\n");
  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))) {
      printk("NAS_IOCTL_RB_ESTABLISHMENT: copy_from_user failure\n");
      return -EFAULT;
  }

  oai_nw_drv_set_msg_rb_establishment_reply(&msgrep, &msgreq,priv);

  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))  {
      printk("NAS_IOCTL_RB_ESTABLISHMENT: copy_to_user failure\n");
      return -EFAULT;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Radio Bearer Release
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_rb_release_reply(struct oai_nw_drv_msg_rb_release_reply *msgrep,
                  struct oai_nw_drv_msg_rb_release_request *msgreq,
                  struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  if (msgreq->lcr<OAI_NW_DRV_CX_MAX)
    {
      struct rb_entity *rb;
      struct cx_entity *cx;
      cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);
      rb=oai_nw_drv_common_search_rb(cx, msgreq->rab_id);
      if (rb!=NULL) {
    //msgrep->status=oai_nw_drv_rg_DC_send_rb_release_request(cx, rb);
      }
      else
    msgrep->status=-OAI_NW_DRV_ERROR_NOTCONNECTED;
      //      msgrep->cnxid  = msgreq->cnxid;
    }
  else
    msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTLCR;
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_rb_release_request(struct oai_nw_drv_ioctl *gifr,
                 struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_rb_release_request msgreq;
  struct oai_nw_drv_msg_rb_release_reply msgrep;
  printk("NAS_IOCTL_RB_RELEASE: Radio bearer release requested\n");
  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
    {
      printk("NAS_IOCTL_RB_RELEASE: copy_from_user failure\n");
      return -EFAULT;
    }
  oai_nw_drv_set_msg_rb_release_reply(&msgrep, &msgreq, priv);
  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
    {
      printk("NAS_IOCTL_RB_RELEASE: copy_to_user failure\n");
      return -EFAULT;
    }
  return 0;
}



///////////////////////////////////////////////////////////////////////////////
// Classifier List
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_class_list_reply(u8 *msgrep,
                  struct oai_nw_drv_msg_class_list_request *msgreq,
                  struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct cx_entity *cx;
  struct classifier_entity *gc;
  struct oai_nw_drv_msg_class_list_reply *list;
  u8 cli;
  list=(struct oai_nw_drv_msg_class_list_reply *)(msgrep+1);
  switch(msgreq->dir)
    {
    case OAI_NW_DRV_DIRECTION_SEND:
      cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);
      if (cx==NULL) {
          msgrep[0]=0;
          return;
      }
      gc=cx->sclassifier[msgreq->dscp];
      break;
    case OAI_NW_DRV_DIRECTION_RECEIVE:
      cx=NULL;
      gc=priv->rclassifier[msgreq->dscp];
      break;
    default:
      cx=NULL;
      msgrep[0]=0;
      return;
    }
  for (cli=0; (gc!=NULL)&&(cli<OAI_NW_DRV_LIST_CLASS_MAX); gc=gc->next, ++cli)
    {
      list[cli].classref=gc->classref;
      list[cli].lcr=msgreq->lcr;
      list[cli].dir=msgreq->dir;
      list[cli].dscp=msgreq->dscp;
      list[cli].rab_id=gc->rab_id;
      list[cli].version=gc->ip_version;
      switch(gc->ip_version)
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
      list[cli].fct=oai_nw_drv_TOOL_invfct(gc);
    }
  msgrep[0]=cli;
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_class_list_request(struct oai_nw_drv_ioctl *gifr,
                 struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_class_list_request msgreq;
  printk("NAS_IOCTL_CLASS_LIST: classifier list requested\n");
  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq)))
    {
      printk("NAS_IOCTL_CLASS_LIST: copy_from_user failure\n");
      return -EFAULT;
    }
  oai_nw_drv_set_msg_class_list_reply(g_msgrep, &msgreq,priv);
  if (copy_to_user(gifr->msg, g_msgrep, OAI_NW_DRV_LIST_CLASS_MAX*sizeof(struct oai_nw_drv_msg_class_list_reply)+1))
    {
      printk("NAS_IOCTL_CLASS_LIST: copy_to_user failure\n");
      return -EFAULT;
    }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the addition of a classifier rule
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_class_add_reply(struct oai_nw_drv_msg_class_add_reply *msgrep,
                struct oai_nw_drv_msg_class_add_request *msgreq,
                struct oai_nw_drv_priv *priv){
//---------------------------------------------------------------------------
    struct classifier_entity *gc,*gc2;
    unsigned char *saddr,*daddr;
    unsigned int *saddr32,*daddr32;

    printk("[OAI_IP_DRV][CLASS] oai_nw_drv_set_msg_class_add_reply\n");


    if (msgreq->dscp>OAI_NW_DRV_DSCP_MAX){
        printk("NAS_SET_MSG_CLASS_ADD_REPLY: Incoherent parameter value\n");
        msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTDSCP;
        return;
    }

    if (msgreq->dir==OAI_NW_DRV_DIRECTION_SEND){

        struct cx_entity *cx;
        cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);

        if (cx!=NULL){
            printk("NAS_SET_MSG_CLASS_ADD_REPLY: DSCP/EXP %d, Classref %d, RB %u\n", msgreq->dscp, msgreq->classref,msgreq->rab_id );
            gc=oai_nw_drv_class_add_send_classifier(cx, msgreq->dscp, msgreq->classref);

            printk("NAS_SET_MSG_CLASS_ADD_REPLY: %p %p\n" , msgreq, gc);

            if (gc==NULL){
                msgrep->status=-OAI_NW_DRV_ERROR_NOMEMORY;
                return;
            }
        }else{
            msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTLCR;
            return;
        }
        gc->rab_id=msgreq->rab_id;

        gc->rb=oai_nw_drv_common_search_rb(cx, gc->rab_id);
        printk("NAS_SET_MSG_CLASS_ADD_REPLY: gc_rb %p %u \n", gc->rb, gc->rab_id);
    }else{
        if (msgreq->dir==OAI_NW_DRV_DIRECTION_RECEIVE) {
            gc=oai_nw_drv_class_add_recv_classifier(msgreq->dscp,
                            msgreq->classref,
                            priv);
            if (gc==NULL){
                msgrep->status=-OAI_NW_DRV_ERROR_NOMEMORY;
                return;
            }
            gc->rab_id=msgreq->rab_id;

        } else {
            msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTDIR;
            return;
        }
        for (gc2 = priv->rclassifier[msgreq->dscp]; gc2!=NULL ; gc2 = gc2->next)
            printk("[OAI_IP_DRV][CLASS] Add Receive Classifier dscp %d: rab_id %d (%p,next %p)\n",msgreq->dscp,gc2->rab_id,gc2,gc2->next);
    }
    printk("[OAI_IP_DRV][CLASS] Getting addresses ...\n");

    oai_nw_drv_TOOL_fct(gc, msgreq->fct);
    gc->ip_version=msgreq->version;

    switch(gc->ip_version){

    case 4:
        gc->saddr.ipv4=msgreq->saddr.ipv4;
        gc->daddr.ipv4=msgreq->daddr.ipv4;

        // #ifdef NAS_CLASS_DEBUG
        saddr = (unsigned char *)&gc->saddr.ipv4;
        daddr = (unsigned char *)&gc->daddr.ipv4;

        printk("[OAI_IP_DRV][CLASS] Adding IPv4 %d.%d.%d.%d/%d -> %d.%d.%d.%d/%d\n",
            saddr[0],saddr[1],saddr[2],saddr[3],msgreq->splen,
            daddr[0],daddr[1],daddr[2],daddr[3],msgreq->dplen);
        //#endif
        gc->splen=msgreq->splen;
        gc->dplen=msgreq->dplen;
        break;

    case 6:
        memcpy(&gc->saddr.ipv6,&msgreq->saddr.ipv6,16);
        memcpy(&gc->daddr.ipv6,&msgreq->daddr.ipv6,16);

        saddr32 = (unsigned int *)&gc->saddr.ipv6;
        daddr32 = (unsigned int *)&gc->daddr.ipv6;

        printk("[OAI_IP_DRV][CLASS] Adding IPv6 %X:%X:%X:%X:%X:%X:%X:%X/%d -> %X:%X:%X:%X:%X:%X:%X:%X/%d\n",
        NIP6ADDR(&gc->saddr.ipv6), msgreq->splen, NIP6ADDR(&gc->daddr.ipv6), msgreq->dplen);
        gc->splen=msgreq->splen;
        gc->dplen=msgreq->dplen;
        break;

    case OAI_NW_DRV_MPLS_VERSION_CODE:
        printk("[OAI_IP_DRV][CLASS] Adding MPLS label %d with exp %d\n",
        msgreq->daddr.mpls_label,msgreq->dscp);
        gc->daddr.mpls_label = msgreq->daddr.mpls_label;

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
        msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTVERSION;
        kfree(gc);
        return;
    }
    gc->protocol=msgreq->protocol;
    gc->protocol_message_type=msgreq->protocol_message_type;
    gc->sport=htons(msgreq->sport);
    gc->dport=htons(msgreq->dport);
    msgrep->status=0;
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_class_add_request(struct oai_nw_drv_ioctl *gifr,
                struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_class_add_request msgreq;
  struct oai_nw_drv_msg_class_add_reply msgrep;


  printk("NAS_IOCTL_CLASS_ADD: Add classifier components requested\n");
  printk("NAS_IOCTL_CLASS_ADD: size of gifr msg %d\n", sizeof(gifr->msg));


  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))){
    printk("NAS_IOCTL_CLASS_ADD: copy_from_user failure\n");
    return -EFAULT;
  }

  oai_nw_drv_set_msg_class_add_reply(&msgrep, &msgreq,priv);
  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep))){
    printk("NAS_IOCTL_CLASS_ADD: copy_to_user failure\n");
    return -EFAULT;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Request the deletion of a classifier rule
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_class_del_reply(struct oai_nw_drv_msg_class_del_reply *msgrep,
                 struct oai_nw_drv_msg_class_del_request *msgreq,
                 struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  if (msgreq->dscp>OAI_NW_DRV_DSCP_DEFAULT) {
      printk("NAS_SET_MSG_CLASS_DEL_REPLY: Incoherent parameter value\n");
      msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTDSCP;
      return;
  }

  if (msgreq->dir==OAI_NW_DRV_DIRECTION_SEND) {
      struct cx_entity *cx;
      cx=oai_nw_drv_common_search_cx(msgreq->lcr,priv);
      if (cx!=NULL) {
          oai_nw_drv_class_del_send_classifier(cx, msgreq->dscp, msgreq->classref);
      } else {
          msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTLCR;
          return;
      }
  } else {
      if (msgreq->dir==OAI_NW_DRV_DIRECTION_RECEIVE) {
          oai_nw_drv_class_del_recv_classifier(msgreq->dscp, msgreq->classref,priv);
      } else {
          msgrep->status=-OAI_NW_DRV_ERROR_NOTCORRECTDIR;
          return;
      }
  }
  msgrep->status=0;
}

//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_class_del_request(struct oai_nw_drv_ioctl *gifr,
                struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_class_del_request msgreq;
  struct oai_nw_drv_msg_class_del_reply msgrep;
  printk("NAS_IOCTL_CLASS_DEL: Del classifier components requested\n");
  if (copy_from_user(&msgreq, gifr->msg, sizeof(msgreq))) {
      printk("NAS_IOCTL_CLASS_DEL: copy_from_user failure\n");
      return -EFAULT;
  }

  oai_nw_drv_set_msg_class_del_reply(&msgrep, &msgreq,priv);

  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))  {
      printk("NAS_IOCTL_CLASS_DEL: copy_to_user failure\n");
      return -EFAULT;
  }
  return 0;
}



///////////////////////////////////////////////////////////////////////////////
// IMEI
// Messages for IMEI transfer
//---------------------------------------------------------------------------
void oai_nw_drv_set_msg_imei_reply(struct oai_nw_drv_msg_l2id_reply *msgrep,
                struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct cx_entity *cx;
  int lcr=0; // Temp lcr->mt =0

  cx=oai_nw_drv_common_search_cx(lcr,priv);
  if (cx!=NULL)
    {
      msgrep->l2id[0] = cx->iid6[0];
      msgrep->l2id[1] = cx->iid6[1];
    }
}
//---------------------------------------------------------------------------
int oai_nw_drv_ioCTL_imei_request(struct oai_nw_drv_ioctl *gifr,
               struct oai_nw_drv_priv *priv){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_msg_l2id_reply msgrep;
  printk("NAS_IOCTL_IMEI: IMEI requested\n");
  oai_nw_drv_set_msg_imei_reply(&msgrep,priv);
  if (copy_to_user(gifr->msg, &msgrep, sizeof(msgrep)))
    {
      printk("NAS_IOCTL_IMEI: copy_to_user failure\n");
      return -EFAULT;
    }
  return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IOCTL command
//---------------------------------------------------------------------------
int oai_nw_drv_CTL_ioctl(struct net_device *dev,
          struct ifreq *ifr,
          int cmd){
  //---------------------------------------------------------------------------
  struct oai_nw_drv_ioctl *gifr;
  struct oai_nw_drv_priv *priv=netdev_priv(dev);

  int r;

  //  printk("NAS_CTL_IOCTL: begin ioctl for instance %d\n",find_inst(dev));

  switch(cmd)
    {
    case OAI_NW_DRV_IOCTL_RRM:
      gifr=(struct oai_nw_drv_ioctl *)ifr;
      switch(gifr->type)
    {
    case OAI_NW_DRV_MSG_STATISTIC_REQUEST:
      r=oai_nw_drv_ioCTL_statistic_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_CX_LIST_REQUEST:
      r=oai_nw_drv_ioCTL_cx_list_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_RB_ESTABLISHMENT_REQUEST:
      r=oai_nw_drv_ioCTL_rb_establishment_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_RB_RELEASE_REQUEST:
      r= oai_nw_drv_ioCTL_rb_release_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_RB_LIST_REQUEST:
      r=oai_nw_drv_ioCTL_rb_list_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_CLASS_ADD_REQUEST:
      r=oai_nw_drv_ioCTL_class_add_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_CLASS_LIST_REQUEST:
      r=oai_nw_drv_ioCTL_class_list_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_CLASS_DEL_REQUEST:
      r=oai_nw_drv_ioCTL_class_del_request(gifr,priv);
      break;
    case OAI_NW_DRV_MSG_IMEI_REQUEST:
      r=oai_nw_drv_ioCTL_imei_request(gifr,priv);
      break;
    default:
      //  printk("NAS_IOCTL_RRM: unkwon request type, type=%x\n", gifr->type);
      r=-EFAULT;
    }
      break;
    default:
      //      printk("NAS_CTL_IOCTL: Unknown ioctl command, cmd=%x\n", cmd);
      r=-EFAULT;
    }
  //  printk("NAS_CTL_IOCTL: end\n");
  return r;
}

//---------------------------------------------------------------------------
void oai_nw_drv_CTL_send(struct sk_buff *skb,
          struct cx_entity *cx,
          struct classifier_entity *gc,int inst){
  //---------------------------------------------------------------------------
  printk("NAS_CTL_SEND - void \n");
}