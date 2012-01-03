
Attention! Test code in this directory requires the PDCP_UNIT_TEST symbolic
constant to be defined in pdcp.h since this definition alters the behaviour 
of PDCP by isolation it from the rest of the system (from RLC, etc.). Otherwise
you'll get a compilation error caused by the unmatched interface of pdcp_data_req()

To configure, see following symbolic constants in test_pdcp.c,

#define NUMBER_OF_TEST_PACKETS 10000
#define WINDOW_SIZE 4096       // 12-bit SN
#define TEST_RX_AND_TX_WINDOW 1
#define TEST_PDCP_DATA_REQUEST_AND_INDICATION 1

To compile,

$ make

To run,

$ ./test_pdcp


baris.demiray@eurecom.fr

