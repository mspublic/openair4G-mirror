PHY/CODING/TESTBENCH

This directory contains a testbench for performance evalulation (antipodal modulation) of the various channel decoding algorithms.

Files:

Makefile                 : Makefile for generating the two testbenches
ltetest.c                : Testbench for LTE Turbo code and WIFI/WIMAX 
			   convolutional code with rate matching.  Compares the
			   performance of the two for equal rates and variable
			   transport block sizes
viterbi.c		 : Testbench for Viterbi decoder

Directories:

TESTBENCH                : Testbench for channel coding/decoding