/*
 * PDCP test code
 *
 * Author: Baris Demiray <baris.demiray@eurecom.fr>
 */

#include "MAC_INTERFACE/vars.h"
#include "LAYER2/MAC/vars.h"
#include "OCG_vars.h"
#include "test_util.h"
#include "test_pdcp.h"

// PDCP entities
pdcp_t pdcp_array[2];

/*
 * This is used by a number of methods to determine how 
 * many times a packet should be generated/a test run
 */
#define NUMBER_OF_TEST_PACKETS 1000

/*
 * Test configuration is STATEFUL. If you want to run DATA_REQUEST after TX_WINDOW 
 * then you have to reset/reinitialize PDCP entity state!
 * 
 *         TEST_TX_WINDOW Tests TX window code by repetitively asking for new TX 
 *                        sequence numbers without generating any packets
 *
 *         TEST_RX_WINDOW Tests RX window code by supplying sequenced RX sequence
 *                        numbers and asking RX window code to validate, without
 *                        generating any packets
 *
 * TEST_PDCP_DATA_REQUEST Tests pdcp_data_req() method by repetitively asking it 
 *                        to create a PDCP PDU out of supplied SDU and parsing 
 *                        particular fields of relevant PDU lateron
 */
#define TEST_TX_WINDOW 0
#define TEST_RX_WINDOW 0
#define TEST_PDCP_DATA_REQUEST 1
#define TEST_PDCP_DATA_INDICATION 0

int main(int argc, char **argv) {
  unsigned char index = 0;
  unsigned char test_result = 0;

  pool_buffer_init();

  if (init_pdcp_entity(&pdcp_array[0]) == TRUE && init_pdcp_entity(&pdcp_array[1]) == TRUE)
    msg("[TEST] PDCP entity initialization OK\n");
  else {
    msg("[TEST] Cannot initialize PDCP entities!\n");
    return 1;
  }

  /* Initialize PDCP state variables */
  for (index = 0; index < 2; ++index) {
    if (pdcp_init_seq_numbers(&pdcp_array[index]) == FALSE) {
      msg("[TEST] Cannot initialize sequence numbers of PDCP entity %d!\n", index);
      exit(1);
    } else {
      msg("[TEST] Sequence number state of PDCP entity %d is initialized\n", index);
    }
  }

#if TEST_TX_WINDOW
  /* Test TX window */
  if (test_tx_window() == FALSE)
    test_result = 1;
#endif

#if TEST_RX_WINDOW
  /* Test RX window */
  if (test_rx_window() == FALSE)
    test_result = 1;
#endif

#if TEST_PDCP_DATA_REQUEST
  /* Test pdcp_data_req() method in pdcp.c */
  if (test_pdcp_data_req() == FALSE)
    test_result = 1;
#endif

#if TEST_PDCP_DATA_INDICATION
  /* Test pdcp_data_ind() method in pdcp.c */
  if (test_pdcp_data_ind() == FALSE)
    test_result = 1;
#endif

  if (test_result) {
    msg("\n\nOne or more tests failed!\n");
  } else {
    msg("\n\nAll tests are successfull!\n");
  }

  return test_result;
}

BOOL init_pdcp_entity(pdcp_t *pdcp_entity)
{
  if (pdcp_entity == NULL)
    return FALSE;

  /*
   * Initialize sequence number state variables of relevant PDCP entity
   */
  pdcp_entity->next_pdcp_tx_sn = 0;
  pdcp_entity->next_pdcp_rx_sn = 0;
  pdcp_entity->tx_hfn = 0;
  pdcp_entity->rx_hfn = 0;
  /* SN of the last PDCP SDU delivered to upper layers */
  pdcp_entity->last_submitted_pdcp_rx_sn = 4095;
  pdcp_entity->seq_num_size = 12;

  msg("PDCP entity is initialized: Next TX: %d, Next Rx: %d, TX HFN: %d, RX HFN: %d, " \
      "Last Submitted RX: %d, Sequence Number Size: %d\n", pdcp_entity->next_pdcp_tx_sn, \
      pdcp_entity->next_pdcp_rx_sn, pdcp_entity->tx_hfn, pdcp_entity->rx_hfn, \
      pdcp_entity->last_submitted_pdcp_rx_sn, pdcp_entity->seq_num_size);

  return TRUE;
}

BOOL test_tx_window()
{
  unsigned long index = 0;

  for (index = 0; index < NUMBER_OF_TEST_PACKETS; ++index) {
    u16 pseudo_tx_sn = pdcp_get_next_tx_seq_number(&pdcp_array[0]);
    if (pseudo_tx_sn == index % 4096)
      msg("TX packet # %07lu seq # %04d hfn # %04d\n", index, pseudo_tx_sn, pdcp_array[0].tx_hfn);
    else {
      msg("TX packet is out-of-window!\n");
      return FALSE;
    }
  }

  return TRUE;
}

BOOL test_rx_window()
{
  unsigned long index = 0;

  for (index = 0; index < NUMBER_OF_TEST_PACKETS; ++index) {
    u16 pseudo_rx_sn = (index == 0) ? 0 : index % 4096;
    if (pdcp_is_rx_seq_number_valid(pseudo_rx_sn, &pdcp_array[1]) == TRUE) {
      msg("RX packet # %07lu seq # %04d last-submitted # %04d hfn # %04d\n", \
          index, pdcp_array[1].next_pdcp_rx_sn, pdcp_array[1].last_submitted_pdcp_rx_sn, pdcp_array[1].rx_hfn);
    } else {
      msg("RX packet seq # %04lu is not valid!\n", index);
      return FALSE;
    }
  }

  return TRUE;
}

#define DUMMY_BUFFER_SIZE 10
BOOL test_pdcp_data_req()
{
  unsigned char dummy_buffer[DUMMY_BUFFER_SIZE] = "123456789";
  /* Following two will be filled in by pdcp_data_req() */
  unsigned char* pdcp_test_pdu_buffer = NULL;
  unsigned int pdcp_test_pdu_buffer_size = PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE + DUMMY_BUFFER_SIZE;
  unsigned int index = 0;

  /*
   * Create an unsigned char buffer out of mem_block_t
   */
  pdcp_test_pdu_buffer = (unsigned char*) calloc(1, pdcp_test_pdu_buffer_size);
  if (pdcp_test_pdu_buffer == NULL) {
    msg("Cannot allocate a buffer for test!\n");
    return FALSE;
  }
 
  for (index = 0; index < NUMBER_OF_TEST_PACKETS; ++index) {
    msg("\n\nAsking PDCP to send %d/%d SDU...\n", index+1, NUMBER_OF_TEST_PACKETS);

    /*
     * Reset test pdu buffer for every run
     */
    memset(pdcp_test_pdu_buffer, 0x00, pdcp_test_pdu_buffer_size);

    /*
     * Ask PDCP to create a PDU with given buffer
     */
    if (pdcp_data_req(dummy_buffer, 10, &pdcp_array[0], pdcp_test_pdu_buffer, &pdcp_test_pdu_buffer_size) == TRUE) {
      msg("[TEST] Starting to dissect PDU created by PDCP...\n");

      if (pdcp_test_pdu_buffer_size == 0 || pdcp_test_pdu_buffer == NULL) {
        msg("[TEST] PDU created by pdcp_data_req() is invalid!\n");
        return FALSE;
      }

      /*
       * Verify that this is a data packet by checking 
       * if the first bit is 0x00 (PDCP_DATA_PDU)
       */
      if (pdcp_test_pdu_buffer[0] & 0x80) {
        msg("[TEST] First bit is not 0, which means this is not a Data PDU!\n");
        return FALSE;
      } else {
        msg("[TEST] First bit is 0 so this is a Data PDU, OK\n");
      }

      /*
       * Verify that all three reserved bits are 0
       */
      if ((pdcp_test_pdu_buffer[0] & 0x70) != 0) {
        msg("[TEST] Reserved bits are not 0!\n");
        return FALSE;
      } else {
        msg("[TEST] Reserved bits are all 0, OK\n");
      }

      /*
       * Parse verify sequence number
       */
      u16 sequence_number = pdcp_get_sequence_number_of_pdu_with_long_sn(pdcp_test_pdu_buffer);
      msg("[TEST] Parsed sequence number is %04d\n", sequence_number);

      if (sequence_number != index) {
        msg("[TEST] Sequence numbers are out-of-order!\n");
        return FALSE;
      } else {
        msg("[TEST] Sequence number is correct\n");
      }

    } else {
      msg("[TEST] pdcp_data_req() returned FALSE!\n");
      return FALSE;
    }
  }

  return TRUE;
}

BOOL test_pdcp_data_ind()
{
  pdcp_data_ind(1, 1, 0, NULL);

  return TRUE;
}


