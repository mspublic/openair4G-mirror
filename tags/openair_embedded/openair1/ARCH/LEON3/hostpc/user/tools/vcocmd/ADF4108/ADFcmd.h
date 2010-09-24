
/* References      [ADF4108]     Component datasheet, Analog Devices
                                 PLL Frequency Synthesizer, ADF4108 */

#ifndef ADFCMD_H
#define ADFCMD_H

/* Reference Input Frequence (see [ADF4108] p3) */
#define F_REFIN_DEFAULT_MHZ   10      // or 20 ?
#define F_REFIN_MIN_MHZ       10      // or 20 ?
#define F_REFIN_MAX_MHZ       250

/* Prescaler constraint (see [ADF4108] p9) */
#define PRESC_OUT_MAX_KHZ     300000  /* 300 MHz */

/* Default values for the Function Register */
#define FUNC_PWDN2            0x0
#define FUNC_CUR_SET_2        0x001c0000
#define FUNC_CUR_SET_1        0x00038000
#define FUNC_TIMER_CNT_CTRL   0x0
#define FUNC_FAST_LOCK_MODE   0x0
#define FUNC_FAST_LOCK_EN     0x0
#define FUNC_CP_TRISTATE      0x0
#define FUNC_PD_POLARITY      0x80
#define FUNC_MUXOUT_CTRL      0x0
#define FUNC_PWDN1            0x0
#define FUNC_CNT_RST          0x0
#define FUNC_CTRL_BITS        0x2
#define FUNC_DEFAULT_BITS_21_DWTO_0    ( FUNC_PWDN2            \
                                       | FUNC_CUR_SET_2        \
                                       | FUNC_CUR_SET_1        \
                                       | FUNC_TIMER_CNT_CTRL   \
                                       | FUNC_FAST_LOCK_MODE   \
                                       | FUNC_FAST_LOCK_EN     \
                                       | FUNC_CP_TRISTATE      \
                                       | FUNC_PD_POLARITY      \
                                       | FUNC_MUXOUT_CTRL      \
                                       | FUNC_PWDN1            \
                                       | FUNC_CNT_RST          \
                                       | FUNC_CTRL_BITS)
#define FUNC_SET_CNT_RST      0x4
#define FUNC_UNSET_CNT_RST    0x0

/* Default values for the Reference Counter Register */
#define REFCNT_RESERVED            0x0
#define REFCNT_LOCK_TCT_PREC       0x0
#define REFCNT_TEST_MODE_BITS      0x0
#define REFCNT_ANTI_BKLSH_WIDTH    0x0
#define REFCNT_CTRL_BITS           0x0
#define REFCNT_DEFAULT_BITS_23_DWTO_16_1_0     ( REFCNT_RESERVED \
                                               | REFCNT_LOCK_TCT_PREC \
                                               | REFCNT_TEST_MODE_BITS \
                                               | REFCNT_ANTI_BKLSH_WIDTH \
                                               | REFCNT_CTRL_BITS)

/* Default values for the AB Counters (aka N) Register */
#define ABCNT_RESERVED     0x0
//#define ABCNT_CP_GAIN      0x00200000
#define ABCNT_CP_GAIN      0x00000000
#define ABCNT_CTRL_BITS    0x1
#define ABCNT_DEFAULT_BITS_23_DWTO_21_1_0      ( ABCNT_RESERVED \
                                               | ABCNT_CP_GAIN \
                                               | ABCNT_CTRL_BITS \
                                               | ABCNT_CTRL_BITS)

/* Authorized range for A Counter */
#define A_CNT_MIN   0
#define A_CNT_MAX   63

/* Authorized range for B Counter */
#define B_CNT_MIN   3
#define B_CNT_MAX   8191

/* Authorized range for Reference Counter */
#define REF_CNT_MIN   1
#define REF_CNT_MAX   16383

/* Verbose levels */
#define VERBOSE_LEVEL_VALUES       1
#define VERBOSE_LEVEL_RAW_VALUES   2
#define VERBOSE_LEVEL_IO           3

struct struct_grpci_ctrl {
  unsigned int func0;
  unsigned int rcnt;
  unsigned int abcnt;
  unsigned int func1;
  unsigned int init;
};

#endif /* ADFCMD_H */
