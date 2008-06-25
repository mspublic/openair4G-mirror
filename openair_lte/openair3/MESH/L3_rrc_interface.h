/** @defgroup _mesh_layer3_ OpenAirInterface MESH L2/L3 Interfaces 

*/


/*@{
\brief Ask RRC to establish a radio bearer.  Used mainly by CH, except during initialization phase of MR for default bearers 
(SRB0,SRB1).  Sends CH IPAddr to RRC for attachment signaling (for example during DTCH_B configuration).
 */
void rrm_rb_establish_req(LCHAN_DESC *Lchan_desc,       //!< Logical Channel Descriptor Array
			  MAC_RLC_MEAS_DESC *Mac_rlc_meas_desc, //!< MAC/RLC Measurement descriptors for RB 
			  L2_ID *L2_id,                 //!< Layer 2 (MAC) IDs for link
			  unsigned int Trans_id,        //!< Transaction ID
			  unsigned char *L3_info,       //!< Optional L3 Information
			  L3_INFO_T L3_info_t         //!< Type of L3 Information
			  );			  
/*
\brief RRC response to rb_establish_req.  RRC Acknowledgement of reception of rrc_rb_establishment_req.
 */
void rrc_rb_establish_resp(unsigned int Trans_id       //!< Transaction ID
			   );

/**
 \brief RRC confirmation of rb_establish_req.  RRC confirmation of rrc_rb_establishment_req after transactions are complete.  
Essentially for CH only (except SRB0/1)
 */
void rrc_rb_establish_cfm(RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
			  RB_TYPE RB_type,              //!< Radio Bearer Type
			  unsigned int Trans_id         //!< Transaction ID
			  );

/**
\brief RRC Connection Establishment indication.  Message received by RRM in CH at completion of attachment phase of a new MR 
(after configuration MR IPAddr). Here L3_info contains MR IPAddr. Message received by RRCI in MR after configuration of initial RBs and reception of CH IPAddr.  Here L3_info contains CH IPAddr.  For MR the RBID's of basic IP services are also required.
*/
void rrc_cx_establish_ind(L2_ID L2_id,                  //!< Layer 2 (MAC) ID
			  unsigned int Trans_id,        //!< Transaction ID
			  unsigned char *L3_info,       //!< Optional L3 Information
			  L3_INFO_T L3_info_t,          //!< Type of L3 Information
			  RB_id DTCH_B_id,              //!< RBID of broadcast IP service (MR only)
			  RB_id DTCH_id                 //!< RBID of default IP service (MR only)
			  );

/**
\brief RRCI Connection Establishment response.  Received by RRC in MR at completion of attachment phase and
address configuration of a new MR. L3_info contains IPAddr of MR.
*/

void rrci_cx_establish_resp(unsigned int Trans_id,        //!< Transaction ID
			    unsigned char *L3_info,       //!< Optional L3 Information
			    L3_INFO_T L3_info_t         //!< Type of L3 Information
			    );

/*
\brief Ask RRC to modify the QoS/Measurements of a radio bearer
 */
void rrm_rb_modify_req(LCHAN_DESC *Lchan_desc,       //!< Logical Channel Descriptor Array
		       MAC_RLC_MEAS_DESC *Mac_meas_desc, //!< MAC/RLC Measurement descriptors for RB 
		       RB_ID Rb_id,                  //!< Radio Bearer ID
		       unsigned int Trans_id       //!< Transaction ID
		       );
/**
\brief RRC response to rb_modify_req
 */
void rrc_rb_modify_resp(unsigned int Trans_id       //!< Transaction ID
			);

/**
\brief RRC confirmation of rb_modify_req
 */
void rrc_rb_modify_cfm(RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
		       unsigned int Trans_id       //!< Transaction ID
		       );
/**
\brief Ask RRC to release a radio bearer
 */
void rrm_rb_release_req(RB_ID Rb_id,                  //!< Radio Bearer ID
			unsigned int Trans_id       //!< Transaction ID
			);
/**
\brief RRC response to rb_release_req
 */
void rrc_rb_release_resp(unsigned int Trans_id       //!< Transaction ID
			 );
/**
\brief RRC measurement indication 
 */
void rrc_rb_meas_ind(RB_ID Rb_id,                     //!< Radio Bearer ID
		     L2_ID L2_id,                     //!< Layer 2 (MAC) IDs for link
		     MEAS_MODE Meas_mode,             //!< Measurement mode (periodic or event-driven)
		     MAC_RLC_MEAS_T Mac_rlc_meas_t,   //!< MAC/RLC measurements
		     unsigned int Trans_id            //!< Transaction ID
		     );

/**
\brief RRM response to rb_meas_ind
 */
void rrm_rb_meas_resp(unsigned int Trans_id       //!< Transaction ID
		      );
/**
\brief Configure a sensing measurement
 */
void rrm_sensing_meas_req(L2_ID L2_id,                           //!< Layer 2 (MAC) ID
			  SENSING_MEAS_DESC Sensing_meas_desc,   //!< Sensing Measurement Descriptor
			  unsigned int Trans_id                //!< Transaction ID
			  );

/**
\brief RRC response to sensing_meas_req
 */
void rrc_sensing_meas_resp(unsigned int Trans_id       //!< Transaction ID
			   );

/**
\brief RRC sensing measurement indication 
 */
void rrc_sensing_meas_ind(L2_ID L2_id,                        //!< Layer 2 ID (MAC) of sensing node
			  unsigned int NB_meas,               //!< Layer 2 ID (MAC) of sensing node
			  SENSING_MEAS_T *Sensing_meas,       //!< Sensing Information
			  unsigned int Trans_id             //!< Transaction ID
			  );
/**
\brief RRM response to sensing_meas_resp
 */
void rrm_sensing_meas_resp(unsigned int Trans_id);       //!< Transaction ID


/**
\brief Clusterhead PHY-Synch Indication
 */
void rrc_phy_synch_to_CH_ind(unsigned int Ch_index       //!< Clusterhead index
			     );
/**
\brief Mesh router PHY-Synch Indication
 */
void rrc_phy_synch_to_MR_ind(void);        

/**
\brief Clusterhead PHY-Out-of-Synch Indication
 */
void rrc_phy_out_of_synch_CH_ind(unsigned int Ch_index       //!< Clusterhead Index
);
/**
\brief MR loss indication
 */
void rrc_MR_loss_ind(L2_ID L2_id       //!< Layer 2 (MAC) ID
		     );
/**
\brief Release all resources for MR
 */

void rrm_MR_release_all(L2_ID L2_id       //!< Layer 2 (MAC) ID
			);
/**
\brief MR attachement indication. Sent by RRC to RRM to indicate the MAC ID of a new MR attached to CH at layer 2 
 */

void rrc_MR_attach_ind(L2_id L2_id
		       );



/** @} */

