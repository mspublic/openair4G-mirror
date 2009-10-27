
#ifndef SET_TX_H
#define SET_TX_H

/* Verbose levels */
#define VERBOSE_LEVEL_VALUES       1
#define VERBOSE_LEVEL_IO           2

/* Allowed range for gains 1 and 2 */
#define SETTX_GAIN1_MIN        0
#define SETTX_GAIN1_MAX        63
#define SETTX_GAIN2_MIN        0
#define SETTX_GAIN2_MAX        63

struct struct_grpci_ctrl {
  unsigned int settx_raw_word;
  unsigned int settx_gain1;
  unsigned int settx_gain2;
  unsigned int settx_switches_onoff;
};

#endif /* SET_TX_H */
