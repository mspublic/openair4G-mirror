
/* References      [ADF4108]     Component datasheet, Analog Devices
                                 PLL Frequency Synthesizer, ADF4108 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#include <sys/ioctl.h>
#include <cbmimo1_device.h>

#include "ADFcmd.h"

/* Interface with Grpci driver */
#define DEVICE_NAME "/dev/openair0"
#define ACCESS_MODE O_RDONLY

/* Interface with getopt_long() libC function
 * 
 * On the use of getopt_long() function, see for instance the GNU C library
 * documentation, at:
 *   http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
 * This URL was available on Nov 2006.
 * 
 * Quoting it: "
 * o When getopt_long encounters a short option, it does the same thing that
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
static unsigned int helpflag = SWITCH_IS_OFF;
static unsigned int directflag = SWITCH_IS_OFF;
static unsigned int initflag = SWITCH_IS_OFF;
static unsigned int gflag = SWITCH_IS_OFF;
static unsigned int mflag = SWITCH_IS_OFF;
static unsigned int kflag = SWITCH_IS_OFF;
static unsigned int indirectflag = SWITCH_IS_OFF;
static unsigned int rflag = SWITCH_IS_OFF;
static unsigned int aflag = SWITCH_IS_OFF;
static unsigned int bflag = SWITCH_IS_OFF;
static unsigned int sflag = SWITCH_IS_OFF;
static unsigned int nflag = SWITCH_IS_OFF;
static unsigned int pflag = SWITCH_IS_OFF;
static unsigned int force8200flag = SWITCH_IS_OFF;
static unsigned int postedflag = SWITCH_IS_OFF;

static struct struct_grpci_ctrl grpci_ctrl;

#define LONGOPTIONS_NB 15
struct option ADF4108cmd_longopts[LONGOPTIONS_NB+1] = {
  {  .name = "help",      .has_arg = no_argument,       .flag = &helpflag,      .val = SWITCH_IS_ON  },
  {  .name = "init",      .has_arg = no_argument,       .flag = &initflag,      .val = SWITCH_IS_ON  },
  {  .name = "direct",    .has_arg = no_argument,       .flag = &directflag,    .val = SWITCH_IS_ON  },
  {  .name = "ghz",       .has_arg = required_argument, .flag = NULL,           .val = 'g'           },
  {  .name = "mhz",       .has_arg = required_argument, .flag = NULL,           .val = 'm'           },
  {  .name = "khz",       .has_arg = required_argument, .flag = NULL,           .val = 'k'           },
  {  .name = "indirect",  .has_arg = no_argument,       .flag = &indirectflag,  .val = SWITCH_IS_ON  },
  {  .name = "refcnt",    .has_arg = required_argument, .flag = NULL,           .val = 'r'           },
  {  .name = "acnt",      .has_arg = required_argument, .flag = NULL,           .val = 'a'           },
  {  .name = "bcnt",      .has_arg = required_argument, .flag = NULL,           .val = 'b'           },
  {  .name = "psc",       .has_arg = required_argument, .flag = NULL,           .val = 's'           },
  {  .name = "refin",     .has_arg = required_argument, .flag = NULL,           .val = 'n'           },
  {  .name = "pretend",   .has_arg = no_argument,       .flag = &pflag,         .val = 'p'           },
  {  .name = "force8200", .has_arg = no_argument,       .flag = &force8200flag, .val = SWITCH_IS_ON  },
  {  .name = "posted",    .has_arg = no_argument,       .flag = &postedflag,    .val = 'P'           },
  {0, 0, 0, 0}
};

#define ADF4108_INIT_REG_VALUE 0x1f8013
void show_usage(char* pgname) {
  unsigned int i;
  fprintf(stderr, "  %s : Tool to program the registers of the ADF4108 Frequency Synthesizer.\n", pgname);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s [-d|--direct [-g|--ghz val] | [m|--mhz val] | [-k|--khz val]]\n", pgname);
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, " [-i|--indirect [-r|--refcnt val] [-a|--acnt val] [-b|--bcnt val] [-s|--psc val]]\n");
  fprintf(stderr, "  "); for (i=0;i<strlen(pgname);i++) fprintf(stderr, " ");
  fprintf(stderr, " [--init] [-v|-vv|-vvv] [-h|--help] [-p|--pretend]\n\n");
  fprintf(stderr, " [  -d|--direct  ]   To specify direct value of the frequency to set.\n");
  fprintf(stderr, "                     This option implies use of one (and only one) of the following sub-options:\n");
  fprintf(stderr, "        [-g|--ghz]   Directly specifies output frequency in GHz\n");
  fprintf(stderr, "                     Mandatory range is 1-8.\n");
  fprintf(stderr, "        [-m|--mhz]   Directly specifies output frequency in Mhz\n");
  fprintf(stderr, "                     Mandatory range is 1000-8200.\n");
  fprintf(stderr, "        [-k|--khz]   Directly specifies output frequency in kHz)\n");
  fprintf(stderr, "                     Mandatory range is 1000000-8200000.\n");
  fprintf(stderr, "                     Mind that any value specified here shall be rounded down to ?? kHz\n");
  fprintf(stderr, "                     so 1900430 kHz actually stands for ?? kHz\n");
  fprintf(stderr, " [ -i|--indirect ]   To specify indirectly the value of the frequency to set, using the\n");
  fprintf(stderr, "                     following sub-options (all of them are then required):\n");
  fprintf(stderr, "       [-r|--rcnt]   Specifies the value of the 14-bits Reference Counter to set.\n");
  fprintf(stderr, "                     Mandatory range is 1-16383.\n");
  fprintf(stderr, "       [-a|--acnt]   Specifies the value of the 6-bits A Counter to set.\n");
  fprintf(stderr, "                     Mandatory range is 0-63.\n");
  fprintf(stderr, "       [-b|--bcnt]   Specifies the value of the 13-bits B Counter to set.\n");
  fprintf(stderr, "                     Mandatory range is 3-8191.\n");
  fprintf(stderr, "        [-s|--psc]   Specifies the value of the Prescaler value to set.\n");
  fprintf(stderr, "                     Mandatory values are 8, 16, 32, 64.\n");
  fprintf(stderr, "  [-f|--force8200]   Force setting to 8.2 Ghz.\n");
  fprintf(stderr, "                     In this case, curring settings are applied:\n");
  fprintf(stderr, "                       o Refin is supposed to be 26 MHz.\n");
  fprintf(stderr, "                       o A=8, B=256, R=26, Prescaler=32/33\n");
  fprintf(stderr, "  [-P|--posted]      Do not issue an Irq to the Leon processor. Only post\n");
  fprintf(stderr, "                     configuration data (in this case, the Leon's firmware should\n");
  fprintf(stderr, "                     intentionnally read them back).\n");
  fprintf(stderr, "  [  --init  ]       Load the value 0x%06x in Initialization register.\n", ADF4108_INIT_REG_VALUE);
  fprintf(stderr, "  [-n|--refin]       Specify the Reference Input Frequency, in MHz (default is %d).\n", F_REFIN_DEFAULT_MHZ);
  fprintf(stderr, "                     Mandatory range is %d-%d MHz.\n", F_REFIN_MIN_MHZ, F_REFIN_MAX_MHZ);
  fprintf(stderr, "  [-p|--pretend]     Just pretend to transfer commands, don't do it actually.\n");
  fprintf(stderr, "                     Useful together with verbose -vv switch.\n");
  fprintf(stderr, "  [-v|-vv|-vvv]      Verbose modes\n");
  fprintf(stderr, "  [-h|--help]        Displays this help\n");
  fprintf(stderr, "Notes: 1) Usage of [-d|--direct] or [-i|--indirect] options are exclusive.\n");
  fprintf(stderr, "       2) One may use the -vv switch together with the direct option to get a display\n");
  fprintf(stderr, "          of the values actually written into the component registers, thus providing a set\n");
  fprintf(stderr, "          of values for suitable reuse with the indirect option.\n");
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
  char* p_str_ghz = NULL;
  char* p_str_mhz = NULL;
  char* p_str_khz = NULL;
  char* p_str_rcnt = NULL;
  char* p_str_acnt = NULL;
  char* p_str_bcnt = NULL;
  char* p_str_psc = NULL;
  char* p_str_refin = NULL;
  unsigned int rcnt, acnt, bcnt, func0, func1, psc, fvcokhz, Prescaler;
  unsigned int refin = F_REFIN_DEFAULT_MHZ;
  unsigned int PBplusA, acnt_try, bcnt_try, abcnt;
  unsigned int verboselevel = 0;

  /*****************
   * Parse options *
   *****************/
  while ((getoptret = getopt_long_only (argc, argv, "dg:m:k:ir:a:b:f:n:vhps:", ADF4108cmd_longopts, &indexptr)) != -1)
    switch (getoptret) {
      /* Without-argument options */
      case 0: /* means that the option just sets a flag. Nothing has to be done,
                 since getopt_long already sets the flag. */
        break;
      /* With-argument options & equivalent short options */
      case 'v':
		verboselevel++;
        break;
      case 'h': /* short switch -h was used */
        helpflag = SWITCH_IS_ON;
        break;
      case 'd': /* short switch -d was used */
        directflag = SWITCH_IS_ON;
        break;
      case 'g': /* short switch -g was used (or long option --ghz) */
        gflag = SWITCH_IS_ON; p_str_ghz = optarg;
        break;
      case 'm': /* short switch -m was used (or long option --mhz) */
        mflag = SWITCH_IS_ON; p_str_mhz = optarg;
        break;
      case 'k': /* short switch -k was used (or long option --khz) */
        kflag = SWITCH_IS_ON; p_str_khz = optarg;
        break;
      case 'i': /* short switch -i was used */
        indirectflag = SWITCH_IS_ON;
        break;
      case 'r': /* short switch -r was used (or long option --rcnt) */
        rflag = SWITCH_IS_ON; p_str_rcnt = optarg;
        break;
      case 'a': /* short switch -a was used (or long option --acnt) */
        aflag = SWITCH_IS_ON; p_str_acnt = optarg;
        break;
      case 'b': /* short switch -b was used (or long option --bcnt) */
        bflag = SWITCH_IS_ON; p_str_bcnt = optarg;
        break;
      case 's': /* short switch -s was used (or long option --psc) */
        sflag = SWITCH_IS_ON; p_str_psc = optarg;
        break;
      case 'n': /* short switch -n was used (or long option --refin) */
        nflag = SWITCH_IS_ON; p_str_refin = optarg;
        break;
      case 'p': /* short switch -p was used */
        pflag = SWITCH_IS_ON;
        break;
      case 'f': /* short switch -f was used */
        force8200flag=SWITCH_IS_ON;
        break;
      case 'P': /* short switch -P was used */
        postedflag=SWITCH_IS_ON;
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
  /* Let print the help if it has been explicitly asked for */
  if (helpflag == SWITCH_IS_ON) {
    show_usage(argv[0]);
    exit(0);
  }
  /* Check that if --init was used, nor --direct nor --indirect nor --force8200 options were too. */
  if (initflag) {
    if (directflag || indirectflag || force8200flag) {
      fprintf(stderr, "%s: --init switch must be used alone\n", argv[0]);
      fprintf(stderr, "(use --help to show usage).\n");
      exit(-1);
    } else {
      /* Opening Grpci device file (if --pretend option not set). */
      if (!pflag) {
        ifile = open(DEVICE_NAME, ACCESS_MODE, 0);
        if (ifile<0) {
          fprintf(stderr, "%s: Error, could not open %s (open() returned %d, errno=%u)\n", argv[0], DEVICE_NAME, ifile, errno);
          exit(-1);
        }
        if (verboselevel >= VERBOSE_LEVEL_IO) printf("Successful open %s\n", DEVICE_NAME);
      }
      /* Print I/O info if verbose level requires it. */
      grpci_ctrl.init = ADF4108_INIT_REG_VALUE;
      /* 1. Perform one ioctl system-call to latch Initialization Register
            (if --pretend option not set). */
      if (!pflag) {
        ioctlretval = ioctl(ifile, openair_NEWRF_ADF4108_INIT, (unsigned int)grpci_ctrl.init);
        if (ioctlretval) {
          printf("NOK, could not write Initialiation Register of ADF4108 using ioctl"
                 " (sys call returned %d, errno = %u)\n", ioctlretval, errno);
          close(ifile);
          exit(-1);
        }
        /* Print I/O info if verbose level requires it. */
        if (verboselevel >= VERBOSE_LEVEL_IO)
          printf("Successful ioctl (sys call returned %d)\n", ioctlretval);
      }
      exit(0);
    } /* directflag || indirectflag */
  } /* initflag */

  /* Check that one and only one options among --direct, --indirect and --force8200 is used. */
  if (  (   directflag && (indirectflag || force8200flag))
     || ( indirectflag && (  directflag || force8200flag))
     || (force8200flag && (  directflag || indirectflag ))) {
    fprintf(stderr, "%s: Specify one (and only one) of the 3 options -d|--direct, or -i|--indirect, or -f|--force8200\n", argv[0]);
    fprintf(stderr, "(use --help to show usage).\n");
    exit(-1);
  }
  /* Check that if --direct option was used, exactly one and only one suboption among -g, -m and -k was also used. */
  if (directflag)
    if ( (!(gflag | mflag | kflag))
          ||
         ( (gflag && ( mflag | kflag )) || (mflag && ( kflag | gflag )) || (kflag && ( gflag | mflag )) )
       )
    {
      fprintf(stderr, "%s: Use of -d|--direct option requires use of one (and only one) suboption among these:\n", argv[0]);
      fprintf(stderr, "  [-g|--ghz val], [-m|--mhz val] and [-k|--khz val]\n");
      fprintf(stderr, "(use --help to show usage).\n");
      exit(-1);
    }
  /* Check that if --indirect option was used, all the -r, -a, -b and -s suboptions are also used. */
  if (indirectflag && !(rflag && aflag && bflag && sflag)) {
    fprintf(stderr, "%s: Use of -i|--indirect option requires use of all the following suboptions:\n", argv[0]);
    fprintf(stderr, "  [-r|--rcnt val] [-a|--acnt val] [-b|--bcnt val] [-s|--psc val]\n");
    fprintf(stderr, "(use --help to show usage).\n");
    exit(-1);
  }
  
  /* If a Reference Input Frequency was specified, use it (but only if -f|--force8200 was not set). */
  if (p_str_refin) {
    if (force8200flag)
      fprintf(stderr, "%s: Ignoring REFIN frequency setting (Forcing 8200MHz option implies REFIN=26MHz).\n", argv[0]);
    else
      refin = atoi(p_str_refin);
  }

  /* If -f|--force8200 switch was set, Reference Input Frequency is assumed to equal 26MHz. */
  if (force8200flag) refin=26;

  /* Check correct range for Reference Input Frequency. */
  if ((refin < F_REFIN_MIN_MHZ) || (refin > F_REFIN_MAX_MHZ)) {
    fprintf(stderr, "%s: Range of Reference Input Frequency is %d-%d MHz\n", argv[0], F_REFIN_MIN_MHZ, F_REFIN_MAX_MHZ);
    exit(-1);
  }
  /* Convert Reference Input Frequency in KHz */
  refin *= 1000;


  if (force8200flag) {
    /***************************************************************************
     * Management of force8200 usage (user specified -f or --force8200 switch) *
     ***************************************************************************/
    /* These are the forced settings for 8.2 GHz. */
    Prescaler = 32;
    bcnt=256;
    acnt=8;
	rcnt=26;

  } else if (indirectflag) {
    /*************************************************************************
     * Management of indirect usage (user specified -i or --indirect switch) *
     *************************************************************************/
    Prescaler = atoi(p_str_psc);
    bcnt = atoi(p_str_bcnt);
    acnt = atoi(p_str_acnt);
    rcnt = atoi(p_str_rcnt);
    /* Check ranges */
    if ((acnt < A_CNT_MIN) || (acnt > A_CNT_MAX)) {
      fprintf(stderr, "%s: Range of A Counter is %u - %u.\n", A_CNT_MIN, A_CNT_MAX);
      exit(-1);
    }
    if ((bcnt < B_CNT_MIN) || (bcnt > B_CNT_MAX)) {
      fprintf(stderr, "%s: Range of B Counter is %u - %u.\n", B_CNT_MIN, B_CNT_MAX);
      exit(-1);
    }
    if ((rcnt < REF_CNT_MIN) || (rcnt > REF_CNT_MAX)) {
      fprintf(stderr, "%s: Range of Reference Counter is %u - %u.\n", REF_CNT_MIN, REF_CNT_MAX);
      exit(-1);
    }
    if (! ((Prescaler == 8) || (Prescaler == 16) || (Prescaler == 32) || (Prescaler==64)) ) {
      fprintf(stderr, "%s: The prescaler value must be 8, 16, 32 or 64.\n");
      exit(-1);
    }
    /* Check that the Prescaler value specified in the Function Register value
       matches the requirement of prescaler freq <= 300 MHz (see [ADF4108] p9). */
    if (!((refin / Prescaler) <= PRESC_OUT_MAX_KHZ)) {
      fprintf(stderr, "%s: The value of Prescaler (%u) does not fit the requirements 'REFIN / Prescaler <= %u kHz'.\n", argv[0], Prescaler, PRESC_OUT_MAX_KHZ);
      fprintf(stderr, "(see [ADF4108-datasheet] p9). Use another Prescaler value.\n");
      exit(-1);
    }

  } else {
    /*********************************************************************
     * Management of direct usage (user specified -d or --direct switch) *
     *********************************************************************/
    /* Choose the 1st (smallest) Prescaler factor that is suitable so that the Prescaler output is 300 MHz or less
       (see [ADF4108]) p9). */
    if ((refin / 64) <= PRESC_OUT_MAX_KHZ) Prescaler = 64;
    if ((refin / 32) <= PRESC_OUT_MAX_KHZ) Prescaler = 32;
    if ((refin / 16) <= PRESC_OUT_MAX_KHZ) Prescaler = 16;
    if ((refin / 8) <= PRESC_OUT_MAX_KHZ) Prescaler = 8;
    /* Print Prescaler value if verbose level requires it. */
    if (verboselevel >= VERBOSE_LEVEL_VALUES)
      printf("Using %d/%d as prescaling factor\n", Prescaler, Prescaler+1);
    if (gflag) {
      /* user specified a Giga-Hertz value, convert in Kilo-Hertz */
      fvcokhz=atoi(p_str_ghz)*1000000;
    } else if (mflag) {
      /* user specified a Mega-Hertz value, convert in Kilo-Hertz */
      fvcokhz=atoi(p_str_mhz)*1000;
    } else {
      /* user specified a Kilo-Hertz value */
      fvcokhz=atoi(p_str_khz);
    }
    /* Since user asked us for direct mode, we must process the values of R, A and B counters.
       Algo: we try to use the least possible value for R, and then we choose A and B */
    rcnt = refin; acnt = 0;
    while (rcnt >= 1) {
      PBplusA = fvcokhz / (refin / rcnt);
      acnt_try = 0; bcnt = 0;
      while (acnt_try <= 63) {
        bcnt_try = (PBplusA - acnt_try) / Prescaler;
        if ((bcnt_try >= 3) && (bcnt_try <= 8191)) {bcnt = bcnt_try; break;}
        acnt_try++;
      }
      if (bcnt) {acnt = acnt_try; break;}
      rcnt--;
    }
    if (!bcnt) {
      fprintf(stderr, "Could not find appropriate values for Ref, A & B counters. Aborting.\n");
      exit(-1);
    }
    /* Print values if verbose level requires it. */
    if (verboselevel >= VERBOSE_LEVEL_VALUES) {
      printf("Ok, found these values for counter registers:\n");
      printf("      Ref counter: %d\n", rcnt);
      printf("      A counter:   %d\n", acnt);
      printf("      B counter:   %d\n", bcnt);
    }
  } /* if (indirectflag) */

  /*************************************
   * Transfer settings to grpci driver *
   *************************************/
  /* The Device Programming sequence consists in the following:
     1. Do a Function latch (with F1 bit unset, that is 0)
     2. Do an R counter load
     3. Do an AB counter load
     Onyl one ioctl is needed for these. */
  /* First, convert the Prescaler value from decimal into 2-bits
     according to Function Register spec. (see [ADF4108] p14) */
  Prescaler = ( Prescaler == 64 ? 0x3 : Prescaler >> 4);
  /* Put into shape the actual values to latch. */
  grpci_ctrl.func0 = (Prescaler << 22) | FUNC_DEFAULT_BITS_21_DWTO_0 | FUNC_UNSET_CNT_RST;
  grpci_ctrl.rcnt = ((rcnt & 0x00003fff) << 2) | REFCNT_DEFAULT_BITS_23_DWTO_16_1_0;
  grpci_ctrl.abcnt = ((bcnt & 0x00001fff) << 8) | ((acnt & 0x0000003f) << 2) | ABCNT_DEFAULT_BITS_23_DWTO_21_1_0;
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
  /* Perform one ioctl system-call to latch Function Register with F1 set to 1
     (if --pretend option not set). */
  if (!pflag) {
    if (!postedflag)
      ioctlretval = ioctl(ifile, openair_NEWRF_ADF4108_WRITE_REG,        (unsigned int*)(&(grpci_ctrl.func0)));
    else
      ioctlretval = ioctl(ifile, openair_NEWRF_ADF4108_WRITE_REG_POSTED, (unsigned int*)(&(grpci_ctrl.func0)));
    if (ioctlretval) {
      fprintf(stderr, "%s: NOK, could not transfer settings to ADF4108 using ioctl"
             " (sys call returned %d, errno = %u)\n", argv[0], ioctlretval, errno);
      close(ifile);
      exit(-1);
    }
    /* Print I/O info if verbose level requires it. */
    if (verboselevel >= VERBOSE_LEVEL_IO)
      printf("Successful ioctl (sys call returned %d)\n", ioctlretval);
  }

  close(ifile);

  /* Print values if verbose level requires it. */
  if (verboselevel >= VERBOSE_LEVEL_RAW_VALUES) {
    printf("Raw value for Function Register:          0x%08x\n", grpci_ctrl.func0);
    printf("Raw value for Reference Counter Register: 0x%08x\n", grpci_ctrl.rcnt);
    printf("Raw value for AB Counters Register:       0x%08x\n", grpci_ctrl.abcnt);
  }
  /* Did we pretend ? */
  if (pflag) printf("Nothing done.\n");

} /* main */
