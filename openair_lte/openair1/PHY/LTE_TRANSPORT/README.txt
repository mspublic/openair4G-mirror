This directory contains functions related to physical/transport channel processing:

dlsch.c          : This contains routines pertaining to the transmission of the dlsch
dlsch_rx.c       : This contains routines pertaining to the inner MODEM for the dlsch (up to LLR generation)
dlsch_decoding.c : This contains routines for channel decoding/deinterleaving/rate (de)matching of dlsch
pilots.c         : This contains routines pertaining to the generation of cell-specific pilots
defs.h           : This contains function prototypes (and Doxygen docs), definitions and structures pertaining 
                   to physical/transport channel processing
