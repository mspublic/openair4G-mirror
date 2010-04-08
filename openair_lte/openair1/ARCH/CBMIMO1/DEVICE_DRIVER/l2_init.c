void l2_init() {

  int ret;
  int i,j;

  msg("[MAIN]MAC_INIT_GLOBAL_PARAM IN...\n");
    //    NB_NODE=2; 
    //    NB_INST=2;

  NB_CH_INST=1;
  //    NODE_ID[0]=0;
  NB_UE_INST=1;
  
  mac_init_global_param(); 
  
  
  mac_xface->macphy_init=mac_top_init;
  msg("[MAIN]MAC_INIT IN...\n");
  ret = mac_init();
  
  if (ret >= 0) {
    printf("Initialized MAC variables\n");
    
    //    last_slot = SLOTS_PER_FRAME-1;
    mac_xface->macphy_scheduler = macphy_scheduler;
    printf("Initialized MAC SCHEDULER\n");
    
    msg("ALL INIT OK\n");
    
    
    //Allocate memory for MAC/PHY communication primitives
    //NB_REQ_MAX = 16;
    for(i=0;i<NB_INST;i++){
      Macphy_req_table[i].Macphy_req_table_entry
	= (MACPHY_DATA_REQ_TABLE_ENTRY *)malloc16(NB_REQ_MAX*sizeof(MACPHY_DATA_REQ_TABLE_ENTRY));
      clear_macphy_data_req(i);
      
    }

    
    mac_xface->macphy_exit=exit;
    
    
    
    mac_xface->slots_per_frame = 20;//SLOTS_PER_FRAME;
    mac_xface->frame=0;
    
    mac_xface->macphy_init();
    Mac_rlc_xface->Is_cluster_head[0] = 1;
    Mac_rlc_xface->Is_cluster_head[1] = 0;
  }
}
