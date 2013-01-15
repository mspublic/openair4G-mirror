//**************************************************************
// Compile with:
// $ make colabsim
//**************************************************************

#include <string.h>
#include <math.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include "SIMULATION/TOOLS/defs.h"
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/vars.h"
#include "MAC_INTERFACE/vars.h"

#include "ARCH/CBMIMO1/DEVICE_DRIVER/vars.h"
#include "SCHED/defs.h"
#include "SCHED/vars.h"
#include "LAYER2/MAC/vars.h"

#include "OCG_vars.h"

#ifndef RA_RNTI
#define RA_RNTI 0xfffe
#endif

typedef unsigned char bool;
const bool false = 0;
const bool true = 1;

#define BW 7.68
#define N_PRB 25
#define RBG_SIZE 2
#define NID_CELL 0
#define MAX_RELAYS 8
#define MAX_HARQ_ROUNDS 4
#define MAX_FRAMES (2*MAX_HARQ_ROUNDS+1)
#define DATA_BUFFER_SIZE (14*12*N_PRB*6/8+16)

const u8 cp_type = 0;         // Normal cyclic prefix
const u8 n_antenna_ch = 1;  // Number of CH antennas
const u8 n_antenna_mr = 1;  // Number of MR antennas
const u8 n_pdcch_symbols = 3; // Number of PDCCH symbols in DL subframes
const double awgn_stddev = 100.0;

#if MAX_RELAYS <= 8
typedef u8 relay_bit_array_t;
#elif MAX_RELAYS <= 16
typedef u16 relay_bit_array_t;
#elif MAX_RELAYS <= 32
typedef u32 relay_bit_array_t;
#else
#error "MAX_RELAYS > 32 not supported"
#endif

typedef enum {
  analysis_single,      // Simulate one SNR point
  analysis_snrsweep_a,  // Sweep SNR of first relay from negative to positive 
                        // for both hops
  analysis_snrsweep_b,  // Sweep SNR of first relay from negative to positive 
                        // for hop 1 and from postive to negative for hop 2
  analysis_snrsweep_c
} analysis_t;

typedef enum {
  comm_mode_only_pdu, // Only do transmissions of MAC PDUs, control is error-free
  comm_mode_full      // Include transmissions of control information (DCI, HARQ, BSR)
} comm_mode_t;

typedef enum {
  strategy_wait_all,  // Wait for all relays to decode before starting hop 2
  strategy_wait_one   // Start hop 2 when one relay has decoded
} strategy_t;

// AMC mode
typedef enum {
  amc_none,             // No AMC
  amc_rbadjust          // Adjust bandwidth for specified MCS and MAC PDU size
} amc_t;

// Structure for command line parsed arguments
typedef struct {
  bool debug_output;              // Output MATLAB signal files
  int verbose;                    // Verbosity level
  analysis_t analysis;            // Analysis mode
  int range;                      // Sweep range
  double step;                    // Sweep step size
  comm_mode_t comm_mode;          // Communication mode
  bool perfect_channel_estimation;// Use perfect channel estimation
  strategy_t strategy;            // HARQ strategy
  int subframe_pdsch;             // Subframe for CH1 PDSCH transmissions
  int n_relays;                   // Number of relays
  int n_pdu;                      // Number of MAC PDUs to simulate
  int n_harq;                     // Maximum number of HARQ rounds
  amc_t amc;                      // AMC mode
  int mcs_hop1;                   // MCS for hop 1
  int mcs_hop2;                   // MCS for hop 2
  int n_prb_hop1;                 // Number of PRB utilized in hop 1
  int n_prb_hop2;                 // Number of PRB utilized in hop 2
  SCM_t channel_model;            // Channel model
  double channel_correlation;     // Channel reutilization factor
  double snr_hop1[MAX_RELAYS];    // SNR used in hop 1 for all links
  double snr_hop2[MAX_RELAYS];    // SNR used in hop 2 for all links
  const char* results_fn;         // File to save simulation results to
} args_t;

typedef struct {
  u8 frame;                 // LTE frame and subframe that the PDU transmission occurred in
  u8 subframe;
  u16 tbs;                  // TBS used
  u8 mcs;                   // MCS used
  u8 n_prb;                 // number of used PRBs
  u16 n_sent_bits;          // number of sent raw bits
  u16 n_correct_bits[MAX_RELAYS]; // number of correctly received bits
  relay_bit_array_t decoded; // bit array of relays that successfully decoded the MAC PDU
} hop1_round_results_t;

typedef struct {
  hop1_round_results_t round[MAX_HARQ_ROUNDS];
  u8 n_rounds;              // Number of HARQ rounds used
} hop1_results_t;

typedef struct {
  u8 frame;                 // LTE frame and subframe that the PDU transmission occurred in
  u8 subframe;
  u16 tbs;                  // TBS used
  u8 mcs;                   // MCS used
  u8 n_prb;                 // number of used PRBs
  u16 n_sent_bits;          // number of sent raw bits
  u16 n_correct_bits;       // number of correctly received bits
  relay_bit_array_t active; // bit array of active relays in transmission
  u8 decoded;               // non-zero if MAC PDU decoded by CH1
} hop2_round_results_t;

typedef struct {
  hop2_round_results_t round[MAX_HARQ_ROUNDS];
  u8 n_rounds;              // Number of HARQ rounds used
} hop2_results_t;

// Structure of transmission results for one MAC PDU
typedef struct {
  hop1_results_t hop1;      // Hop 1 transmission results
  hop2_results_t hop2;      // Hop 2 transmission results
  u16 n_bytes;              // Number of bytes in transmission
} tx_results_t;

// Structure containing link simulation results for one test
typedef struct {
  u32 n_pdu;                // number of transmitted MAC PDUs
  u8 n_relays;              // number of relays
  u8 n_harq;                // maximum number of HARQ rounds
  SCM_t channel_model;      // used channel model
  float snr_hop1[MAX_RELAYS]; // SNRs for each link in hop 1
  float snr_hop2[MAX_RELAYS]; // SNRs for each link in hop 2
  tx_results_t* tx;         // Transmission results
} results_t;

// Relay role in distributed Alamouti coding
typedef enum {
  RELAY_ROLE_STANDARD,    // Relay sends [ x1   x2 ]
  RELAY_ROLE_ALTERNATE    // Relay sends [-x2*  x1*]
} relay_role_t;

typedef struct {
  double* s_re[1];
  double* s_im[1];
  double* r_re[1];
  double* r_im[1];
  double* r_re_t[1];
  double* r_im_t[1];
} channel_vars_t;

typedef struct {
  channel_vars_t* cvars;
  channel_desc_t* channel;
} sh_channel_t;

typedef struct {
  bool pdsch;
  bool pdsch_harq;
  bool bsr;
  bool pusch_dci;
  bool pusch;
  bool pusch_harq;
} schedule_t;

typedef enum {
  hop1_harq_ack, hop1_harq_nack, hop1_harq_eack
} hop1_harq_t;

typedef enum {
  hop2_harq_ack, hop2_harq_nack
} hop2_harq_t;

// Simulation context
typedef struct {
  LTE_DL_FRAME_PARMS* frame_parms;
  PHY_VARS_eNB* phy_vars_ch_src;      // CH0 PHY variables
  PHY_VARS_eNB* phy_vars_ch_dest;     // CH1 PHY variables
  PHY_VARS_UE* phy_vars_mr[MAX_RELAYS]; // MR PHY variables
  sh_channel_t** channels_hop1;       // CH0->MR channels
  sh_channel_t** channels_hop2;       // MR->CH1 channels
  s32* rxdata[1];                     // Temporary vector used by MR->CH1 channels
  double* snr_hop1;                   // SNRs in hop 1
  double* snr_hop2;                   // SNRs in hop 2
  int mcs_hop1;                       // MCS in hop 1
  int mcs_hop2;                       // MCS in hop 2
  u32 tbs_hop1;                       // TBS in hop 1
  u32 tbs_hop2;                       // TBS in hop 2
  int n_prb_hop1;                     // Number of PRB used in hop 1
  int n_prb_hop2;                     // Number of PRB used in hop 2
  u16 rnti_hop1;                      // RNTI in hop 1
  u16 rnti_hop2;                      // RNTI in hop 2
  u8* ch0_buffer;                     // Data to send from CH0
  u8* mr_buffer[MAX_RELAYS];          // Contains data received at and transmitted from MRs
  u32 pdu_size;                       // MAC PDU size in bytes
  //u32 n_coded_bits_hop1;              // Number of coded bits in hop 1
  //u32 n_coded_bits_hop2;              // Number of coded bits in hop 2
  int subframe_pdsch;                 // Subframe for CH1 PDSCH transmissions
  int subframe_pusch;                 // Subframe for MR PUSCH transmissions
  u8 harq_pid_hop2;
  relay_role_t relay_role[MAX_RELAYS];        // Role of relays in Alamouti coding

  bool hop1_active;                           
  bool hop2_active;
  bool hop2_finished;
  bool hop1_received_pdcch[MAX_RELAYS];       // True if the MR has successfully received the last PDCCH from CH0.
  bool hop1_decoded_pdsch[MAX_RELAYS];        // True if the MR has successfully decoded the MAC PDU from CH0.
  hop1_harq_t hop1_harq_response[MAX_RELAYS]; // The HARQ response to send from each MR
  hop1_harq_t hop1_harq_received;             // The HARQ received at CH0
  bool hop2_received_bsr[MAX_RELAYS];
  bool hop2_received_pusch_dci[MAX_RELAYS];
  bool hop2_decoded_at_ch;
  hop2_harq_t hop2_harq_received[MAX_RELAYS]; // The HARQ response to send from each MR
  int round_hop1;
  int round_hop2;
} context_t;

void transmit_one_pdu(args_t* args, context_t* context, int pdu, results_t* results);
void transmit_pdsch(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe);
void transmit_pusch_dci(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe);
void transmit_pusch(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe);
void determine_pdu_size(args_t* args, context_t* context);
void hop1_amc(args_t* args, context_t* context);
void hop2_amc(args_t* args, context_t* context);
int parse_args(int argc, char** argv, args_t* args);
bool parse_snr(const char* str, double* snr, int n);
int parse_channel_model(const char* str, SCM_t* model);
void print_usage(const char* prog);
void print_channel_usage();
void print_info();
void signal_handler(int sig);
void setup_single(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays);
void setup_snrsweep_a(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end);
void setup_snrsweep_b(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end);
void setup_snrsweep_c(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end);
LTE_DL_FRAME_PARMS* alloc_frame_params(void);
void free_frame_params(LTE_DL_FRAME_PARMS* frame_parms);
PHY_VARS_eNB* alloc_ch_vars(LTE_DL_FRAME_PARMS* frame_parms, int id);
void free_ch_vars(PHY_VARS_eNB* phy_vars);
PHY_VARS_UE* alloc_mr_vars(LTE_DL_FRAME_PARMS* frame_parms, int id);
void free_mr_vars(PHY_VARS_UE* phy_vars);
void alloc_dlsch_co(PHY_VARS_eNB* phy_vars_ch_src, PHY_VARS_UE** phy_vars_mr, PHY_VARS_eNB* phy_vars_ch_dest, int n_relays);
void free_dlsch_co(PHY_VARS_eNB* phy_vars_ch_src, PHY_VARS_UE** phy_vars_mr, PHY_VARS_eNB* phy_vars_ch_dest, int n_relays);
u16 rballoc_type0(int n_rb, int rbg_size);
void setup_broadcast_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs, int n_rb);
void setup_distributed_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs, int n_rb);
void ofdm_modulation(mod_sym_t** tx_f, s32** tx_t, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb);
channel_vars_t alloc_channel_vars(LTE_DL_FRAME_PARMS* frame_parms);
void free_channel_vars(channel_vars_t v);
sh_channel_t* alloc_sh_channel(channel_vars_t* cvars, SCM_t channel_model, int n_txantennas, int n_rxantennas, double channel_correlation);
void free_sh_channel(sh_channel_t* c);
void transmit_subframe(sh_channel_t* channel, s32** src, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb, double ampl, bool accumulate);
void deliver_subframe(sh_channel_t* channel, s32** dst, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb, double stddev);
void slot_fep_est(args_t* args, PHY_VARS_UE* phy_vars, int l, int slot);
int rx_dlsch_co(args_t* args, PHY_VARS_UE* phy_vars, u8 subframe, u8 first_symbol);
u32 get_ulsch_G(LTE_UE_ULSCH_t *ulsch, u8 harq_pid);
int correct_bits_soft(u8* ref, s16* rec, int n);
void print_dlsch_eNB_stats(LTE_eNB_DLSCH_t* d);
void print_dlsch_ue_stats(LTE_UE_DLSCH_t* d);
void print_ulsch_ue_stats(LTE_UE_ULSCH_t* d);
void print_ulsch_eNB_stats(LTE_eNB_ULSCH_t* d);
int block_valid(u8* ref, u8* rec, int n);
results_t* alloc_results(args_t* a);
void init_results(results_t* r, args_t* a, context_t* c);
void free_results(results_t* r);
void print_results(results_t* r);
double compute_ber_hop1(results_t* r, int k);
double compute_ber_hop2(results_t* r);
void compute_harq_stats_hop1(results_t* r, int k, int* harq_tries, int* harq_success);
void compute_harq_stats_hop2(results_t* r, int* harq_tries, int* harq_success);
void compute_colab_bler(results_t* r, int* n_failed, int* n_total);
void compute_colab_stat(results_t* r, double* throughput, double* spectral_eff, double* latency, int* relay_activity);
void write_results_header(FILE* f, results_t* r, int n_tests);
void write_results_data(FILE* f, results_t* r);
void determine_pdsch_harq_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s);
void determine_pusch_dci_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s);
void determine_pusch_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s);
void determine_pusch_harq_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s);

// Function declarations missing in LTE_TRANSPORT/proto.h:
u8 pdcch_alloc2ul_subframe(LTE_DL_FRAME_PARMS* frame_parms, u8 n);
u8 ul_subframe2pdcch_alloc_subframe(LTE_DL_FRAME_PARMS* frame_parms, u8 n);

int main(int argc, char **argv)
{
  args_t args;
  results_t* results;
  context_t context;

  DCI_ALLOC_t dci_hop1;
  DCI_ALLOC_t dci_hop2;

  channel_vars_t channel_vars;

  double* snrs;
  int n_tests;

  int test;  // Current test
  int pdu;   // Current MAC PDU
  int k;

  bool store_results = false;
  FILE* results_file = 0;

  // Parse arguments
  k = parse_args(argc, argv, &args);
  if(k == 1) {
    print_usage(argv[0]);
    exit(1);
  }
  else if(k == 2) {
    print_channel_usage();
    exit(1);
  }
  else if(k == 3) {
    print_info();
    exit(1);
  }

  if(!args.perfect_channel_estimation || args.channel_model != AWGN) {
    printf("Channel estimation is broken, use -C AWGN --perfect_ce\n");
    exit(1);
  }

  // Check argument bounds
  if(args.n_relays > MAX_RELAYS) {
    printf("Too many relays, increase MAX_RELAYS\n");
    exit(1);
  }

  if(args.n_harq > MAX_HARQ_ROUNDS) {
    printf("Too many HARQ rounds, increase MAX_HARQ_ROUNDS\n");
    exit(1);
  }

  if(args.comm_mode == comm_mode_full) {
    printf("Communication mode full not implemented\n");
    exit(1);
  }

  switch(args.subframe_pdsch) {
    case 0: case 1: case 4: case 5: case 6:
      break;
    case 9:
      printf("subframe 9 currently not implemented\n");
      exit(1);
    default:
      printf("specified subframe is not valid\n");
      exit(1);
  }

  // General setup
  signal(SIGSEGV, signal_handler);
  randominit(0);
  set_taus_seed(0);

  // Initialize log
  logInit();

  // Initialize frame parameters and node structures
  context.frame_parms = alloc_frame_params();
  context.phy_vars_ch_src = alloc_ch_vars(context.frame_parms, 0);
  for(k = 0; k < args.n_relays; k++)
    context.phy_vars_mr[k] = alloc_mr_vars(context.frame_parms, k);
  context.phy_vars_ch_dest = alloc_ch_vars(context.frame_parms, 1);

  // Initialize result data
  results = alloc_results(&args);

  // Allocate channel structures
  context.channels_hop1 = malloc(args.n_relays*sizeof(sh_channel_t*));
  context.channels_hop2 = malloc(args.n_relays*sizeof(sh_channel_t*));
  memset(context.channels_hop1, 0, args.n_relays*sizeof(sh_channel_t*));
  memset(context.channels_hop2, 0, args.n_relays*sizeof(sh_channel_t*));

  // Setup analysis structures
  switch(args.analysis) {
  case analysis_single:
    setup_single(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, args.n_relays);
    break;
  case analysis_snrsweep_a:
    setup_snrsweep_a(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, args.n_relays, args.step, -args.range, args.range);
    break;
  case analysis_snrsweep_b:
    setup_snrsweep_b(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, args.n_relays, args.step, -args.range, args.range);
    break;
  case analysis_snrsweep_c:
    setup_snrsweep_c(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, args.n_relays, args.step, -args.range, args.range);
    break;
  }

  // Open results file (if requested)
  if(args.results_fn) {
    store_results = true;
    results_file = fopen(args.results_fn, "w");
    if(!results_file) {
      perror("fopen");
      exit(1);
    }
  }

  if(store_results) {
    results->n_pdu = args.n_pdu;
    results->n_relays = args.n_relays;
    results->n_harq = args.n_harq;
    results->channel_model = args.channel_model;
    write_results_header(results_file, results, n_tests);
  }

  // Setup simulation context
  context.rnti_hop1 = 0x1515;
  context.rnti_hop2 = 0x1516;
  context.subframe_pdsch = args.subframe_pdsch;
  determine_pdsch_harq_subframe(context.frame_parms, 0, context.subframe_pdsch, &k, &context.subframe_pusch);

  // Allocate temporary signal structures
  context.rxdata[0] = malloc(10*context.frame_parms->samples_per_tti);

  // Allocate collaborative transport channels
  alloc_dlsch_co(context.phy_vars_ch_src, context.phy_vars_mr, context.phy_vars_ch_dest, args.n_relays);

  // Setup channel structures
  channel_vars = alloc_channel_vars(context.frame_parms);
  for(k = 0; k < args.n_relays; k++) {
    context.channels_hop1[k] = alloc_sh_channel(&channel_vars, args.channel_model, n_antenna_ch, n_antenna_mr, args.channel_correlation);
    context.channels_hop2[k] = alloc_sh_channel(&channel_vars, args.channel_model, n_antenna_mr, n_antenna_ch, args.channel_correlation);
  }

  if(args.verbose > 1) {
    //print_dlsch_eNB_stats(context.phy_vars_ch_src->dlsch_eNB[0][0]);
    //print_ulsch_ue_stats(context.phy_vars_mr[0]->ulsch_ue[0]);
    //dump_dci(context.frame_parms, &dci_hop1);
    //dump_dci(context.frame_parms, &dci_hop2);
  }
  //printf("Hop 1: TBS=%d, G=%d, rate=%f. Hop 2: TBS=%d, G=%d, rate=%f\n", 
  //    context.tbs_hop1, context.n_coded_bits_hop1, (float)context.tbs_hop1/(float)context.n_coded_bits_hop1,
  //    context.tbs_hop2, context.n_coded_bits_hop2, (float)context.tbs_hop2/(float)context.n_coded_bits_hop2);

  // Allocate data buffers
  context.ch0_buffer = malloc(DATA_BUFFER_SIZE);
  memset(context.ch0_buffer, 0, DATA_BUFFER_SIZE);
  for(k = 0; k < args.n_relays; k++) {
    context.mr_buffer[k] = malloc(DATA_BUFFER_SIZE);
    memset(context.mr_buffer[k], 0, DATA_BUFFER_SIZE);
  }

  for(test = 0; test < n_tests; test++) {
    // Set SNRs
    context.snr_hop1 = &snrs[2*args.n_relays*test];
    context.snr_hop2 = &snrs[2*args.n_relays*test + args.n_relays];

    // Initialize results
    init_results(results, &args, &context);

    // Print test info.
    printf("\n*** Test %d/%d ***\n", test+1, n_tests);

    for(pdu = 0; pdu < args.n_pdu; pdu++) {
      transmit_one_pdu(&args, &context, pdu, results);
    }

    print_results(results);
    if(store_results)
      write_results_data(results_file, results);
  }

  if(store_results)
    fclose(results_file);

  free_results(results);
  free(context.ch0_buffer);
  for(k = 0; k < args.n_relays; k++) {
    free(context.mr_buffer[k]);
  }
  free(context.rxdata[0]);
  free_dlsch_co(context.phy_vars_ch_src, context.phy_vars_mr, context.phy_vars_ch_dest, args.n_relays);
  free(snrs);
  for(k = 0; k < args.n_relays; k++) {
    free_sh_channel(context.channels_hop1[k]);
    free_sh_channel(context.channels_hop2[k]);
  }
  free_channel_vars(channel_vars);
  free(context.channels_hop1);
  free(context.channels_hop2);
  free_ch_vars(context.phy_vars_ch_src);
  free_ch_vars(context.phy_vars_ch_dest);
  for(k = 0; k < args.n_relays; k++)
    free_mr_vars(context.phy_vars_mr[k]);
  free_frame_params(context.frame_parms);

  return 0;
}

void transmit_one_pdu(args_t* args, context_t* context, int pdu, results_t* results)
{
  // State variables:
  int frame;          // Current LTE frame
  int subframe;       // Current LTE subframe
  schedule_t schedule[MAX_FRAMES][10];

  // Temporary variables:
  int k;
  int f;
  int s;
  bool b;

  if(args->verbose == 0) {
    printf("Transmitting MAC PDU %d\r", pdu);
    fflush(stdout);
  } else
    printf("Transmitting MAC PDU %d\n", pdu);

  // Set role of each relay (alternating STANDARD and ALTERNATE)
  for(k = 0; k < args->n_relays; k++) {
    context->relay_role[k] = k & 1;
  }

  // Initialize schedule
  for(frame = 0; frame < MAX_FRAMES; frame++) {
    for(subframe = 0; subframe < 10; subframe++) {
      schedule[frame][subframe].pdsch = false;
      schedule[frame][subframe].pdsch_harq = false;
      schedule[frame][subframe].bsr = false;
      schedule[frame][subframe].pusch_dci = false;
      schedule[frame][subframe].pusch = false;
      schedule[frame][subframe].pusch_harq = false;
    }
  }
  schedule[0][context->subframe_pdsch].pdsch = true;

  // Compute pdu size
  determine_pdu_size(args, context);
  if(context->pdu_size > DATA_BUFFER_SIZE) {
    fprintf(stderr, "DATA_BUFFER_SIZE too small\n");
    exit(1);
  }
  if(args->verbose > 0) {
    printf("MAC PDU size: %d bytes\n", context->pdu_size);
  }

  // Generate source data
  for(k = 0; k < context->pdu_size; k++)
    context->ch0_buffer[k] = (u8)(taus()&0xff);

  results->tx[pdu].n_bytes = context->pdu_size;

  context->hop1_active = true;
  context->hop2_active = false;
  context->hop2_finished = false;
  for(k = 0; k < args->n_relays; k++) {
    context->hop1_received_pdcch[k] = false;
    context->hop1_decoded_pdsch[k] = false;
    context->hop1_harq_response[k] = hop1_harq_nack;
    context->hop2_received_bsr[k] = false;
    context->hop2_received_pusch_dci[k] = false;
    context->hop2_harq_received[k] = hop2_harq_nack;
  }
  context->hop1_harq_received = hop1_harq_nack;
  context->hop2_decoded_at_ch = false;
  context->round_hop1 = 0;
  context->round_hop2 = 0;

  for(k = 0; k < args->n_relays; k++) {
    context->channels_hop1[k]->channel->first_run = 1;
    context->channels_hop2[k]->channel->first_run = 1;
  }

  for(frame = 0; frame < MAX_FRAMES; frame++) {
    for(subframe = 0; subframe < 10; subframe++) {
      if(schedule[frame][subframe].pdsch) {
        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: PDSCH CH0->MRs, round %d\n", frame, subframe, context->round_hop1);
        }

        results->tx[pdu].hop1.round[context->round_hop1].frame = frame;
        results->tx[pdu].hop1.round[context->round_hop1].subframe = subframe;

        // Determine hop 1 mcs and n_prb
        hop1_amc(args, context);
        if(args->verbose > 0) {
          printf("Hop 1: MCS=%d, n_PRB=%d, TBS=%d\n", context->mcs_hop1, context->n_prb_hop1, context->tbs_hop1);
        }

        // Do the PDSCH transmission
        transmit_pdsch(args, context, results, pdu, subframe);

        // Schedule the PDSCH HARQ transmission
        determine_pdsch_harq_subframe(context->frame_parms, frame, subframe, &f, &s);
        if(f < MAX_FRAMES) {
          schedule[f][s].pdsch_harq = true;
          if(!context->hop2_finished && !context->hop2_active)
            for(k = 0; k < args->n_relays; k++)
              if(context->hop1_decoded_pdsch[k])
                schedule[f][s].bsr = true;
        }
        else {
          printf("Increase MAX_FRAMES\n");
          exit(1);
        }

        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: PDSCH decoded at these MRs:", frame, subframe);
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop1_decoded_pdsch[k])
              printf(" %d", k);
          }
          printf("\n");
        }
      }

      if(schedule[frame][subframe].pdsch_harq) {
        // Determine the HARQ response from each MR
        for(k = 0; k < args->n_relays; k++) {
          if(context->hop2_harq_received[k] == hop2_harq_ack)
            context->hop1_harq_response[k] = hop1_harq_eack;
          else if(context->hop1_decoded_pdsch[k])
            context->hop1_harq_response[k] = hop1_harq_ack;
          else if(context->hop1_received_pdcch[k])
            context->hop1_harq_response[k] = hop1_harq_nack;
          else // If PDCCH was not received, nothing is sent (silent ACK)
            context->hop1_harq_response[k] = hop1_harq_ack;
        }

        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: PDSCH HARQ", frame, subframe);
          for(k = 0; k < args->n_relays; k++) {
            printf(" %d(%s)", k, 
                context->hop1_harq_response[k] == hop1_harq_eack ? "EACK" :
                (context->hop1_harq_response[k] == hop1_harq_nack ? "NACK" :
                (context->hop1_harq_response[k] == hop1_harq_ack ? "ACK" : "INVALID")));
          }
          printf("\n");
        }

        if(args->comm_mode == comm_mode_full) {
          //transmit_pdsch_harq();
        }
        else {
          // if at least one MR send eack, received harq is eack
          // otherwise, if at least one MR send nack, received harq is nack
          // otherwise, received harq is ack
          context->hop1_harq_received = hop1_harq_ack;
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop1_harq_response[k] == hop1_harq_eack) {
              context->hop1_harq_received = hop1_harq_eack;
              break;
            }
            else if(context->hop1_harq_response[k] == hop1_harq_nack) {
              context->hop1_harq_received = hop1_harq_nack;
            }
          }
        }

        // Decide whether to schedule a retransmission based on the received HARQ ack/nack
        if(context->hop1_harq_received == hop1_harq_eack) {
          if(args->verbose > 0) {
            printf("Frame %d, subframe %d: CH0 received EACK, hop 1 finished\n", frame, subframe);
          }
        }
        else if(context->hop1_harq_received == hop1_harq_ack) {
          if(args->verbose > 0) {
            printf("Frame %d, subframe %d: CH0 received ACK, hop 1 finished\n", frame, subframe);
          }
        }
        else if(context->hop1_harq_received == hop1_harq_nack) {
          if(context->round_hop1 == args->n_harq) {
            if(args->verbose > 0) {
              printf("Frame %d, subframe %d: CH0 received NACK, maximum HARQ round reached\n", frame, subframe);
            }
          }
          else {
            if(args->verbose > 0) {
              printf("Frame %d, subframe %d: CH0 received NACK, scheduling retransmission\n", frame, subframe);
            }
            // Determine next PDSCH subframe
            if(context->subframe_pdsch > subframe) {
              f = frame;
              s = context->subframe_pdsch;
            }
            else {
              f = frame+1;
              s = context->subframe_pdsch;
            }
            if(f < MAX_FRAMES)
              schedule[f][s].pdsch = true;
            else {
              printf("Increase MAX_FRAMES\n");
              exit(1);
            }
          }
        }
      }

      if(schedule[frame][subframe].bsr) {
        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: BSR MRs->CH1 from MRs", frame, subframe);
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop1_decoded_pdsch[k])
              printf(" %d", k);
          }
          printf("\n");
        }

        if(args->comm_mode == comm_mode_full) {
          //transmit_bsr();
        }
        else {
          for(k = 0; k < args->n_relays; k++)
            context->hop2_received_bsr[k] = context->hop1_decoded_pdsch[k];
        }

        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: CH1, received BSR from MRs", frame, subframe);
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop2_received_bsr[k])
              printf(" %d", k);
          }
          printf("\n");
        }

        // Determine if hop 2 should be started
        if(!context->hop2_finished && !context->hop2_active) {
          switch(args->strategy) {
            case strategy_wait_all:
              context->hop2_active = true;
              for(k = 0; k < args->n_relays; k++)
                if(!context->hop2_received_bsr[k])
                  context->hop2_active = false;
              break;
            case strategy_wait_one:
              for(k = 0; k < args->n_relays; k++)
                if(context->hop2_received_bsr[k])
                  context->hop2_active = true;
              break;
          }
          if(context->hop2_active) {
            if(args->verbose > 0) {
              printf("Frame %d, subframe %d: starting hop 2\n", frame, subframe);
            }
            hop2_amc(args, context);
            if(args->verbose > 0) {
              printf("Hop 2: MCS=%d, n_PRB=%d, TBS=%d\n", context->mcs_hop2, context->n_prb_hop2, context->tbs_hop2);
            }
          }
        }

        if(context->hop2_active) {
          // Schedule PUSCH DCI if BSR received from at least one MR
          b = false;
          for(k = 0; k < args->n_relays; k++)
            if(context->hop2_received_bsr[k])
              b = true;

          if(b) {
            if(args->verbose > 0) {
              //printf("Frame %d, subframe %d: received BSR, scheduling PUSCH DCI\n", frame, subframe);
            }
            determine_pusch_dci_subframe(context->frame_parms, frame+1, subframe, &f, &s);
            if(f < MAX_FRAMES)
              schedule[f][s].pusch_dci = true;
            else {
              printf("Increase MAX_FRAMES\n");
              exit(1);
            }
          }
        }
      }

      if(schedule[frame][subframe].pusch) {
        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: PUSCH MRs->CH1, HARQ round %d, active MRs:", frame, subframe, context->round_hop2);
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop2_received_pusch_dci[k])
              printf(" %d", k);
          }
          printf("\n");
        }

        results->tx[pdu].hop2.round[context->round_hop2].frame = frame;
        results->tx[pdu].hop2.round[context->round_hop2].subframe = subframe;

        // Do PUSCH transmission
        transmit_pusch(args, context, results, pdu, subframe);

        // Schedule PHICH
        determine_pusch_harq_subframe(context->frame_parms, frame, subframe, &f, &s);
        if(f < MAX_FRAMES)
          schedule[f][s].pusch_harq = true;
        else {
          printf("Increase MAX_FRAMES\n");
          exit(1);
        }

        // If not decoded, schedule retransmission
        if(!context->hop2_decoded_at_ch) {
          determine_pusch_dci_subframe(context->frame_parms, frame+1, subframe, &f, &s);
          if(f < MAX_FRAMES)
            schedule[f][s].pusch_dci = true;
          else {
            printf("Increase MAX_FRAMES\n");
            exit(1);
          }
        }

        if(args->verbose > 0) {
          if(context->hop2_decoded_at_ch) {
            printf("Frame %d, subframe %d: successfully decoded at CH1\n", frame, subframe);
          }
        }
      }

      if(schedule[frame][subframe].pusch_harq) {
        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: PHICH CH1->MRs\n", frame, subframe);
        }

        if(args->comm_mode == comm_mode_full) {
          // transmit_pusch_harq();
        }
        else {
          for(k = 0; k < args->n_relays; k++)
            context->hop2_harq_received[k] = (context->hop2_decoded_at_ch ? hop2_harq_ack : hop2_harq_nack);
        }

        if(args->verbose > 0) {
          printf("Frame %d, subframe %d: received CH1 PHICH at MRs:", frame, subframe);
          for(k = 0; k < args->n_relays; k++) {
            if(context->hop2_harq_received[k] == hop2_harq_ack || context->hop2_harq_received[k] == hop2_harq_nack)
              printf(" %d(%s)", k, context->hop2_harq_received[k] == hop2_harq_ack ? "ACK" : "NACK");
          }
          printf("\n");
        }

        /*
        b = false;
        for(k = 0; k < args->n_relays; k++)
          if(context->hop2_harq_received[k] == hop2_harq_nack)
            b = true;

        if(b) {
          determine_pusch_subframe(context->frame_parms, frame, subframe, &f, &s);
          if(f < MAX_FRAMES)
            schedule[f][s].pusch = true;
          else {
            printf("Increase MAX_FRAMES\n");
            exit(1);
          }
        }
        */
      }

      if(schedule[frame][subframe].pusch_dci) {
        if(context->round_hop2 < args->n_harq) {
          if(args->verbose > 0) {
            printf("Frame %d, subframe %d: PDCCH DCI CH1->MRs, HARQ round %d, scheduling MRs", frame, subframe, context->round_hop2);
            for(k = 0; k < args->n_relays; k++) {
              if(context->hop2_received_bsr[k])
                printf(" %d", k);
            }
            printf("\n");
          }

          transmit_pusch_dci(args, context, results, pdu, subframe);

          determine_pusch_subframe(context->frame_parms, frame, subframe, &f, &s);
          if(f < MAX_FRAMES)
            schedule[f][s].pusch = true;
          else {
            printf("Increase MAX_FRAMES\n");
            exit(1);
          }
        }
      }

    }
  }
  results->tx[pdu].hop1.n_rounds = context->round_hop1;
  results->tx[pdu].hop2.n_rounds = context->round_hop2;
}

void transmit_pdsch(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe) 
{
  LTE_DL_FRAME_PARMS* frame_parms = context->frame_parms;
  LTE_eNB_DLSCH_t** dlsch_enb = context->phy_vars_ch_src->dlsch_eNB_co[0];
  mod_sym_t** txdataF = context->phy_vars_ch_src->lte_eNB_common_vars.txdataF[0];
  s32** txdata = context->phy_vars_ch_src->lte_eNB_common_vars.txdata[0];
  s32** rxdata[MAX_RELAYS];
  //s32** rxdataF[MAX_RELAYS];
  s16* llr;
  LTE_UE_DLSCH_t** dlsch_ue;
  int n_re;
  DCI_ALLOC_t dci;
  int n_used_pdcch_symbols;
  u32 tx_energy;
  double awgn_stddev;
  double tx_ampl;
  //double raw_ber;
  int k;
  int l;
  int n_iter;
  int n_coded_bits;
  int n_correct_bits;

  int n_symbols_per_slot = frame_parms->symbols_per_tti>>1;
  int pilot1_symbol = (cp_type == 0 ? 4 : 3);

  // Temporary strings
  char fnbuf[80];
  char varbuf[80];

  for(k = 0; k < args->n_relays; k++) {
    rxdata[k] = context->phy_vars_mr[k]->lte_ue_common_vars[0]->rxdata;
    //rxdataF[k] = context->phy_vars_mr[k]->lte_ue_common_vars.rxdataF;
  }

  // Clear txdataF vector
  memset(txdataF[0], 0, FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));

  // Create DCI
  setup_broadcast_dci(&dci, context->rnti_hop1, context->round_hop1, context->mcs_hop1, context->n_prb_hop1);
  if(args->verbose > 2)
    dump_dci(frame_parms, &dci);

  // Generate eNB transport channel parameters
  generate_eNB_dlsch_params_from_dci(subframe, dci.dci_pdu, context->rnti_hop1, 
      format1, dlsch_enb, frame_parms,
      &context->phy_vars_ch_src->pdsch_config_dedicated[0],
      SI_RNTI, RA_RNTI, P_RNTI,
      context->phy_vars_ch_src->eNB_UE_stats[0].DL_pmi_single);
  n_coded_bits = get_G(frame_parms, dlsch_enb[0]->nb_rb, dlsch_enb[0]->rb_alloc,
      get_Qm(context->mcs_hop1), n_pdcch_symbols, subframe);

  // Create PDCCH
  n_used_pdcch_symbols = generate_dci_top(1, 0, &dci, 0, 1024, frame_parms, txdataF, subframe);
  if(n_used_pdcch_symbols > n_pdcch_symbols) {
    printf("Need %d PDCCH symbols\n", n_used_pdcch_symbols);
    exit(1);
  }
      
  // Encode source data
  if(dlsch_encoding(context->ch0_buffer, frame_parms, n_pdcch_symbols, dlsch_enb[0], subframe) < 0)
    exit(-1);

  // Scramble data
  dlsch_scrambling(frame_parms, n_pdcch_symbols, dlsch_enb[0], n_coded_bits, 0, subframe << 1);

  // Modulate data
  n_re = dlsch_modulation(txdataF, 1024, subframe, frame_parms, n_pdcch_symbols, dlsch_enb[0]);
  if(args->verbose > 1)
    printf("Hop 1, HARQ round %d: %d coded bits, Modulated %d REs\n", 
        context->round_hop1, n_coded_bits, n_re);

  // Fill results
  results->tx[pdu].hop1.round[context->round_hop1].mcs = (u8)context->mcs_hop1;
  results->tx[pdu].hop1.round[context->round_hop1].tbs = (u16)context->tbs_hop1;
  results->tx[pdu].hop1.round[context->round_hop1].n_prb = (u8)context->n_prb_hop1;
  results->tx[pdu].hop1.round[context->round_hop1].n_sent_bits = (u16)n_coded_bits;

  if(args->verbose > 2)
    print_dlsch_eNB_stats(dlsch_enb[0]);

  // Generate pilots
  generate_pilots(context->phy_vars_ch_src, txdataF, 1024, LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

  // OFDM modulation
  ofdm_modulation(txdataF, txdata, frame_parms, subframe, n_symbols_per_slot*3);

  // Compute transmitter signal energy ( E{abs(X)^2} )
  tx_energy = signal_energy(&txdata[0][subframe*frame_parms->samples_per_tti], frame_parms->samples_per_tti);

  // Transmit over channel
  for(k = 0; k < args->n_relays; k++) {
    awgn_stddev = sqrt((double)tx_energy)/pow(10.0, ((double)context->snr_hop1[k])/20.0);
    tx_ampl = awgn_stddev/sqrt((double)tx_energy)*pow(10.0, ((double)context->snr_hop1[k])/20.0);

    transmit_subframe(context->channels_hop1[k], txdata, frame_parms, subframe, 
        frame_parms->symbols_per_tti+1, tx_ampl, false);
    deliver_subframe(context->channels_hop1[k], rxdata[k], frame_parms, subframe, 
        frame_parms->symbols_per_tti+1, awgn_stddev);
  }

  results->tx[pdu].hop1.round[context->round_hop1].decoded = 0;

  // Decode at all relays that have not yet decoded
  for(k = 0; k < args->n_relays; k++) {
    if(context->hop1_decoded_pdsch[k])
      continue;

    dlsch_ue = context->phy_vars_mr[k]->dlsch_ue_co[0];
    llr = context->phy_vars_mr[k]->lte_ue_pdsch_vars_co[0]->llr[0];

    // Front end processor up to first pilot
    for(l = 0; l <= pilot1_symbol; l++)
      slot_fep(context->phy_vars_mr[k], l, subframe<<1, 0, 0);

    // Skip decoding of DCI
    context->phy_vars_mr[k]->lte_ue_pdcch_vars[0]->crnti = context->rnti_hop1;
    context->phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols = n_pdcch_symbols;
    generate_ue_dlsch_params_from_dci(subframe, dci.dci_pdu, context->rnti_hop1, 
        format1, dlsch_ue, frame_parms,
	&context->phy_vars_mr[k]->pdsch_config_dedicated[0],
	SI_RNTI, RA_RNTI, P_RNTI);

    context->hop1_received_pdcch[k] = true;

    // Receive DLSCH data
    if(rx_dlsch_co(args, context->phy_vars_mr[k], subframe, pilot1_symbol) == -1)
      break;
    
    // Compute raw bit error rate
    n_correct_bits = correct_bits_soft(dlsch_enb[0]->e, llr, n_coded_bits);
    results->tx[pdu].hop1.round[context->round_hop1].n_correct_bits[k] = n_correct_bits;
    if(args->verbose > 1)
      printf("Received %d bits at MR %d, raw BER: %f\n", n_coded_bits, k, 
          (double)n_correct_bits/(double)n_coded_bits);

    // Unscramble received bits
    dlsch_unscrambling(frame_parms, context->phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
        dlsch_ue[0], n_coded_bits, llr, 0, subframe << 1);

    // Decode received bits
    n_iter = dlsch_decoding(llr, frame_parms, dlsch_ue[0], subframe, 
        context->phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols, 0);

    if(args->verbose > 2)
      print_dlsch_ue_stats(dlsch_ue[0]);

    if(n_iter <= MAX_TURBO_ITERATIONS) {
      if(args->verbose > 1)
        printf("Successfully decoded at MR %d\n", k);
      context->hop1_decoded_pdsch[k] = true;

      results->tx[pdu].hop1.round[context->round_hop1].decoded |= (1 << k);

      // copy received data to intermediate buffer
      memcpy(context->mr_buffer[k], dlsch_ue[0]->harq_processes[0]->b, context->pdu_size);
      memset(&context->mr_buffer[k][context->pdu_size], 0, DATA_BUFFER_SIZE-context->pdu_size);
    }
  }

  // Write debug signals if required
  if(args->debug_output) {
    if(context->round_hop1 == 0)
      write_output("hop1_e.m", "e", context->phy_vars_ch_src->dlsch_eNB_co[0][0]->e, n_coded_bits, 1, 4);
    snprintf(fnbuf, 80, "hop1_r%d_ch_txdataFv.m", context->round_hop1);
    snprintf(varbuf, 80, "hop1_r%d_ch_txdataF", context->round_hop1);
    write_output(fnbuf, varbuf, context->phy_vars_ch_src->lte_eNB_common_vars.txdataF[0][0],
        10*frame_parms->ofdm_symbol_size*2*n_symbols_per_slot, 1, 1);
    snprintf(fnbuf, 80, "hop1_r%d_ch_txdatav.m", context->round_hop1);
    snprintf(varbuf, 80, "hop1_r%d_ch_txdata", context->round_hop1);
    write_output(fnbuf, varbuf, txdata, 10*frame_parms->samples_per_tti, 1, 1);
    for(k = 0; k < args->n_relays; k++) {
      snprintf(fnbuf, 80, "hop1_r%d_mr%d_rxdatav.m", context->round_hop1, k);
      snprintf(varbuf, 80, "hop1_r%d_mr%d_rxdata", context->round_hop1, k);
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_common_vars[0]->rxdata[0], 
          10*frame_parms->samples_per_tti, 1, 1);
      snprintf(fnbuf, 80, "hop1_r%d_mr%d_rxdataFv.m", context->round_hop1, k);
      snprintf(varbuf, 80, "hop1_r%d_mr%d_rxdataF", context->round_hop1, k);
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_common_vars[0]->rxdataF[0],
          2*frame_parms->ofdm_symbol_size*2*n_symbols_per_slot, 2, 1);
      snprintf(fnbuf, 80, "hop1_r%d_mr%d_dl_ch_estimatesv.m", context->round_hop1, k);
      snprintf(varbuf, 80, "hop1_r%d_mr%d_dl_ch_estimates", context->round_hop1, k);
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_common_vars[0]->dl_ch_estimates[0][0],
          frame_parms->symbols_per_tti*frame_parms->ofdm_symbol_size, 1, 1);
      snprintf(fnbuf, 80, "hop1_r%d_mr%d_llrv.m", context->round_hop1, k);
      snprintf(varbuf, 80, "hop1_r%d_mr%d_llr", context->round_hop1, k);
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_pdsch_vars_co[0]->llr[0],
          n_coded_bits, 1, 0);
    }
  }
  context->round_hop1++;
  if(context->round_hop1 == args->n_harq)
    context->hop1_active = false;
}

void transmit_pusch_dci(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe) 
{
  LTE_DL_FRAME_PARMS* frame_parms = context->frame_parms;
  LTE_UE_ULSCH_t* ulsch_ue;
  LTE_eNB_ULSCH_t* ulsch_enb = context->phy_vars_ch_dest->ulsch_eNB_co[0];
  mod_sym_t** txdataF;
  s32** txdata;
  s32** rxdata = context->phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0];
  //s32** rxdataF = context->phy_vars_ch_dest->lte_eNB_common_vars.rxdataF[0];
  int n_re;
  DCI_ALLOC_t dci_ch;
  DCI_ALLOC_t dci_mr[MAX_RELAYS];
  u32 tx_energy;
  double tx_ampl;
  int k;
  double raw_ber;
  int i;
  int l;
  relay_bit_array_t active;
  int n_iter;
  int n_active_relays;
  bool accumulate_at_rx;
  int n_symbols_per_slot = frame_parms->symbols_per_tti>>1;
  int f;
  int s;

  // Temporary strings
  char fnbuf[80];
  char varbuf[80];

  determine_pusch_subframe(frame_parms, 0, subframe, &f, &s);

  // Create distributed DCI
  context->harq_pid_hop2 = subframe2harq_pid(frame_parms, 0, s);
  setup_distributed_dci(&dci_ch, context->rnti_hop2, context->round_hop2, context->mcs_hop2, context->n_prb_hop2);
  if(args->verbose > 2)
    dump_dci(frame_parms, &dci_ch);

  if(args->comm_mode == comm_mode_full) {
    // Transmit DCI
  }
  else {
    for(k = 0; k < args->n_relays; k++) {
      dci_mr[k] = dci_ch;
      context->hop2_received_pusch_dci[k] = context->hop2_received_bsr[k];
    }
  }

  // Generate eNB transport channel parameters
  generate_eNB_ulsch_params_from_dci(dci_ch.dci_pdu, context->rnti_hop2, subframe,
       format0, 0, context->phy_vars_ch_dest, context->phy_vars_ch_dest->ulsch_eNB_co[0],
       SI_RNTI, RA_RNTI, P_RNTI, 0);

  // Generate UE transport channel parameters
  for(k = 0; k < args->n_relays; k++) {
    if(context->hop2_received_pusch_dci[k]) {
      generate_ue_ulsch_params_from_dci(dci_mr[k].dci_pdu, context->rnti_hop2, subframe,
          format0, context->phy_vars_mr[k], context->phy_vars_mr[k]->ulsch_ue_co[0], 0,
          SI_RNTI, RA_RNTI, P_RNTI, 0, 0);
    }
  }
  //context->n_coded_bits_hop2 = 0; //get_ulsch_G(context->phy_vars_mr[k]->ulsch_ue[0], context.harq_pid_hop2);
}

void transmit_pusch(args_t* args, context_t* context, results_t* results, int pdu, u8 subframe) 
{
  LTE_DL_FRAME_PARMS* frame_parms = context->frame_parms;
  LTE_UE_ULSCH_t* ulsch_ue;
  LTE_eNB_ULSCH_t* ulsch_enb = context->phy_vars_ch_dest->ulsch_eNB_co[0];
  mod_sym_t** txdataF;
  s32** txdata;
  s32** rxdata = context->phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0];
  //s32** rxdataF = context->phy_vars_ch_dest->lte_eNB_common_vars.rxdataF[0];
  int n_re;
  u32 tx_energy;
  double tx_ampl;
  int k;
  double raw_ber;
  int i;
  int l;
  relay_bit_array_t active;
  int n_iter;
  int n_active_relays;
  bool accumulate_at_rx;
  int n_symbols_per_slot = frame_parms->symbols_per_tti>>1;

  // Temporary strings
  char fnbuf[80];
  char varbuf[80];

  // Fill results
  results->tx[pdu].hop2.round[context->round_hop2].mcs = (u8)context->mcs_hop2;
  results->tx[pdu].hop2.round[context->round_hop2].tbs = (u16)context->tbs_hop2;
  results->tx[pdu].hop2.round[context->round_hop2].n_prb = (u8)context->n_prb_hop2;
  active = 0;
  for(k = args->n_relays-1; k >= 0; k--)
    if(context->hop2_received_pusch_dci[k])
      active = (active << 1) + 1;
    else
      active = (active << 1);
  results->tx[pdu].hop2.round[context->round_hop2].active = active;
  results->tx[pdu].hop2.round[context->round_hop2].n_sent_bits = 0; //(u16)context->n_coded_bits_hop2;

  if(args->verbose > 1)
    printf("Hop 2, HARQ round %d\n", context->round_hop2);

  // Clear eNB receive vector
  memset(rxdata[0], 0, FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));

  // Determine how many relays are active for this transmission, split the total power between them
  n_active_relays = 0;
  for(k = 0; k < args->n_relays; k++)
    if(context->hop2_received_pusch_dci[k])
      n_active_relays++;

  // Normalization of received signal, fix this..
  accumulate_at_rx = false;

  for(k = 0; k < args->n_relays; k++) {
    if(!context->hop2_received_pusch_dci[k])
      continue;

    txdataF = context->phy_vars_mr[k]->lte_ue_common_vars[0]->txdataF;
    txdata = context->phy_vars_mr[k]->lte_ue_common_vars[0]->txdata;
    ulsch_ue = context->phy_vars_mr[k]->ulsch_ue_co[0];

    // Clear txdataF vector
    memset(txdataF[0], 0, FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));

    // Set relay role in Alamouti coding (this could be done better)
    if(context->relay_role[k] == RELAY_ROLE_STANDARD) {
      ulsch_ue->cooperation_flag = 0;
    }
    else {
      ulsch_ue->cooperation_flag = 2;
    }

    // Generate uplink reference signal
    generate_drs_pusch(context->phy_vars_mr[k], 0, AMP, subframe, 0, context->n_prb_hop2);

    // Encode ULSCH data
    if(ulsch_encoding(context->mr_buffer[k], frame_parms, ulsch_ue,
          context->harq_pid_hop2, 1, 0, 1) == -1) {
      printf("ulsch_encoding failed\n");
      exit(1);
    }

    // Modulate ULSCH data
    ulsch_modulation(txdataF, AMP, 0, subframe, frame_parms, ulsch_ue);

    // Compute number of resource elements from coded bits and modulation order
    n_re = 0; //context->n_coded_bits_hop2/get_Qm(mcs);
    if(args->verbose > 1)
      printf("Hop 2, MR%d, HARQ round %d: %d coded bits, Modulated %d REs\n", 
          k, context->round_hop2, 0 /* context->n_coded_bits_hop2 */, n_re);

    if(args->verbose > 2)
      print_ulsch_ue_stats(ulsch_ue);

    // OFDM modulation
    ofdm_modulation(txdataF, txdata, frame_parms, subframe, frame_parms->symbols_per_tti);

    tx_energy = signal_energy(&txdata[0][frame_parms->samples_per_tti*subframe], frame_parms->samples_per_tti);

    // Transmit over channel
    tx_ampl = awgn_stddev/sqrt((double)tx_energy)*pow(10.0, ((double)context->snr_hop2[k])/20.0)/sqrt((double)n_active_relays);
    transmit_subframe(context->channels_hop2[k], txdata, frame_parms,
        subframe, frame_parms->symbols_per_tti, tx_ampl, accumulate_at_rx);
    accumulate_at_rx = true;
  }

  // This is ugly. Fix it.
  deliver_subframe(context->channels_hop2[0], rxdata, frame_parms,
      subframe, frame_parms->symbols_per_tti, awgn_stddev);

  // Fill the last symbol of the frame with random data (used for SNR estimation?)
  for (i=0;i<OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
    ((short*) &context->phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0]
     [(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i] = 
      (short) ((awgn_stddev*0.707*gaussdouble(0.0,1.0)));
    ((short*) &context->phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0]
     [(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i+1] = 
      (short) ((awgn_stddev*0.707*gaussdouble(0.0,1.0)));
  }

  // Front end processing at destination CH
  for(l = 0; l < frame_parms->symbols_per_tti>>1; l++)
    slot_fep_ul(frame_parms, &context->phy_vars_ch_dest->lte_eNB_common_vars, l, 2*subframe, 0, 0);
  for(l = 0; l < frame_parms->symbols_per_tti>>1; l++)
    slot_fep_ul(frame_parms, &context->phy_vars_ch_dest->lte_eNB_common_vars, l, 2*subframe+1, 0, 0);

  // Receive ULSCH data
  rx_ulsch(context->phy_vars_ch_dest, subframe, 0, 0, &ulsch_enb, 2);

  // Compute uncoded bit error rate
  k = 0;
  while(!context->hop2_received_pusch_dci[k])
    k++;
  //results->tx[pdu].hop2.round[context->round_hop2].n_correct_bits = correct_bits_soft(
  //    context->phy_vars_mr[k]->ulsch_ue_co[0]->b_tilde,
  //    context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->llr, context->n_coded_bits_hop2);
  raw_ber = 0.0;//(double)(context->n_coded_bits_hop2-results->tx[pdu].hop2.round[context->round_hop2].n_correct_bits)/
  //  (double)context->n_coded_bits_hop2;
  if(args->verbose > 1) {
    //printf("Received %d bits at dest CH, raw BER: %f (%d/%d)\n", context->n_coded_bits_hop2, raw_ber, 
    //    context->n_coded_bits_hop2-results->tx[pdu].hop2.round[context->round_hop2].n_correct_bits, context->n_coded_bits_hop2);
  }

  // Decode ULSCH data
  n_iter = ulsch_decoding(context->phy_vars_ch_dest, CO_PUSCH, 0, subframe, 0, 1);

  if(args->verbose > 2)
    print_ulsch_eNB_stats(ulsch_enb);

  if(n_iter <= MAX_TURBO_ITERATIONS) {
    context->hop2_decoded_at_ch = true;
    context->hop2_active = false;
    context->hop2_finished = true;
    results->tx[pdu].hop2.round[context->round_hop2].decoded = 1;
  }
  else {
    results->tx[pdu].hop2.round[context->round_hop2].decoded = 0;
  }

  // Write debug output if requested
  if(args->debug_output) {
    for(k = 0; k < args->n_relays; k++) {
      snprintf(fnbuf, 80, "hop2_r%d_mr%d_txdataFv.m", context->round_hop2, k);
      snprintf(varbuf, 80, "hop2_r%d_mr%d_txdataF", context->round_hop2, k); 
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_common_vars[0]->txdataF[0],
          FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX, 1, 1);
      snprintf(fnbuf, 80, "hop2_r%d_mr%d_txdatav.m", context->round_hop2, k);
      snprintf(varbuf, 80, "hop2_r%d_mr%d_txdata", context->round_hop2, k);
      write_output(fnbuf, varbuf, context->phy_vars_mr[k]->lte_ue_common_vars[0]->txdata[0], 
          10*frame_parms->samples_per_tti, 1, 1);
    }
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdatav.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdata", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0], 
        10*frame_parms->samples_per_tti, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataFv.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_common_vars.rxdataF[0][0],
        20*frame_parms->ofdm_symbol_size*2*n_symbols_per_slot, 2, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_ext2v.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_ext2", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_ext2[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp_0.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp_0", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp_0[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp_1.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp_1", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp_1[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_drs_ch_estimates_0.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_drs_ch_estimates_0", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->drs_ch_estimates_0[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
    snprintf(fnbuf, 80, "hop2_r%d_ch_drs_ch_estimates_1.m", context->round_hop2);
    snprintf(varbuf, 80, "hop2_r%d_ch_drs_ch_estimates_1", context->round_hop2);
    write_output(fnbuf, varbuf, context->phy_vars_ch_dest->lte_eNB_pusch_vars[0]->drs_ch_estimates_1[0][0],
        12*context->phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
  }
  context->round_hop2++;
  if(context->round_hop2 == args->n_harq)
  {
    context->hop2_active = false;
    context->hop2_finished = true;
  }
}

void determine_pdu_size(args_t* args, context_t* context)
{
  int tbs_hop1;
  int tbs_hop2;

  switch(args->amc) {
    case amc_none:
    case amc_rbadjust:
      tbs_hop1 = get_TBS(args->mcs_hop1, args->n_prb_hop1);
      tbs_hop2 = get_TBS_UL(args->mcs_hop2, args->n_prb_hop2);
      break;
    default:
      break;
  }
  context->pdu_size = min(tbs_hop1, tbs_hop2);
}
      
void hop1_amc(args_t* args, context_t* context)
{
  switch(args->amc) {
    case amc_none:
      context->mcs_hop1 = args->mcs_hop1;
      context->n_prb_hop1 = args->n_prb_hop1;
      break;
    case amc_rbadjust:
      context->mcs_hop1 = args->mcs_hop1;
      context->n_prb_hop1 = args->n_prb_hop1;
      while(get_TBS(context->mcs_hop1, context->n_prb_hop1-1) > context->pdu_size)
        context->n_prb_hop1--;
      break;
    default:
      break;
  }
  context->tbs_hop1 = get_TBS(context->mcs_hop1, context->n_prb_hop1);
}

void hop2_amc(args_t* args, context_t* context)
{
  switch(args->amc) {
    case amc_none:
      context->mcs_hop2 = args->mcs_hop2;
      context->n_prb_hop2 = args->n_prb_hop2;
      break;
    case amc_rbadjust:
      context->mcs_hop2 = args->mcs_hop2;
      context->n_prb_hop2 = args->n_prb_hop2;
      while(get_TBS_UL(context->mcs_hop2, context->n_prb_hop2-1) > context->pdu_size)
        context->n_prb_hop2--;
      break;
    default:
      break;
  }
  context->tbs_hop2 = get_TBS_UL(context->mcs_hop2, context->n_prb_hop2);
}

int parse_args(int argc, char** argv, args_t* args)
{
  int c;
  int k;
  //bool snr_set;
  const struct option long_options[] = {
    {"info", no_argument, NULL, 300},
    {"corr", required_argument, NULL, 400},
    {"comm", required_argument, NULL, 401},
    {"perfect_ce", no_argument, NULL, 402},
    {"strategy", required_argument, NULL, 500},
    {"snr", required_argument, NULL, 600},
    {"snr1", required_argument, NULL, 601},
    {"snr2", required_argument, NULL, 602},
    {"single", no_argument, NULL, 700},
    {"snrsweep_a", no_argument, NULL, 701},
    {"snrsweep_b", no_argument, NULL, 702},
    {"snrsweep_c", no_argument, NULL, 703},
    {"range", required_argument, NULL, 710},
    {"step", required_argument, NULL, 711},
    {"mcs1", required_argument, NULL, 800},
    {"mcs2", required_argument, NULL, 801},
    {"rb1", required_argument, NULL, 810},
    {"rb2", required_argument, NULL, 811},
    {"amc", required_argument, NULL, 812},
    {"subframe", required_argument, NULL, 820},
    {NULL, 0, NULL, 0}};

  args->n_relays = 2;
  args->debug_output = false;
  args->verbose = 0;
  args->n_pdu = 1;
  args->n_harq = 4;
  args->comm_mode = comm_mode_only_pdu;
  args->perfect_channel_estimation = true;
  args->mcs_hop1 = 0;
  args->mcs_hop2 = 0;
  args->n_prb_hop1 = N_PRB;
  args->n_prb_hop2 = N_PRB;
  args->amc = amc_none;
  args->channel_model = AWGN;
  args->channel_correlation = 0.0;
  args->results_fn = 0;
  args->analysis = analysis_single;
  args->strategy = strategy_wait_one;
  args->range = 10.0;
  args->step = 1.0;
  args->subframe_pdsch = 4;

  for(k = 0; k < args->n_relays; k++)
    args->snr_hop1[k] = 10.0;
  for(k = 0; k < args->n_relays; k++)
    args->snr_hop2[k] = 10.0;

  //snr_set = false;
  while((c = getopt_long(argc, argv, "hvor:C:eN:n:H:S:s:m:", long_options, NULL)) != -1) {
    switch(c) {
      // General options:
    case 'h':
      return 1;
    case 300: // --info
      return 3;
    case 'v':
      args->verbose++;
      break;
    case 'o':
      args->debug_output = true;
      break;
    case 'r':
      args->results_fn = optarg;
      break;

      // Simulation model options:
    case 'C':
      if(strcmp(optarg, "help") == 0)
        return 2;
      if(!parse_channel_model(optarg, &args->channel_model))
        return 1;
      break;
    case 400: // --corr
      args->channel_correlation = atof(optarg);
      if(args->channel_correlation < 0.0 || args->channel_correlation > 1.0)
        return 1;
      break;
    case 401: // --comm
      if(strcmp(optarg, "pdu") == 0)
        args->comm_mode = comm_mode_only_pdu;
      else if(strcmp(optarg, "full") == 0)
        args->comm_mode = comm_mode_full;
      else
        return 1;
      break;
    case 402: // --perfect_ce
      args->perfect_channel_estimation = true;
      break;

      // Simulation scenario:
    case 'N':
      args->n_relays = atoi(optarg);
      if(args->n_relays <= 0)
        return 1;
      break;
    case 'n':
      args->n_pdu = atoi(optarg);
      if(args->n_pdu <= 0)
        return 1;
      break;
    case 'H':
      args->n_harq = atoi(optarg);
      break;
    case 'S':
    case 500: // --strategy
      switch(atoi(optarg)) {
        case 1:
          args->strategy = strategy_wait_all;
          break;
        case 2:
          args->strategy = strategy_wait_one;
          break;
        default:
          return 1;
      }
      break;

      // SNR options:
    case 's':
    case 600: // --snr
      for(k = 0; k < args->n_relays; k++)
        args->snr_hop1[k] = atof(optarg);
      for(k = 0; k < args->n_relays; k++)
        args->snr_hop2[k] = atof(optarg);
      //snr_set = true;
      break;
    case 601: // --snr1
      if(!parse_snr(optarg, args->snr_hop1, args->n_relays))
        return 1;
      //snr_set = true;
      break;
    case 602: // --snr2
      if(!parse_snr(optarg, args->snr_hop2, args->n_relays))
        return 1;
      //snr_set = true;
      break;

      // Analysis setup:
    case 700: // --single
      args->analysis = analysis_single;
      break;
    case 701: // --snrsweep_a
      args->analysis = analysis_snrsweep_a;
      break;
    case 702: // --snrsweep_b
      args->analysis = analysis_snrsweep_b;
      break;
    case 703: // --snrsweep_c
      args->analysis = analysis_snrsweep_c;
      break;
    case 710: // --range
      args->range = atof(optarg);
      if(args->range <= 0.0)
        return 1;
      break;
    case 711: // --step
      args->step = atof(optarg);
      if(args->step <= 0.0)
        return 1;
      break;

      // Link and resource parameters:
    case 'm':
      args->mcs_hop1 = args->mcs_hop2 = atoi(optarg);
      break;
    case 800: // --mcs1
      args->mcs_hop1 = atoi(optarg);
      break;
    case 801: // --mcs2
      args->mcs_hop2 = atoi(optarg);
      break;
    case 810: // --rb1
      args->n_prb_hop1 = atoi(optarg);
      if(args->n_prb_hop1 <= 0 || args->n_prb_hop1 > N_PRB)
        return 1;
      break;
    case 811: // --rb2
      args->n_prb_hop2 = atoi(optarg);
      if(args->n_prb_hop2 <= 0 || args->n_prb_hop2 > N_PRB)
        return 1;
      break;
    case 812: // --amc
      if(strcmp(optarg, "none") == 0)
        args->amc = amc_none;
      else if(strcmp(optarg, "rbadjust") == 0)
        args->amc = amc_rbadjust;
      else
        return 1;
      break;
    case 820: // --subframe
      args->subframe_pdsch = atoi(optarg);
      if(args->subframe_pdsch < 0 || args->subframe_pdsch > 10)
        return 1;
      break;
    default:
      return 1;
    }
  }

  return 0;
}

bool parse_snr(const char* str, double* snr, int n)
{
  char* p;
  int k;

  for(k = 0; k < n; k++) {
    snr[k] = strtod(str, &p);
    if(p == str)
      break;
    str = p;
  }

  if(k == 0)
    for(k = 1; k < n; k++)
      snr[k] = snr[0];
  else if(k < n-1)
    return false;
  return true;
}

int parse_channel_model(const char* str, SCM_t* model)
{
  if(strcmp(str, "0") == 0) *model = AWGN;
  else if(strcmp(str, "A") == 0) *model = SCM_A;
  else if(strcmp(str, "B") == 0) *model = SCM_B;
  else if(strcmp(str, "C") == 0) *model = SCM_C;
  else if(strcmp(str, "D") == 0) *model = SCM_D;
  else if(strcmp(str, "E") == 0) *model = EPA;
  else if(strcmp(str, "F") == 0) *model = EVA;
  else if(strcmp(str, "G") == 0) *model = ETU;
  else if(strcmp(str, "H") == 0) *model = Rayleigh8;
  else if(strcmp(str, "I") == 0) *model = Rayleigh1;
  else if(strcmp(str, "J") == 0) *model = Rayleigh1_corr;
  else if(strcmp(str, "K") == 0) *model = Rayleigh1_anticorr;
  else if(strcmp(str, "L") == 0) *model = Rice8;
  else if(strcmp(str, "M") == 0) *model = Rice1;
  else return false;
  return true;
}

void print_usage(const char* prog)
{
  printf("Usage: %s [options]\n", prog);
  printf("\n");
  printf("    General options:\n");
  printf("  -h         : print usage\n");
  printf("  --info     : print information about what the program does\n");
  printf("  -v         : increase verbosity level [0]\n");
  printf("  -o         : output MATLAB signal files (implies -n 1) [no]\n");
  printf("  -r FILE    : write results to FILE\n");
  printf("\n");
  printf("    Simulation model options:\n");
  printf("  -C CHANNEL : set the channel model, use -C help for available models [AWGN]\n");
  printf("  --corr CORR : set channel realization correlation (0.0 .. 1.0) [0.0]\n");
  printf("  --comm MODE : set communication mode (full: include control, pdu: only PDSCH/PUSCH) [pdu]\n");
  printf("\n");
  printf("    Simulation scenario:\n");
  printf("  -N NRELAYS : simulate using NRELAYS relays [2]\n");
  printf("  -n NUM     : simulate NUM MAC PDUs [1]\n");
  printf("  -H NUM     : do NUM HARQ rounds in each hop [4]\n");
  //printf("     note: the hop 1 RVs are 0,0,1,1,2,2,3,3,0,0,..., the hop 2 RVs are 0,2,3,1,...\n");
  printf("  -S X, --strategy X : set the HARQ strategy to X [2]\n");
  printf("     1: decode at all relays before starting hop 2\n");
  printf("     2: start hop 2 when at least one relay has decoded\n");
  printf("\n");
  printf("    SNR options:\n");
  printf("  -s SNR, --snr SNR  : set snr for all links to SNR [10.0]\n");
  printf("  --snr1 SNR : set snr for hop 1 to SNR\n");
  printf("  --snr2 SNR : set snr for hop 2 to SNR\n");
  printf("      for --snr1 and --snr2, SNR may be either a single value or a vector with n_relays elements\n");
  printf("      e.g.: --snr1 \"4.0 6.0\" sets the SNR from CH1 to MR1 and MR2 to 4.0 and 6.0, respectively\n");
  printf("\n");
  printf("    Analysis setup:\n");
  printf("  --single : single point analysis [default]\n");
  printf("  --snrsweep_a : sweep snr of first relay of both hops [-RANGE*STEP..+RANGE*STEP]\n");
  printf("  --snrsweep_b : sweep first relay of hop 1 [-RANGE*STEP..+RANGE*STEP],\n");
  printf("                 sweep first relay of hop 2 [+RANGE*STEP..-RANGE*STEP]\n");
  printf("  --snrsweep_c : sweep snr of all relays of both hops [-RANGE*STEP..+RANGE*STEP]\n");
  printf("  --range RANGE : set sweep range [10]\n");
  printf("  --step STEP   : set sweep step size [1.0]\n");
  printf("      note: the swept range is relative to SNR specified with --snr* options\n");
  printf("\n");
  printf("    Link and resource parameters:\n");
  printf("  -m MCS     : set mcs for both hops to MCS [0]\n");
  printf("  --mcs1 MCS : set mcs for hop 1 to MCS\n");
  printf("  --mcs2 MCS : set mcs for hop 2 to MCS\n");
  printf("  --rb1 NUM  : set number of resource blocks for hop 1 [%d]\n", N_PRB);
  printf("  --rb2 NUM  : set number of resource blocks for hop 2 [%d]\n", N_PRB);
  printf("  --amc AMC  : set the amc strategy [none]\n");
  printf("     none:     use fixed mcs and rb allocations\n");
  printf("     rbadjust: use fixed mcs, adjust the hop bandwidths to have similar TBS\n");
  printf("  --subframe X : do PDSCH transmissions in subframe X [4]\n");
}

void print_channel_usage()
{
  printf("Available channel models:\n");
  printf("  0: AWGN\n");
  printf("  A: SCM-A\n");
  printf("  B: SCM-B\n");
  printf("  C: SCM-C\n");
  printf("  D: SCM-D\n");
  printf("  E: EPA\n");
  printf("  F: EVA\n");
  printf("  G: ETU\n");
  printf("  H: Rayleigh8\n");
  printf("  I: Rayleigh1\n");
  printf("  J: Rayleigh1_corr\n");
  printf("  K: Rayleigh1_anticorr\n");
  printf("  L: Rice8\n");
  printf("  M: Rice1\n");
}

void print_info()
{
  printf(
      "Definition of results:\n"
      "\n"
      "BER is the bit error rate of the raw (coded) bits sent over the channel.\n"
      "The BER is presented individually for each relay in the broadcast phase,\n"
      "and is computed on all HARQ transmissions until the MAC PDU is decoded by\n"
      "the relay.\n"
      "\n"
      "The HARQ statistics for hop 1 present, for each HARQ retransmission, the\n"
      "number of transmission attempts from CH0 and the number of successfully\n"
      "decoded MAC PDUs for each relay. Each MAC PDU is only counted once for\n"
      "each relay, and is ignored if it is retransmitted for another relay.\n"
      "\n"
      "The average throughput is computed assuming that transmissions of\n"
      "sequential MAC PDUs are overlapped, i.e., as soon as all relays have\n"
      "decoded a MAC PDU from CH0, transmission of the next MAC PDU can start.\n"
      "Thus, the average throughput is computed as\n"
      "n_total_bits_transmitted/max(n_tx_hop1,n_tx_hop2), where n_tx_hop[12] is\n"
      "the sum of the number of HARQ transmissions for all MAC PDUs. The\n"
      "throughput is computed based on the use of only one subframe in each\n"
      "frame.\n"
      "\n"
      "The spectral efficiency is the number of bits transmitted divided by the\n"
      "used spectral resources. These resources include the PDCCH and PDSCH of\n"
      "the first hop and PUSCH of the second hop, but does not include the\n"
      "PDCCH scheduling the PUSCH, nor any of the HARQ ACK/NACK resources.\n"
      "\n"
      "The latency of a MAC PDU is defined as the number of subframes from the\n"
      "first HARQ transmission from CH0 to when the MAC PDU is successfully\n"
      "decoded at CH1.\n");
}

void signal_handler(int sig) 
{
  void *array[10];
  size_t size;

  size = backtrace(array, 10);
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

void setup_single(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays)
{
  int k;

  *snrs = malloc(2*n_relays*sizeof(double));

  for(k = 0; k < n_relays; k++)
    (*snrs)[k] = snr_hop1[k];
  for(k = 0; k < n_relays; k++)
    (*snrs)[k+n_relays] = snr_hop2[k];

  *n_tests = 1;
}

void setup_snrsweep_a(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end)
{
  int l;
  int k;
  *n_tests = end-start+1;

  *snrs = malloc((*n_tests)*2*n_relays*sizeof(double));
  for(l = 0; l < *n_tests; l++) {
    (*snrs)[2*n_relays*l] = snr_hop1[0] + step*(start+l);
    (*snrs)[2*n_relays*l + n_relays] = snr_hop2[0] + step*(start+l);
    for(k = 1; k < n_relays; k++) {
      (*snrs)[2*n_relays*l + k] = snr_hop1[k];
      (*snrs)[2*n_relays*l + n_relays + k] = snr_hop2[k];
    }
  }
}

void setup_snrsweep_b(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end)
{
  int l;
  int k;
  *n_tests = end-start+1;

  *snrs = malloc((*n_tests)*2*n_relays*sizeof(double));
  for(l = 0; l < *n_tests; l++) {
    (*snrs)[2*n_relays*l] = snr_hop1[0] + step*(start+l);
    (*snrs)[2*n_relays*l + n_relays] = snr_hop2[0] + step*(end-l);
    for(k = 1; k < n_relays; k++) {
      (*snrs)[2*n_relays*l + k] = snr_hop1[k];
      (*snrs)[2*n_relays*l + n_relays + k] = snr_hop2[k];
    }
  }
}

void setup_snrsweep_c(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end)
{
  int l;
  int k;
  *n_tests = end-start+1;

  *snrs = malloc((*n_tests)*2*n_relays*sizeof(double));
  for(l = 0; l < *n_tests; l++) {
    for(k = 0; k < n_relays; k++) {
      (*snrs)[2*n_relays*l + k] = snr_hop1[k] + step*(start+l);
      (*snrs)[2*n_relays*l + n_relays + k] = snr_hop2[k] + step*(start+l);
    }
  }
}

LTE_DL_FRAME_PARMS* alloc_frame_params(void)
{
  LTE_DL_FRAME_PARMS* frame_parms = malloc16(sizeof(LTE_DL_FRAME_PARMS));

  memset(frame_parms, 0, sizeof(LTE_DL_FRAME_PARMS));

  frame_parms->N_RB_DL = N_PRB;
  frame_parms->N_RB_UL = N_PRB;
  frame_parms->Nid_cell = NID_CELL;
  frame_parms->Ncp = cp_type;
  frame_parms->Ncp_UL = cp_type;
  frame_parms->nushift = 0;
  frame_parms->frame_type = 1; // TDD frames
  frame_parms->tdd_config = 1; // TDD frame type 1
  frame_parms->mode1_flag = 1;
  frame_parms->nb_antennas_tx = n_antenna_ch;
  frame_parms->nb_antennas_rx = n_antenna_mr;
  frame_parms->nb_antennas_tx_eNB = n_antenna_ch;

  init_frame_parms(frame_parms, 1);
  phy_init_top(frame_parms);
  phy_init_lte_top(frame_parms);

  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;
  init_ul_hopping(frame_parms);
  //dump_frame_parms(frame_parms);
  return frame_parms;
}

void free_frame_params(LTE_DL_FRAME_PARMS* frame_parms)
{
  free(frame_parms);
}

PHY_VARS_eNB* alloc_ch_vars(LTE_DL_FRAME_PARMS* frame_parms, int id)
{
  PHY_VARS_eNB* phy_vars = malloc16(sizeof(PHY_VARS_eNB));

  memset(phy_vars, 0, sizeof(PHY_VARS_eNB));

  phy_vars->Mod_id = id;
  phy_vars->lte_frame_parms = *frame_parms;
  phy_vars->frame = 1;
  if(id == 0)
    phy_init_lte_eNB(phy_vars, 0, 0, 0);
  else if(id == 1) {
    phy_init_lte_eNB(phy_vars, 0, 2, 0);
    phy_vars->transmission_mode[0] = 2;
    phy_vars->pucch_config_dedicated[0].tdd_AckNackFeedbackMode = bundling;
    phy_vars->pusch_config_dedicated[0].betaOffset_ACK_Index = 0;
    phy_vars->pusch_config_dedicated[0].betaOffset_RI_Index  = 0;
    phy_vars->pusch_config_dedicated[0].betaOffset_CQI_Index = 2;
  }
  else
    return 0;

  return phy_vars;
}

void free_ch_vars(PHY_VARS_eNB* phy_vars)
{
  free(phy_vars);
}

PHY_VARS_UE* alloc_mr_vars(LTE_DL_FRAME_PARMS* frame_parms, int id)
{
  PHY_VARS_UE* phy_vars = malloc16(sizeof(PHY_VARS_UE));

  memset(phy_vars, 0, sizeof(PHY_VARS_UE));

  phy_vars->lte_frame_parms[0] = frame_parms;
  phy_vars->frame = 1;
  phy_vars->n_connected_eNB = 2;
  lte_gold(frame_parms, phy_vars->lte_gold_table[0], 0);
  lte_gold(frame_parms, phy_vars->lte_gold_table[1], 1);
  lte_gold(frame_parms, phy_vars->lte_gold_table[2], 2);

  phy_vars->lte_ue_common_vars[0] = malloc16(sizeof(LTE_UE_COMMON));
  memset(phy_vars->lte_ue_common_vars[0], 0, sizeof(LTE_UE_COMMON));
  phy_init_lte_ue_common(phy_vars, 0, 0);

  phy_init_lte_ue(phy_vars, 0, 1);
  phy_vars->pucch_config_dedicated[0].tdd_AckNackFeedbackMode = bundling;
  phy_vars->pusch_config_dedicated[0].betaOffset_ACK_Index = 0;
  phy_vars->pusch_config_dedicated[0].betaOffset_RI_Index  = 0;
  phy_vars->pusch_config_dedicated[0].betaOffset_CQI_Index = 2;

  return phy_vars;
}

void free_mr_vars(PHY_VARS_UE* phy_vars)
{
  free(phy_vars->lte_ue_common_vars[0]);
  free(phy_vars);
}

void alloc_dlsch_co(PHY_VARS_eNB* phy_vars_ch_src, PHY_VARS_UE** phy_vars_mr, PHY_VARS_eNB* phy_vars_ch_dest, int n_relays)
{
  int k;
  int l;

  phy_vars_ch_src->dlsch_eNB_co[0][0] = new_eNB_dlsch(1, MAX_HARQ_ROUNDS, 0);

  for(k = 0; k < n_relays; k++) {
    phy_vars_mr[k]->dlsch_ue_co[0][0] = new_ue_dlsch(1, MAX_HARQ_ROUNDS, 0);
    phy_vars_mr[k]->dlsch_ue_co[0][0]->mode1_flag = 0;
    memset(phy_vars_mr[k]->dlsch_ue_co[0][0]->rb_alloc, 0, 16);

    phy_vars_mr[k]->ulsch_ue_co[0] = new_ue_ulsch(MAX_HARQ_ROUNDS, 0);
    phy_vars_mr[k]->ulsch_ue_co[0]->o_ACK[0] = 0;
    phy_vars_mr[k]->ulsch_ue_co[0]->o_ACK[1] = 0;
    phy_vars_mr[k]->ulsch_ue_co[0]->o_ACK[2] = 0;
    phy_vars_mr[k]->ulsch_ue_co[0]->o_ACK[3] = 0;
    for(l = 0; l < MAX_HARQ_ROUNDS; l++)
      if(phy_vars_mr[k]->ulsch_ue_co[0]->harq_processes[l]) {
        phy_vars_mr[k]->ulsch_ue_co[0]->harq_processes[l]->status = DISABLED;
        phy_vars_mr[k]->ulsch_ue_co[0]->harq_processes[l]->B = 0;
      }
  }
  phy_vars_ch_dest->ulsch_eNB_co[0] = new_eNB_ulsch(MAX_HARQ_ROUNDS, 0);
}

void free_dlsch_co(PHY_VARS_eNB* phy_vars_ch_src, PHY_VARS_UE** phy_vars_mr, PHY_VARS_eNB* phy_vars_ch_dest, int n_relays)
{
  int k;

  free_eNB_dlsch(phy_vars_ch_src->dlsch_eNB[0][0]);
  for(k = 0; k < n_relays; k++) {
    free_ue_dlsch(phy_vars_mr[k]->dlsch_ue[0][0]);
    free_ue_ulsch(phy_vars_mr[k]->ulsch_ue[0]);
  }
  free_eNB_ulsch(phy_vars_ch_dest->ulsch_eNB[0]);
}

u16 rballoc_type0(int n_rb, int rbg_size)
{
  int rb = 0;
  int k;

  for(k = 0; k < n_rb; k += rbg_size)
    rb = (rb << 1) + 1;
  return rb;
}

void setup_broadcast_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs, int n_rb)
{
  DCI1_5MHz_TDD_t* dci_data = (DCI1_5MHz_TDD_t*) dci->dci_pdu;

  memset(dci, 0, sizeof(DCI_ALLOC_t));

  dci_data->dai = 1;
  dci_data->TPC = 0;
  dci_data->rv = (harq_round >> 1) & 0x03;
  dci_data->ndi = (harq_round == 0 ? 1 : 0);
  dci_data->harq_pid = 0;
  dci_data->mcs = mcs;
  dci_data->rballoc = rballoc_type0(n_rb, RBG_SIZE);
  dci_data->rah = 0;

  dci->dci_length = sizeof_DCI1_5MHz_TDD_t;
  dci->L = 1;
  dci->rnti = rnti;
  dci->format = format1;
}

void setup_distributed_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs, int n_rb)
{
  DCI0_5MHz_TDD_1_6_t* dci_data = (DCI0_5MHz_TDD_1_6_t*) dci->dci_pdu;

  memset(dci, 0, sizeof(DCI_ALLOC_t));

  dci_data->cqi_req = 0;
  dci_data->dai = 1;
  dci_data->cshift = 0;
  dci_data->TPC = 0;
  if(harq_round == 0) {
    dci_data->ndi = 1;
    dci_data->mcs = mcs;
  } else {
    dci_data->ndi = 0;
    switch(harq_round % 4) {
      case 0:
        dci_data->mcs = mcs;
        break;
      case 1:
	//        dci_data->mcs = 30;
        dci_data->mcs = mcs;
        break;
      case 2:
	//        dci_data->mcs = 31;
        dci_data->mcs = 29;
        break;
      case 3:
        dci_data->mcs = 29;
        break;
    }
  }
  dci_data->rballoc = computeRIV(N_PRB,0,n_rb);
  dci_data->hopping = 0;
  dci_data->type = 0;

  dci->dci_length = sizeof_DCI0_5MHz_TDD_1_6_t;
  dci->L = 1;
  dci->rnti = rnti;
  dci->format = format0;
}

void ofdm_modulation(mod_sym_t** tx_f, s32** tx_t, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb)
{
  mod_sym_t* src;
  s32* dst;

  if(frame_parms->Ncp == 0) { // Normal prefix
    src = &tx_f[0][subframe*14*frame_parms->ofdm_symbol_size];
    dst = &tx_t[0][subframe*frame_parms->samples_per_tti];
    normal_prefix_mod(src, dst, nsymb, frame_parms);
  } else { // Extended prefix
    src = &tx_f[0][subframe*12*frame_parms->ofdm_symbol_size];
    dst = &tx_t[0][subframe*frame_parms->samples_per_tti];
    PHY_ofdm_mod(src, dst, frame_parms->log2_symbol_size,
        nsymb, frame_parms->nb_prefix_samples, frame_parms->twiddle_ifft, 
        frame_parms->rev, CYCLIC_PREFIX);
  }
}

channel_vars_t alloc_channel_vars(LTE_DL_FRAME_PARMS* frame_parms)
{
  channel_vars_t v;
  v.s_re[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  v.s_im[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  v.r_re[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  v.r_im[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  v.r_re_t[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  v.r_im_t[0] = malloc(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(double));
  return v;
}

void free_channel_vars(channel_vars_t v)
{
  free(v.s_re[0]);
  free(v.s_im[0]);
  free(v.r_re[0]);
  free(v.r_im[0]);
  free(v.r_re_t[0]);
  free(v.r_im_t[0]);
}

sh_channel_t* alloc_sh_channel(channel_vars_t* cvars, SCM_t channel_model, int n_txantennas, int n_rxantennas,
    double channel_correlation)
{
  sh_channel_t* ch = malloc(sizeof(sh_channel_t));

  ch->cvars = cvars;
  ch->channel = new_channel_desc_scm(n_txantennas, n_rxantennas, channel_model, BW, channel_correlation, 0, 0.0);

  return ch;
}

void free_sh_channel(sh_channel_t* c)
{
  free(c->channel);
}

void transmit_subframe(sh_channel_t* channel, s32** src, LTE_DL_FRAME_PARMS* frame_parms, 
    u8 subframe, u8 nsymb, double ampl, bool accumulate)
{
  int k;
  int symbols_per_slot = (frame_parms->Ncp == 0 ? 7 : 6);
  int nsamples = 0;

  for(k = 0; k < nsymb; k++) {
    if(k % symbols_per_slot == 0)
      nsamples += frame_parms->nb_prefix_samples0;
    else
      nsamples += frame_parms->nb_prefix_samples;
    nsamples += frame_parms->ofdm_symbol_size;
  }

  for(k = 0; k < nsamples; k++) {
    channel->cvars->s_re[0][k] = (double)((s16*)src[0])[2*subframe*frame_parms->samples_per_tti + (k<<1)];
    channel->cvars->s_im[0][k] = (double)((s16*)src[0])[2*subframe*frame_parms->samples_per_tti + (k<<1) + 1];
  }

  if(accumulate) {
    multipath_channel(channel->channel, channel->cvars->s_re, channel->cvars->s_im,
        channel->cvars->r_re_t, channel->cvars->r_im_t, nsamples, 0);
    for(k = 0; k < nsamples; k++) {
      channel->cvars->r_re[0][k] += channel->cvars->r_re_t[0][k] * ampl;
      channel->cvars->r_im[0][k] += channel->cvars->r_im_t[0][k] * ampl;
    }
  } else {
    multipath_channel(channel->channel, channel->cvars->s_re, channel->cvars->s_im,
        channel->cvars->r_re, channel->cvars->r_im, nsamples, 0);
    for(k = 0; k < nsamples; k++) {
      channel->cvars->r_re[0][k] *= ampl;
      channel->cvars->r_im[0][k] *= ampl;
    }
  }
}

void deliver_subframe(sh_channel_t* channel, s32** dst, LTE_DL_FRAME_PARMS* frame_parms,
    u8 subframe, u8 nsymb, double stddev)
{
  int k;
  int symbols_per_slot = (frame_parms->Ncp == 0 ? 7 : 6);
  int nsamples = 0;

  for(k = 0; k < nsymb; k++) {
    if(k % symbols_per_slot == 0)
      nsamples += frame_parms->nb_prefix_samples0;
    else
      nsamples += frame_parms->nb_prefix_samples;
    nsamples += frame_parms->ofdm_symbol_size;
  }

  for(k = 0; k < nsamples; k++) {
    ((s16*)dst[0])[2*subframe*frame_parms->samples_per_tti + (k<<1)] = 
      (s16) (channel->cvars->r_re[0][k] + stddev*0.707*gaussdouble(0.0, 1.0));
    ((s16*)dst[0])[2*subframe*frame_parms->samples_per_tti + (k<<1) + 1] = 
      (s16) (channel->cvars->r_im[0][k] + stddev*0.707*gaussdouble(0.0, 1.0));
  }
}

void ofdm_fep(PHY_VARS_UE* phy_vars_mr, u8 subframe)
{
  int n_symbols_per_slot = (phy_vars_mr->lte_frame_parms[0]->Ncp == 0 ? 7 : 6);
  int slot;
  int symbol;

  for(slot = 2*subframe; slot < 2*subframe+2; slot++)
    for(symbol = 0; symbol < n_symbols_per_slot; symbol++)
      slot_fep(phy_vars_mr, symbol, slot, 0, 0);
  slot_fep(phy_vars_mr, 0, 2*subframe+2, 0, 0);
}

void slot_fep_est(args_t* args, PHY_VARS_UE* phy_vars, int l, int slot)
{
  LTE_DL_FRAME_PARMS* frame_parms = phy_vars->lte_frame_parms[0];
  int* sym;
  int i;

  slot_fep(phy_vars, l, slot, 0, 0);

  if(args->perfect_channel_estimation) {
    for(i = 0; i < frame_parms->N_RB_DL*12; i++) {
      sym = &phy_vars->lte_ue_common_vars[0]->dl_ch_estimates[0][0][l*frame_parms->ofdm_symbol_size+LTE_CE_FILTER_LENGTH+i];
      ((short*)sym)[0] = AMP/2;
      ((short*)sym)[1] = 0;
    }
  }
}

int rx_dlsch_co(args_t* args, PHY_VARS_UE* phy_vars, u8 subframe, u8 first_symbol)
{
  LTE_DL_FRAME_PARMS* frame_parms = phy_vars->lte_frame_parms[0];
  int n_symbols_per_slot = frame_parms->symbols_per_tti >> 1;
  int pilot1_symbol = (cp_type == 0 ? 4 : 3);
  int l;

  // Front end processor up to second pilot
  for(l = first_symbol+1; l < n_symbols_per_slot; l++)
    slot_fep_est(args, phy_vars, l, subframe<<1);
  slot_fep_est(args, phy_vars, 0, (subframe<<1)+1);

  // Receive PDSCH for first slot
  if(rx_pdsch(phy_vars, CO_PDSCH, 0, 0, subframe, n_pdcch_symbols, 1, 0, 0) == -1) {
    printf("DLSCH receiver error\n");
    return -1;
  }
  for(l = n_pdcch_symbols + 1; l < n_symbols_per_slot; l++) {
    if(rx_pdsch(phy_vars, CO_PDSCH, 0, 0, subframe, l, 0, 0, 0) == -1) {
      printf("DLSCH receiver error\n");
      return -1;
    }
  }

  // Front end processor up to third pilot
  for(l = 1; l <= pilot1_symbol; l++)
    slot_fep_est(args, phy_vars, l, (subframe<<1)+1);
  
  // Receive DLSCH up to third pilot
  for(l = n_symbols_per_slot; l < n_symbols_per_slot+pilot1_symbol; l++) {
    if(rx_pdsch(phy_vars, CO_PDSCH, 0, 0, subframe, l, 0, 0, 0) == -1) {
      printf("DLSCH receiver error\n");
      return -1;
    }
  }

  // Front end processor for rest of subframe
  for(l = pilot1_symbol+1; l < n_symbols_per_slot; l++)
    slot_fep_est(args, phy_vars, l, (subframe<<1)+1);
  slot_fep_est(args, phy_vars, 0, (subframe<<1)+2);

  // Receive DLSCH for rest of subframe
  for(l = n_symbols_per_slot+pilot1_symbol; l < n_symbols_per_slot<<1; l++) {
    if(rx_pdsch(phy_vars, CO_PDSCH, 0, 0, subframe, l, 0, 0, 0) == -1) {
      printf("DLSCH receiver error\n");
      return -1;
    }
  }

  return 0;
}

u32 get_ulsch_G(LTE_UE_ULSCH_t *ulsch, u8 harq_pid)
{
  u8 Q_m = 0;
  u32 Kr = 0;
  int r;
  u32 sumKr = 0;
  u32 Qprime;
  u32 L;
  u32 G;
  u32 Q_CQI = 0;
  u32 Q_RI = 0;

  Q_m = get_Qm(ulsch->harq_processes[harq_pid]->mcs);

  sumKr = 0;
  for (r=0;r<ulsch->harq_processes[harq_pid]->C;r++) {
    if (r<ulsch->harq_processes[harq_pid]->Cminus)
      Kr = ulsch->harq_processes[harq_pid]->Kminus;
    else
      Kr = ulsch->harq_processes[harq_pid]->Kplus;
    sumKr += Kr;
  }

  Qprime = ulsch->O_RI * ulsch->harq_processes[harq_pid]->Msc_initial * 
    ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_ri_times8;

  if (Qprime > 0) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
    
    if (Qprime > 4*ulsch->harq_processes[harq_pid]->nb_rb * 12)
      Qprime = 4*ulsch->harq_processes[harq_pid]->nb_rb * 12;
  }

  Q_RI = Q_m*Qprime;

  if (ulsch->O < 12)
    L=0;
  else 
    L=8;

  Qprime = (ulsch->O + L) * ulsch->harq_processes[harq_pid]->Msc_initial * 
    ulsch->harq_processes[harq_pid]->Nsymb_initial * ulsch->beta_offset_cqi_times8;

  if (Qprime > 0) {
    if ((Qprime % (8*sumKr)) > 0)
      Qprime = 1+(Qprime/(8*sumKr));
    else
      Qprime = Qprime/(8*sumKr);
  }
    
  G = ulsch->harq_processes[harq_pid]->nb_rb * (12 * Q_m) * (ulsch->Nsymb_pusch);

  if (Qprime > (G - ulsch->O_RI))
    Qprime = G - ulsch->O_RI;
  Q_CQI = Q_m * Qprime;
  
  G = G - Q_RI - Q_CQI;
  
  return G;
}

int correct_bits_soft(u8* ref, s16* rec, int n)
{
  int k;
  int e = 0;

  for(k = 0; k < n; k++) {
    if((ref[k]==1) == (rec[k]<0)) {
      e++;
    }
  }

  return e;
}

void print_dlsch_eNB_stats(LTE_eNB_DLSCH_t* d)
{
  int k;
  LTE_DL_eNB_HARQ_t* h;
  if(d)
  {
    printf("eNB dlsch: rnti=%04x, active=%d, current_harq_pid=%d, rb_alloc=%08x %08x %08x %08x, nb_rb=%d, G=%d, layer_index=%d, codebook_index=%d, Mdlharq=%d, Kmimo=%d\n",
        d->rnti, d->active, d->current_harq_pid, 
        d->rb_alloc[0], d->rb_alloc[1], d->rb_alloc[2], d->rb_alloc[3], d->nb_rb, d->G,
        d->layer_index, d->codebook_index, d->Mdlharq, d->Kmimo);
    for(k = 0; k < 8; k++) {
      if(d->harq_processes[k]) {
        h = d->harq_processes[k];
        if(h->status == ACTIVE) {
          printf("HARQ process %d: Ndi=%d, status=%d, TBS=%d, B=%d, round=%d, mcs=%d, rvidx=%d, Nl=%d\n",
              k, h->Ndi, h->status, h->TBS, h->B, h->round, h->mcs, h->rvidx, h->Nl);
        }
      }
    }
  }
}

void print_dlsch_ue_stats(LTE_UE_DLSCH_t* d)
{
  int k;
  LTE_DL_UE_HARQ_t* h;
  if(d)
  {
    printf("UE dlsch: rnti=%04x, active=%d, mode1_flag=%d, current_harq_pid=%d, rb_alloc=%08x %08x %08x %08x, nb_rb=%d, G=%d, layer_index=%d, Mdlharq=%d, Kmimo=%d\n",
        d->rnti, d->active, d->mode1_flag, d->current_harq_pid, 
        d->rb_alloc[0], d->rb_alloc[1], d->rb_alloc[2], d->rb_alloc[3], d->nb_rb, d->G,
        d->layer_index, d->Mdlharq, d->Kmimo);
    for(k = 0; k < 8; k++) {
      if(d->harq_processes[k]) {
        h = d->harq_processes[k];
        if(h->status == ACTIVE || h->TBS > 0) {
          printf("HARQ process %d: Ndi=%d, status=%d, TBS=%d, B=%d, round=%d, mcs=%d, rvidx=%d, Nl=%d\n",
              k, h->Ndi, h->status, h->TBS, h->B, h->round, h->mcs, h->rvidx, h->Nl);
        }
      }
    }
  }
}

void print_ulsch_ue_stats(LTE_UE_ULSCH_t* d)
{
  int k;
  LTE_UL_UE_HARQ_t* h;
  if(d) {
    printf("UE ulsch: Nsymb_pusch=%d, O=%d, o_ACK=%d %d %d %d, Mdlharq=%d, cooperation_flag=%d\n",
        d->Nsymb_pusch, d->O, d->o_ACK[0], d->o_ACK[1], d->o_ACK[2], d->o_ACK[3], d->Mdlharq, d->cooperation_flag);
    for(k = 0; k < 3; k++) {
      if(d->harq_processes[k]) {
        h = d->harq_processes[k];
        if(h->status == ACTIVE) {
          printf("HARQ process %d: Ndi=%d, status=%d, subframe_scheduling_flag=%d, first_rb=%d, nb_rb=%d, TBS=%d, B=%d, round=%d, mcs=%d, rvidx=%d\n",
              k, h->Ndi, h->status, h->subframe_scheduling_flag, h->first_rb, h->nb_rb, h->TBS, h->B, h->round, h->mcs, h->rvidx);
        }
      }
    }
  }
}

void print_ulsch_eNB_stats(LTE_eNB_ULSCH_t* d)
{
  int k;
  LTE_UL_eNB_HARQ_t* h;
  if(d) {
    printf("eNB ulsch: Nsymb_pusch=%d, Mdlharq=%d, cqi_crc_status=%d, Or1=%d, Or2=%d, o_RI=%d %d, O_RI=%d, o_ACK=%d %d %d %d, o_RCC=%d, beta_offset_cqi_times8=%d, beta_offset_ri_times8=%d, beta_offset_harqack_times8=%d, rnti=%x, cyclicShift=%d, cooperation_flag=%d\n",
        d->Nsymb_pusch, d->Mdlharq, d->cqi_crc_status, d->Or1, d->Or2, d->o_RI[0], d->o_RI[1], d->O_RI, d->o_ACK[0], d->o_ACK[1], d->o_ACK[2], d->o_ACK[3], d->o_RCC, d->beta_offset_cqi_times8, d->beta_offset_ri_times8, d->beta_offset_harqack_times8, d->rnti, d->cyclicShift, d->cooperation_flag);
    for(k = 0; k < 3; k++) {
      if(d->harq_processes[k]) {
        h = d->harq_processes[k];
        if(h->status == ACTIVE) {
          printf("HARQ process %d: Ndi=%d, status=%d, subframe_scheduling_flag=%d, phich_active=%d, phich_ACK=%d, TPC=%d, first_rb=%d, nb_rb=%d, TBS=%d, B=%d, round=%d, mcs=%d, rvidx=%d\n",
              k, h->Ndi, h->status, h->subframe_scheduling_flag, h->phich_active, h->phich_ACK, h->TPC, h->first_rb, h->nb_rb, h->TBS, h->B, h->round, h->mcs, h->rvidx);
        }
      }
    }
  }
}

int block_valid(u8* ref, u8* rec, int n)
{
  int k;

  for(k = 0; k < n; k++) {
    if(ref[k] != rec[k])
      return 0;
      //printf("%d ",k);
  }
  //printf("\n");
  return 1;
}

results_t* alloc_results(args_t* a)
{
  results_t* r;

  r = malloc(sizeof(results_t));
  r->tx = malloc(a->n_pdu*sizeof(tx_results_t));

  fprintf(stderr, "Allocated %d bytes for results\n", sizeof(results_t)+a->n_pdu*sizeof(tx_results_t));

  return r;
}

void init_results(results_t* r, args_t* a, context_t* c)
{
  int k;

  r->n_pdu = a->n_pdu;
  r->n_relays = a->n_relays;
  r->n_harq = a->n_harq;
  r->channel_model = a->channel_model;

  for(k = 0; k < a->n_relays; k++) {
    r->snr_hop1[k] = c->snr_hop1[k];
    r->snr_hop2[k] = c->snr_hop2[k];
  }

  for(k = 0; k < a->n_pdu; k++) {
    r->tx[k].hop1.n_rounds = 0;
    r->tx[k].hop2.n_rounds = 0;
  }
}

void free_results(results_t* r)
{
  free(r->tx);
  free(r);
}

void print_results(results_t* r)
{
  int k;
  int l;
  int harq_tries[MAX_HARQ_ROUNDS];
  int harq_success[MAX_HARQ_ROUNDS];
  int n_failed;
  int n_total;
  double throughput;
  double spectral_eff;
  double latency;
  int relay_activity[1 << MAX_RELAYS];
  bool first;

  // Print SNR and raw BER for hop 1
  printf("Hop 1: SNR (");
  for(k = 0; k < r->n_relays; k++)
    printf("%.1f%s", r->snr_hop1[k], k < r->n_relays-1 ? ", " : "");
  printf("), BER (");
  for(k = 0; k < r->n_relays; k++)
    printf("%f%s", compute_ber_hop1(r, k), k < r->n_relays-1 ? ", " : "");
  printf(")\n");

  // Print HARQ stats for hop 1
  compute_harq_stats_hop1(r, -1, harq_tries, harq_success);
  printf("  HARQ, n_tries from CH0:");
  for(l = 0; l < r->n_harq; l++)
    printf(" %d", harq_tries[l]);
  printf("\n");

  for(k = 0; k < r->n_relays; k++) {
    compute_harq_stats_hop1(r, k, harq_tries, harq_success);
    printf("  HARQ, n_success at MR%d:", k);
    for(l = 0; l < r->n_harq; l++)
      printf(" %d", harq_success[l]);
    printf("\n");
  }

  // Print SNR and raw BER for hop 2
  printf("Hop 2: SNR (");
  for(k = 0; k < r->n_relays; k++)
    printf("%.1f%s", r->snr_hop2[k], k < r->n_relays-1 ? ", " : "");
  printf("), BER %f\n", compute_ber_hop2(r));

  // Print HARQ stats for hop 2
  compute_harq_stats_hop2(r, harq_tries, harq_success);
  printf("  HARQ (n_success/n_tries):");
  for(l = 0; l < r->n_harq; l++)
    printf(" %d/%d", harq_success[l], harq_tries[l]);
  printf("\n");
  
  // Hop 1,2: PDF of mcs and n_prb

  // Print colaborative link statistics
  compute_colab_bler(r, &n_failed, &n_total);
  compute_colab_stat(r, &throughput, &spectral_eff, &latency, relay_activity);
  printf("Collaborative link, BLER: %d/%d\n", n_failed, n_total);
  printf("  avg throughput: %f [bits/s]\n", throughput);
  printf("  avg spectral efficiency: %f [bits/s/Hz]\n", spectral_eff);
  printf("  avg latency, CH0 to CH1: %f [ms]\n", latency);
  printf("  relay activity:");
  for(k = 1; k < (1 << r->n_relays); k++) {
    first = true;
    printf(" ");
    for(l = 0; l < r->n_relays; l++) {
      if(k & (1 << l)) {
        if(first)
          first = false;
        else
          printf("+");
        printf("MR%d", l);
      }
    }
    printf("(%d)", relay_activity[k]);
  }
  printf("\n");
}

double compute_ber_hop1(results_t* r, int k)
{
  u32 l;
  u8 h;
  u32 n_bits = 0;
  u32 n_correct = 0;

  for(l = 0; l < r->n_pdu; l++) {
    for(h = 0; h < r->tx[l].hop1.n_rounds; h++) {
      n_bits += r->tx[l].hop1.round[h].n_sent_bits;
      n_correct += r->tx[l].hop1.round[h].n_correct_bits[k];
      if(r->tx[l].hop1.round[h].decoded & (1 << k))
        break;
    }
  }

  return (double)(n_bits-n_correct)/(double)n_bits;
}

double compute_ber_hop2(results_t* r)
{
  u32 l;
  u8 h;
  u32 n_bits = 0;
  u32 n_correct = 0;

  for(l = 0; l < r->n_pdu; l++) {
    for(h = 0; h < r->tx[l].hop2.n_rounds; h++) {
      n_bits += r->tx[l].hop2.round[h].n_sent_bits;
      n_correct += r->tx[l].hop2.round[h].n_correct_bits;
    }
  }

  return (double)(n_bits-n_correct)/(double)n_bits;
}

void compute_harq_stats_hop1(results_t* r, int k, int* harq_tries, int* harq_success)
{
  int l;
  int h;
  
  for(l = 0; l < MAX_HARQ_ROUNDS; l++) {
    harq_tries[l] = 0;
    harq_success[l] = 0;
  }

  if(k == -1) {
    for(l = 0; l < r->n_pdu; l++) {
      for(h = 0; h < r->tx[l].hop1.n_rounds; h++) {
        harq_tries[h]++;
      }
    }
  }
  else {
    for(l = 0; l < r->n_pdu; l++) {
      for(h = 0; h < r->tx[l].hop1.n_rounds; h++) {
        harq_tries[h]++;
        if(r->tx[l].hop1.round[h].decoded & (1 << k)) {
          harq_success[h]++;
          break;
        }
      }
    }
  }
}

void compute_harq_stats_hop2(results_t* r, int* harq_tries, int* harq_success)
{
  int l;
  int h;
  
  for(l = 0; l < MAX_HARQ_ROUNDS; l++) {
    harq_tries[l] = 0;
    harq_success[l] = 0;
  }

  for(l = 0; l < r->n_pdu; l++) {
    for(h = 0; h < r->tx[l].hop2.n_rounds; h++) {
      harq_tries[h]++;
      if(r->tx[l].hop2.round[h].decoded) {
        harq_success[h]++;
        break;
      }
    }
  }
}

void compute_colab_bler(results_t* r, int* n_failed, int* n_total)
{
  u32 l;
  u8 h;
  u32 n_decoded = 0;

  for(l = 0; l < r->n_pdu; l++) {
    for(h = 0; h < r->tx[l].hop2.n_rounds; h++) {
      if(r->tx[l].hop2.round[h].decoded) {
        n_decoded++;
        break;
      }
    }
  }

  *n_total = r->n_pdu;
  *n_failed = r->n_pdu-n_decoded;
}

void compute_colab_stat(results_t* r, double* throughput, double* spectral_eff, double* latency, int* relay_activity)
{
  int n_tx_hop1 = 0;
  int n_tx_hop2 = 0;
  int n_bits = 0;
  int n_prb = 0;
  int n_subframes = 0;
  int n_pdu = 0;
  int k;
  int l;
  int h;
  bool success;

  for(k = 0; k < (1 << r->n_relays); k++)
    relay_activity[k] = 0;

  for(l = 0; l < r->n_pdu; l++) {
    success = false;
    n_tx_hop1 += r->tx[l].hop1.n_rounds;
    n_tx_hop2 += r->tx[l].hop2.n_rounds;
    for(h = 0; h < r->tx[l].hop1.n_rounds; h++) {
      n_prb += r->tx[l].hop1.round[h].n_prb;
    }
    for(h = 0; h < r->tx[l].hop2.n_rounds; h++) {
      n_prb += r->tx[l].hop2.round[h].n_prb;
      if(r->tx[l].hop2.round[h].decoded && !success) {
        success = true;
        n_subframes += (10*r->tx[l].hop2.round[h].frame + r->tx[l].hop2.round[h].subframe) -
            (10*r->tx[l].hop1.round[0].frame + r->tx[l].hop1.round[0].subframe);
        n_pdu++;
      }
      relay_activity[r->tx[l].hop2.round[h].active]++;
    }
    if(success) {
      n_bits += 8*r->tx[l].n_bytes;
    }
  }

  *throughput = (double)n_bits/((double)max(n_tx_hop1,n_tx_hop2)*0.01);
  *spectral_eff = (double)n_bits/((double)n_prb*12.0*15000.0*0.001);
  *latency = (double)n_subframes/(double)n_pdu;
}

void write_results_header(FILE* f, results_t* r, int n_tests)
{
  fprintf(f, "%d %d %d %d %d\n", r->n_relays, r->channel_model, n_tests, r->n_pdu, r->n_harq);
}

void write_results_data(FILE* f, results_t* r)
{
  int k;
  int l;
  int h;
  hop1_round_results_t* r1;
  hop2_round_results_t* r2;

  for(k = 0; k < r->n_relays; k++)
    fprintf(f, "%f ", r->snr_hop1[k]);
  fprintf(f, "\n");
  for(k = 0; k < r->n_relays; k++)
    fprintf(f, "%f ", r->snr_hop2[k]);
  fprintf(f, "\n");

  for(l = 0; l < r->n_pdu; l++) {
    fprintf(f, "%d %d %d\n", r->tx[l].n_bytes, r->tx[l].hop1.n_rounds, r->tx[l].hop2.n_rounds);

    for(h = 0; h < r->tx[l].hop1.n_rounds; h++) {
      r1 = &r->tx[l].hop1.round[h];
      fprintf(f, "%d %d %d %d %d %d", r1->frame, r1->subframe, r1->tbs, r1->mcs, r1->n_prb, r1->n_sent_bits);
      for(k = 0; k < r->n_relays; k++)
        fprintf(f, " %d", r1->n_correct_bits[k]);
      fprintf(f, " %d\n", r1->decoded);
    }

    for(h = 0; h < r->tx[l].hop2.n_rounds; h++) {
      r2 = &r->tx[l].hop2.round[h];
      fprintf(f, "%d %d %d %d %d %d %d %d %d\n", r2->frame, r2->subframe, r2->tbs, r2->mcs, r2->n_prb, 
          r2->n_sent_bits, r2->n_correct_bits, r2->active, r2->decoded);
    }
  }

  /*
  fprintf(f, "%d %d %d %d %d %d\n", r->n_frames_hop1, r->n_frames_hop2,
      r->n_bits_hop1, r->n_bits_hop2, r->n_pdu_success_hop1, r->n_pdu_success_hop2);

  for(k = 0; k < r->n_relays; k++)
    fprintf(f, "%f ", r->ber_hop1[k]);
  fprintf(f, "%f\n", r->ber_hop2);

  for(k = 0; k < r->n_harq; k++)
    fprintf(f, "%d ", r->n_harq_tries_hop1[k]);
  fprintf(f, "\n");
  for(k = 0; k < r->n_harq; k++)
    fprintf(f, "%d ", r->n_harq_success_hop1[k]);
  fprintf(f, "\n");
  for(k = 0; k < r->n_harq; k++)
    fprintf(f, "%d ", r->n_harq_tries_hop2[k]);
  fprintf(f, "\n");
  for(k = 0; k < r->n_harq; k++)
    fprintf(f, "%d ", r->n_harq_success_hop2[k]);
  fprintf(f, "\n");

  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->mcs_hop1[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->mcs_hop2[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->tbs_hop1[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->tbs_hop2[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->n_prb_hop1[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->n_prb_hop2[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_harq; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->n_transmissions[l][k]);
    fprintf(f, "\n");
  }
  for(k = 0; k < (1 << r->n_relays); k++)
    fprintf(f, "%d ", r->relay_activity[k]);
  fprintf(f, "\n");
*/
}

void determine_pdsch_harq_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s)
{
  switch(frame_parms->tdd_config) {
    case 1:
      switch(subframe) {
        case 0:
        case 1:
          *f = frame;
          *s = 7;
          break;
        case 4:
          *f = frame;
          *s = 8;
          break;
        case 5:
        case 6:
          *f = frame+1;
          *s = 2;
          break;
        case 9:
          *f = frame+1;
          *s = 3;
          break;
        default:
          printf("illegal subframe %d\n", subframe);
          exit(1);
      }
      break;
    default:
      printf("TDD mode %d not implemented\n", frame_parms->tdd_config);
      exit(1);
  }
}

void determine_pusch_dci_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s)
{
  switch(frame_parms->tdd_config) {
    case 1:
      switch(subframe) {
        case 2:
          *f = frame-1;
          *s = 6;
          break;
        case 3:
          *f = frame-1;
          *s = 9;
          break;
        case 7:
          *f = frame;
          *s = 1;
          break;
        case 8:
          *f = frame;
          *s = 4;
          break;
        default:
          fprintf(stderr, "illegal subframe %d\n", subframe);
          exit(1);
      }
      break;
    default:
      fprintf(stderr, "TDD mode %d not implemented\n", frame_parms->tdd_config);
      exit(1);
  }
}

void determine_pusch_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s)
{
  switch(frame_parms->tdd_config) {
    case 1:
      switch(subframe) {
        case 1:
          *f = frame;
          *s = 7;
          break;
        case 4:
          *f = frame;
          *s = 8;
          break;
        case 6:
          *f = frame+1;
          *s = 2;
          break;
        case 9:
          *f = frame+1;
          *s = 3;
          break;
        default:
          fprintf(stderr, "illegal subframe %d\n", subframe);
          exit(1);
      }
      break;
    default:
      fprintf(stderr, "TDD mode %d not implemented\n", frame_parms->tdd_config);
      exit(1);
  }
}

void determine_pusch_harq_subframe(LTE_DL_FRAME_PARMS* frame_parms, int frame, int subframe, int* f, int* s)
{
  determine_pusch_dci_subframe(frame_parms, frame+1, subframe, f, s);
}

