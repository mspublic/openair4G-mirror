#ifndef __PHY_SPEC_DEFS_TOP_H__
#define __PHY_SPEC_DEFS_TOP_H__

#include "types.h"



/*! \brief Extension Type */
typedef enum {
   CYCLIC_PREFIX,
   CYCLIC_SUFFIX,
   ZEROS,
   NONE
 } Extension_t;


/*! \brief Transmit Signal Format */
typedef enum {
  OFDM=0,
  Digital_FDM
 } Signal_format_t;


/*! \brief  PHY Framing Structure 
*/

typedef struct PHY_FRAMING {
  u_long	 fc_khz;         /*!< \brief Carrier Frequency (kHz)*/
  u_long	 fs_khz;         /*!< \brief Sampling Frequency (kHz)*/
  u_short         Nsymb ;         /*!< \brief Number of OFDM Symbols per TTI */
  u_short        Nd;             /*!< \brief Number of OFDM Carriers */
  u_char         log2Nd;         /*!< \brief Log2 of Number of OFDM Carriers */
  u_short	 Nc;             /*!< \brief Number of Prefix Samples*/
  u_short	 Nz;             /*!< \brief Number of Zero Carriers*/
  u_char	 Nf;             /*!< \brief Number of Frequency Groups*/
  Extension_t    Extension_type; /*!< \brief Prefix method*/
} PHY_FRAMING;


#ifdef IFFT_FPGA

  #ifndef RAW_IFFT
  typedef unsigned char mod_sym_t;
  #else
  typedef int mod_sym_t;
  #endif //RAW_IFFT

#else
  #ifdef IFFT_FPGA_UE
    #ifndef RAW_IFFT
    typedef unsigned char mod_sym_t;
    #else
    typedef int mod_sym_t;
    #endif //RAW_IFFT
  #else
  typedef int mod_sym_t;
  #endif
#endif




#endif /*__PHY_SPEC_DEFS_TOP_H__ */ 















