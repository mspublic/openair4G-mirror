
uint8_t do_SIB1(uint8_t *buffer,
		SystemInformationBlockType1_t *sib1);

uint8_t do_SIB23(uint8_t *buffer,
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3);

uint8_t do_RRCConnectionRequest(uint8_t *buffer,u8 *rv);

uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer);

uint8_t do_RRCConnectionReconfigurationComplete(uint8_t *buffer);

uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB1_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);

uint8_t do_RRCConnectionReconfiguration(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB2_config,
			      struct DRB_ToAddMod **DRB_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);




