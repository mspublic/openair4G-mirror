Compiling
---------

To generate the testbench use 

	# make ltetest

Environment Variables
---------------------

To run this testbench you first need to create the following variables:

	# export OPENAIR0_DIR = path to OPENAIR0 top-level directory
	# export OPENAIR1_DIR = path to OPENAIR1 top-level directory

Running
-------

ltetest with no variables does the test for a PDU size of 40 bytes, 4-bit LLR quantization and rate .333 (no puncturing)

Otherwise

ltetest .4 21 6 does rate .4, 21 byte PDU and LLR quantization 6 bits.
