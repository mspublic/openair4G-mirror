
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>

#include <sys/ioctl.h>
#include <cbmimo1_device.h>

#include "RFswitch.h"
#include "from_grlib_softregs.h"

/* Interface with Grpci driver */
#define DEVICE_NAME "/dev/openair0"
#define ACCESS_MODE O_RDONLY

/* Interface with getopt_long() libC function
 * 
 * On the use of getopt_long() function, see for instance the GNU C library
 * documentation, at:
 *   http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
 * This URL was available on Nov 2006. Quoting it:
 * 
 * o "When getopt_long encounters a short option, it does the same thing that
 *   getopt would do: it returns the character code for the option, and stores
 *   the options argument (if it has one) in optarg.
 * o When getopt_long encounters a long option, it takes actions based on the
 *   flag and val fields of the definition of that option.
 *     o If flag is a NULL pointer, then getopt_long returns the contents of
 *       val to indicate which option it found. You should arrange distinct
 *       values in the val field for options with different meanings, so you
 *       can decode these values after getopt_long returns. If the long option
 *       is equivalent to a short option, you can use the short option's
 *       character code in val.
 *     o If flag is NOT A NULL pointer, that means this option should just
 *       set a flag in the program. The flag is a variable of type int that
 *       you define. Put the address of the flag in the flag field. Put in
 *       the val field the value you would like this option to store in the
 *       flag. In this case, getopt_long returns 0."
 */
#define SWITCH_IS_OFF        0
#define SWITCH_IS_ON         1
static unsigned int helpflag   = SWITCH_IS_OFF;
static unsigned int pflag      = SWITCH_IS_OFF;
static unsigned int postedflag = SWITCH_IS_OFF;

#define SWITCH_LO_F     1
#define SWITCH_RX_TX    2
#define RESET_LFSW      3
#define SWITCH_TX3_1    4
#define POWERDOWN_ADF   5
#define SWITCH_TX3_2    6
#define SWITCH_TX3_3    7
#define SWITCH_LO1      8
#define SWITCH_LO2      9

static struct struct_grpci_ctrl grpci_ctrl;

#define LONGOPTIONS_NB 12
struct option SWRFcmd_longopts[LONGOPTIONS_NB+1] = {
  {  .name = "help",    .has_arg = no_argument,       .flag = &helpflag,   .val = SWITCH_IS_ON  },
  {  .name = "lo_f",      .has_arg = required_argument, .flag = NULL,      .val = SWITCH_LO_F   },
  {  .name = "rx_tx",      .has_arg = required_argument, .flag = NULL,     .val = SWITCH_RX_TX  },
  {  .name = "reset_lfsw",     .has_arg = required_argument, .flag = NULL, .val = RESET_LFSW    },
  {  .name = "tx3_1",    .has_arg = required_argument, .flag = NULL,       .val = SWITCH_TX3_1  },
  {  .name = "powd_ADF",  .has_arg = required_argument, .flag = NULL,        .val = POWERDOWN_ADF },
  {  .name = "tx3_2",   .has_arg = required_argument, .flag = NULL,        .val = SWITCH_TX3_2  },
  {  .name = "tx3_3",  .has_arg = required_argument, .flag = NULL,        .val = SWITCH_TX3_3  },
  {  .name = "lo1",      .has_arg = required_argument, .flag = NULL,        .val = SWITCH_LO1   },
  {  .name = "lo2",      .has_arg = required_argument, .flag = NULL,        .val = SWITCH_LO2   },
  {  .name = "pretend", .has_arg = no_argument,       .flag = &pflag,      .val = SWITCH_IS_ON  },
  {  .name = "posted",  .has_arg = no_argument,       .flag = &postedflag, .val = SWITCH_IS_ON  },
  {0, 0, 0, 0}
};

void show_usage(char* pgname) {
  unsigned int i;
  fprintf(stderr, "  %s : Tool to set on/off the switches of the RF prototype chain, v2 \n", pgname);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s [--lo_f [on|off]] [--rx_tx [on|off]] [--reset_lfsw [on|off]]\n", pgname);
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, " [--tx3_1 [on|off]] [--tx3_2 [on|off]] [--tx3_3 [on|off]]\n");
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, " [--powd_ADF [on|off]] [--lo1 [on|off]] [--lo2 [on|off]] \n");
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, " [-v|-vv] [-h|--help] [-p|--pretend]\n\n");
  fprintf(stderr, " [-P|--posted]    Do not issue an Irq to the Leon processor. Only post\n");
  fprintf(stderr, "                  configuration data (in this case, the Leon's firmware should\n");
  fprintf(stderr, "                  intentionnally read them back).\n");
  fprintf(stderr, " [-p|--pretend]   Just pretend to transfer commands, don't do it actually.\n");
  fprintf(stderr, "                  Useful together with verbose -v switch.\n");
  fprintf(stderr, " [-v|-vv]         Verbose modes\n");
  fprintf(stderr, " [-h|--help]      Displays this help\n");
}

#define TRUE  1
#define FALSE 0

int main(int argc, char** argv) {
  
  /* Interface with Grpci driver */
  int ioctlretval;
  int ifile;
  /* Interface with getopt_long() libC function */
  int getoptret;
  int indexptr;
  int erroroption = FALSE;
  int i; /* good old fellow */
  char* p_str_sw_lo_f = NULL;
  char* p_str_sw_rx_tx = NULL;
  char* p_str_reset_lfsw = NULL;
  char* p_str_sw_tx3_1 = NULL;
  char* p_str_sw_tx3_2 = NULL;
  char* p_str_sw_tx3_3 = NULL;
  char* p_str_powd_ADF = NULL;
  char* p_str_sw_lo1 = NULL;
  char* p_str_sw_lo2 = NULL;
  unsigned int swon_rx_tx, swon_lo_f, swon_tx3_1, swon_tx3_2, swon_tx3_3, swon_powd_ADF,swon_lo1,swon_lo2,swon_reset_lfsw;
  unsigned int verboselevel = 0;

  grpci_ctrl.RFswitches_onoff = 0x0;
  grpci_ctrl.RFswitches_mask = 0x0;

  /*****************
   * Parse options *
   *****************/
  while ((getoptret = getopt_long_only (argc, argv, "vhp", SWRFcmd_longopts, &indexptr)) != -1)
    switch (getoptret) {
      /* Without-argument options */
      case 0: /* means that the option just sets a flag. Nothing has to be done,
                 since getopt_long already sets the flag. */
        break;
      /* With-argument options & equivalent short options */
      case 'v': /* short switch -v was used  */
		verboselevel++;
        break;
      case 'h': /* short switch -h was used (or long options --help) */
        helpflag = SWITCH_IS_ON;
        break;
      case SWITCH_LO_F: /* long option --tx */
        p_str_sw_lo_f = optarg;
        break;
      case SWITCH_RX_TX: /* long option --ant */
        p_str_sw_rx_tx = optarg;
        break;
      case RESET_LFSW: /* long option --rxin */
        p_str_reset_lfsw = optarg;
        break;
      case POWERDOWN_ADF: /* long option --rxin */
        p_str_powd_ADF = optarg;
        break;
      case SWITCH_TX3_1: /* long option --rxdc2g */
        p_str_sw_tx3_1 = optarg;
        break;
      case SWITCH_TX3_2: /* long option --rx27g */
        p_str_sw_tx3_2 = optarg;
        break;
      case SWITCH_TX3_3: /* long option --rxout */
        p_str_sw_tx3_3 = optarg;
        break;
      case SWITCH_LO1: /* long option --rxdc4g */
        p_str_sw_lo1 = optarg;
        break;
      case SWITCH_LO2: /* long option --lo */
        p_str_sw_lo2 = optarg;
        break;
      case 'p': /* short switch -p was used (or long option --pretend) */
        pflag = SWITCH_IS_ON;
        break;
      case 'P': /* short switch -P was used (or long option --posted) */
        postedflag = SWITCH_IS_ON;
        break;
      default:
        erroroption = TRUE;
        break;
    };
  /* End of while */

  /********************************
   * Check consistency of options *
   ********************************/
  /* First, any irregularity in the use of the options ? Leave. */
  if (erroroption == TRUE) {
    fprintf(stderr, "%s: Misuse (--help to show usage)\n", argv[0]);
    exit(-1);
  }
  /* Let print the help if it has been explicitly asked for. */
  if (helpflag == SWITCH_IS_ON) {
    show_usage(argv[0]);
  }
  /* For each switch option used, check that argument on or off is used. */
  if (p_str_sw_lo_f) if ((swon_lo_f= strcmp(p_str_sw_lo_f, "on")) && (strcmp(p_str_sw_lo_f, "off")))
    {fprintf(stderr, "%s: swon_lo_f option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_rx_tx) if ((swon_rx_tx = strcmp(p_str_sw_rx_tx, "on")) && (strcmp(p_str_sw_rx_tx, "off")))
    {fprintf(stderr, "%s: swon_rx_tx option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_reset_lfsw) if ((swon_reset_lfsw = strcmp(p_str_reset_lfsw, "on")) && (strcmp(p_str_reset_lfsw, "off")))
    {fprintf(stderr, "%s: swon_reset_lfsw option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_powd_ADF) if ((swon_powd_ADF = strcmp(p_str_powd_ADF, "on")) && (strcmp(p_str_powd_ADF, "off")))
    {fprintf(stderr, "%s:  swon_powd_ADF option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_tx3_1) if ((swon_tx3_1 = strcmp(p_str_sw_tx3_1, "on")) && (strcmp(p_str_sw_tx3_1, "off")))
    {fprintf(stderr, "%s:  swon_tx3_1 option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_tx3_2) if ((swon_tx3_2 = strcmp(p_str_sw_tx3_2, "on")) && (strcmp(p_str_sw_tx3_2, "off")))
    {fprintf(stderr, "%s:  swon_tx3_2 option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_tx3_3) if ((swon_tx3_3 = strcmp(p_str_sw_tx3_3, "on")) && (strcmp(p_str_sw_tx3_3, "off")))
    {fprintf(stderr, "%s:  swon_tx3_3 option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_lo1) if ((swon_lo1  = strcmp(p_str_sw_lo1, "on")) && (strcmp(p_str_sw_lo1, "off")))
    {fprintf(stderr, "%s: swon_lo1 option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  if (p_str_sw_lo2) if ((swon_lo2 = strcmp(p_str_sw_lo2, "on")) && (strcmp(p_str_sw_lo2, "off")))
    {fprintf(stderr, "%s: swon_lo2 option requires argument 'on' or 'off'.\n", argv[0]); exit(-1);}
  
  /*************************************************
   * See what RF switches the user wants to access *
   *************************************************/

  if (p_str_sw_lo_f) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_LO_F_BITPOS;
    if (!swon_lo_f) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_LO_F_BITPOS;
  }
  if (p_str_sw_rx_tx) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_RX_TX_BITPOS;
    if (!swon_rx_tx) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_RX_TX_BITPOS;
  }
  if (p_str_reset_lfsw) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_RESET_LFSW_BITPOS;
    if (!swon_reset_lfsw) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_RESET_LFSW_BITPOS;
  }
  if (p_str_sw_tx3_1) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_TX3_1_BITPOS;
    if (!swon_tx3_1) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_TX3_1_BITPOS;
  }
  if (p_str_sw_tx3_2) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_TX3_2_BITPOS;
    if (!swon_tx3_2) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_TX3_2_BITPOS;
  }
  if (p_str_sw_tx3_3) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_TX3_3_BITPOS;
    if (!swon_tx3_3) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_TX3_3_BITPOS;
  }
  if (p_str_sw_lo1) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_LO1_BITPOS;
    if (!swon_lo1) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_LO1_BITPOS;
  }
  if (p_str_sw_lo2) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_SW_LO2_BITPOS;
    if (!swon_lo2) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_SW_LO2_BITPOS;
  }
  if (p_str_powd_ADF) {
    grpci_ctrl.RFswitches_mask |= 0x1 << FROM_GRLIB_RF2_POWERDOWN_ADF_BITPOS;
    if (!swon_powd_ADF) grpci_ctrl.RFswitches_onoff |= 0x1 << FROM_GRLIB_RF2_POWERDOWN_ADF_BITPOS;
  }

  /* printf("p_str_sw_lo_f = 0x%08x\n", p_str_sw_lo_f);
  printf("p_str_sw_rx_tx = 0x%08x\n", p_str_sw_rx_tx);
  printf("p_str_reset_lfsw = 0x%08x\n", p_str_reset_lfsw);
  printf("p_str_sw_tx3_1 = 0x%08x\n", p_str_sw_tx3_1);
  printf("p_str_sw_tx3_2 = 0x%08x\n", p_str_sw_tx3_2);
  printf("p_str_sw_tx3_3 = 0x%08x\n", p_str_sw_tx3_3);
  printf("p_str_sw_lo1 = 0x%08x\n", p_str_sw_lo1);
  printf("p_str_sw_lo2 = 0x%08x\n", p_str_sw_lo2);
  printf("p_str_powd_ADF = 0x%08x\n", p_str_powd_ADF); */

  if (!(  p_str_sw_lo_f  || p_str_sw_rx_tx || p_str_reset_lfsw || p_str_sw_tx3_1 || p_str_sw_tx3_2
       || p_str_sw_tx3_3 || p_str_sw_lo1   || p_str_sw_lo2     || p_str_powd_ADF))
  exit(0);

  /***************************************************
   * Ok, open device file (in /dev) to perform ioctl *
   ***************************************************/
  /* Opening Grpci device file (if --pretend option not set). */
  if (!pflag) {
    ifile = open(DEVICE_NAME, ACCESS_MODE, 0);
    if (ifile<0) {
      fprintf(stderr, "Error, could not open %s (open() returned %d, errno=%u)\n", DEVICE_NAME, ifile, errno);
      exit(-1);
    }
    /* Print I/O info if verbose level requires it. */
    if (verboselevel >= VERBOSE_LEVEL_IO)
      printf("Successful open %s\n", DEVICE_NAME);
  }
  /* Perform one ioctl system-call to configure (on/off) switches
     (if --pretend option not set). */
  if (!pflag) {
    if (!postedflag)
      ioctlretval = ioctl(ifile, openair_NEWRF_RF_SWITCH_CTRL,        (unsigned int*)(&(grpci_ctrl.RFswitches_onoff)));
    else
      ioctlretval = ioctl(ifile, openair_NEWRF_RF_SWITCH_CTRL_POSTED, (unsigned int*)(&(grpci_ctrl.RFswitches_onoff)));
    if (ioctlretval) {
      printf("NOK, could not configure RF switches using ioctl"
             " (sys call returned %d, errno = %u)\n", ioctlretval, errno);
      close(ifile);
      exit(-1);
    }
    /* Print I/O info if verbose level requires it. */
    if (verboselevel >= VERBOSE_LEVEL_IO)
      printf("Successful ioctl (sys call returned %d)\n", ioctlretval);
  }

  close(ifile);

  /* Print values if verbose level requires it. */
  if (verboselevel >= VERBOSE_LEVEL_VALUES) {
    printf("Raw value for RF switches on/off Register: 0x%03x\n", grpci_ctrl.RFswitches_onoff);
    printf("Raw value for RF switches  mask  Register: 0x%03x\n", grpci_ctrl.RFswitches_mask);
  }
  /* Did we pretend ? */
  if (pflag) printf("Nothing done.\n");

} /* main */
