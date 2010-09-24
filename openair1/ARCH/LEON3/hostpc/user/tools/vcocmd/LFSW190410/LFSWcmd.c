
/* References      [LFSW190410-50]     Component datasheet, Synergy Microwave
 *                                     Interactive Synthesizer, LFSW190410-50
 *                 [AN7100A]           Application Note, Synergy Microwave
 *                                     Pinout & Programming Functions for Interactive Synthesizer */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
//#include <unistd.h>
#include <getopt.h>

#include <sys/ioctl.h>
#include <cbmimo1_device.h>

/* Interface with Grpci driver */
#define DEVICE_NAME "/dev/openair0"
#define ACCESS_MODE O_RDONLY
char kHzFreq_str[9] = "\0\0\0\0\0\0\0\0\0";  /* 8 ASCII-coded chars + 1 null byte */

/* Component constraints */
#define MIN_KHZ_FREQ 1900000
#define MIN_MHZ_FREQ 1900
#define MAX_KHZ_FREQ 4100000
#define MAX_MHZ_FREQ 4100
#define STEP_SIZE_KHZ_FREQ 500
#define STEP_SIZE_MHZ_FREQ 0.5

/* Interface with getopt_long() libC function */
#define HELP_LONGOPTION_INDEX 0
#define VERBOSE_LONGOPTION_INDEX 0
#define MOREVERBOSE_LONGOPTION_INDEX 0
#define CH_LONGOPTION_INDEX 0
#define N_LONGOPTION_INDEX 0
#define KHZ_LONGOPTION_INDEX 0
#define MHZ_LONGOPTION_INDEX 0
#define POSTED_LONGOPTION_INDEX 0

#define HELP_SWITCH_IS_OFF    0
#define HELP_SWITCH_IS_ON     1
static unsigned int helpflag = HELP_SWITCH_IS_OFF;

#define VERBOSE_SWITCH_IS_OFF    0
#define VERBOSE_SWITCH_IS_ON     1
static unsigned int verboseflag = VERBOSE_SWITCH_IS_OFF;

#define MOREVERBOSE_SWITCH_IS_OFF    0
#define MOREVERBOSE_SWITCH_IS_ON     1
static unsigned int moreverboseflag = MOREVERBOSE_SWITCH_IS_OFF;

#define POSTED_SWITCH_IS_OFF    0
#define POSTED_SWITCH_IS_ON     1
static unsigned int postedflag = POSTED_SWITCH_IS_OFF;

#define TRUE  1
#define FALSE 0

#define LONGOPTIONS_NB 7
struct option LFSWcmd_longopts[LONGOPTIONS_NB+1] = {
  {
    .name = "help",
    .has_arg = no_argument,
    .flag = &helpflag,
    .val = HELP_SWITCH_IS_ON
  },
  {
    .name = "verbose",
    .has_arg = no_argument,
    .flag = &verboseflag,
    .val = 'v'
  },
  {
    .name = "ch",
    .has_arg = required_argument,
    .flag = NULL,
    .val = 'c'
  },
  {
    .name = "Ndr",
    .has_arg = required_argument,
    .flag = NULL,
    .val = 'N'
  },
  {
    .name = "kHz",
    .has_arg = required_argument,
    .flag = NULL,
    .val = 'k'
  },
  {
    .name = "MHz",
    .has_arg = required_argument,
    .flag = NULL,
    .val = 'm'
  },
  {
    .name = "posted",
    .has_arg = no_argument,
    .flag = &postedflag,
    .val = 'P'
  },
  {0, 0, 0, 0}
};

void show_usage(char* pgname) {
  fprintf(stderr, "  %s : Tool to program the registers of the LFSW190410-50 Frequency Synthesizer.\n", pgname);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s [--ch val] [-N|--Ndr val] [--kHz val] [--MHz val ] [-v|--verbose] [-h|--help]\n", pgname);
  fprintf(stderr, "  --ch           Specifies the channel for the synthesizer to tune (min=0)\n");
  fprintf(stderr, "                   Using this switch, output frequency is given by Fout = Fc0 + (N * Cs)\n");
  fprintf(stderr, "                   (where N is the value specified to --ch switch)\n");
  fprintf(stderr, "                   Since Fmax = 4.1 GHz and Step Size Cs = 500 kHz, max value is 4400.\n");
  fprintf(stderr, "  -N|--Ndr       Specifies the Division Ratio\n");
  fprintf(stderr, "                   Using this switch, output frequency is given by Fout = N * Step Size)\n");
  fprintf(stderr, "                   Since Fmin = 1.9 GHz, Fmax = 4.1 GHz and Step Size Cs = 500 MHz,\n");
  fprintf(stderr, "                   min value is 3800, and max value is 8200\n");
  fprintf(stderr, "  --kHz          Directly specifies output frequency in kHz (min=1900000, max=4100000)\n");
  fprintf(stderr, "                 Mind that any value specified shall be rounded down to 500 kHz (Step Size)\n");
  fprintf(stderr, "                   so 1900430 kHz actually stands for 1900000 kHz\n");
  fprintf(stderr, "  --MHz          Directly specifies output frequency in MHz (min=1900, max=4100)\n");
  fprintf(stderr, "  [-P|--posted]  Do not issue an Irq to the Leon processor. Only post\n");
  fprintf(stderr, "                 configuration data (in this case, the Leon's firmware should\n");
  fprintf(stderr, "                 intentionnally read them back).\n");
  fprintf(stderr, "  -v|--verbose   verbose mode\n");
  fprintf(stderr, "  -h|--help      Displays this help\n");
  fprintf(stderr, "Note: component settled values:\n");
  fprintf(stderr, "  Step Size (aka Channel Spacing)    = 500  kHz\n");
  fprintf(stderr, "  Specified starting frequency (Fc0) = 1900 kHz\n");
}

int main(int argc, char** argv) {
  
  /* Interface with Grpci driver */
  int ioctlretval;
  int ifile;
  /* Interface with getopt_long() libC function */
  int getoptret;
  int indexptr;
  int erroroption = FALSE;
  char* p_str_Nth_channel = NULL;
  char* p_str_N_dev_ratio = NULL;
  char* p_str_kHz = NULL;
  char* p_str_MHz = NULL;
  unsigned int kHzFreq;
  int i; /* good old fellow */

  /* On the use of getopt() function, see for instance the GNU C library
     documentation, at:
       http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html
     (This URL was available on Nov 2006) */

  while ((getoptret = getopt_long (argc, argv, "vhN:", LFSWcmd_longopts, &indexptr)) != -1)
    switch (getoptret) {
      /* Without-argument options */
      case 0: /* means that the option just sets a flag. Nothing has to be done,
                 since getopt_long already sets the flag. */
        break;
      /* With-argument options & equivalent short options */
      case 'v':
        verboseflag = VERBOSE_SWITCH_IS_ON;
        break;
      case 'h':
        helpflag = HELP_SWITCH_IS_ON;
        break;
      case 'c': /* Long switch --ch was specified */
        p_str_Nth_channel = optarg;
        break;
      case 'N': /* Long switch --Ndr *(or short swith -N) was specified */
        p_str_N_dev_ratio = optarg;
        break;
      case 'k': /* Long switch --ch was specified */
        p_str_kHz = optarg;
        break;
      case 'm': /* Long switch --ch was specified */
        p_str_MHz = optarg;
        break;
      case 'P': /* Long switch --posted was specified */
        postedflag = POSTED_SWITCH_IS_ON;
        break;
      default:
        erroroption = TRUE;
        break;
    };
  /* End of while */

  if (erroroption == TRUE) {
    fprintf(stderr, "Use --help to show usage\n");
    exit(-1);
  }

  if (optind != argc) {
    fprintf(stderr, "Use --help to show usage\n");
    exit(-1);
  }

  if (helpflag == HELP_SWITCH_IS_ON) {
    show_usage(argv[0]);
    exit(0);
  }

  if (  ((p_str_Nth_channel) && (p_str_N_dev_ratio || p_str_kHz || p_str_MHz))
     || ((p_str_N_dev_ratio) && ( p_str_Nth_channel || p_str_kHz || p_str_MHz))
     || ((p_str_kHz) && ( p_str_Nth_channel || p_str_N_dev_ratio || p_str_MHz))
     || ((p_str_MHz) && ( p_str_Nth_channel || p_str_N_dev_ratio || p_str_kHz))  ) {
    fprintf(stderr, "Use only one of the four specified ways to set output frequency!\n");
    fprintf(stderr, "Use --help to show usage\n");
    exit(-1);
  }

  /* For now, we only support --kHz switch to set frequency ... */
  if (p_str_Nth_channel) {
    fprintf(stderr, "Sorry, for now only --kHz switch is supported to set frequency!\n");
    exit(0);
  }

  if (p_str_N_dev_ratio) {
    fprintf(stderr, "Sorry, for now only --kHz switch is supported to set frequency!\n");
    exit(0);
  }

  if (p_str_MHz) {
    fprintf(stderr, "Sorry, for now only --kHz switch is supported to set frequency!\n");
    exit(0);
  }

  if (p_str_kHz) {
    /* Check consistency of passed parameter */
    /* Checkmin */
    kHzFreq = atoi(p_str_kHz);
    if (kHzFreq < MIN_KHZ_FREQ) {
      fprintf(stderr, "Error: Specified incorrect %d kHz frequency (minimum is %d)\n", kHzFreq, MIN_KHZ_FREQ);
      exit(-1);
    }
    /* Checkmax */
    if (kHzFreq > MAX_KHZ_FREQ) {
      fprintf(stderr, "Error: Specified incorrect %d kHz frequency (maximum is %d)\n", kHzFreq, MAX_KHZ_FREQ);
      exit(-1);
    }
    /* Warn if kHz frequency parameter is not rounded to Step Size (Channel Spacing) */
    if (kHzFreq % STEP_SIZE_KHZ_FREQ) {
      fprintf(stderr, "Warning: Rounding %d kHz frequency to %d (Step Size is %d)\n", kHzFreq,
                      (kHzFreq / STEP_SIZE_KHZ_FREQ) * STEP_SIZE_KHZ_FREQ, STEP_SIZE_KHZ_FREQ);
      kHzFreq = (kHzFreq / STEP_SIZE_KHZ_FREQ) * STEP_SIZE_KHZ_FREQ;
    }

    if (verboseflag == TRUE) {
      printf("Using kHzFreq = %d (0x%08x)\n", kHzFreq, kHzFreq);
    }

    /* Opening Grpci device file */
    ifile = open(DEVICE_NAME, ACCESS_MODE, 0);
    if (ifile<0) {
      fprintf(stderr, "Error, could not open %s (open() returned %d, errno=%u)\n", DEVICE_NAME, ifile, errno);
  	  exit(-1);
    }
    /* verbose */
    if (verboseflag == TRUE) {
      printf("Info: %s was successfully opened\n", DEVICE_NAME);
    }

    /* Perform ioctl on the device file, specifying kHz freq value to transmit to CardBus MIMO1 firmware
     * (as a pointer to the an 8 chars string, so transcode it now) */
    for (i=0; i<8; i++) {
      //kHzFreq_str[i] = (kHzFreq & (0xf<<((7-i)<<2))) >> ((7-i)<<2); /* for now, value in decimal, not in ascii */
      kHzFreq_str[i] = ((kHzFreq % (1<<((8-i)<<2)))/ (1<<((7-i)<<2))); /* for now, value in decimal, not in ascii */
      if (kHzFreq_str[i] >= 0 && kHzFreq_str[i] <= 9) kHzFreq_str[i] += '0';
      else if (kHzFreq_str[i] >= 10 && kHzFreq_str[i] <= 15) kHzFreq_str[i] += 'A'-10;
      else {
        fprintf(stderr, "Error: weird value of kHz Frequency\n");
        close(ifile);
        exit(-1);
      } /* now it is in ascii */
    }
    kHzFreq_str[8] = '\0';
    /* verbose */
    if (verboseflag == TRUE) {
      printf("Writing hex bit sequence: K", kHzFreq_str);
      for (i=0; i<8; i++) printf("%c", kHzFreq_str[i]);
      printf("\n");
    }

    /* Actually perform the ioctl system-call */
    if (!postedflag)
      ioctlretval = ioctl(ifile, openair_NEWRF_LFSW190410_WRITE_KHZ,        kHzFreq_str);
    else
      ioctlretval = ioctl(ifile, openair_NEWRF_LFSW190410_WRITE_KHZ_POSTED, kHzFreq_str);
    if (ioctlretval) {
      printf("NOK, could not write LFSW190410-50 kHz Frequency using ioctl (sys call returned %d, errno = %u)\n", ioctlretval, errno);
      close(ifile);
  	  exit(-1);
    }
    /* verbose */
    if (verboseflag == TRUE) {
      printf("OK, succesful ioctl (sys call returned %d)\n", ioctlretval);
    }
    close(ifile);

  }  /* end of if (p_str_kHz) */

  /* Complete verbose info displayed, if needed */
  
}
