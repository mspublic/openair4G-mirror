/**  @addtogroup _mesh_layer3_ 
 @{
 */


/** 
  \brief CMM connection setup request.  Only in CH.
  @returns status indication
*/
int cmm_cx_setup_req(L2_ID Src,             //!< L2 source MAC address
		     L2_ID Dst,             //!< L2 destination MAC address
		     QOS_CLASS_T QoS_class, //!< QOS class index
		     unsigned int Trans_id  //!< Transaction ID
		     );

/**
  \brief RRM connection confirm.  Only in CH. Confirms a cmm_cx_setup_req
*/
void rrm_cx_setup_cnf(RB_ID Rb_id,           //!< L2 Rb_id
		      unsigned int Trans_id  //!< Transaction ID
		      );

/**
  \brief CMM connection modify request.  Only in CH.
  @returns status indication
*/
int cmm_cx_modify_req(RB_ID Rb_id            //!< L2 Rb_id
		      QOS_CLASS_T QoS_class, //!< QOS class index
		      unsigned int Trans_id  //!< Transaction ID
		      );

/**
  \brief RRM connection modify confirm.  Only in CH. Confirms a cmm_cx_modify_req
*/
void rrm_cx_modify_cnf(unsigned int Trans_id  //!< Transaction ID
		       );

/**
  \brief CMM connection release request.  Only in CH.
  @returns status indication
*/
int cmm_cx_release_req(RB_ID Rb_id            //!< L2 Rb_id
		       unsigned int Trans_id  //!< Transaction ID
		       );

/**
  \brief RRM connection modify confirm.  Only in CH. Confirms a cmm_cx_modify_req
*/
void rrm_cx_release_cnf(unsigned int Trans_id  //!< Transaction ID
			);

/**
  \brief CMM connection release all resources request.  Only in CH.
  @returns status indication
*/
int cmm_cx_release_all_req(L2_ID L2_id            //!< L2 Rb_id
			   unsigned int Trans_id  //!< Transaction ID
			   );

/**
  \brief RRM connection release all confirm.  Only in CH. Confirms a cmm_cx_release_all_req
*/
void rrm_cx_release_all_cnf(unsigned int Trans_id  //!< Transaction ID
			    );


/**
  \brief  L3 Connection attachment request.  Message sent by RRCI in MR after configuration of initial RBs and reception of CH IPAddr.  Here L3_info contains CH IPAddr.  The RBID's of basic IP services are also required.
@returns status indication
*/
int rrci_attach_req(L2_ID L2_id,               //!< Layer 2 (MAC) ID
		     L3_INFO_T L3_info_t,       //!< Type of L3 Information
		     unsigned char *L3_info,    //!< L3 addressing Information
		     RB_id DTCH_B_id,           //!< RBID of broadcast IP service (MR only)
		     RB_id DTCH_id,             //!< RBID of default IP service (MR only)
		     unsigned int Trans_id      //!< Transaction ID
		     );

/**
  \brief Connection Attachment indication.  Message sent by RRM in CH at completion of attachment phase of a new MR 
(after configuration MR IPAddr). Here L3_info contains MR IPAddr. 
*/
void rrm_attach_ind(L2_ID L2_id,               //!< Layer 2 (MAC) ID
		    L3_INFO_T L3_info_t,       //!< Type of L3 Information
		    unsigned char *L3_info,    //!< L3 addressing Information
		    RB_id DTCH_id              //!< RBID of default IP service (MR only)
		     );

/**
  \brief L3 Connection Attachment confirmation.  Message sent by CMM in MR at completion of L3 attachment phase of a new MR 
Here L3_info contains MR IPAddr. 
*/
void cmm_attach_cnf(L3_INFO_T L3_info_t,       //!< Type of L3 Information
		    unsigned char *L3_info,    //!< L3 addressing Information
		    unsigned int Trans_id      //!< Transaction ID
);


/**
  \brief  Message sent by RRM to CMM to indicate attachement at layer 2 of a new MR 
*/
void rrm_MR_attach_ind(L2_ID L2_id      //!< MR Layer 2 (MAC) ID
			);

/**
  \brief  Message sent by RRM to CMM to indicate that the node function is Cluster head. CMM initializes then the CH configuration 
*/
void router_is_CH_ind(L2_ID L2_id      //!< CH Layer 2 (MAC) ID
			);



/**
  \brief 
*/
void rrci_CH_synch_ind(void);

/**
  \brief 
*/
void cmm_init_mr_req(void);

/**
  \brief 
*/
void rrm_MR_synch_ind(void);

/**
  \brief 
*/
void rrm_no_synch_ind(void);

/**
  \brief 
*/
void cmm_init_ch_req(L3_INFO_T L3_info_t,
		     void *L3_info);



/** @} */
