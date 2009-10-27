PHY/CODING

This directory contains the transport channel coding/decoding functions for both OpenAirInterface and a subset of LTE/WiMAX/WIFI MODEMS.

Files:

crc_byte.c               : 3GPP CRC routines (8,16,24 bit)
ccoding_byte.c           : WIFI/WIMAX rate 1/2 convolutional code
logmap8.c                : 3GPP/LTE turbo decoder (non-optimized)
logmap8_sse2.c           : 3GPP/LTE turbo decoder (optimized for SSE2/SSE3)
lte_interleaver.h        : 3GPP/LTE interleaver coefficient table
lte_interleaver_inline.h : 3GPP/LTE Interleaver inline functions
lte_tf.m                 : 3GPP/LTE transport format size calculator
rate_matching.c          : OpenAirInterface pseudo-random rate matching
viterbi.c                : WIFI/WIMAX rate 1/2 Viterbi decoder (non-optimized and optimized versions)

Directories:

TESTBENCH                : Testbench for channel coding/decoding