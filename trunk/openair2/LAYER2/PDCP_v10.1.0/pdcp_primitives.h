/*
                             pdcp_primitives.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#ifndef __PDCP_PRIMITIVES_H__
#    define __PDCP_PRIMITIVES_H__

#    include "platform.h"
//----------------------------------------------------------
// primitives
//----------------------------------------------------------
#    define PDCP_DATA_REQ     0x01
#    define PDCP_DATA_IND     0x02
//----------------------------------------------------------
// control primitives
//----------------------------------------------------------
#    define CPDCP_CONFIG_REQ  0x04
#    define CPDCP_RELEASE_REQ 0x08
#    define CPDCP_SN_REQ      0x10
#    define CPDCP_RELOC_REQ   0x20
#    define CPDCP_RELOC_CNF   0x40
//----------------------------------------------------------
// primitives definition
//----------------------------------------------------------
struct pdcp_data_req {
  u16             rb_id;
  u16             data_size;
};
struct pdcp_data_ind {
  u16             rb_id;
  u16             data_size;
};

//----------------------------------------------------------
// control primitives definition
//----------------------------------------------------------
// TO DO
struct cpdcp_config_req {
  void           *rlc_sap;
  u8              rlc_type_sap; // am, um, tr
  u8              header_compression_type;
};
struct cpdcp_release_req {
  void           *rlc_sap;
};

struct cpdcp_sn_req {
  u32             sn;
};

struct cpdcp_relloc_req {
  u32             receive_sn;
};

struct cpdcp_relloc_conf {
  u32             receive_sn;
  u32             send_sn;
};

struct cpdcp_primitive {
  u8              type;
  union {
    struct cpdcp_config_req config_req;
    struct cpdcp_release_req release_req;
    struct cpdcp_sn_req sn_req;
    struct cpdcp_relloc_req relloc_req;
    struct cpdcp_relloc_conf relloc_conf;
  } primitive;
};

/*
 * 3GPP TS 36.323 V10.1.0 (2011-03)
 */

// Data or control (1-bit, see 6.3.7)
#define PDCP_CONTROL_PDU 0x01
#define PDCP_DATA_PDU 0x00
// PDU-type (3-bit, see 6.3.8)
#define PDCP_STATUS_REPORT 0x00
#define INTERSPERSED_ROHC_FEEDBACK_PACKET 0x01

/*
 * 6.1 Protocol Data Units
 * 6.2.2 Control Plane PDCP Data PDU
 */
typedef struct {
  u8 sn;      // PDCP sequence number will wrap around 2^5-1 so 
              // reserved field is unnecessary here
  u32 mac-i;  // Integration protection is not implemented (pad with 0)
} pdcp_control_plane_data_pdu_header;

/*
 * 6.2.3 User Plane PDCP Data PDU with long PDCP SN (12-bit)
 */
#define PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE 2
typedef struct {
  u8 dc;      // Data or control (see 6.3.7)
  u16 sn;     // 12-bit sequence number
} pdcp_user_plane_data_pdu_header_with_long_sn;

/*
 * 6.2.4 User Plane PDCP Data PDU with short PDCP SN (7-bit)
 */
#define PDCP_USER_PLANE_DATA_PDU_SHORT_SN_HEADER_SIZE 1
typedef struct {
  u8 dc;
  u8 sn;      // 7-bit sequence number
} pdcp_user_plane_data_pdu_header_with_short_sn;

/*
 * 6.2.5 PDCP Control PDU for interspersed ROHC feedback packet
 */
#define PDCP_CONTROL_PDU_INTERSPERSED_ROHC_FEEDBACK_HEADER_SIZE 1
typedef struct {
  u8 dc;
  u8 pdu_type; // PDU type (see 6.3.8)
} pdcp_control_pdu_for_interspersed_rohc_feedback_packet_header;

#endif
