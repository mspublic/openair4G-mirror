
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>

#include <sys/ioctl.h>
#include <cbmimo1_device.h>

#include "set_spi_rx.h"
#include "from_grlib_softregs.h"

/* Interface with openair driver */
#define DEVICE_NAME "/dev/openair0"
#define ACCESS_MODE O_RDONLY

/* Interface with getopt_long() libC function
 * 
 * On the use of getopt_long() function, see for instance the GNU C library
 * documentation, at:
 *   http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
 * This URL was available on Nov 2006. Quoting it:
 * 
 * o "When getopt_long() encounters a SHORT option, it does the same thing that
 *   getopt() would do: it returns the character code for the option, and stores
 *   the options argument (if it has one) in optarg.
 * o When getopt_long() encounters a LONG option, it takes actions based on the
 *   flag and val fields of the definition of that option.
 *     o If flag is a NULL pointer, then getopt_long() returns the contents of
 *       val to indicate which option it found. You should arrange distinct
 *       values in the val field for options with different meanings, so you
 *       can decode these values after getopt_long() returns. If the long option
 *       is equivalent to a short option, you can use the short option's
 *       character code in val.
 *     o If flag is NOT A NULL pointer, that means this option should just
 *       set a flag in the program. The flag is a variable of type int that
 *       you define. Put the address of the flag in the flag field. Put in
 *       the val field the value you would like this option to store in the
 *       flag. In this case, getopt_long() returns 0."
 */
#define SWITCH_IS_OFF        0
#define SWITCH_IS_ON         1
static unsigned int helpflag   = SWITCH_IS_OFF;
static unsigned int pflag      = SWITCH_IS_OFF;
static unsigned int postedflag = SWITCH_IS_OFF;
#define GAIN1           1
#define GAIN2           2
#define SWITCH_SWRX1_1  3
#define SWITCH_SWRX1_2  4
#define SWITCH_SWRX2    5

static struct struct_grpci_ctrl grpci_ctrl;

#define LONGOPTIONS_NB 8
struct option SWRFcmd_longopts[LONGOPTIONS_NB+1] = {
  {  .name = "help",    .has_arg = no_argument,       .flag = &helpflag,   .val = SWITCH_IS_ON   },
  {  .name = "gain1",   .has_arg = required_argument, .flag = NULL,        .val = GAIN1          },
  {  .name = "gain2",   .has_arg = required_argument, .flag = NULL,        .val = GAIN2          },
  {  .name = "swrx1_1", .has_arg = required_argument, .flag = NULL,        .val = SWITCH_SWRX1_1 },
  {  .name = "swrx1_2", .has_arg = required_argument, .flag = NULL,        .val = SWITCH_SWRX1_2 },
  {  .name = "swrx2",   .has_arg = required_argument, .flag = NULL,        .val = SWITCH_SWRX2   },
  {  .name = "pretend", .has_arg = no_argument,       .flag = &pflag,      .val = SWITCH_IS_ON   },
  {  .name = "posted",  .has_arg = no_argument,       .flag = &postedflag, .val = SWITCH_IS_ON   },
  {0, 0, 0, 0}
};

void show_usage(char* pgname) {
  unsigned int i;
  fprintf(stderr, "  %s : Tool to set on/off the Rx switches of the RF prototype chain.\n", pgname);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s --gain1 GAIN1 --gain2 GAIN2 --swrx1_1 0|1 --swrx1_2 0|1 --swrx2 0|1 \n", pgname);
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, "(these 14 options are mandatory)\n");
  fprintf(stderr, " [-v|-vv] [-h|--help] [-p|--pretend]\n");
  fprintf(stderr, " [-p|--pretend]   Just pretend to transfer commands, don't do it actually.\n");
  fprintf(stderr, "                  Useful together with verbose -v switch.\n");
  fprintf(stderr, " [-P|--posted]    Do not issue an Irq to the Leon processor. Only post\n");
  fprintf(stderr, "                  configuration data (in this case, the Leon's firmware should\n");
  fprintf(stderr, "                  intentionnally read them back).\n");
  fprintf(stderr, " [-v|-vv]         Verbose modes\n");
  fprintf(stderr, " [-h|--help]      Displays this help\n");
}

#define TRUE  1
#define FALSE 0

static char* p_str_gain1  = NULL;
static char* p_str_gain2  = NULL;
static char* p_str_swrx1_1  = NULL;
static char* p_str_swrx1_2  = NULL;
static char* p_str_swrx2  = NULL;


int main(int argc, char** argv) {
  
  /* Interface with Grpci driver */
  int ioctlretval;
  int ifile;
  /* Interface with getopt_long() libC function */
  int getoptret;
  int indexptr;
  int erroroption = FALSE;
  int i; /* good old fellow */
  unsigned int  swon_rx1_1, swon_rx1_2, swon_rx2;
  unsigned int verboselevel = 0;

  grpci_ctrl.setrx_gain1 = 0;
  grpci_ctrl.setrx_gain2 = 0;
  grpci_ctrl.setrx_switches_onoff = 0x0;

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
      case GAIN1: /* long option --gain1 */
        p_str_gain1 = optarg;
        break;
      case GAIN2: /* long option --gain2 */
        p_str_gain2 = optarg;
        break;
      case SWITCH_SWRX1_1: /* long option --swrx1_1 */
        p_str_swrx1_1 = optarg;
        break;
      case SWITCH_SWRX1_2: /* long option --swrx1_2 */
        p_str_swrx1_2 = optarg;
        break;
      case SWITCH_SWRX2: /* long option --swrx2 */
        p_str_swrx2 = optarg;
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

  /* Check that all options are present (since they are all assumed to be mandatory) */
  if ( !  ((p_str_swrx1_1) && (p_str_swrx1_2)  && (p_str_swrx2)
         && (p_str_gain1) && (p_str_gain2))
     ) {
    fprintf(stderr, "%s: --gain1, --gain2, --swrx1_1, --swrx1_2  , --swrx2 options are mandatory.\n", argv[0]);
    fprintf(stderr, "(--help to show usage)\n");
    exit(-1);
  }

  /* For each switch option used, check that argument 0 or 1 is used. */
   if (p_str_swrx1_1) if ((swon_rx1_1 = strcmp(p_str_swrx1_1, "1")) && (strcmp(p_str_swrx1_1, "0")))
    {fprintf(stderr, "%s: --swrx1_1 option requires argument 0 or 1.\n", argv[0]); exit(-1);}
  if (p_str_swrx1_2) if ((swon_rx1_2 = strcmp(p_str_swrx1_2, "1")) && (strcmp(p_str_swrx1_2, "0")))
    {fprintf(stderr, "%s: --swrx1_2 option requires argument 0 or 1.\n", argv[0]); exit(-1);}
  if (p_str_swrx2) if ((swon_rx2 = strcmp(p_str_swrx2, "1")) && (strcmp(p_str_swrx2, "0")))
    {fprintf(stderr, "%s: --swrx2 option requires argument 0 or 1.\n", argv[0]); exit(-1);}
  
  if (p_str_gain1) {
    grpci_ctrl.setrx_gain1 = atoi(p_str_gain1);
    if ((grpci_ctrl.setrx_gain1 < SETRX_GAIN1_MIN) || (grpci_ctrl.setrx_gain1 > SETRX_GAIN1_MAX))
      {fprintf(stderr, "%s: Invalid argument to --gain1 option (required range: %u - %u).\n", argv[0], SETRX_GAIN1_MIN, SETRX_GAIN1_MAX); exit(-1);}
  } else
    {fprintf(stderr, "%s: --gain1 option is mandatory.\n", argv[0]); exit(-1);}
  if (p_str_gain2) {
    grpci_ctrl.setrx_gain2 = atoi(p_str_gain2);
    if ((grpci_ctrl.setrx_gain2 < SETRX_GAIN2_MIN) || (grpci_ctrl.setrx_gain2 > SETRX_GAIN2_MAX))
      {fprintf(stderr, "%s: Invalid argument to --gain2 option (required range: %u - %u).\n", argv[0], SETRX_GAIN2_MIN, SETRX_GAIN2_MAX); exit(-1);}
  } else
    {fprintf(stderr, "%s: --gain2 option is mandatory.\n", argv[0]); exit(-1);}

  /*************************************************
   * See what RF switches the user wants to access *
   *************************************************/
  if  (!swon_rx1_1) grpci_ctrl.setrx_switches_onoff |= 0x1 << FROM_GRLIB_SETRX_SWRX1_1_POS;
  if  (!swon_rx1_2) grpci_ctrl.setrx_switches_onoff |= 0x1 << FROM_GRLIB_SETRX_SWRX1_2_POS;
  if  (!swon_rx2) grpci_ctrl.setrx_switches_onoff |= 0x1 << FROM_GRLIB_SETRX_SWRX2_POS;


  /* The final information is formatted in a unique 24-bits word */
  grpci_ctrl.setrx_raw_word = ( grpci_ctrl.setrx_gain1          << FROM_GRLIB_SETRX_RAW_WORD_GAIN1_POS)
                            | ( grpci_ctrl.setrx_gain2          << FROM_GRLIB_SETRX_RAW_WORD_GAIN2_POS)
                            | ( grpci_ctrl.setrx_switches_onoff << FROM_GRLIB_SETRX_RAW_WORD_RXSW_POS );

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
  /* Perform one ioctl system-call BOTH to configure (on/off) switches and specify values of gain1 & gain2
     (if --pretend option not set). */
  if (!pflag) {
    if (!postedflag)
      ioctlretval = ioctl(ifile, openair_NEWRF_SETRX_SWITCH_GAIN,        (unsigned int*)(&(grpci_ctrl.setrx_raw_word)));
    else
      ioctlretval = ioctl(ifile, openair_NEWRF_SETRX_SWITCH_GAIN_POSTED, (unsigned int*)(&(grpci_ctrl.setrx_raw_word)));
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
    printf("Raw value transmitted to board: 0x%08x\n", grpci_ctrl.setrx_raw_word);
  }
  /* Did we pretend ? */
  if (pflag) printf("Nothing done.\n");

} /* main */
