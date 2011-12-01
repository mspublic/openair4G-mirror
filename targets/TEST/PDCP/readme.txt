
Attention! Test code in this directory requires the PDCP_UNIT_TEST symbolic
constant to be defined in pdcp.h since this definition alters the behaviour 
of PDCP by isolation it from the rest of the system (from RLC, etc.). Otherwise
you'll get a compilation error caused by the unmatched interface of pdcp_data_req()

To compile,

$ make

To run,

$ ./test_pdcp

Baris

