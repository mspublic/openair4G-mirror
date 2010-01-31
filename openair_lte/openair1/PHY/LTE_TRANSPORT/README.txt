This directory contains functions related to physical/transport channel processing:

defs.h             : Function prototypes and data structures for DLSCH/ULSCH/DCI/UCI  processing
vars.h             : Some global variables (cqi format tables and SINR thresholds)
dlsch.c            : This contains routines pertaining to the transmission of the dlsch
dlsch_rx.c         : This contains routines pertaining to the inner MODEM for the dlsch (up to LLR generation)
dlsch_coding.c     : This contains routines pertaining to the channel coding procedures for dlsch
dlsch_decoding.c   : This contains routines for channel decoding/deinterleaving/rate (de)matching of dlsch
dlsch_modulation.c : This contains routines for the modulation procedures for dlsch 
pilots.c           : This contains routines pertaining to the generation of cell-specific pilots
defs.h             : This contains function prototypes (and Doxygen docs), definitions and structures pertaining 
                     to physical/transport channel processing
dci.h              : Data structures for LTE dci formats 0,1,1A,1B,1C,1D,2,2A,2B,3
dci.c              : Top-level encoding/decoding routines for DCI
ulsch_coding.c     : Top-level routines for ulsch encoding
  

