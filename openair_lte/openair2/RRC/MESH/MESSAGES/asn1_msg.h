
uint8_t do_SIB1(uint8_t *buffer,
		SystemInformationBlockType1_t *sib1);

uint8_t do_SIB23(uint8_t *buffer,
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3);
