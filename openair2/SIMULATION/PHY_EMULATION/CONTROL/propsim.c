
// compute channels for link between node Src_id and the target node, every frame
// ID

void propsim(unsigned short CH_id,unsigned char UE_id) {


  // Compute PATH_LOSS_dB (path loss + shadowing)
  propsim_p2p_channel_desc[CH_id][UE_id].PATH_LOSS_dB = -70;  // get_path_loss(CH_id,UE_id) 

  // Get TX_POWER_dBm
  propsim_p2p_channel_desc[CH_id][UE_id].TX_POWER_CH_dBm = 30;  // get_tx_power(CH_id,UE_id) 
  propsim_p2p_channel_desc[CH_id][UE_id].TX_POWER_UE_dBm = 23;  // get_tx_power(CH_id,UE_id) 

  //  Fill the PROPSIM_P2P_CHANNEL_DESC, 
  //  ID

  // generate random correlated frequency samples

  //	channel_desc[CH_id][UE_id].freq_response_CH_to_UE[a][txa][rxa].re 
  //	channel_desc[CH_id][UE_id].freq_response_CH_to_UE[a][txa][rxa].im 

  // if not reciprocal

  //	channel_desc[CH_id][UE_id].freq_response_UE_to_CH[a][txa][rxa].re 
  //	channel_desc[CH_id][UE_id].freq_response_UE_to_CH[a][txa][rxa].im 
}
