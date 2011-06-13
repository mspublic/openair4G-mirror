/** 
\brief Generate a default configuration for SIB1 (eNB).
@param buffer Pointer to PER-encoded ASN.1 description of SIB1
@param sib1 Pointer to asn1c C representation of SIB1
@return size of encoded bit stream in bytes*/

uint8_t do_SIB1(uint8_t *buffer,
		SystemInformationBlockType1_t *sib1);

/** 
\brief Generate a default configuration for SIB2/SIB3 in one System Information PDU (eNB).
@param buffer Pointer to PER-encoded ASN.1 description of SI PDU
@param systemInformation Pointer to asn1c C representation of SI PDU
@param sib2 Pointer (returned) to sib2 component withing SI PDU
@param sib3 Pointer (returned) to sib3 component withing SI PDU
@return size of encoded bit stream in bytes*/

uint8_t do_SIB23(uint8_t *buffer,
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3);

/** 
\brief Generate an RRCConnectionRequest UL-CCCH-Message (UE) based on random string or S-TMSI.  This 
routine only generates an mo-data establishment cause.
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@param rv 5 byte random string or S-TMSI
@returns Size of encoded bit stream in bytes*/

uint8_t do_RRCConnectionRequest(uint8_t *buffer,u8 *rv);

/** \brief Generate an RRCConnectionSetupComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer);

/** \brief Generate an RRCConnectionReconfigurationComplete UL-DCCH-Message (UE)
@param buffer Pointer to PER-encoded ASN.1 description of UL-DCCH-Message PDU
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionReconfigurationComplete(uint8_t *buffer);

/** 
\brief Generate an RRCConnectionSetup DL-CCCH-Message (eNB).  This routine configures SRB_ToAddMod (SRB1/SRB2) and 
PhysicalConfigDedicated IEs.  The latter does not enable periodic CQI reporting (PUCCH format 2/2a/2b) or SRS.
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param SRB1_config Pointer (returned) to SRB1_config IE for this UE
@param physicalConfigDedicated Pointer (returned) to PhysicalConfigDedicated IE for this UE
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB1_config,
			      struct SRB_ToAddMod **SRB2_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);

/** 
\brief Generate an RRCConnectionReconfiguration DL-DCCH-Message (eNB).  This routine configures SRBToAddMod (SRB2) and one DRBToAddMod 
(DRB3).  PhysicalConfigDedicated is not updated.
@param buffer Pointer to PER-encoded ASN.1 description of DL-CCCH-Message PDU
@param UE_id UE index for this message
@param Transaction_id Transaction_ID for this message
@param SRB2_config Pointer (returned) to SRB_ToAddMod IE for this UE
@param DRB_config Pointer (returned) to DRB_ToAddMod IE for this UE
@param physicalConfigDedicated Pointer (returned void) to PhysicalConfigDedicated IE for this UE
@returns Size of encoded bit stream in bytes*/
uint8_t do_RRCConnectionReconfiguration(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB2_config,
			      struct DRB_ToAddMod **DRB_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated);




