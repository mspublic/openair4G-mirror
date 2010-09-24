PHY/CODING

This directory contains the transport channel coding/decoding functions for both OpenAirInterface and a subset of LTE/WiMAX/WIFI MODEMS.

Files:

crc_byte.c                     : 3GPP CRC routines (8,16,24 bit, LTE - 36-212 and WCDMA 25-212)
ccoding_byte.c                 : WIFI/WIMAX rate 1/2 convolutional code
ccoding_byte_lte.c             : 3GPP LTE rate 1/3 tail-biting convolutional code with CRC embedding
3gpp_turbo_decoder.c           : 3GPP/LTE turbo decoder (non-optimized)
3gpplte_turbo_decoder_sse.c    : 3GPP/LTE turbo decoder (optimized for SSE2/SSE3)
lte_interleaver.h              : 3GPP/LTE interleaver coefficient table (from 3GPP 36-212)
lte_interleaver_inline.h       : 3GPP/LTE Interleaver inline functions (from 3GPP 36-212)
lte_tf.m                       : 3GPP/LTE transport format size calculator
rate_matching.c                : OpenAirInterface pseudo-random rate matching
viterbi.c                      : WIFI/WIMAX rate 1/2 Viterbi decoder (non-optimized and optimized versions)
viterbi_lte.c                  : 3GPP LTE Viterbi decoder (SIMD optimized)
lte_rate_matching.c            : 3GPP/LTE Rate (de)matching and sub-block (de)interleaving routines (strict compliance with 3GPP 36-212, Release 8, 2009-03)

Directories:

TESTBENCH                      : Testbench for channel coding/decoding
