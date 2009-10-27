
#ifndef RF_SWITCH_H
#define RF_SWITCH_H

/* Verbose levels */
#define VERBOSE_LEVEL_VALUES       1
#define VERBOSE_LEVEL_IO           2

struct struct_grpci_ctrl {
  unsigned int RFswitches_onoff;
  unsigned int RFswitches_mask;
};

#endif /* RF_SWITCH_H */
