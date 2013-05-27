/***************************************************************************
                          lteRALenb_ioctl.c  -  description
 ***************************************************************************
  Eurecom OpenAirInterface 3
  Copyright(c) 1999 - 2013 Eurecom

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
  Address      : Eurecom, 450 route des Chappes, 06410 Biot Sophia Antipolis, France
*******************************************************************************/
/*! \file lteRALenb_ioctl.c
 * \brief Handling of ioctl for LTE driver in LTE-RAL-ENB
 * \author WETTERWALD Michelle, GAUTHIER Lionel, MAUREL Frederic
 * \date 2013
 * \company EURECOM
 * \email: michelle.wetterwald@eurecom.fr, lionel.gauthier@eurecom.fr, frederic.maurel@eurecom.fr
 */
/*******************************************************************************/
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
//#include <linux/ipv6.h>
//#include <linux/in.h>
//#include <linux/in6.h>

#include <netinet/in.h>

//
#include "rrc_d_types.h"
//-----------------------------------------------------------------------------
// LTE AS sub-system
#include "rrc_nas_primitives.h"
#include "nasrg_constant.h"
#include "nasrg_iocontrol.h"
//-----------------------------------------------------------------------------
#include "lteRALenb_mih_msg.h"
#include "lteRALenb_constants.h"
#include "lteRALenb_variables.h"
#include "lteRALenb_proto.h"
#include "MIH_C.h"
//#include "MIH_C_Types.h"

extern struct nas_ioctl gifr;
extern int fd;
extern int s_mgr; //socket with QoS Mgr
extern int init_flag;

//---------------------------------------------------------------------------
void print_state(u8 state){
//---------------------------------------------------------------------------
 switch(state){
    case  NAS_IDLE:DEBUG("NAS_IDLE\n");return;
    case  NAS_CX_FACH:DEBUG("NAS_CX_FACH\n");return;
    case  NAS_CX_DCH:DEBUG("NAS_CX_DCH\n");return;
    case  NAS_CX_RECEIVED:DEBUG("NAS_CX_RECEIVED\n");return;
    case  NAS_CX_CONNECTING:DEBUG("NAS_CX_CONNECTING\n");return;
    case  NAS_CX_RELEASING:DEBUG("NAS_CX_RELEASING\n");return;
    case  NAS_CX_CONNECTING_FAILURE:DEBUG("NAS_CX_CONNECTING_FAILURE\n");return;
    case  NAS_CX_RELEASING_FAILURE:DEBUG("NAS_CX_RELEASING_FAILURE\n");return;
    case  NAS_RB_ESTABLISHING:DEBUG("NAS_RB_ESTABLISHING\n");return;
    case  NAS_RB_RELEASING:DEBUG("NAS_RB_RELEASING\n");return;
    case  NAS_RB_ESTABLISHED:DEBUG("NAS_RB_ESTABLISHED\n");return;

   default: ERR(" Unknown state\n");
 }
}

//---------------------------------------------------------------------------
void RAL_NASinitMTlist(u8 *msgrep, int num_mts){
//---------------------------------------------------------------------------
  int mt_ix, ch_ix;
  struct nas_msg_cx_list_reply *list;

  memcpy(ralpriv->plmn, DefaultPLMN, DEFAULT_PLMN_SIZE); // DUMMY
  list=(struct nas_msg_cx_list_reply *)(msgrep+1);
  num_mts = msgrep[0];
  for(mt_ix=0; mt_ix<num_mts; ++mt_ix){
    if (list[mt_ix].state != NAS_IDLE){
      ralpriv->curr_cellId =  list[mt_ix].cellid;
      ralpriv->mt[mt_ix].ue_id = list[mt_ix].lcr;
      ralpriv->mt[mt_ix].ipv6_l2id[0]= list[mt_ix].iid6[0];
      ralpriv->mt[mt_ix].ipv6_l2id[1]= list[mt_ix].iid6[1];
      ralpriv->mt[mt_ix].num_rbs = list[mt_ix].num_rb;
      ralpriv->mt[mt_ix].num_class = list[mt_ix].nsclassifier;
      ralpriv->mt[mt_ix].nas_state = list[mt_ix].state;
      if (ralpriv->mt[mt_ix].num_class>=2)
        ralpriv->mt[mt_ix].mt_state= NAS_CONNECTED;
      // enter default rb
      ch_ix = 0;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].rbId = RAL_DEFAULT_MC_RAB_ID+1;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].RadioQoSclass = 2;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].cnx_id = (RAL_MAX_RB_PER_UE*mt_ix)+ch_ix+1;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].dscpUL = 0;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].dscpDL = 0;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].nas_state = NAS_CX_DCH;
      ralpriv->mt[mt_ix].radio_channel[ch_ix].status = RB_CONNECTED;
      ralpriv->num_connected_mts++;
      //RAL_printMobileData(mt_ix);
      DEBUG(" MT%d initialized : address %d %d\n", mt_ix, ralpriv->mt[mt_ix].ipv6_l2id[0], ralpriv->mt[mt_ix].ipv6_l2id[1]);
    }
  }
}

//---------------------------------------------------------------------------
void RAL_NASupdatetMTlist(u8 *msgrep, int num_mts){
//---------------------------------------------------------------------------
  int mt_ix, ch_ix;
  struct nas_msg_cx_list_reply *list;
  //MIH_C_LINK_TUPLE_ID_T* ltid;
  MIH_C_LINK_ADDR_T new_ar;
  int previous_num_class;

  list=(struct nas_msg_cx_list_reply *)(msgrep+1);
  num_mts = msgrep[0];
  for(mt_ix=0; mt_ix<num_mts; ++mt_ix){
  // check if MT already known
    if ((ralpriv->mt[mt_ix].ipv6_l2id[0]== list[mt_ix].iid6[0])&&
          (ralpriv->mt[mt_ix].ipv6_l2id[1]== list[mt_ix].iid6[1])){
      // MT already known - update
        ralpriv->mt[mt_ix].num_rbs = list[mt_ix].num_rb;
        previous_num_class = ralpriv->mt[mt_ix].num_class;
        ralpriv->mt[mt_ix].num_class = list[mt_ix].nsclassifier;
        ralpriv->mt[mt_ix].nas_state = list[mt_ix].state;
        if (ralpriv->mt[mt_ix].num_class>=2)
          ralpriv->mt[mt_ix].mt_state= RB_CONNECTED;
      //check if state has changed - MT disconnected FFS
      if ((ralpriv->mt[mt_ix].nas_state==NAS_CX_DCH)&&(list[mt_ix].state == NAS_IDLE)){
         DEBUG ("\n\n");
         DEBUG (" MOBILE TERMINAL %d IS NOW IDLE.\n\n",mt_ix);
         // TODO Send linkdown
      }
      //check if state has changed - MT reconnected FFS
      if ((ralpriv->mt[mt_ix].nas_state==NAS_IDLE)&&(list[mt_ix].state == NAS_CX_DCH)){
         DEBUG ("\n\n");
         DEBUG (" MOBILE TERMINAL %d WAS IDLE AND IS NOW CONNECTED.\n\n",mt_ix);
      }
      //check if MT is completey connected
      if ((ralpriv->mt[mt_ix].num_class - previous_num_class)&&(list[mt_ix].state == NAS_CX_DCH)){
         DEBUG ("\n\n");
         DEBUG (" MOBILE TERMINAL %d IS NOW COMPLETELY CONNECTED.\n\n",mt_ix);
         // send linkup
         eRALlte_send_link_up_indication(&ralpriv->pending_req_transaction_id, &ralpriv->mt[mt_ix].ltid, NULL, NULL, NULL, NULL);
      }
    }else{
      // MT unknown or different
      if (list[mt_ix].state != NAS_IDLE){
        DEBUG ("\n\n");
        DEBUG (" NEW TERMINAL %d DETECTED.\n\n",mt_ix);
        ralpriv->mt[mt_ix].ue_id = list[mt_ix].lcr;
        ralpriv->mt[mt_ix].ipv6_l2id[0]= list[mt_ix].iid6[0];
        ralpriv->mt[mt_ix].ipv6_l2id[1]= list[mt_ix].iid6[1];
        ralpriv->mt[mt_ix].num_rbs = list[mt_ix].num_rb;
        ralpriv->mt[mt_ix].num_class = list[mt_ix].nsclassifier;
        // initialize ltid (MIH_C_LINK_TUPLE_ID_T) for that mobile
        // first version
        //ralpriv->mt[mt_ix].ltid.link_id.link_type = MIH_C_WIRELESS_UMTS;
        //ralpriv->mt[mt_ix].ltid.link_id.link_addr.choice = MIH_C_CHOICE_3GPP_ADDR;
        //MIH_C_3GPP_ADDR_set(&ralpriv->mt[mt_ix].ltid.link_id.link_addr._union._3gpp_addr, (u_int8_t*)&(ralpriv->mt[mt_ix].ipv6_l2id[0]), strlen(DEFAULT_ADDRESS_3GPP));
        //ralpriv->mt[mt_ix].ltid.choice = MIH_C_LINK_TUPLE_ID_CHOICE_LINK_ADDR;
        // SECOND Version
        ralpriv->mt[mt_ix].ltid.link_id.link_type = MIH_C_WIRELESS_UMTS;
        ralpriv->mt[mt_ix].ltid.choice = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;
        ralpriv->mt[mt_ix].ltid.link_id.link_addr.choice = MIH_C_CHOICE_3GPP_3G_CELL_ID;
        Bit_Buffer_t *plmn = new_BitBuffer_0();
        BitBuffer_wrap(plmn, (unsigned char*) ralpriv->plmn, DEFAULT_PLMN_SIZE);
        MIH_C_PLMN_ID_decode(plmn, &ralpriv->mt[mt_ix].ltid.link_id.link_addr._union._3gpp_3g_cell_id.plmn_id);
        free_BitBuffer(plmn);
        ralpriv->mt[mt_ix].ltid.link_id.link_addr._union._3gpp_3g_cell_id.cell_id = ralpriv->curr_cellId;
        ralpriv->mt[mt_ix].ltid.choice = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;

        // check state of the UE connection
        ralpriv->mt[mt_ix].nas_state = list[mt_ix].state;
        if (ralpriv->mt[mt_ix].num_class>=2){
          ralpriv->mt[mt_ix].mt_state= RB_CONNECTED;
          // send linkup
          //ltid = &ralpriv->mt[mt_ix].ltid;
          DEBUG (" MOBILE TERMINAL %d IS COMPLETELY CONNECTED.\n\n",mt_ix);
          // new_ar will contain the address from the MT
          /*MIH_C_LINK_ADDR_T new_ar;
typedef struct MIH_C_LINK_ADDR {
    MIH_C_CHOICE_T               choice;
    union  {
        MIH_C_MAC_ADDR_T         mac_addr;
        MIH_C_3GPP_3G_CELL_ID_T  _3gpp_3g_cell_id;
        MIH_C_3GPP_2G_CELL_ID_T  _3gpp_2g_cell_id;
        MIH_C_3GPP_ADDR_T        _3gpp_addr;
        MIH_C_3GPP2_ADDR_T       _3gpp2_addr;
        MIH_C_OTHER_L2_ADDR_T    other_l2_addr;
    } _union;
} MIH_C_LINK_ADDR_T;

          */
          new_ar.choice = MIH_C_CHOICE_3GPP_ADDR;
          MIH_C_3GPP_ADDR_set(&(new_ar._union._3gpp_addr), (u_int8_t*)&(ralpriv->mt[mt_ix].ipv6_l2id[0]), strlen(DEFAULT_ADDRESS_3GPP));

          eRALlte_send_link_up_indication(&ralpriv->pending_req_transaction_id, &ralpriv->mt[mt_ix].ltid, NULL, &new_ar, NULL, NULL);
        }
        // enter default rb
        ch_ix = 0;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].rbId = RAL_DEFAULT_MC_RAB_ID+1;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].RadioQoSclass = 2;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].cnx_id = (RAL_MAX_RB_PER_UE*mt_ix)+ch_ix+1;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].dscpUL = 0;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].dscpDL = 0;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].nas_state = NAS_CX_DCH;
        ralpriv->mt[mt_ix].radio_channel[ch_ix].status = RB_CONNECTED;
        ralpriv->num_connected_mts++;
        //RAL_printMobileData(mt_ix);
      }
    } // end if MT unknonwn
  } // end for loop
}

//---------------------------------------------------------------------------
void RAL_verifyPendingRbStatus(void){
//---------------------------------------------------------------------------
// ralpriv->mcast.radio_channel.status = RB_CONNECTED;
    int mt_ix, ch_ix;
    MIH_C_LINK_TUPLE_ID_T* ltid;
    //int is_unicast;

    mt_ix =  ralpriv->pending_req_mt_ix;
    ch_ix =  ralpriv->pending_req_ch_ix;

    if ((ralpriv->pending_req_flag)%5==0){
        DEBUG("Pending Req Flag %d, Mobile %d, channel %d\n", ralpriv->pending_req_flag, mt_ix, ch_ix);
        if (ralpriv->pending_req_multicast == RAL_TRUE){
          mt_ix =0;
          ch_ix =1;
        }
        RAL_process_NAS_message(IO_OBJ_RB, IO_CMD_LIST,mt_ix,0);
        if ((ralpriv->mt[mt_ix].radio_channel[ch_ix].status == RB_CONNECTED)||((ralpriv->pending_req_flag) > 100)){
           DEBUG("RAL_verifyPendingRbStatus -in- mt_ix %d - ch_ix %d \n", mt_ix, ch_ix );
           // send confirmation to upper layer
           ralpriv->pending_req_status = MIH_C_STATUS_SUCCESS;
           if ((ralpriv->pending_req_flag) > 100){
              ralpriv->pending_req_status = MIH_C_STATUS_REJECTED;
              if (mt_ix == RAL_MAX_MT)
                 eRALlte_process_clean_channel(&(ralpriv->mcast.radio_channel));
              else
                 eRALlte_process_clean_channel(&(ralpriv->mt[mt_ix].radio_channel[ch_ix]));
           }
           if (mt_ix == RAL_MAX_MT)
              ltid = &ralpriv->mcast.ltid;
           else
              ltid = &ralpriv->mt[mt_ix].ltid;

           if (!ralpriv->pending_mt_flag)
              // To be updated and completed
              //aRALu_send_link_res_activate_cnf();
              eRALlte_send_link_up_indication(&ralpriv->pending_req_transaction_id, ltid, NULL, NULL, NULL, NULL);
           else
              ralpriv->pending_mt_flag = 0;
           ralpriv->pending_req_flag = 0;
           DEBUG("After response, Pending Req Flag = 0 , %d\n", ralpriv->pending_req_flag);
        }
        //DEBUG("RAL_verifyPendingRbStatus - 2-  \n");
    }
}

//---------------------------------------------------------------------------
int RAL_process_NAS_message(int ioctl_obj, int ioctl_cmd, int mt_ix, int ch_ix){
//---------------------------------------------------------------------------
 int err, rc;
//  int mt_ix, ch_ix;
  unsigned int cnxid;

// DEBUG ("\n%d , %d,", mt_ix, ch_ix);
  switch (ioctl_obj){
/***************************/
      case IO_OBJ_STATS:
          {
            DEBUG("Statistics requested -- FFS \n");
          }
      break;
/***************************/
      case IO_OBJ_CNX:
         switch (ioctl_cmd){
      /***/
            case IO_CMD_LIST:
              {
                 // printf("Usage: gioctl cx list\n");
                u8 *msgrep;
                u8 i;
                struct nas_msg_cx_list_reply *list;
                u8 lcr;
                short int num_mts;

                gifr.type=NAS_MSG_CX_LIST_REQUEST;
                //gifr.msg=(char *)malloc(NAS_LIST_CX_MAX*sizeof(struct nas_msg_cx_list_reply)+1);
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgrep=(u8 *)(gifr.msg);
                //
                DEBUG("--\n");
                DEBUG("Connexion list requested\n");
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                // Print result
                DEBUG("Lcr\t\tCellId\tIID4\tIID6\t\t\tnum_rb\tnsclass\tState\n");
                list=(struct nas_msg_cx_list_reply *)(msgrep+1);
                num_mts = msgrep[0];
                for(lcr=0; lcr<num_mts; ++lcr){
                  DEBUG("%u\t\t%u\t%u\t", list[lcr].lcr, list[lcr].cellid, list[lcr].iid4);
                  for (i=0;i<8;++i)
                    DEBUG("%02x", *((u8 *)list[lcr].iid6+i));
                  DEBUG("\t%u\t%u\t", list[lcr].num_rb, list[lcr].nsclassifier);
                  print_state(list[lcr].state);
                }
                if (init_flag){
                  RAL_NASinitMTlist(msgrep, num_mts);
                }else{
                  RAL_NASupdatetMTlist(msgrep, num_mts);
                }
                rc = 0;
              }
          break;
      /***/
          default:
          ERR ("RAL_process_NAS_message : invalid ioctl command %d\n",ioctl_cmd);
        rc= -1;
      } //end switch ioctl_cmd 
      break;

/***************************/
      case IO_OBJ_RB:
         switch (ioctl_cmd){
      /***/
            case IO_CMD_ADD:
              {
                // printf("Usage: gioctl rb add <lcr> <rab_id> <qos>\n");
                struct nas_msg_rb_establishment_request *msgreq;
                struct nas_msg_rb_establishment_reply *msgrep;
                struct ral_lte_channel *currChannel;
                MIH_C_LINK_TUPLE_ID_T* ltid;
                MIH_C_LINK_DN_REASON_T reason_code;
                //
                gifr.type=NAS_MSG_RB_ESTABLISHMENT_REQUEST;
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgreq=(struct nas_msg_rb_establishment_request *)(gifr.msg);
                msgrep=(struct nas_msg_rb_establishment_reply *)(gifr.msg);
                //
                if (mt_ix == RAL_MAX_MT){
                    // multicast
                    currChannel = &(ralpriv->mcast.radio_channel);
                    msgreq->lcr = mt_ix;
                    memcpy ((char *)&(msgreq->mcast_group), (char *)&(ralpriv->mcast.mc_group_addr), 16);
                }else{
                    // unicast
                    currChannel = &(ralpriv->mt[mt_ix].radio_channel[ch_ix]);
                    msgreq->lcr = ralpriv->mt[mt_ix].ue_id;
                }
                msgreq->cnxid  = currChannel->cnx_id;
                msgreq->rab_id = currChannel->rbId;
                msgreq->qos    = currChannel->RadioQoSclass;
                msgreq->dscp_ul = currChannel->dscpUL;
                msgreq->dscp_dl = currChannel->dscpDL;
                msgreq->mcast_flag  = currChannel->multicast;
                cnxid = msgreq->cnxid;
                currChannel->status = RB_DISCONNECTED;
                //
                DEBUG("Radio bearer establishment requested, cnxid %d\n", cnxid);
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                //  check answer from NAS
                msgrep->cnxid = cnxid; // Temp - hardcoded

                if ((msgrep->status<0)||(msgrep->cnxid!=cnxid)||(err<0)){
                  ERR(" Radio bearer establishment failure: %d\n",msgrep->status);
                  currChannel->status = RB_DISCONNECTED;
                  rc = -1;
                  ralpriv->pending_req_status = MIH_C_STATUS_REJECTED;
                  if (ralpriv->pending_mt_flag){
                     reason_code = MIH_C_LINK_DOWN_REASON_NO_RESOURCE;
                     if (mt_ix == RAL_MAX_MT)
                         ltid = &ralpriv->mcast.ltid;
                     else
                         ltid = &ralpriv->mt[mt_ix].ltid;
                     eRALlte_send_link_down_indication(&ralpriv->pending_req_transaction_id, ltid, NULL, &reason_code);
                    // aRALu_send_link_res_activate_cnf();
                  }
                  if (mt_ix == RAL_MAX_MT)
                     eRALlte_process_clean_channel(&(ralpriv->mcast.radio_channel));
                  else
                     eRALlte_process_clean_channel(&(ralpriv->mt[mt_ix].radio_channel[ch_ix]));
                }else{
                  rc = 0;
                  ralpriv->pending_req_flag = 1;
                  ralpriv->pending_req_mt_ix = mt_ix;
                  ralpriv->pending_req_ch_ix = ch_ix;
                  ralpriv->pending_req_multicast = currChannel->multicast;
                  DEBUG("-1- pending_req_mt_ix %d, pending_req_ch_ix %d\n", ralpriv->pending_req_mt_ix, ralpriv->pending_req_ch_ix);
                }
              }
          break;

      /***/
             case IO_CMD_DEL:
              {
                //  printf("Usage: gioctl rb del <lcr> <rab_id>\n");
                struct nas_msg_rb_release_request *msgreq;
                struct nas_msg_rb_release_reply *msgrep;
                struct ral_lte_channel *currChannel;
                MIH_C_LINK_TUPLE_ID_T* ltid;
                MIH_C_LINK_DN_REASON_T reason_code;
                //
                gifr.type=NAS_MSG_RB_RELEASE_REQUEST;
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgreq=(struct nas_msg_rb_release_request *)(gifr.msg);
                msgrep=(struct nas_msg_rb_release_reply *)(gifr.msg);
                //
                if (mt_ix == RAL_MAX_MT){
                    currChannel = &(ralpriv->mcast.radio_channel);
                    msgreq->lcr = mt_ix;
                }else{
                    currChannel = &(ralpriv->mt[mt_ix].radio_channel[ch_ix]);
                    msgreq->lcr = ralpriv->mt[mt_ix].ue_id;
                }
                msgreq->rab_id = currChannel->rbId;
                msgreq->cnxid = currChannel->cnx_id;
                msgreq->mcast_flag  = currChannel->multicast;
                cnxid = msgreq->cnxid;
                //
                DEBUG(" Radio Bearer release requested\n");
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                //  check answer from NAS
                msgrep->cnxid = cnxid; // Temp - hardcoded
                if ((msgrep->status<0)||(msgrep->cnxid!=cnxid)||(err<0)){
                  ERR(" Radio bearer release failure: status %d\n", msgrep->status);
                  rc = -1;
                  ralpriv->pending_req_status = MIH_C_STATUS_REJECTED;
                }else{
                  ralpriv->pending_req_status = MIH_C_STATUS_SUCCESS;
                }
                if (ralpriv->pending_mt_flag){
                   reason_code = MIH_C_LINK_DOWN_REASON_EXPLICIT_DISCONNECT;
                   if (mt_ix == RAL_MAX_MT)
                       ltid = &ralpriv->mcast.ltid;
                   else
                       ltid = &ralpriv->mt[mt_ix].ltid;
                   eRALlte_send_link_down_indication(&ralpriv->pending_req_transaction_id, ltid, NULL, &reason_code);
                    // aRALu_send_link_res_deactivate_cnf();
                }

                // mark resource as free again anyway
                if (mt_ix == RAL_MAX_MT)
                   eRALlte_process_clean_channel(&(ralpriv->mcast.radio_channel));
                else
                   eRALlte_process_clean_channel(&(ralpriv->mt[mt_ix].radio_channel[ch_ix]));
                DEBUG("Channel released : UE %d, channel %d, cnx_id %d \n\n",mt_ix, ch_ix, cnxid);
              }
          break;

    /***/

      /***/
            case IO_CMD_LIST:
              {
               // printf("Usage: gioctl rb list <lcr>\n");
                u8 *msgrep;
                u8 rbi, i;
                u8 num_rbs;
                struct nas_msg_rb_list_reply *list;
                struct nas_msg_rb_list_request *msgreq;
                gifr.type=NAS_MSG_RB_LIST_REQUEST;
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgreq=(struct nas_msg_rb_list_request *)(gifr.msg);
                msgrep=(u8 *)(gifr.msg);
                if (mt_ix < RAL_MAX_MT){
                    msgreq->lcr = ralpriv->mt[mt_ix].ue_id;
                }else{
                    msgreq->lcr = 0;
                    mt_ix =0;  //Temp
                }
                //
                DEBUG(" Radio bearer list requested\n");
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                num_rbs = msgrep[0];
                //RAL_print_buffer (msgrep, 50);
                DEBUG("number of radio bearers %d \n", num_rbs);
                DEBUG("rab_id\tcnxid\tSapi\t\tQoS\t\tState\n");
                list=(struct nas_msg_rb_list_reply *)(msgrep+1);
                for(rbi=0; rbi<num_rbs; ++rbi){
                  DEBUG("%u\t%u\t%u\t\t%u\t\t", list[rbi].rab_id,list[rbi].cnxid, list[rbi].sapi, list[rbi].qos);
                  print_state(list[rbi].state);
                  rc = 0;
                  // store channel status
                  for (i=0;i<num_rbs; i++){
                    if (ralpriv->mt[mt_ix].radio_channel[i].cnx_id == list[rbi].cnxid){
                      ralpriv->mt[mt_ix].radio_channel[i].nas_state = list[rbi].state;
                      if (list[rbi].state == NAS_CX_DCH)
                        ralpriv->mt[mt_ix].radio_channel[i].status = RB_CONNECTED;
                    }
                  }
                  if ((mt_ix==0)&&(i==num_rbs)){
                    if (ralpriv->mcast.radio_channel.cnx_id == list[rbi].cnxid){
                      ralpriv->mcast.radio_channel.nas_state = list[rbi].state;
                      if (list[rbi].state == NAS_CX_DCH){
                        ralpriv->mcast.radio_channel.status = RB_CONNECTED;
                        ralpriv->mt[mt_ix].radio_channel[1].status = RB_CONNECTED;
                      }
                    }
                  }
                }
                DEBUG("List complete \n");
               }
            break;
      /***/
            default:
            ERR ("RAL_process_NAS_message : invalid ioctl command %d\n",ioctl_cmd);
            rc= -1;
         } //end switch ioctl_cmd 
      break;

/***************************/
      case IO_OBJ_MC:
         switch (ioctl_cmd){
      /***/
            case IO_CMD_ADD:
              {
                struct nas_msg_mt_mcast_join *msgreq;
                struct nas_msg_mt_mcast_reply *msgrep;
                struct ral_lte_channel *currChannel;
                //
                gifr.type=NAS_RG_MSG_MT_MCAST_JOIN;
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgreq=(struct nas_msg_mt_mcast_join *)(gifr.msg);
                msgrep=(struct nas_msg_mt_mcast_reply *)(gifr.msg);
                //
                currChannel = &(ralpriv->mcast.radio_channel);
                msgreq->ue_id = mt_ix;
                msgreq->rab_id = currChannel->rbId;
                msgreq->cnxid = currChannel->cnx_id;
                //
                DEBUG("UE multicast join notification requested, ue_id %d\n", mt_ix);
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                //  check answer from NAS
                if ((msgrep->result<0)||(msgrep->ue_id!=mt_ix)||(err<0)){
                  ERR(" UE multicast join notification failure: %d\n",msgrep->result);
                  ralpriv->pending_req_status = MIH_C_STATUS_REJECTED;
                  rc = -1;
                }else{
                  DEBUG(" ++ UE multicast join notification transmitted to MT \n");
                  ralpriv->pending_req_status = MIH_C_STATUS_SUCCESS;
                  rc = 0;
                }
                // TODO aRALu_send_link_mc_join_cnf();
              }
            break;

      /***/
             case IO_CMD_DEL:
              {
                struct nas_msg_mt_mcast_leave *msgreq;
                struct nas_msg_mt_mcast_reply *msgrep;
                struct ral_lte_channel *currChannel;
                //
                gifr.type=NAS_RG_MSG_MT_MCAST_LEAVE;
                memset (ralpriv->buffer,0,800);
                gifr.msg= &(ralpriv->buffer[0]);
                msgreq=(struct nas_msg_mt_mcast_leave *)(gifr.msg);
                msgrep=(struct nas_msg_mt_mcast_reply *)(gifr.msg);
                //
                currChannel = &(ralpriv->mcast.radio_channel);
                msgreq->ue_id = mt_ix;
                msgreq->rab_id = currChannel->rbId;
                msgreq->cnxid = currChannel->cnx_id;
                //
                DEBUG("UE multicast leave notification requested, ue_id %d\n", mt_ix);
                err=ioctl(fd, NASRG_IOCTL_RAL, &gifr);
                if (err<0){
                  ERR("IOCTL error, err=%d\n",err);
                  rc = -1;
                }
                //  check answer from NAS
                if ((msgrep->result<0)||(msgrep->ue_id!=mt_ix)||(err<0)){
                  ERR(" UE multicast leave notification failure: %d\n",msgrep->result);
                  ralpriv->pending_req_status = MIH_C_STATUS_REJECTED;
                  rc = -1;
                }else{
                  DEBUG(" ++ UE multicast leave notification transmitted to MT \n" );
                  ralpriv->pending_req_status = MIH_C_STATUS_SUCCESS;
                  rc = 0;
                }
                // TODO aRALu_send_link_mc_leave_cnf();
              }
            break;
    /***/
      /***/
            default:
            ERR ("RAL_process_NAS_message : invalid ioctl command %d\n",ioctl_cmd);
            rc= -1;
         } //end switch ioctl_cmd 
      break;
    /***/
/***************************/
      default:
        ERR ("RAL_process_NAS_message : invalid ioctl object %d\n",ioctl_obj);
        rc= -1;
  } //end switch ioctl_obj 
  //rc=0;
 return rc;
}



