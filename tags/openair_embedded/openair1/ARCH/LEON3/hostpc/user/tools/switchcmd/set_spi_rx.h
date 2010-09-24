
#ifndef SET_RX_H
#define SET_RX_H

/* Verbose levels */
#define VERBOSE_LEVEL_VALUES       1
#define VERBOSE_LEVEL_IO           2

/* Allowed range for gains 1 and 2 */
#define SETRX_GAIN1_MIN        0
#define SETRX_GAIN1_MAX        63
#define SETRX_GAIN2_MIN        0
#define SETRX_GAIN2_MAX        63

struct struct_grpci_ctrl {
  unsigned int setrx_raw_word;
  unsigned int setrx_gain1;
  unsigned int setrx_gain2;
  unsigned int setrx_switches_onoff;
};

#endif /* SET_RX_H */
