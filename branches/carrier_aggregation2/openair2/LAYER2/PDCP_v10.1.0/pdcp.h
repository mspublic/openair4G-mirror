/*
                                 pdcp.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#ifndef __PDCP_H__
#    define __PDCP_H__
//-----------------------------------------------------------------------------
#    ifdef PDCP_C
#        define private_pdcp(x) x
#        define protected_pdcp(x) x
#        define public_pdcp(x) x
#    else
#        define private_pdcp(x)
#        define public_pdcp(x) extern x
#        ifdef PDCP_FIFO_C
#            define protected_pdcp(x) extern x
#        else
#            define protected_pdcp(x)
#        endif
#    endif

#    ifdef PDCP_FIFO_C
#        define private_pdcp_fifo(x) x
#        define protected_pdcp_fifo(x) x
#        define public_pdcp_fifo(x) x
#    else
#        define private_pdcp_fifo(x)
#        define public_pdcp_fifo(x) extern x
#        ifdef PDCP_C
#            define protected_pdcp_fifo(x) extern x
#        else
#            define protected_pdcp_fifo(x)
#        endif
#    endif
//-----------------------------------------------------------------------------
#ifndef NON_ACCESS_STRATUM
  #include "UTIL/MEM/mem_block.h"
  #include "UTIL/LISTS/list.h"
  #include "COMMON/mac_rrc_primitives.h"
#endif //NON_ACCESS_STRATUM
//-----------------------------------------------------------------------------

#define TRUE 0x01
#define FALSE 0x00
typedef unsigned char BOOL;

public_pdcp(unsigned int Pdcp_stats_tx[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_tx_bytes[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_tx_bytes_last[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_tx_rate[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_rx[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_rx_bytes[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_rx_bytes_last[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);
public_pdcp(unsigned int Pdcp_stats_rx_rate[NB_MODULES_MAX][NB_CNX_CH][NB_RAB_MAX]);

typedef struct pdcp_t {
  BOOL header_compression_active;
  u8 seq_num_size;

  /*
   * Sequence number state variables
   *
   * TX and RX window
   */
  u16  next_pdcp_tx_sn;
  u16  next_pdcp_rx_sn;
  /*
   * TX and RX Hyper Frame Numbers
   */
  u16  tx_hfn;
  u16  rx_hfn;
  /*
   * SN of the last PDCP SDU delivered to upper layers
   */
  u16  last_submitted_pdcp_rx_sn;

  /*
   * Following array is used as a bitmap holding missing sequence
   * numbers to generate a PDCP Control PDU for PDCP status
   * report (see 6.2.6)
   */
  u8 missing_pdu_bitmap[512];
  /*
   * This is intentionally signed since we need a 'NULL' value
   * which is not also a valid sequence number
   */
  short int first_missing_pdu;
} pdcp_t;

/*
 * Following symbolic constant alters the behaviour of PDCP
 * and makes it linked to PDCP test code under targets/TEST/PDCP/
 *
 * For the version at SVN repository this should be UNDEFINED!
 * XXX And later this should be configured through the Makefile
 * under targets/TEST/PDCP/
 */
#undef PDCP_UNIT_TEST

#ifdef PDCP_UNIT_TEST
/*! \fn BOOL pdcp_data_req(module_id_t, u32_t, u8_t, rb_id_t, sdu_size_t, unsigned char*, pdcp_t*, list_t*)
* \brief This functions handles data transfer requests coming from test code (see below for actual code)
* \param[in] module_id Module ID
* \param[in] frame Frame number
* \param[in] Shows if relevant PDCP entity is part of an eNB or a UE
* \param[in] rab_id Radio Bearer ID
* \param[in] sdu_buffer_size Size of incoming SDU in bytes
* \param[in] sdu_buffer Buffer carrying SDU
* \param[in] test_pdcp_entity PDCP entity used for testing purposes
* \param[in] test_list list_t used for testing purposes
* \param[out] test_list Newly created PDU is enqueued into the list
* \param[out] test_pdcp PDCP sequence numbering state is updated
* \return TRUE on success, FALSE otherwise
* \note None
* @ingroup _pdcp
*/
public_pdcp(BOOL pdcp_data_req (module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                                unsigned char* sdu_buffer, pdcp_t* test_pdcp_entity, list_t* test_list);)
/*! \fn BOOL pdcp_data_ind(module_id_t, u32_t, u8_t, rb_id_t, sdu_size_t, unsigned char*)
* \brief This functions handles data transfer indications coming from test code (see below for actual code)
* \param[in] module_id Module ID
* \param[in] frame Frame number
* \param[in] Shows if relevant PDCP entity is part of an eNB or a UE
* \param[in] rab_id Radio Bearer ID
* \param[in] sdu_buffer_size Size of incoming SDU in bytes
* \param[in] sdu_buffer Buffer carrying SDU
* \param[in] test_list Incoming PDU is received/dequeued from this list
* \param[in] test_pdcp_entity PDCP entity used for testing purposes
* \return TRUE on success, FALSE otherwise
* \note None
* @ingroup _pdcp
*/
public_pdcp(BOOL pdcp_data_ind (module_id_t module_id, u32_t frame, u8_t eNB_flag,rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                                mem_block_t* sdu_buffer, pdcp_t* test_pdcp_entity, list_t* test_list);)
#else
/*! \fn BOOL pdcp_data_req(module_id_t, u32_t, u8_t, rb_id_t, sdu_size_t, unsigned char*)
* \brief This functions handles data transfer requests coming either from RRC or from IP
* \param[in] module_id Module ID
* \param[in] frame Frame number
* \param[in] Shows if relevant PDCP entity is part of an eNB or a UE
* \param[in] rab_id Radio Bearer ID
* \param[in] sdu_buffer_size Size of incoming SDU in bytes
* \param[in] sdu_buffer Buffer carrying SDU
* \return TRUE on success, FALSE otherwise
* \note None
* @ingroup _pdcp
*/
public_pdcp(BOOL pdcp_data_req (module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                                unsigned char* sdu_buffer);)
/*! \fn BOOL pdcp_data_ind(module_id_t, u32_t, u8_t, rb_id_t, sdu_size_t, unsigned char*)
* \brief This functions handles data transfer indications coming from RLC
* \param[in] module_id Module ID
* \param[in] frame Frame number
* \param[in] Shows if relevant PDCP entity is part of an eNB or a UE
* \param[in] rab_id Radio Bearer ID
* \param[in] sdu_buffer_size Size of incoming SDU in bytes
* \param[in] sdu_buffer Buffer carrying SDU
* \return TRUE on success, FALSE otherwise
* \note None
* @ingroup _pdcp
*/
public_pdcp(BOOL pdcp_data_ind (module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                                mem_block_t* sdu_buffer);)
#endif // PDCP_UNIT_TEST

/*! \fn void pdcp_config_req(module_id_t, rb_id_t)
* \brief This functions initializes relevant PDCP entity
* \param[in] module_id Module ID of relevant PDCP entity
* \param[in] rab_id Radio Bearer ID of relevant PDCP entity
* \return none
* \note None
* @ingroup _pdcp
*/
public_pdcp(void pdcp_config_req     (module_id_t, rb_id_t);)
/*! \fn void pdcp_config_release(module_id_t, rb_id_t)
* \brief This functions is unused
* \param[in] module_id Module ID of relevant PDCP entity
* \param[in] rab_id Radio Bearer ID of relevant PDCP entity
* \return none
* \note None
* @ingroup _pdcp
*/
public_pdcp(void pdcp_config_release (module_id_t, rb_id_t);)
/*! \fn void pdcp_run(u32_t, u8_t)
* \brief Runs PDCP entity to let it handle incoming/outgoing SDUs
* \param[in] frame Frame number
* \param[in] eNB_flag Indicates if this PDCP entity belongs to an eNB or to a UE
* \return none
* \note None
* @ingroup _pdcp
*/
public_pdcp(void pdcp_run (u32_t frame, u8_t eNB_flag, u8 UE_index,u8 eNB_index);)
public_pdcp(int pdcp_module_init ();)
public_pdcp(void pdcp_module_cleanup ();)
public_pdcp(void pdcp_layer_init ();)
public_pdcp(void pdcp_layer_cleanup ();)

#define PDCP2NAS_FIFO 21
#define NAS2PDCP_FIFO 22

protected_pdcp_fifo(int pdcp_fifo_flush_sdus (u32_t,u8_t);)
protected_pdcp_fifo(int pdcp_fifo_read_input_sdus_remaining_bytes (u32_t,u8_t);)
protected_pdcp_fifo(int pdcp_fifo_read_input_sdus(u32_t,u8_t);)
//-----------------------------------------------------------------------------

/*
 * Following two types are utilized between NAS driver and PDCP
 */
typedef struct pdcp_data_req_header_t {
  rb_id_t             rb_id;
  sdu_size_t           data_size;
  int       inst;
} pdcp_data_req_header_t;
typedef struct pdcp_data_ind_header_t {
  rb_id_t             rb_id;
  sdu_size_t           data_size;
  int       inst;
} pdcp_data_ind_header_t;

#if 0
/*
 * Missing PDU information struct, a copy of this will be enqueued 
 * into pdcp.missing_pdus for every missing PDU
 */
typedef struct pdcp_missing_pdu_info_t {
  u16 sequence_number;
} pdcp_missing_pdu_info_t;
#endif

/*
 * PDCP limit values
 */
#define PDCP_MAX_SDU_SIZE 8188 // octets, see 4.3.1 Services provided to upper layers
#define PDCP_MAX_SN_5BIT  31   // 2^5-1
#define PDCP_MAX_SN_7BIT  127  // 2^7-1
#define PDCP_MAX_SN_12BIT 4095 // 2^12-1

protected_pdcp(signed int             pdcp_2_nas_irq;)
protected_pdcp(pdcp_t                 pdcp_array[MAX_MODULES][MAX_RAB];)
protected_pdcp(sdu_size_t             pdcp_output_sdu_bytes_to_write;)
protected_pdcp(sdu_size_t             pdcp_output_header_bytes_to_write;)
protected_pdcp(list_t                 pdcp_sdu_list;)
protected_pdcp(int                    pdcp_sent_a_sdu;)
protected_pdcp(pdcp_data_req_header_t pdcp_input_header;)
protected_pdcp(unsigned char          pdcp_input_sdu_buffer[MAX_IP_PACKET_SIZE];)
protected_pdcp(sdu_size_t             pdcp_input_index_header;)
protected_pdcp(sdu_size_t             pdcp_input_sdu_size_read;)
protected_pdcp(sdu_size_t             pdcp_input_sdu_remaining_size_to_read;)

#endif
