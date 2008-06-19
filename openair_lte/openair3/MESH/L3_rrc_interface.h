/** @defgroup _mesh_layer3_ OpenAirInterface MESH L2/L3 Interfaces 

*/


/*@{
\brief Ask RRC to establish a radio bearer
 */
void rrm_rb_establish_req(LCHAN_DESC *Lchan_desc,       //!< Logical Channel Descriptor Array
			  MAC_MEAS_DESC *Mac_meas_desc, //!< MAC Measurement descriptors for RB 
			  RLC_MEAS_DESC *Rlc_meas_desc, //!< RLC Measurement descriptors for RB 
			  L2_ID *L2_id,                 //!< Layer 2 (MAC) IDs for link
			  unsigned int Trans_id,        //!< Transaction ID
			  unsigned char *L3_info,       //!< Optional L3 Information
			  L3_INFO_T L3_info_t         //!< Type of L3 Information
			  );			  
/*
\brief RRC response to rb_establish_req
 */
void rrc_rb_establish_resp(unsigned int Trans_id       //!< Transaction ID
			   );

/*
\brief RRC confirmation of rb_establish_req
 */
void rrc_rb_establish_cfm(RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
			  unsigned int Trans_id,        //!< Transaction ID
			  unsigned char *L3_info,       //!< Optional L3 Information
			  L3_INFO_T L3_info_t         //!< Type of L3 Information
			  );
/*
\brief Ask RRC to modify the QoS/Measurements of a radio bearer
 */
void rrm_rb_modify_req(LCHAN_DESC *Lchan_desc,       //!< Logical Channel Descriptor Array
		       MAC_MEAS_DESC *Mac_meas_desc, //!< MAC Measurement descriptors for RB 
		       RLC_MEAS_DESC *Rlc_meas_desc, //!< RLC Measurement descriptors for RB 
		       RB_ID Rb_id,                  //!< Radio Bearer ID
		       unsigned int Trans_id       //!< Transaction ID
		       );
/*
\brief RRC response to rb_modify_req
 */
void rrc_rb_modify_resp(unsigned int Trans_id       //!< Transaction ID
			);

/*
\brief RRC confirmation of rb_modify_req
 */
void rrc_rb_modify_cfm(RB_ID Rb_id,                  //!< Radio Bearer ID used by RRC
		       unsigned int Trans_id       //!< Transaction ID
		       );
/*
\brief Ask RRC to release a radio bearer
 */
void rrm_rb_release_req(RB_ID Rb_id,                  //!< Radio Bearer ID
			unsigned int Trans_id       //!< Transaction ID
			);
/*
\brief RRC response to rb_release_req
 */
void rrc_rb_release_resp(unsigned int Trans_id       //!< Transaction ID
			 );
/*
\brief RRC measurement indication 
 */
void rrc_rb_meas_ind(RB_ID Rb_id,                 //!< Radio Bearer ID
		     L2_ID L2_id,                 //!< Layer 2 (MAC) IDs for link
		     MEAS_MODE Meas_mode,         //!< Measurement mode (periodic or event-driven)
		     MAC_MEAS_T Mac_meas_t,       //!< MAC measurements
		     RLC_MEAS_T Rlc_meas_t,       //!< RLC measurements
		     unsigned int Trans_id      //!< Transaction ID
		     );

/*
\brief RRM response to rb_meas_ind
 */
void rrm_rb_meas_resp(unsigned int Trans_id       //!< Transaction ID
		      );
/*
\brief Configure a sensing measurement
 */
void rrm_sensing_meas_req(L2_ID L2_id,                           //!< Layer 2 (MAC) ID
			  SENSING_MEAS_DESC Sensing_meas_desc,   //!< Sensing Measurement Descriptor
			  unsigned int Trans_id                //!< Transaction ID
			  );

/*
\brief RRC response to sensing_meas_req
 */
void rrc_sensing_meas_resp(unsigned int Trans_id       //!< Transaction ID
			   );

/*
\brief RRC sensing measurement indication 
 */
void rrc_sensing_meas_ind(L2_ID L2_id,                        //!< Layer 2 ID (MAC) of sensing node
			  unsigned int NB_meas,               //!< Layer 2 ID (MAC) of sensing node
			  SENSING_MEAS_T *Sensing_meas,       //!< Sensing Information
			  unsigned int Trans_id             //!< Transaction ID
			  );
/*
\brief RRM response to sensing_meas_resp
 */
void rrm_sensing_meas_resp(unsigned int Trans_id);       //!< Transaction ID


/*
\brief Clusterhead PHY-Synch Indication
 */
void rrc_phy_synch_to_CH_ind(unsigned int Ch_index       //!< Clusterhead index
			     );
/*
\brief Mesh router PHY-Synch Indication
 */
void rrc_phy_synch_to_MR_ind(void);        

/*
\brief Clusterhead PHY-Out-of-Synch Indication
 */
void rrc_phy_out_of_synch_CH_ind(unsigned int Ch_index       //!< Clusterhead Index
);
/*
\brief MR loss indication
 */
void rrc_MR_loss_ind(L2_ID L2_id       //!< Layer 2 (MAC) ID
		     );
/*
\brief Release all resources for MR
 */
void rrm_MR_release_all(L2_ID L2_id       //!< Layer 2 (MAC) ID
			);
/* @} */
