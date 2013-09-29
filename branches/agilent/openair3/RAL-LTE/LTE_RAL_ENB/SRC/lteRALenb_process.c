/*****************************************************************************
 *   Eurecom OpenAirInterface 3
 *    Copyright(c) 2012 Eurecom
 *
 * Source eRALlte_process.c
 *
 * Version 0.1
 *
 * Date  07/10/2012
 *
 * Product MIH RAL LTE
 *
 * Subsystem 
 *
 * Authors Michelle Wetterwald, Lionel Gauthier, Frederic Maurel
 *
 * Description 
 *
 *****************************************************************************/
#include "lteRALenb_variables.h"
#include "lteRALenb_proto.h"

#include "lteRALenb_mih_msg.h"
#include "MIH_C.h"
/****************************************************************************/
/*******************  G L O C A L    D E F I N I T I O N S  *****************/
/****************************************************************************/

extern int eRALlte_action_save_flow_id(MIH_C_FLOW_ID_T* flowId, int cnxid);

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

static void _eRALlte_process_clean_pending_mt(void);
static void _eRALlte_process_waiting_RB(int mt_ix);

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_find_channel()                            **
 **                                                                        **
 ** Description: Returns the Mobile Terminal index and the Radio Bearer    **
 **   channel index for the specified Connection identifier.    **
 **                                                                        **
 ** Inputs:  cnxid:  Connection identifier                      **
 **     Others: ralpriv                                    **
 **                                                                        **
 ** Outputs:  mt_ix:  Mobile Terminal index                      **
 **     ch_ix:  Radio Bearer channel index                 **
 **   Return: 0 if no MT and RB channel indexes exist    **
 **    for the specified Connection identifier.   **
 **    1 otherwise.                               **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
int eRALlte_process_find_channel(unsigned int cnxid, int* mt_ix, int* ch_ix){
    int mt, ch;
    int found = 0;

    DEBUG(" %s : cnxid = %d\n", __FUNCTION__, cnxid);

    for (mt = 0; !found && (mt < RAL_MAX_MT); mt++) {
      for (ch = 0; ch < RAL_MAX_RB; ch++) {
        if (ralpriv->mt[mt].radio_channel[ch].cnx_id == cnxid) {
          found = 1;
          *mt_ix = mt;
          *ch_ix = ch;
          break;
        }
      }
    }

    if (!found) {
      *mt_ix = RAL_MAX_MT;
      *ch_ix = 0;
    }

    DEBUG (" %s : return %d (mt=%d, ch=%d)\n" , __FUNCTION__, found, *mt_ix, *ch_ix);
    return found;
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_find_new_channel()                        **
 **                                                                        **
 ** Description: Returns the index of the first available Radio Bearer     **
 **   channel for the specified Mobile Terminal identifier.     **
 **                                                                        **
 ** Inputs:  mt_ix:  Mobile Terminal index                      **
 **     Others: ralpriv                                    **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: The index of the first available RB chan-  **
 **    nel; RAL_MAX_RB if no any RB channel is    **
 **    available.                                 **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
int eRALlte_process_find_new_channel(int mt_ix)
{
    int ch_ix;

    DEBUG(" %s : mt_ix = %d\n", __FUNCTION__, mt_ix);

    for (ch_ix = 0; ch_ix < RAL_MAX_RB; ch_ix++) {
 if (ralpriv->mt[mt_ix].radio_channel[ch_ix].cnx_id == 0) {
     break;
 }
    }

    DEBUG (" %s : return %d\n" , __FUNCTION__, ch_ix);
    return ch_ix;
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_clean_channel()                           **
 **                                                                        **
 ** Description: Cleans data of the specified Radio Bearer channel.        **
 **                                                                        **
 ** Inputs:  None                                                      **
 **     Others: None                                       **
 **                                                                        **
 ** Outputs:  channel: Channel data to clean                      **
 **   Return: None                                       **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
void eRALlte_process_clean_channel(struct ral_lte_channel* channel)
{
    DEBUG(" %s : cnx_id = %d, rbId = %d\n", __FUNCTION__,
   channel->cnx_id, channel->rbId);
    memset(channel, 0, sizeof (struct ral_lte_channel));
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_mt_addr_to_string()                       **
 **                                                                        **
 ** Description: Display the specified IPv6 address.                       **
 **                                                                        **
 ** Inputs:  ip_addr: The IP address                             **
 **     Others: None                                       **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: Pointer to the string buffer.              **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
char* eRALlte_process_mt_addr_to_string(const unsigned char* ip_addr)
{
    static char buffer[40];
    int i, index = 0;

    for (i = 0; i < 16; i++) {
 index += sprintf(&buffer[index], "%.2hX", ip_addr[i]);
 if (i % 2) buffer[index++] = ':';
    }
    buffer[--index] = '\0';
    return buffer;
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_mt_addr_to_l2id()                         **
 **                                                                        **
 ** Description: Convert the specified IP address to Layer 2 identifier.   **
 **                                                                        **
 ** Inputs:  mt_addr: MT's IP address                            **
 **     Others: None                                       **
 **                                                                        **
 ** Outputs:  l2id  Layer 2 identifier                         **
 **   Return: None                                       **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
void eRALlte_process_mt_addr_to_l2id(const unsigned char* mt_addr, unsigned int* l2id)
{
    if ( !(mt_addr) || !(l2id) ) {
 ERR(" %s : input parameter is NULL\n", __FUNCTION__);
 return;
    }
    l2id[0] = mt_addr[0]+256 *(mt_addr[1]+256*(mt_addr[2]+256*(mt_addr[3])));
    l2id[1] = mt_addr[4]+256 *(mt_addr[5]+256*(mt_addr[6]+256*(mt_addr[7])));
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_cmp_mt_addr()                             **
 **                                                                        **
 ** Description: Compares MT's IP address to the specified L2 identifier.  **
 **                                                                        **
 ** Inputs:  mt_addr: MT's IP address                            **
 **    l2id  Layer 2 identifier                         **
 **     Others: None                                       **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: 0 if the MT's IP address matches the L2    **
 **    identifier, 1 otherwise.                   **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
int eRALlte_process_cmp_mt_addr(const char* mt_addr, const char* l2id)
{
    int i;
    for (i = 0; i < 8; i++) {
 if ((u8)l2id[i] != (u8)mt_addr[i+8]) {
     return 0;
 }
    }
    return 1;
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_find_mt_by_addr()                         **
 **                                                                        **
 ** Description: Returns the index of the Mobile Terminal with the         **
 **   specified IP address.                                     **
 **                                                                        **
 ** Inputs:  mt_addr: MT's IP address                            **
 **     Others: ralpriv                                    **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: The MT's index in the list of MTs;         **
 **    RAL_MAX_MT if not found.                   **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
int eRALlte_process_find_mt_by_addr(const char* mt_addr)
{
    int mt_ix;

    for (mt_ix = 0; mt_ix < RAL_MAX_MT; mt_ix++) {
 const char* l2id = (const char*)(&ralpriv->mt[mt_ix].ipv6_l2id[0]);
 int i;
 for (i = 0; i < 8; i++) {
     if ((u8)l2id[i] != (u8)mt_addr[i+8]) {
  break;
     }
 }
 if (i == 8) {
     break;
 }
    }

    DEBUG (" %s : return %d\n" , __FUNCTION__, mt_ix);
    return mt_ix;
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_verify_pending_mt_status()                **
 **                                                                        **
 ** Description: Checks the pending MT connection status and simulates RB  **
 **   setup if it is ready for Radio Bearer establishment.      **
 **                                                                        **
 ** Inputs:  None                                                      **
 **     Others: ralpriv                                    **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: None                                       **
 **     Others: None                                       **
 **                                                                        **
 ***************************************************************************/
void eRALlte_process_verify_pending_mt_status(void)
{
    int mt_ix;

    if (!ralpriv->pending_mt_timer) {
        DEBUG(" Pending MT timer expired\n");
        #ifdef RAL_REALTIME
        _eRALlte_process_clean_pending_mt();
        #endif
        #ifdef RAL_DUMMY
        DEBUG(" SIMULATE MT arrival\n");
        mt_ix = eRALlte_NAS_update_MTs_list();
        DEBUG(" SIMULATE RB setup\n");
        _eRALlte_process_waiting_RB(mt_ix);
        #endif
    } else {
        #ifdef RAL_REALTIME
        if (!( ralpriv->pending_mt_timer % 10)) {
            /* Get the list of MTs in the driver waiting for pending 
              * RB establishment */
            RAL_process_NAS_message(IO_OBJ_CNX, IO_CMD_LIST, 0, 0);
            RAL_process_NAS_message(IO_OBJ_RB, IO_CMD_LIST, 0, 0);
            /* Check if the pending MT is in the list */
            mt_ix = eRALlte_process_find_mt_by_addr((char*)ralpriv->pending_mt.ipv6_addr);
            if ( (mt_ix < RAL_MAX_MT) &&
          (ralpriv->mt[mt_ix].mt_state == RB_CONNECTED)) {
          /* The pending MT is ready for RB establishment */
          _eRALlte_process_waiting_RB(mt_ix);
            }
        }
        #endif
    }
}

/****************************************************************************
 **                                                                        **
 ** Name:  eRALlte_process_map_qos()                                 **
 **                                                                        **
 ** Description: Performs mapping of reserved bit rate to Radio QoS class, **
 **   and mapping of traffic class to DCSP.                     **
 **                                                                        **
 ** Inputs:  mt_ix:  Mobile Terminal index                      **
 **     ch_ix:  Radio Bearer channel index                 **
 **     Others: None                                       **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: 1 if the requested reserved bit rate and   **
 **    traffic class are supported and their      **
 **    mapping to Radio Qos class and DSCP        **
 **    succeed. 0 otherwise.                      **
 **     Others: ralpriv                                    **
 **                                                                        **
 ***************************************************************************/
int eRALlte_process_map_qos(int mt_ix, int ch_ix)
{
    int resBitrateDL = 0, resBitrateUL = 0;
    struct ral_lte_channel *currChannel;

    /*
     * Map for multicast flow
     */
    if (mt_ix == RAL_MAX_MT)
    {
 currChannel = &(ralpriv->mcast.radio_channel);

 /* multicast - reserved bit rate */
 resBitrateDL = (int)currChannel->resBitrate[1];
 if (resBitrateDL <= RAL_BITRATE_128k) {
     currChannel->RadioQoSclass = 20;
 } else if (resBitrateDL <= RAL_BITRATE_256k) {
     currChannel->RadioQoSclass = 21;
 } else if (resBitrateDL <= RAL_BITRATE_384k) {
     currChannel->RadioQoSclass = 22;
 } else {
     ERR (" %s : Invalid requested resBitrate %d - Request will be rejected.\n", __FUNCTION__, resBitrateDL);
     return 0;
 }

 /* multicast - DSCP */
 if (currChannel->classId[1] < 64) {
     currChannel->dscpDL = currChannel->classId[1];
 } else {
     ERR (" %s : DSCP %d > 63 - NOT SUPPORTED - Request will be rejected.\n", __FUNCTION__, currChannel->classId[1]);
     return 0;
 }
 DEBUG (" QoS Mapping : Requested radio QoS class %d, DSCP DL %d\n",
        currChannel->RadioQoSclass,
        currChannel->dscpDL);
    }

    /*
     * Map for unicast flow
     */
    else {
 currChannel = &(ralpriv->mt[mt_ix].radio_channel[ch_ix]);

 /* unicast - reserved bit rate - CONVERSATIONAL */
 resBitrateDL = (int)currChannel->resBitrate[1];
 resBitrateUL = (int)currChannel->resBitrate[0];
 if ((resBitrateDL <= RAL_BITRATE_32k) &&
     (resBitrateUL <= RAL_BITRATE_32k)) {
     currChannel->RadioQoSclass = 1;
 } else if ((resBitrateDL <= RAL_BITRATE_64k) &&
     (resBitrateUL <= RAL_BITRATE_64k)) {
     currChannel->RadioQoSclass = 2;
 } else if ((resBitrateDL <= RAL_BITRATE_128k) &&
     (resBitrateUL <= RAL_BITRATE_128k)) {
     currChannel->RadioQoSclass = 3;
 } else if ((resBitrateDL <= RAL_BITRATE_256k) &&
     (resBitrateUL <= RAL_BITRATE_256k)) {
     currChannel->RadioQoSclass = 4;
 } else if ((resBitrateDL <= RAL_BITRATE_320k) &&
     (resBitrateUL <= RAL_BITRATE_320k)) {
     currChannel->RadioQoSclass = 5;
 } else if ((resBitrateDL >= RAL_BITRATE_384k) &&
     (resBitrateDL <= RAL_BITRATE_440k) &&
     (resBitrateUL <= RAL_BITRATE_64k)) {
     currChannel->RadioQoSclass = 14;
 } else {
     ERR (" %s : Invalid requested resBitrate %d , %d - Request will be rejected\n", __FUNCTION__, resBitrateDL, resBitrateUL);
     return 0;
 }

 /* unicast - DSCP */
 if ((currChannel->classId[0] < 64) &&
     (currChannel->classId[1] < 64)) {
     currChannel->dscpUL = currChannel->classId[0];
     currChannel->dscpDL = currChannel->classId[1];
 } else {
     ERR (" %s : DSCP > 63 - NOT SUPPORTED - Request will be rejected.\n", __FUNCTION__);
     return 0;
 }
 DEBUG (" QoS Mapping : Requested radio QoS class %d, DSCP UL %d, DSCP DL %d\n", currChannel->RadioQoSclass, currChannel->dscpUL, currChannel->dscpDL);
    }

    return 1;
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:  _eRALlte_process_waiting_RB()                             **
 **                                                                        **
 ** Description: Allocates a new Radio Bearer parameters to the specified  **
 **   Mobile Terminal which was pending for RB establishment,   **
 **   and sends RB establishment request to the NAS sublayer.   **
 **                                                                        **
 ** Inputs:  mt_ix:  Mobile Terminal index                      **
 **     Others: ralpriv                                    **
 **                                                                        **
 ** Outputs:  None                                                      **
 **   Return: None                                       **
 **     Others: ralpriv                                    **
 **                                                                        **
 ***************************************************************************/
static void _eRALlte_process_waiting_RB(int mt_ix)
{
    struct ral_lte_channel *currChannel;
    struct ral_lte_channel *pendingChannel;
    int ch_ix, f_ix;
    int dir, mapping_result, rc = -1;

    DEBUG(" Establishment of pending RB now starting...\n");
    ch_ix = eRALlte_process_find_new_channel(mt_ix);
    if (ch_ix == RAL_MAX_RB) {
      DEBUG(" No RB available in MT - Deleting request data\n");
      _eRALlte_process_clean_pending_mt();
      return;
    }

    pendingChannel = &(ralpriv->pending_mt.radio_channel[0]);
    currChannel = &(ralpriv->mt[mt_ix].radio_channel[ch_ix]);

    currChannel->cnx_id = (RAL_MAX_RB_PER_UE*mt_ix)+ch_ix+1;
    currChannel->rbId = RAL_DEFAULT_RAB_ID+ch_ix;
    currChannel->multicast = 0;
    DEBUG(" mt_ix %d, ch_ix %d, cnx_id %d, rbId %d\n",
   mt_ix, ch_ix, currChannel->cnx_id, currChannel->rbId);
    memcpy((char *)&(ralpriv->mt[mt_ix].ipv6_addr),
    (char *)&(ralpriv->pending_mt.ipv6_addr), 16);
    DEBUG (" MT's address = %s\n",
    eRALlte_process_mt_addr_to_string(ralpriv->mt[mt_ix].ipv6_addr));

    /* Save the pending data flow identifier into the list of active data flows */
    f_ix = eRALlte_action_save_flow_id(&ralpriv->pending_req_fid, currChannel->cnx_id);
    if (f_ix < 0) {
      DEBUG(" No RB available - Deleting request data\n");
      _eRALlte_process_clean_pending_mt();
      return;
    }
    /* Store resource parameters of the pending MT */
    for (dir = 0; dir < 2; dir++) {
      currChannel->flowId[dir] = f_ix;
      currChannel->classId[dir]= pendingChannel->classId[dir] ;
      currChannel->resBitrate[dir] = pendingChannel->resBitrate[dir];
      currChannel->meanBitrate[dir] = pendingChannel->meanBitrate[dir];
      currChannel->bktDepth[dir] = pendingChannel->bktDepth[dir];
      currChannel->pkBitrate[dir] = pendingChannel->pkBitrate[dir];
      currChannel->MTU[dir] = pendingChannel->MTU[dir];
      DEBUG(" qos value : DIR %d, flowId %d, classId %d, resBitrate %.1f \n",
                 dir, currChannel->flowId[dir], currChannel->classId[dir], currChannel->resBitrate[dir]);
    }

    /* Map Qos */
    mapping_result = eRALlte_process_map_qos(mt_ix, ch_ix);
    if (mapping_result) {
      #ifdef RAL_DUMMY
      rc = eRALlte_NAS_send_rb_establish_request(mt_ix, ch_ix);
      #endif
      #ifdef RAL_REALTIME
      rc = RAL_process_NAS_message(IO_OBJ_RB, IO_CMD_ADD, mt_ix, ch_ix);
      #endif
    }

    if (rc < 0) {
      /* Failed to send RB establishment request; release new RB data */
      eRALlte_process_clean_channel(currChannel);
    }
    else {
      /* RB establishment request has been sent; release pending MT data */
      ralpriv->pending_req_flag = 0;
      _eRALlte_process_clean_pending_mt();
    }
}

/****************************************************************************
 ** Name:  _eRALlte_process_clean_pending_mt()                             **
 ** Description: Deletes previously stored pending MT data.                **
 ***************************************************************************/
static void _eRALlte_process_clean_pending_mt(void)
{
    memset(ralpriv->pending_mt.ipv6_addr, 0 , 16);
    eRALlte_process_clean_channel(&ralpriv->pending_mt.radio_channel[0]);

    ralpriv->pending_mt_timer = -1;
    DEBUG(" Pending MT data deleted\n");
}

/****************************************************************************
 **  MW Added                                                              **
 ***************************************************************************/
//---------------------------------------------------------------------------
void RAL_printInitStatus(void){
//---------------------------------------------------------------------------
    DEBUG("Network status updated \n");
    DEBUG("Mobile : %d, %d, %d, %d, %d, %d \n",
      ralpriv->curr_cellId,
      ralpriv->mt[0].ue_id,
      ralpriv->mt[0].ipv6_l2id[0],
      ralpriv->mt[0].ipv6_l2id[1],
      ralpriv->mt[0].num_rbs ,
      ralpriv->mt[0].nas_state );
    DEBUG("Default rb : %d, %d, %d, %d, %d, %d, %d \n",
      ralpriv->mt[0].radio_channel[0].rbId ,
      ralpriv->mt[0].radio_channel[0].RadioQoSclass ,
      ralpriv->mt[0].radio_channel[0].cnx_id ,
      ralpriv->mt[0].radio_channel[0].dscpUL ,
      ralpriv->mt[0].radio_channel[0].dscpDL ,
      ralpriv->mt[0].radio_channel[0].nas_state ,
      ralpriv->mt[0].radio_channel[0].status );

    DEBUG("Number of connected MTs : %d\n\n", ralpriv->num_connected_mts);
}

//---------------------------------------------------------------------------
// poll for measures in NAS
void RAL_NAS_measures_polling(void){
//---------------------------------------------------------------------------
   #ifdef RAL_REALTIME
   RAL_process_NAS_message(IO_OBJ_MEAS, IO_CMD_LIST,0,0);
   #endif
   #ifdef RAL_DUMMY 
   eRALlte_NAS_send_measure_request();
   #endif
}

//---------------------------------------------------------------------------
// Common function to report congestion
void RAL_NAS_report_congestion(int ix){
//---------------------------------------------------------------------------
  MIH_C_TRANSACTION_ID_T transaction_id;
  MIH_C_LINK_TUPLE_ID_T  link_identifier;
  LIST(MIH_C_LINK_PARAM_RPT, LinkParametersReportList);

  DEBUG("Congestion detected for UE%d, sending congestion notification to MIH User \n", ix);
  transaction_id = MIH_C_get_new_transaction_id();
  link_identifier.link_id.link_type = MIH_C_WIRELESS_UMTS;
  link_identifier.link_id.link_addr.choice = MIH_C_CHOICE_3GPP_3G_CELL_ID;
  Bit_Buffer_t *plmn = new_BitBuffer_0();
  BitBuffer_wrap(plmn, (unsigned char*) ralpriv->plmn, DEFAULT_PLMN_SIZE);
  MIH_C_PLMN_ID_decode(plmn, &link_identifier.link_id.link_addr._union._3gpp_3g_cell_id.plmn_id);
  free_BitBuffer(plmn);
  link_identifier.link_id.link_addr._union._3gpp_3g_cell_id.cell_id = ralpriv->curr_cellId;
  link_identifier.choice = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;
  //
  LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
  LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_L2_BUFFER_STATUS;
  // LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_GEN;
  // LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_gen = MIH_C_LINK_PARAM_LTE_L2_BUFFER_STATUS;
  LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
  LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = ralpriv->rlcBufferOccupancy[ix];

  LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_THRESHOLD;
  LinkParametersReportList_list.val[LinkParametersReportList_list.length]._union.threshold.threshold_val  = ralpriv->congestion_threshold;
  LinkParametersReportList_list.val[LinkParametersReportList_list.length]._union.threshold.threshold_xdir = MIH_C_ABOVE_THRESHOLD;
  LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;

  //
  eRALlte_send_link_parameters_report_indication(&transaction_id,  &link_identifier, &LinkParametersReportList_list);
  ralpriv->congestion_flag = RAL_TRUE;

}
//---------------------------------------------------------------------------
// Temp - Enter hard-coded measures in IAL
void RAL_NAS_measures_analyze(void){
//---------------------------------------------------------------------------
  MIH_C_TRANSACTION_ID_T transaction_id;
  MIH_C_LINK_TUPLE_ID_T  link_identifier;
  LIST(MIH_C_LINK_PARAM_RPT, LinkParametersReportList);
  int ix;

  LinkParametersReportList_list.length = 0;

  if (ralpriv->congestion_flag == RAL_FALSE){
  // Check congestion
    for (ix=0; ix<ralpriv->num_UEs; ix++){
       if ((ralpriv->rlcBufferOccupancy[ix] > ralpriv->congestion_threshold)&&
           ((ralpriv->mih_subscribe_req_event_list && MIH_C_BIT_LINK_PARAMETERS_REPORT )>0)){
           RAL_NAS_report_congestion(ix);
           break;
       }
    }
  } else if (ralpriv->measures_triggered_flag == RAL_TRUE){
  // Measures should be reported to MIH user
      DEBUG("Sending traffic measures to MIH User \n");

      transaction_id = MIH_C_get_new_transaction_id();
      link_identifier.link_id.link_type = MIH_C_WIRELESS_UMTS;
      link_identifier.link_id.link_addr.choice = MIH_C_CHOICE_3GPP_3G_CELL_ID;
      Bit_Buffer_t *plmn = new_BitBuffer_0();
      BitBuffer_wrap(plmn, (unsigned char*) ralpriv->plmn, DEFAULT_PLMN_SIZE);
      MIH_C_PLMN_ID_decode(plmn, &link_identifier.link_id.link_addr._union._3gpp_3g_cell_id.plmn_id);
      free_BitBuffer(plmn);
      link_identifier.link_id.link_addr._union._3gpp_3g_cell_id.cell_id = ralpriv->curr_cellId;
      link_identifier.choice = MIH_C_LINK_TUPLE_ID_CHOICE_NULL;
      //
      // send parameters  !!! Value link_param_val must be in 0...65535 range
      // UE0 scheduledPRB
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_AVAILABLE_BW;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = ralpriv->scheduledPRB[0];
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
      LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;
      // UE0 totalDataVolume
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_AVAILABLE_BW;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = (ralpriv->totalDataVolume[0]/1000);
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
      LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;
      // UE1 scheduledPRB
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_AVAILABLE_BW;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = ralpriv->scheduledPRB[1];
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
      LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;
      // UE1 totalDataVolume
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_AVAILABLE_BW;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = (ralpriv->totalDataVolume[1]/1000);
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
      LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;
      // totalNumPRBs
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type.choice = MIH_C_LINK_PARAM_TYPE_CHOICE_LTE;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.link_param_type._union.link_param_lte = MIH_C_LINK_PARAM_LTE_AVAILABLE_BW;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param.choice  = MIH_C_LINK_PARAM_CHOICE_LINK_PARAM_VAL;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].link_param._union.link_param_val = ralpriv->totalNumPRBs;
      LinkParametersReportList_list.val[LinkParametersReportList_list.length].choice = MIH_C_LINK_PARAM_RPT_CHOICE_NULL;
      LinkParametersReportList_list.length = LinkParametersReportList_list.length + 1;
//
      eRALlte_send_link_parameters_report_indication(&transaction_id,  &link_identifier, &LinkParametersReportList_list);

  }
}
