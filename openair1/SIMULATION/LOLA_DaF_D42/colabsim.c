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
#define N_RB 25
#define NID_CELL 0
#define MAX_RELAYS 8
#define MAX_HARQ_ROUNDS 4

const u8 cp_type = 0; // Normal cyclic prefix
const u8 n_txantenna_ch = 1;
const u8 n_rxantenna_ch = 1;
const u8 n_txantenna_mr = 1;
const u8 n_rxantenna_mr = 1;
const u8 oversampling = 1;
const u16 rb_alloc = 0x1fff;
const u8 subframe_hop1 = 4;
const u8 subframe_hop2 = 7;
const u8 n_avail_pdcch_symbols = 3;

typedef enum {
  analysis_single, analysis_sweep, analysis_bsweep
} analysis_t;

// Structure for command line parsed arguments
typedef struct {
  bool debug_output;
  bool debug_print;
  int verbose;
  analysis_t analysis;
  int n_relays;
  int n_pdu;
  int n_harq;
  int mcs_hop1;
  int mcs_hop2;
  SCM_t channel_model;
  double snr_hop1[MAX_RELAYS];
  double snr_hop2[MAX_RELAYS];
  const char* results_fn;
} args_t;

// Structure containing link simulation results for one test
// Note: mcs_hop*, tbs_hop*, n_rb_hop* are matrices containing values for each
//   transmission attempt in the simulation. The first index is the MAC PDU
//   index and the second is the HARQ round index. Currently no AMC is
//   implemented, and all these values are the same.
// Note: The meaning of n_harq_success_hop1 is dependent on the HARQ strategy.
//   In HARQ strategy 1, both relays must be decode.
//   Other HARQ strategies are not implemented yet.
typedef struct {
  int n_relays;             // number of relays
  int n_pdu;                // number of transmitted MAC PDUs
  int n_harq;               // number of HARQ rounds
  SCM_t channel_model;      // used channel model
  double* snr_hop1;         // SNRs for each link in hop 1
  double* snr_hop2;         // SNRs for each link in hop 2
  int** mcs_hop1;           // MCS used in hop 1
  int** mcs_hop2;           // MCS used in hop 2
  int** tbs_hop1;           // transport block size for hop 1
  int** tbs_hop2;           // transport block size for hop 2
  int** n_rb_hop1;          // number of used RBs in hop 1
  int** n_rb_hop2;          // number of used RBs in hop 2
  int n_frames_hop1;        // number of transmitted LTE frames in hop 1
  int n_frames_hop2;        // number of transmitted LTE frames in hop 2
  int n_bits_hop1;          // number of correctly received information bits over hop 1
  int n_bits_hop2;          // number of correctly received information bits over hop 2
  double ber_hop1[MAX_RELAYS]; // raw BER in hop 1
  double ber_hop2;          // raw BER in hop 2
  int n_pdu_success_hop1;   // number of correctly received MAC PDUs in hop 1
  int n_pdu_success_hop2;   // number of correctly received MAC PDUs in hop 2
  int n_harq_tries_hop1[MAX_HARQ_ROUNDS]; // number of transmitted MAC PDUs in each HARQ round in hop 1
  int n_harq_success_hop1[MAX_HARQ_ROUNDS]; // number of successfully decoded MAC PDUs in each HARQ round in hop 1
  int n_harq_tries_hop2[MAX_HARQ_ROUNDS]; // number of transmitted MAC PDUs in each HARQ round in hop 2
  int n_harq_success_hop2[MAX_HARQ_ROUNDS]; // number of successfully decoded MAC PDUs in each HARQ round in hop 2
  int n_transmissions[MAX_HARQ_ROUNDS][MAX_HARQ_ROUNDS]; // PDF of number of transmissions in the two hops for
                                                         // MAC PDUs correctly received at CH2
  int* relay_activity;      // PDF of relay activity, [1]: MR1 active, [2]: MR2 active, [3]: MR1+MR2 active
} results_t;

typedef enum {
  RELAY_ROLE_STANDARD, RELAY_ROLE_ALTERNATE
} relay_role_t;

typedef struct {
  double* s_re[1];
  double* s_im[1];
  double* r_re[1];
  double* r_im[1];
} channel_vars_t;

typedef struct {
  channel_vars_t* cvars;
  channel_desc_t* channel;
} sh_channel_t;

int parse_args(int argc, char** argv, args_t* args);
int parse_channel_model(const char* str, SCM_t* model);
bool parse_snr(const char* str, double* snr, int n);
void print_usage(const char* prog);
void print_channel_usage();
void signal_handler(int sig);
void setup_snrs_single(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays);
void setup_snrs_sweep(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end);
void setup_snrs_bsweep(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end);
void setup_frame_params(LTE_DL_FRAME_PARMS* frame_parms, unsigned char transmission_mode);
void setup_broadcast_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs);
void setup_distributed_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs);
void alloc_broadcast_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays, u16 rnti);
void free_broadcast_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays);
void alloc_distributed_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays, u16 rnti);
void free_distributed_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays);
void ofdm_modulation(mod_sym_t** tx_f, s32** tx_t, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb);
channel_vars_t alloc_channel_vars(LTE_DL_FRAME_PARMS* frame_parms);
void free_channel_vars(channel_vars_t v);
sh_channel_t* alloc_sh_channel(channel_vars_t* cvars, SCM_t channel_model, int n_txantennas, int n_rxantennas);
void free_sh_channel(sh_channel_t* c);
void transmit_subframe(sh_channel_t* channel, s32** src, s32** dst, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb, double awgn_stddev);
void ofdm_fep(PHY_VARS_UE* phy_vars_mr, u8 subframe);
int rx_dlsch_symbol(PHY_VARS_UE* phy_vars, u8 subframe, u8 symbol, u8 first_symbol);
u32 get_ulsch_G(LTE_UE_ULSCH_t *ulsch, u8 harq_pid);
double compute_ber_soft(u8* ref, s16* rec, int n);
void print_dlsch_eNB_stats(LTE_eNB_DLSCH_t* d);
void print_dlsch_ue_stats(LTE_UE_DLSCH_t* d);
void print_ulsch_ue_stats(LTE_UE_ULSCH_t* d);
void print_ulsch_eNB_stats(LTE_eNB_ULSCH_t* d);
int block_valid(u8* ref, u8* rec, int n);
void init_results(results_t* r, args_t* a);
void clear_results(results_t* r);
void free_results(results_t* r);
void print_results(results_t* r);
void write_results_header(FILE* f, results_t* r, int n_tests);
void write_results_data(FILE* f, results_t* r);

int main(int argc, char **argv) {
  args_t args;
  results_t results;

  LTE_DL_FRAME_PARMS* frame_parms;

  PHY_VARS_eNB* phy_vars_ch_src;
  PHY_VARS_eNB* phy_vars_ch_dest;
  PHY_VARS_UE** phy_vars_mr;

  DCI_ALLOC_t dci_hop1;
  DCI_ALLOC_t dci_hop2;

  channel_vars_t channel_vars;
  sh_channel_t** channels_hop1;
  sh_channel_t** channels_hop2;
  s32* rxdata[1];

  double* snrs;
  double* snr_hop1;
  double* snr_hop2;
  int n_tests;
  int n_relays;

  u16 rnti_hop1 = 0x1515;
  u16 rnti_hop2 = 0x1516;

  u32 tbs_hop1;
  u32 tbs_hop2;
  u32 tbs_col;

  int input_buffer_length;
  u8* input_buffer;
  int mr_buffer_length;
  u8* mr_buffer[MAX_RELAYS];
  u32 n_coded_bits_hop1;
  u32 n_coded_bits_hop2;
  int n_re_hop1;
  int n_re_hop2;
  int n_symbols_per_slot;
  int pilot1_symbol;
  u8 n_used_pdcch_symbols;
  u8 harq_pid_hop2;
  u32 tx_energy;
  double awgn_stddev;

  // State variables
  int test;  // Current test
  int pdu;   // Current MAC PDU
  int round; // Current HARQ round
  relay_role_t relay_role[MAX_RELAYS];
  bool decoded_at_all_mr;
  bool decoded_at_mr[MAX_RELAYS];
  bool decoded_at_ch;

  // Temporary variables
  int i;
  int k;
  int l;
  int n_iter;
  double raw_ber;

  // Interim results variables
  int n_ber_frames_hop1[MAX_RELAYS];
  int n_ber_frames_hop2;
  int rounds_hop1;
  int rounds_hop2;

  // Temporary strings
  char fnbuf[80];
  char varbuf[80];

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

  n_relays = args.n_relays;

  // General setup
  signal(SIGSEGV, signal_handler);
  randominit(0);
  set_taus_seed(0);

  if(n_relays > MAX_RELAYS) {
    printf("Too many relays, increase MAX_RELAYS\n");
    exit(1);
  }

  if(args.n_harq > MAX_HARQ_ROUNDS) {
    printf("Too many HARQ rounds, increase MAX_HARQ_ROUNDS\n");
    exit(1);
  }

  // Allocate memory for frame parameters and node structures
  frame_parms = malloc(sizeof(LTE_DL_FRAME_PARMS));
  memset(frame_parms, 0, sizeof(LTE_DL_FRAME_PARMS));
  phy_vars_ch_src = malloc(sizeof(PHY_VARS_eNB));
  phy_vars_ch_dest = malloc(sizeof(PHY_VARS_eNB));
  phy_vars_mr = malloc(n_relays*sizeof(PHY_VARS_UE*));
  for(k = 0; k < n_relays; k++) {
    phy_vars_mr[k] = malloc(sizeof(PHY_VARS_UE));
    memset(phy_vars_mr[k], 0, sizeof(PHY_VARS_UE));
  }
  memset(phy_vars_ch_src, 0, sizeof(PHY_VARS_eNB));
  memset(phy_vars_ch_dest, 0, sizeof(PHY_VARS_eNB));

  // Initialize result data
  init_results(&results, &args);

  // Allocate channel structures
  channels_hop1 = malloc(n_relays*sizeof(sh_channel_t*));
  channels_hop2 = malloc(n_relays*sizeof(sh_channel_t*));
  memset(channels_hop1, 0, n_relays*sizeof(sh_channel_t*));
  memset(channels_hop2, 0, n_relays*sizeof(sh_channel_t*));

  // Allocate and setup vector of SNR tests
  switch(args.analysis) {
  case analysis_single:
    setup_snrs_single(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, n_relays);
    break;
  case analysis_sweep:
    setup_snrs_sweep(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, n_relays, 1.0, -10, 10);
    break;
  case analysis_bsweep:
    setup_snrs_bsweep(&snrs, &n_tests, args.snr_hop1, args.snr_hop2, n_relays, 1.0, -10, 10);
    break;
  }

  // Open results file (if requested)
  if(args.results_fn) {
    store_results = true;
    results_file = fopen(args.results_fn, "w");
  }

  n_symbols_per_slot = (cp_type == 0 ? 7 : 6);
  pilot1_symbol = (cp_type == 0 ? 4 : 3);

  if(store_results) {
    write_results_header(results_file, &results, n_tests);
  }

  // Setup PHY structures
  setup_frame_params(frame_parms, 1);
  phy_vars_ch_src->lte_frame_parms = *frame_parms;
  phy_init_lte_eNB(phy_vars_ch_src, 0, 0, 0);
  for(k = 0; k < n_relays; k++) {
    phy_vars_mr[k]->lte_frame_parms = *frame_parms;
    lte_gold(frame_parms, phy_vars_mr[k]->lte_gold_table[0], 0);
    lte_gold(frame_parms, phy_vars_mr[k]->lte_gold_table[1], 1);
    lte_gold(frame_parms, phy_vars_mr[k]->lte_gold_table[2], 2);
    phy_init_lte_ue(phy_vars_mr[k], 0);
  }
  phy_vars_ch_dest->lte_frame_parms = *frame_parms;
  phy_init_lte_eNB(phy_vars_ch_dest, 0, 2, 0);

  // Allocate temporary signal structures
  rxdata[0] = malloc(10*frame_parms->samples_per_tti);

  // Allocate first hop transport channel
  alloc_broadcast_transport_channel(phy_vars_ch_src, phy_vars_mr, n_relays, rnti_hop1);
        
  // Allocate second hop transport channel
  alloc_distributed_transport_channel(phy_vars_ch_dest, phy_vars_mr, n_relays, rnti_hop2);
  
  // Setup channel structures
  channel_vars = alloc_channel_vars(frame_parms);
  for(k = 0; k < n_relays; k++) {
    channels_hop1[k] = alloc_sh_channel(&channel_vars, args.channel_model, n_txantenna_ch, n_rxantenna_mr);
    channels_hop2[k] = alloc_sh_channel(&channel_vars, args.channel_model, n_txantenna_mr, n_rxantenna_ch);
  }

  // Create broadcast DCI and generate transport channel parameters,
  // in order to determine hop 1 transfer block size and number of coded bits
  setup_broadcast_dci(&dci_hop1, rnti_hop1, 0, args.mcs_hop1);
  generate_eNB_dlsch_params_from_dci(subframe_hop1, dci_hop1.dci_pdu, 
      rnti_hop1, format1, phy_vars_ch_src->dlsch_eNB[0], frame_parms, 
      SI_RNTI, RA_RNTI, P_RNTI,
      phy_vars_ch_src->eNB_UE_stats[0].DL_pmi_single);
  tbs_hop1 = phy_vars_ch_src->dlsch_eNB[0][0]->harq_processes[0]->TBS;
  n_coded_bits_hop1 = get_G(frame_parms, phy_vars_ch_src->dlsch_eNB[0][0]->nb_rb,
      phy_vars_ch_src->dlsch_eNB[0][0]->rb_alloc,
      get_Qm(phy_vars_ch_src->dlsch_eNB[0][0]->harq_processes[0]->mcs),
      n_avail_pdcch_symbols, subframe_hop1);

  // Create distributed DCI and generate transport channel parameters,
  // in order to determine hop 2 transfer block size and number of coded bits
  harq_pid_hop2 = subframe2harq_pid(frame_parms, 0, subframe_hop2);
  setup_distributed_dci(&dci_hop2, rnti_hop2, 0, args.mcs_hop2);
  generate_ue_ulsch_params_from_dci(dci_hop2.dci_pdu, rnti_hop2, (subframe_hop2+6)%10,
      format0, phy_vars_mr[0], SI_RNTI, RA_RNTI, P_RNTI, 0, 0);
  tbs_hop2 = phy_vars_mr[0]->ulsch_ue[0]->harq_processes[harq_pid_hop2]->TBS;
  n_coded_bits_hop2 = get_ulsch_G(phy_vars_mr[0]->ulsch_ue[0], harq_pid_hop2);

  if(args.verbose > 1) {
    print_dlsch_eNB_stats(phy_vars_ch_src->dlsch_eNB[0][0]);
    //print_dlsch_ue_stats(phy_vars_mr[0]->dlsch_ue[0][0]);
    print_ulsch_ue_stats(phy_vars_mr[0]->ulsch_ue[0]);
    //print_ulsch_eNB_stats(phy_vars_ch_dest->ulsch_eNB[0]);
    dump_dci(frame_parms, &dci_hop1);
    dump_dci(frame_parms, &dci_hop2);
  }
  printf("Hop 1: TBS=%d, G=%d, rate=%f. Hop 2: TBS=%d, G=%d, rate=%f\n", 
      tbs_hop1, n_coded_bits_hop1, (float)tbs_hop1/(float)n_coded_bits_hop1,
      tbs_hop2, n_coded_bits_hop2, (float)tbs_hop2/(float)n_coded_bits_hop2);

  tbs_col = tbs_hop1 < tbs_hop2 ? tbs_hop1 : tbs_hop2;

  // Allocate input buffer
  input_buffer_length = tbs_hop1/8;
  input_buffer = malloc(input_buffer_length+4);
  memset(input_buffer, 0, input_buffer_length+4);
  if(args.verbose > 0)
    printf("Input buffer: %d bytes\n", input_buffer_length);

  // Allocate MR data buffers
  mr_buffer_length = tbs_hop2/8;
  for(k = 0; k < n_relays; k++) {
    mr_buffer[k] = malloc(mr_buffer_length+4);
    memset(mr_buffer[k], 0, mr_buffer_length+4);
  }

  for(test = 0; test < n_tests; test++) {
    // Set SNRs
    snr_hop1 = &snrs[2*n_relays*test];
    snr_hop2 = &snrs[2*n_relays*test + n_relays];

    // Clear results
    clear_results(&results);
    for(k = 0; k < n_relays; k++) {
      n_ber_frames_hop1[k] = 0;
    }
    n_ber_frames_hop2 = 0;
    results.snr_hop1 = snr_hop1;
    results.snr_hop2 = snr_hop2;

    // Print test info.
    printf("\n*** Test %d/%d ***\n", test+1, n_tests);

    for(pdu = 0; pdu < args.n_pdu; pdu++) {
      if(args.verbose == 0) {
        printf("Transmitting MAC PDU %d\r", pdu);
        fflush(stdout);
      } else
        printf("Transmitting MAC PDU %d\n", pdu);

      decoded_at_all_mr = false;
      for(k = 0; k < n_relays; k++)
        decoded_at_mr[k] = false;

      // Generate input data
      for(k = 0; k < input_buffer_length; k++)
        input_buffer[k] = (u8)(taus()&0xff);

      for(round = 0; round < args.n_harq && !decoded_at_all_mr; round++) {
        // Clear txdataF vector
        memset(&phy_vars_ch_src->lte_eNB_common_vars.txdataF[0][0][0], 0, 
            FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));

        // Fill results
        results.mcs_hop1[pdu][round] = args.mcs_hop1;
        results.tbs_hop1[pdu][round] = tbs_hop1;
        results.n_rb_hop1[pdu][round] = N_RB;

        // Create first hop DCI
        setup_broadcast_dci(&dci_hop1, rnti_hop1, round, args.mcs_hop1);
        if(args.verbose > 1)
          dump_dci(frame_parms, &dci_hop1);

        // Generate eNB transport channel parameters
        generate_eNB_dlsch_params_from_dci(subframe_hop1, dci_hop1.dci_pdu, 
            rnti_hop1, format1, phy_vars_ch_src->dlsch_eNB[0], frame_parms, 
            SI_RNTI, RA_RNTI, P_RNTI,
            phy_vars_ch_src->eNB_UE_stats[0].DL_pmi_single);

        // Create PDCCH
        n_used_pdcch_symbols = generate_dci_top(1, 0, &dci_hop1, 0, 1024, frame_parms,
            phy_vars_ch_src->lte_eNB_common_vars.txdataF[0], subframe_hop1);

        if(n_used_pdcch_symbols > n_avail_pdcch_symbols) {
          printf("Need %d PDCCH symbols\n", n_used_pdcch_symbols);
          exit(1);
        }
        
        // Encode source data
        if(dlsch_encoding(input_buffer, frame_parms, n_avail_pdcch_symbols,
              phy_vars_ch_src->dlsch_eNB[0][0], subframe_hop1) < 0)
          exit(-1);

        // Scramble data
        dlsch_scrambling(frame_parms, n_avail_pdcch_symbols,
            phy_vars_ch_src->dlsch_eNB[0][0], n_coded_bits_hop1, 0, subframe_hop1 << 1);

        // Modulate data
        n_re_hop1 = dlsch_modulation(phy_vars_ch_src->lte_eNB_common_vars.txdataF[0],
            1024, subframe_hop1, frame_parms, n_avail_pdcch_symbols, 
            phy_vars_ch_src->dlsch_eNB[0][0]);
        if(args.verbose > 0)
          printf("Hop 1, HARQ round %d: %d coded bits, Modulated %d REs\n", round, n_coded_bits_hop1, n_re_hop1);

        if(args.verbose > 2)
          print_dlsch_eNB_stats(phy_vars_ch_src->dlsch_eNB[0][0]);

        // Generate pilots
        generate_pilots(phy_vars_ch_src, phy_vars_ch_src->lte_eNB_common_vars.txdataF[0],
            1024, LTE_NUMBER_OF_SUBFRAMES_PER_FRAME);

        // OFDM modulation
        ofdm_modulation(phy_vars_ch_src->lte_eNB_common_vars.txdataF[0],
            phy_vars_ch_src->lte_eNB_common_vars.txdata[0],
            frame_parms, subframe_hop1, frame_parms->symbols_per_tti+1);

        // Compute transmitter signal energy
        tx_energy = signal_energy(&phy_vars_ch_src->lte_eNB_common_vars.txdata[0][0]
            [subframe_hop1*frame_parms->samples_per_tti], frame_parms->samples_per_tti);

        // Transmit over channel
        for(k = 0; k < n_relays; k++) {
          awgn_stddev = sqrt((double)tx_energy*((double)frame_parms->ofdm_symbol_size/(N_RB*12))/pow(10.0, ((double)snr_hop1[k])/10.0)/2.0);
          transmit_subframe(channels_hop1[k],
              phy_vars_ch_src->lte_eNB_common_vars.txdata[0],
              phy_vars_mr[k]->lte_ue_common_vars.rxdata,
              frame_parms, subframe_hop1, frame_parms->symbols_per_tti+1, awgn_stddev);
        }
        results.n_frames_hop1++;
        results.n_harq_tries_hop1[round]++;

        // Decode at all relays
        for(k = 0; k < n_relays; k++) {
          if(decoded_at_mr[k])
            continue;

          // Front end processor up to first pilot
          for(l = 0; l <= pilot1_symbol; l++)
            slot_fep(phy_vars_mr[k], l, subframe_hop1<<1, 0, 0);

          // Skip decoding of DCI
          phy_vars_mr[k]->lte_ue_pdcch_vars[0]->crnti = rnti_hop1;
          phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols = n_avail_pdcch_symbols;
          generate_ue_dlsch_params_from_dci(subframe_hop1, dci_hop1.dci_pdu, rnti_hop1, 
              format1, phy_vars_mr[k]->dlsch_ue[0], frame_parms, SI_RNTI, RA_RNTI, P_RNTI);

          // Receive DLSCH data
          // Front end processor up to second pilot
          for(l = pilot1_symbol+1; l < n_symbols_per_slot; l++)
            slot_fep(phy_vars_mr[k], l, subframe_hop1<<1, 0, 0);
          slot_fep(phy_vars_mr[k], 0, (subframe_hop1<<1)+1, 0, 0);
          // Receive DLSCH for first slot
          if(rx_dlsch_symbol(phy_vars_mr[k], subframe_hop1, n_avail_pdcch_symbols, 1) == -1)
            break;
          for(l = n_avail_pdcch_symbols + 1; l < n_symbols_per_slot; l++)
            if(rx_dlsch_symbol(phy_vars_mr[k], subframe_hop1, l, 0) == -1)
              break;
          // Front end processor up to third pilot
          for(l = 1; l <= pilot1_symbol; l++)
            slot_fep(phy_vars_mr[k], l, (subframe_hop1<<1)+1, 0, 0);
          // Receive DLSCH up to third pilot
          for(l = n_symbols_per_slot; l < n_symbols_per_slot+pilot1_symbol; l++)
            if(rx_dlsch_symbol(phy_vars_mr[k], subframe_hop1, l, 0) == -1)
              break;
          // Front end processor for rest of subframe
          for(l = pilot1_symbol+1; l < n_symbols_per_slot; l++)
            slot_fep(phy_vars_mr[k], l, (subframe_hop1<<1)+1, 0, 0);
          slot_fep(phy_vars_mr[k], 0, (subframe_hop1<<1)+2, 0, 0);
          // Receive DLSCH for rest of subframe
          for(l = n_symbols_per_slot+pilot1_symbol; l < 2*n_symbols_per_slot; l++)
            if(rx_dlsch_symbol(phy_vars_mr[k], subframe_hop1, l, 0) == -1)
              break;

          // Compute raw bit error rate
          raw_ber = compute_ber_soft(phy_vars_ch_src->dlsch_eNB[0][0]->e,
              phy_vars_mr[k]->lte_ue_pdsch_vars[0]->llr[0], n_coded_bits_hop1);
          results.ber_hop1[k] += raw_ber;
          n_ber_frames_hop1[k]++;
          if(args.verbose > 0)
            printf("Received %d bits at MR %d, raw BER: %f\n", n_coded_bits_hop1, k, raw_ber);

          // Unscramble received bits
          dlsch_unscrambling(frame_parms, phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols,
              phy_vars_mr[k]->dlsch_ue[0][0], n_coded_bits_hop1, phy_vars_mr[k]->lte_ue_pdsch_vars[0]->llr[0],
              0, subframe_hop1 << 1);

          // Decode received bits
          n_iter = dlsch_decoding(phy_vars_mr[k]->lte_ue_pdsch_vars[0]->llr[0],
              frame_parms, phy_vars_mr[k]->dlsch_ue[0][0], subframe_hop1, 
              phy_vars_mr[k]->lte_ue_pdcch_vars[0]->num_pdcch_symbols);

          if(args.verbose > 2)
            print_dlsch_ue_stats(phy_vars_mr[k]->dlsch_ue[0][0]);

          if(n_iter <= MAX_TURBO_ITERATIONS) {
            if(args.verbose > 0)
              printf("Successfully decoded at MR %d\n", k);
            decoded_at_mr[k] = true;
          }
        }

        // Write debug signals if required
        if(args.debug_output) {
          if(round == 0)
            write_output("hop1_e.m", "e", phy_vars_ch_src->dlsch_eNB[0][0]->e, n_coded_bits_hop1, 1, 4);
          snprintf(fnbuf, 80, "hop1_r%d_ch_txdataFv.m", round);
          snprintf(varbuf, 80, "hop1_r%d_ch_txdataF", round);
          write_output(fnbuf, varbuf, phy_vars_ch_src->lte_eNB_common_vars.txdataF[0][0],
              FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX, 1, 1);
          snprintf(fnbuf, 80, "hop1_r%d_ch_txdatav.m", round);
          snprintf(varbuf, 80, "hop1_r%d_ch_txdata", round);
          write_output(fnbuf, varbuf, phy_vars_ch_src->lte_eNB_common_vars.txdata[0][0], 
              10*frame_parms->samples_per_tti, 1, 1);
          for(k = 0; k < n_relays; k++) {
            snprintf(fnbuf, 80, "hop1_r%d_mr%d_rxdatav.m", round, k);
            snprintf(varbuf, 80, "hop1_r%d_mr%d_rxdata", round, k);
            write_output(fnbuf, varbuf, phy_vars_mr[k]->lte_ue_common_vars.rxdata[0], 
                10*frame_parms->samples_per_tti, 1, 1);
            snprintf(fnbuf, 80, "hop1_r%d_mr%d_rxdataFv.m", round, k);
            snprintf(varbuf, 80, "hop1_r%d_mr%d_rxdataF", round, k);
            write_output(fnbuf, varbuf, phy_vars_mr[k]->lte_ue_common_vars.rxdataF[0],
                2*frame_parms->ofdm_symbol_size*2*n_symbols_per_slot, 2, 1);
          }
        }

        // Check if all relays decoded the PDU
        decoded_at_all_mr = true;
        for(k = 0; k < n_relays; k++)
          if(!decoded_at_mr[k])
            decoded_at_all_mr = false;

        if(decoded_at_all_mr) {
          results.n_harq_success_hop1[round]++;
          results.n_pdu_success_hop1++;
          results.n_bits_hop1 += tbs_hop1;
          rounds_hop1 = round;
        }
      }

      if(!decoded_at_all_mr) {
        if(args.verbose > 0)
          printf("Not decoded at all relays, dropping block\n");
        continue;
      }

      // copy received data to intermediate buffers
      for(k = 0; k < n_relays; k++) {
        memcpy(mr_buffer[k], phy_vars_mr[k]->dlsch_ue[0][0]->harq_processes[0]->b, tbs_col>>3);
      }

      // Set role of each relay (alternating STANDARD and ALTERNATE)
      for(k = 0; k < n_relays; k++) {
        relay_role[k] = k & 1;
      }

      decoded_at_ch = false;
      for(round = 0; round < args.n_harq && !decoded_at_ch; round++) {
        // Fill results
        results.mcs_hop2[pdu][round] = args.mcs_hop2;
        results.tbs_hop2[pdu][round] = tbs_hop2;
        results.n_rb_hop2[pdu][round] = N_RB;
        results.relay_activity[(1 << n_relays)-1]++;

        // create second hop dci
        setup_distributed_dci(&dci_hop2, rnti_hop2, round, args.mcs_hop2);
        if(args.verbose > 1)
          dump_dci(frame_parms, &dci_hop2);

        if(args.verbose > 0)
          printf("Hop 2, HARQ round %d\n", round);

        // Clear eNB receive vector
        memset(phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0], 0, FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));

        // transmit from all relays
        for(k = 0; k < n_relays; k++) {
          // Clear txdataF vector
          memset(phy_vars_mr[k]->lte_ue_common_vars.txdataF[0], 0, 
              FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));

          // Generate transport channel parameters
          generate_ue_ulsch_params_from_dci(dci_hop2.dci_pdu, rnti_hop2, (subframe_hop2+6)%10, 
              format0, phy_vars_mr[k], SI_RNTI, RA_RNTI, P_RNTI, 0, 0);
          if(relay_role[k] == RELAY_ROLE_STANDARD) {
            phy_vars_mr[k]->ulsch_ue[0]->cooperation_flag = 0;
          }
          else {
            phy_vars_mr[k]->ulsch_ue[0]->cooperation_flag = 2;
          }

          // Generate uplink reference signal
          generate_drs_pusch(phy_vars_mr[k], 0, AMP, subframe_hop2, 0, N_RB);
    
          // Encode ULSCH data
          if(ulsch_encoding(mr_buffer[k], frame_parms, phy_vars_mr[k]->ulsch_ue[0],
                harq_pid_hop2, 1, 0, 1) == -1) {
            printf("ulsch_encoding failed\n");
            exit(1);
          }

          // Modulate ULSCH data
          ulsch_modulation(phy_vars_mr[k]->lte_ue_common_vars.txdataF, AMP, 0, subframe_hop2, 
              frame_parms, phy_vars_mr[k]->ulsch_ue[0]);

          // Compute number of resource elements from coded bits and modulation order
          n_re_hop2 = n_coded_bits_hop2/get_Qm(args.mcs_hop2);

          if(args.verbose > 2)
            print_ulsch_ue_stats(phy_vars_mr[k]->ulsch_ue[0]);

          // OFDM modulation
          ofdm_modulation(phy_vars_mr[k]->lte_ue_common_vars.txdataF,
              phy_vars_mr[k]->lte_ue_common_vars.txdata, frame_parms, subframe_hop2, frame_parms->symbols_per_tti);

          tx_energy = signal_energy(&phy_vars_mr[k]->lte_ue_common_vars.txdata[0]
              [frame_parms->samples_per_tti*subframe_hop2], frame_parms->samples_per_tti);

          // Transmit over channel
          awgn_stddev = sqrt((double)tx_energy*((double)frame_parms->ofdm_symbol_size/(N_RB*12))/pow(10.0, ((double)snr_hop2[k])/10.0)/2.0);
          transmit_subframe(channels_hop2[k],
              phy_vars_mr[k]->lte_ue_common_vars.txdata, rxdata, frame_parms, 
              subframe_hop2, frame_parms->symbols_per_tti, awgn_stddev);

          // Accumulate received data at receiver
          for(l = 0; l < frame_parms->samples_per_tti; l++)
            phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0]
              [subframe_hop2*frame_parms->samples_per_tti + l] += 
              rxdata[0][subframe_hop2*frame_parms->samples_per_tti + l];
        }

        results.n_harq_tries_hop2[round]++;

        // Fill the last symbol of the frame with random data (used for SNR estimation?)
        for (i=0;i<OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES;i++) {
          ((short*) &phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0]
           [(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i] = 
            (short) ((awgn_stddev*gaussdouble(0.0,1.0)));
          ((short*) &phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0]
           [(frame_parms->samples_per_tti<<1) -frame_parms->ofdm_symbol_size])[2*i+1] = 
            (short) ((awgn_stddev*gaussdouble(0.0,1.0)));
        }

        // Generate eNB transport channel parameters
        generate_eNB_ulsch_params_from_dci(dci_hop2.dci_pdu, rnti_hop2, (subframe_hop2+6)%10, 
            format0, 0, phy_vars_ch_dest, SI_RNTI, RA_RNTI, P_RNTI, 0);

        // Front end processing at destination CH
        for(l = 0; l < frame_parms->symbols_per_tti>>1; l++)
          slot_fep_ul(frame_parms, &phy_vars_ch_dest->lte_eNB_common_vars, l, 2*subframe_hop2, 0, 0);
        for(l = 0; l < frame_parms->symbols_per_tti>>1; l++)
          slot_fep_ul(frame_parms, &phy_vars_ch_dest->lte_eNB_common_vars, l, 2*subframe_hop2+1, 0, 0);

        // Receive ULSCH data
        rx_ulsch(phy_vars_ch_dest, subframe_hop2, 0, 0, phy_vars_ch_dest->ulsch_eNB, 2);

        // Compute uncoded bit error rate (assuming data was decoded at all MR)
        raw_ber = compute_ber_soft(phy_vars_mr[0]->ulsch_ue[0]->b_tilde,
            phy_vars_ch_dest->lte_eNB_pusch_vars[0]->llr, n_coded_bits_hop2);
        results.ber_hop2 += raw_ber;
        n_ber_frames_hop2++;
        results.n_frames_hop2++;
        if(args.verbose > 0) {
          printf("Received %d bits at dest CH, raw BER: %f\n", n_coded_bits_hop2, raw_ber);
        }

        // Decode ULSCH data
        n_iter = ulsch_decoding(phy_vars_ch_dest, 0, subframe_hop2, 0, 1);

        if(args.verbose > 2)
          print_ulsch_eNB_stats(phy_vars_ch_dest->ulsch_eNB[0]);

        if(n_iter <= MAX_TURBO_ITERATIONS) {
          if(args.verbose > 0)
            printf("Successfully decoded at dest CH\n");
          decoded_at_ch = true;
          results.n_harq_success_hop2[round]++;
          results.n_pdu_success_hop2++;
          results.n_bits_hop2 += tbs_hop2;
          rounds_hop2 = round;
        }

        // Write debug output if requested
        if(args.debug_output) {
          for(k = 0; k < n_relays; k++) {
            snprintf(fnbuf, 80, "hop2_r%d_mr%d_txdataFv.m", round, k);
            snprintf(varbuf, 80, "hop2_r%d_mr%d_txdataF", round, k);
            write_output(fnbuf, varbuf, phy_vars_mr[k]->lte_ue_common_vars.txdataF[0],
                FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX, 1, 1);
            snprintf(fnbuf, 80, "hop2_r%d_mr%d_txdatav.m", round, k);
            snprintf(varbuf, 80, "hop2_r%d_mr%d_txdata", round, k);
            write_output(fnbuf, varbuf, phy_vars_mr[k]->lte_ue_common_vars.txdata[0], 
                10*frame_parms->samples_per_tti, 1, 1);
          }
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdatav.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdata", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_common_vars.rxdata[0][0], 
              10*frame_parms->samples_per_tti, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataFv.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_common_vars.rxdataF[0][0],
              20*frame_parms->ofdm_symbol_size*2*n_symbols_per_slot, 2, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_ext2v.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_ext2", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_ext2[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp_0.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp_0", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp_0[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_rxdataF_comp_1.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_rxdataF_comp_1", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->rxdataF_comp_1[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_drs_ch_estimates_0.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_drs_ch_estimates_0", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->drs_ch_estimates_0[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
          snprintf(fnbuf, 80, "hop2_r%d_ch_drs_ch_estimates_1.m", round);
          snprintf(varbuf, 80, "hop2_r%d_ch_drs_ch_estimates_1", round);
          write_output(fnbuf, varbuf, phy_vars_ch_dest->lte_eNB_pusch_vars[0]->drs_ch_estimates_1[0][0],
              12*phy_vars_ch_dest->lte_frame_parms.N_RB_UL*n_symbols_per_slot*2, 1, 1);
        }
      }

      if(decoded_at_ch) {
        if(!block_valid(input_buffer, phy_vars_ch_dest->ulsch_eNB[0]->harq_processes[harq_pid_hop2]->b,
              input_buffer_length)) {
          printf("MAC PDU %d decoded successfully, but contained errors\n", pdu);
        }
        if(rounds_hop1 < 0 || rounds_hop1 >= MAX_HARQ_ROUNDS)
          fprintf(stderr, "rounds_hop1 has invalid value %d\n", rounds_hop1);
        else if(rounds_hop2 < 0 || rounds_hop2 >= MAX_HARQ_ROUNDS)
          fprintf(stderr, "rounds_hop2 has invalid value %d\n", rounds_hop2);
        else
          results.n_transmissions[rounds_hop1][rounds_hop2]++;
      }
    }

    // Compute average BER for the links
    for(k = 0; k < n_relays; k++)
      if(n_ber_frames_hop1[k] > 0)
        results.ber_hop1[k] /= (double)n_ber_frames_hop1[k];
      else
        results.ber_hop1[k] = 0.0;
    if(n_ber_frames_hop2 > 0)
      results.ber_hop2 /= (double)n_ber_frames_hop2;
    else
      results.ber_hop2 = 0.0;

    print_results(&results);
    if(store_results)
      write_results_data(results_file, &results);
  }

  if(store_results)
    fclose(results_file);

  free_results(&results);
  free(input_buffer);
  free(rxdata[0]);
  free_broadcast_transport_channel(phy_vars_ch_src, phy_vars_mr, n_relays);
  free_distributed_transport_channel(phy_vars_ch_dest, phy_vars_mr, n_relays);
  free(snrs);
  for(k = 0; k < n_relays; k++) {
    free_sh_channel(channels_hop1[k]);
    free_sh_channel(channels_hop2[k]);
  }
  free_channel_vars(channel_vars);
  free(channels_hop1);
  free(channels_hop2);
  free(phy_vars_ch_src);
  free(phy_vars_ch_dest);
  for(k = 0; k < n_relays; k++)
    free(phy_vars_mr[k]);
  free(phy_vars_mr);
  free(frame_parms);

  return 0;
}

int parse_args(int argc, char** argv, args_t* args)
{
  int c;
  int k;
  bool snr_set;
  const struct option long_options[] = {
    {"mcs1", required_argument, NULL, 256},
    {"mcs2", required_argument, NULL, 257},
    {"snr", required_argument, NULL, 258},
    {"snr1", required_argument, NULL, 259},
    {"snr2", required_argument, NULL, 260},
    {"single", no_argument, NULL, 261},
    {"sweep", no_argument, NULL, 262},
    {"bsweep", no_argument, NULL, 263},
    {NULL, 0, NULL, 0}};

  args->n_relays = 2;
  args->debug_output = false;
  args->debug_print = false;
  args->verbose = 0;
  args->n_pdu = 1;
  args->n_harq = 4;
  args->mcs_hop1 = 0;
  args->mcs_hop2 = 0;
  args->channel_model = Rayleigh8;
  args->results_fn = 0;
  args->analysis = analysis_single;

  for(k = 0; k < args->n_relays; k++)
    args->snr_hop1[k] = 10.0;
  for(k = 0; k < args->n_relays; k++)
    args->snr_hop2[k] = 10.0;

  snr_set = false;
  while((c = getopt_long(argc, argv, "hovn:m:r:H:C:", long_options, NULL)) != -1) {
    switch(c) {
    case 'h':
      return 1;
    case 'o':
      args->debug_output = true;
      break;
    case 'v':
      args->verbose++;
      break;
    case 'n':
      args->n_pdu = atoi(optarg);
      break;
    case 'H':
      args->n_harq = atoi(optarg);
      break;
    case 'C':
      if(strcmp(optarg, "help") == 0)
        return 2;
      if(!parse_channel_model(optarg, &args->channel_model))
        return 1;
      break;
    case 'm':
      args->mcs_hop1 = args->mcs_hop2 = atoi(optarg);
      break;
    case 'r':
      args->results_fn = optarg;
      break;
    case 256:
      args->mcs_hop1 = atoi(optarg);
      break;
    case 257:
      args->mcs_hop2 = atoi(optarg);
      break;
    case 258:
      for(k = 0; k < args->n_relays; k++)
        args->snr_hop1[k] = atof(optarg);
      for(k = 0; k < args->n_relays; k++)
        args->snr_hop2[k] = atof(optarg);
      snr_set = true;
      break;
    case 259:
      if(!parse_snr(optarg, args->snr_hop1, args->n_relays))
        return 1;
      snr_set = true;
      break;
    case 260:
      if(!parse_snr(optarg, args->snr_hop2, args->n_relays))
        return 1;
      snr_set = true;
      break;
    case 261:
      args->analysis = analysis_single;
      break;
    case 262:
      args->analysis = analysis_sweep;
      break;
    case 263:
      args->analysis = analysis_bsweep;
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
  if(strcmp(str, "A") == 0) *model = SCM_A;
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
  printf("  -v         : increase verbosity level [0]\n");
  printf("  -o         : output MATLAB signal files (implies -n 1) [no]\n");
  printf("  -r FILE    : write results to FILE\n");
  printf("  -n NUM     : simulate NUM MAC PDUs [1]\n");
  printf("  -H NUM     : do NUM HARQ rounds in each hop [4]\n");
  printf("     note: the hop 1 RVs are 0,0,1,1,2,2,3,3,0,0,..., the hop 2 RV is always 0\n");
  printf("  -C CHANNEL : set the channel model, use -C help for available models [Rayleigh8]\n");
  printf("\n");
  printf("    SNR options:\n");
  printf("  --snr SNR  : set snr for all links to SNR [10.0]\n");
  printf("  --snr1 SNR : set snr for hop 1 to SNR\n");
  printf("  --snr2 SNR : set snr for hop 2 to SNR\n");
  printf("      for --snr1 and --snr2, SNR may be either a single value or a vector with n_relays elements\n");
  printf("      e.g.: --snr1 \"4.0 6.0\" sets the SNR from CH1 to MR1 and MR2 to 4.0 and 6.0, respectively\n");
  printf("\n");
  printf("    Analysis setup (only one may be specified):\n");
  printf("  --single : single point analysis [default]\n");
  printf("  --sweep  : sweep snr of first relay of both hops [-10..+10]\n");
  printf("  --bsweep : sweep first relay of hop 1 [-10..+10], first relay of hop 2 [+10..-10]\n");
  printf("      note: the swept range is relative to the specified SNR\n");
  printf("\n");
  printf("    Modulation and coding options:\n");
  printf("  -m MCS     : set mcs for both hops to MCS [0]\n");
  printf("  --mcs1 MCS : set mcs for hop 1 to MCS\n");
  printf("  --mcs2 MCS : set mcs for hop 2 to MCS\n");
  printf("\n");
  printf("    Definition of results:\n");
  printf("  BLER is the fraction of MAC PDUs that are not successfully delivered from CH1 to CH2\n");
  printf("  BER is the fraction of coded bits that are incorrectly received for each link\n");
  printf("  avg bits/frame is TBS*n_pdu/n_frames, where n_pdu is the number of successfully received MAC PDUs\n");
  printf("    (for hop 1: decoded by both MR), and n_frames is the number of LTE frames transmitted for the hop\n");
  printf("  norm. delay is the average number of LTE frames required for the successful transmission of a MAC PDU\n");
}

void print_channel_usage()
{
  printf("Available channel models:\n");
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

void signal_handler(int sig) 
{
  void *array[10];
  size_t size;

  size = backtrace(array, 10);
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

void setup_snrs_single(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays)
{
  int k;

  *snrs = malloc(2*n_relays*sizeof(double));

  for(k = 0; k < n_relays; k++)
    (*snrs)[k] = snr_hop1[k];
  for(k = 0; k < n_relays; k++)
    (*snrs)[k+n_relays] = snr_hop2[k];

  *n_tests = 1;
}

void setup_snrs_sweep(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end)
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

void setup_snrs_bsweep(double** snrs, int* n_tests, double* snr_hop1, double* snr_hop2, int n_relays, double step, int start, int end)
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

void setup_frame_params(LTE_DL_FRAME_PARMS* frame_parms, unsigned char transmission_mode)
{
  // alloc max_xface?

  frame_parms->N_RB_DL = N_RB;
  frame_parms->N_RB_UL = N_RB;
  frame_parms->Nid_cell = NID_CELL;
  frame_parms->Ncp = cp_type;
  frame_parms->Ncp_UL = cp_type;
  frame_parms->nushift = 0;
  frame_parms->frame_type = 1; // TDD frames
  // TODO: TDD config 1 needs changes to subframe2harq_pid
  frame_parms->tdd_config = 2; // TDD frame type 1
  frame_parms->mode1_flag = (transmission_mode == 1 ? 1 : 0);
  frame_parms->nb_antennas_tx = n_txantenna_ch;
  frame_parms->nb_antennas_rx = n_rxantenna_mr;

  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled = 1;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = 0;
  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH = 0;

  init_frame_parms(frame_parms, oversampling);
  phy_init_top(frame_parms);
  frame_parms->twiddle_fft = twiddle_fft;
  frame_parms->twiddle_ifft = twiddle_ifft;
  frame_parms->rev = rev;

  phy_init_lte_top(frame_parms);
  init_ul_hopping(frame_parms);
  dump_frame_parms(frame_parms);
}

void alloc_broadcast_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays, u16 rnti)
{
  int k;

  // Workaround for memory leak:
  phy_vars_ch->dlsch_eNB[0][0] = new_eNB_dlsch(1, 8, 0);
  free(phy_vars_ch->dlsch_eNB[0][0]->harq_processes[0]->b);

  for(k = 0; k < n_relays; k++) {
    phy_vars_mr[k]->dlsch_ue[0][0] = new_ue_dlsch(1, 8, 0);
    phy_vars_mr[k]->dlsch_ue[0][0]->mode1_flag = 0;
    memset(phy_vars_mr[k]->dlsch_ue[0][0]->rb_alloc, 0, 16);
  }
}

void free_broadcast_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays)
{
  int k;

  // Workaround for memory leak:
  phy_vars_ch->dlsch_eNB[0][0]->harq_processes[0]->b = 0;
  free_eNB_dlsch(phy_vars_ch->dlsch_eNB[0][0]);
  for(k = 0; k < n_relays; k++) {
    free_ue_dlsch(phy_vars_mr[k]->dlsch_ue[0][0]);
  }
}

void alloc_distributed_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays, u16 rnti)
{
  int k;
  int l;

  for(k = 0; k < n_relays; k++) {
    phy_vars_mr[k]->ulsch_ue[0] = new_ue_ulsch(3, 0);
    phy_vars_mr[k]->ulsch_ue[0]->o_ACK[0] = 0;
    phy_vars_mr[k]->ulsch_ue[0]->o_ACK[1] = 0;
    phy_vars_mr[k]->ulsch_ue[0]->o_ACK[2] = 0;
    phy_vars_mr[k]->ulsch_ue[0]->o_ACK[3] = 0;
    for(l = 0; l < 3; l++)
      if(phy_vars_mr[k]->ulsch_ue[0]->harq_processes[l]) {
        phy_vars_mr[k]->ulsch_ue[0]->harq_processes[l]->status = DISABLED;
        phy_vars_mr[k]->ulsch_ue[0]->harq_processes[l]->B = 0;
      }
  }
  phy_vars_ch->ulsch_eNB[0] = new_eNB_ulsch(3, 0);
}

void free_distributed_transport_channel(PHY_VARS_eNB* phy_vars_ch, PHY_VARS_UE** phy_vars_mr, int n_relays)
{
  int k;

  for(k = 0; k < n_relays; k++) {
    free_ue_ulsch(phy_vars_mr[k]->ulsch_ue[0]);
  }
  //free_eNB_ulsch(phy_vars_ch->ulsch_eNB[0]);
}

void setup_broadcast_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs)
{
  DCI1_5MHz_TDD_t* dci_data = (DCI1_5MHz_TDD_t*) dci->dci_pdu;

  memset(dci, 0, sizeof(DCI_ALLOC_t));

  dci_data->dai = 1;
  dci_data->TPC = 0;
  dci_data->rv = (harq_round >> 1) & 0x03;
  dci_data->ndi = (harq_round == 0 ? 1 : 0);
  dci_data->harq_pid = 0;
  dci_data->mcs = mcs;
  dci_data->rballoc = rb_alloc;
  dci_data->rah = 0;

  dci->dci_length = sizeof_DCI1_5MHz_TDD_t;
  dci->L = 1;
  dci->rnti = rnti;
  dci->format = format1;
}

void setup_distributed_dci(DCI_ALLOC_t* dci, u16 rnti, int harq_round, int mcs)
{
  DCI0_5MHz_TDD_1_6_t* dci_data = (DCI0_5MHz_TDD_1_6_t*) dci->dci_pdu;

  memset(dci, 0, sizeof(DCI_ALLOC_t));

  dci_data->cqi_req = 0;
  dci_data->dai = 1;
  dci_data->cshift = 0;
  dci_data->TPC = 0;
  dci_data->ndi = (harq_round == 0 ? 1 : 0);
  dci_data->mcs = mcs;
  dci_data->rballoc = computeRIV(N_RB,0,N_RB);
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
  return v;
}

void free_channel_vars(channel_vars_t v)
{
  free(v.s_re[0]);
  free(v.s_im[0]);
  free(v.r_re[0]);
  free(v.r_im[0]);
}

sh_channel_t* alloc_sh_channel(channel_vars_t* cvars, SCM_t channel_model, int n_txantennas, int n_rxantennas)
{
  sh_channel_t* ch = malloc(sizeof(sh_channel_t));

  ch->cvars = cvars;
  ch->channel = new_channel_desc_scm(n_txantennas, n_rxantennas, channel_model, BW, 0.0, 0, 0.0);

  return ch;
}

void free_sh_channel(sh_channel_t* c)
{
  free(c->channel);
}

void transmit_subframe(sh_channel_t* channel, s32** src, s32** dst, LTE_DL_FRAME_PARMS* frame_parms, u8 subframe, u8 nsymb, double awgn_stddev)
{
  int k;

  for(k = 0; k < frame_parms->samples_per_tti; k++) {
    channel->cvars->s_re[0][k] = (double)((s16*)src[0])[2*subframe*frame_parms->samples_per_tti + (k<<1)];
    channel->cvars->s_im[0][k] = (double)((s16*)src[0])[2*subframe*frame_parms->samples_per_tti + (k<<1) + 1];
    //channel->cvars->s_re[1][k] = 0;
    //channel->cvars->s_im[1][k] = 0;
  }

  multipath_channel(channel->channel, channel->cvars->s_re, channel->cvars->s_im,
      channel->cvars->r_re, channel->cvars->r_im,
      frame_parms->samples_per_tti, 0);

  for(k = 0; k < frame_parms->samples_per_tti; k++) {
    ((s16*)dst[0])[2*subframe*frame_parms->samples_per_tti + (k<<1)] = 
      (s16) (channel->cvars->r_re[0][k] + awgn_stddev*gaussdouble(0.0, 1.0));
    ((s16*)dst[0])[2*subframe*frame_parms->samples_per_tti + (k<<1) + 1] = 
      (s16) (channel->cvars->r_im[0][k] + awgn_stddev*gaussdouble(0.0, 1.0));
  }
}

void ofdm_fep(PHY_VARS_UE* phy_vars_mr, u8 subframe)
{
  int n_symbols_per_slot = (phy_vars_mr->lte_frame_parms.Ncp == 0 ? 7 : 6);
  int slot;
  int symbol;

  //slot = subframe;
  for(slot = 2*subframe; slot < 2*subframe+2; slot++)
    for(symbol = 0; symbol < n_symbols_per_slot; symbol++)
      slot_fep(phy_vars_mr, symbol, slot, 0, 0);
  slot_fep(phy_vars_mr, 0, 2*subframe+2, 0, 0);
}

int rx_dlsch_symbol(PHY_VARS_UE* phy_vars, u8 subframe, u8 symbol, u8 first_symbol)
{
  int s;
  s = rx_pdsch(phy_vars, PDSCH, 0, 0, subframe, symbol, first_symbol, 0, 0);
  if(s == -1)
    printf("DLSCH receiver error\n");
  return s;
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

double compute_ber_soft(u8* ref, s16* rec, int n)
{
  int k;
  int e = 0;

  for(k = 0; k < n; k++)
    if((ref[k]==1) != (rec[k]<0)) {
      //      printf("error pos %d ( %d => %d)\n",k,ref[k],rec[k]);
      e++;
    }

  return (double)e / (double)n;
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
    printf("UE ulsch: Nsymb_pusch=%d, O=%d, o_ACK=%d %d %d %d, O_ACK=%d, Mdlharq=%d, n_DMRS2=%d, cooperation_flag=%d\n",
        d->Nsymb_pusch, d->O, d->o_ACK[0], d->o_ACK[1], d->o_ACK[2], d->o_ACK[3], d->O_ACK, d->Mdlharq, d->n_DMRS2, d->cooperation_flag);
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
    printf("eNB ulsch: Nsymb_pusch=%d, Mdlharq=%d, cqi_crc_status=%d, Or1=%d, Or2=%d, o_RI=%d %d, O_RI=%d, o_ACK=%d %d %d %d, O_ACK=%d, o_RCC=%d, beta_offset_cqi_times8=%d, beta_offset_ri_times8=%d, beta_offset_harqack_times8=%d, rnti=%x, n_DMRS2=%d, cyclicShift=%d, cooperation_flag=%d\n",
        d->Nsymb_pusch, d->Mdlharq, d->cqi_crc_status, d->Or1, d->Or2, d->o_RI[0], d->o_RI[1], d->O_RI, d->o_ACK[0], d->o_ACK[1], d->o_ACK[2], d->o_ACK[3], d->O_ACK, d->o_RCC, d->beta_offset_cqi_times8, d->beta_offset_ri_times8, d->beta_offset_harqack_times8, d->rnti, d->n_DMRS2, d->cyclicShift, d->cooperation_flag);
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
  }
  return 1;
}

void init_results(results_t* r, args_t* a)
{
  int k;

  r->n_relays = a->n_relays;
  r->n_pdu = a->n_pdu;
  r->n_harq = a->n_harq;
  r->channel_model = a->channel_model;
  r->mcs_hop1 = malloc(a->n_pdu*sizeof(int*));
  r->mcs_hop2 = malloc(a->n_pdu*sizeof(int*));
  r->tbs_hop1 = malloc(a->n_pdu*sizeof(int*));
  r->tbs_hop2 = malloc(a->n_pdu*sizeof(int*));
  r->n_rb_hop1 = malloc(a->n_pdu*sizeof(int*));
  r->n_rb_hop2 = malloc(a->n_pdu*sizeof(int*));
  for(k = 0; k < a->n_pdu; k++) {
    r->mcs_hop1[k] = malloc(a->n_harq*sizeof(int));
    r->mcs_hop2[k] = malloc(a->n_harq*sizeof(int));
    r->tbs_hop1[k] = malloc(a->n_harq*sizeof(int));
    r->tbs_hop2[k] = malloc(a->n_harq*sizeof(int));
    r->n_rb_hop1[k] = malloc(a->n_harq*sizeof(int));
    r->n_rb_hop2[k] = malloc(a->n_harq*sizeof(int));
  }
  r->relay_activity = malloc((1 << a->n_relays)*sizeof(int));
}

void clear_results(results_t* r)
{
  int k;
  r->snr_hop1 = 0;
  r->snr_hop2 = 0;
  r->n_frames_hop1 = 0;
  r->n_frames_hop2 = 0;
  r->n_bits_hop1 = 0;
  r->n_bits_hop2 = 0;
  for(k = 0; k < MAX_RELAYS; k++) {
    r->ber_hop1[k] = 0.0;
  }
  r->ber_hop2 = 0.0;
  r->n_pdu_success_hop1 = 0;
  r->n_pdu_success_hop2 = 0;
  for(k = 0; k < MAX_HARQ_ROUNDS; k++) {
    r->n_harq_tries_hop1[k] = 0;
    r->n_harq_tries_hop2[k] = 0;
    r->n_harq_success_hop1[k] = 0;
    r->n_harq_success_hop2[k] = 0;
  }
  for(k = 0; k < r->n_pdu; k++) {
    memset(r->mcs_hop1[k], 0, r->n_harq*sizeof(int));
    memset(r->mcs_hop2[k], 0, r->n_harq*sizeof(int));
    memset(r->tbs_hop1[k], 0, r->n_harq*sizeof(int));
    memset(r->tbs_hop2[k], 0, r->n_harq*sizeof(int));
    memset(r->n_rb_hop1[k], 0, r->n_harq*sizeof(int));
    memset(r->n_rb_hop2[k], 0, r->n_harq*sizeof(int));
  }
  memset(r->n_transmissions, 0, MAX_HARQ_ROUNDS*MAX_HARQ_ROUNDS*sizeof(int));
  memset(r->relay_activity, 0, (1 << (r->n_relays))*sizeof(int));
}

void free_results(results_t* r)
{
  int k;
  for(k = 0; k < r->n_pdu; k++) {
    free(r->mcs_hop1[k]);
    free(r->mcs_hop2[k]);
    free(r->tbs_hop1[k]);
    free(r->tbs_hop2[k]);
    free(r->n_rb_hop1[k]);
    free(r->n_rb_hop2[k]);
  }
  free(r->mcs_hop1);
  free(r->mcs_hop2);
  free(r->tbs_hop1);
  free(r->tbs_hop2);
  free(r->n_rb_hop1);
  free(r->n_rb_hop2);
  free(r->relay_activity);
}

void print_results(results_t* r)
{
  int k;

  printf("Hop 1: SNR (");
  for(k = 0; k < r->n_relays; k++)
    printf("%.1f%s", r->snr_hop1[k], k < r->n_relays-1 ? ", " : "");
  printf("), BER (");
  for(k = 0; k < r->n_relays; k++)
    printf("%f%s", r->ber_hop1[k], k < r->n_relays-1 ? ", " : "");
  printf(")\n");
  printf("  avg bits/frame %f, avg frames/received PDU %f, avg delay %f\n",
      (double)r->n_bits_hop1/(double)r->n_frames_hop1,
      (double)r->n_frames_hop1/(double)r->n_pdu_success_hop1,
      (double)(r->n_frames_hop1-(r->n_harq_tries_hop1[0]-r->n_pdu_success_hop1)*r->n_harq)/(double)r->n_pdu_success_hop1);
  printf("  HARQ (n_success/n_tries):");
  for(k = 0; k < r->n_harq; k++)
    printf(" %d/%d", r->n_harq_success_hop1[k], r->n_harq_tries_hop1[k]);
  printf("\n");
  printf("Hop 2: SNR (");
  for(k = 0; k < r->n_relays; k++)
    printf("%.1f%s", r->snr_hop2[k], k < r->n_relays-1 ? ", " : "");
  printf("), BER %f\n", r->ber_hop2);
  printf("  avg bits/frame %f, avg frames/received PDU %f, avg delay %f\n", 
      (double)r->n_bits_hop2/(double)r->n_frames_hop2,
      (double)r->n_frames_hop2/(double)r->n_pdu_success_hop2,
      (double)(r->n_frames_hop2-(r->n_harq_tries_hop2[0]-r->n_pdu_success_hop2)*r->n_harq)/(double)r->n_pdu_success_hop2);
  printf("  HARQ (n_success/n_tries):");
  for(k = 0; k < r->n_harq; k++)
    printf(" %d/%d", r->n_harq_success_hop2[k], r->n_harq_tries_hop2[k]);
  printf("\n");
  printf("Collaborative link BLER: %d/%d\n", r->n_pdu-r->n_pdu_success_hop2, r->n_pdu);
  printf("Relay activity:");
  if(r->n_relays == 2)
    printf(" MR1(%d) MR2(%d) MR1+MR2(%d)\n", r->relay_activity[1], r->relay_activity[2], r->relay_activity[3]);
  else {
    for(k = 1; k < (1 << r->n_relays); k++)
      printf(" %d", r->relay_activity[k]);
    printf("\n");
  }
}

void write_results_header(FILE* f, results_t* r, int n_tests)
{
  fprintf(f, "%d %d %d %d %d\n", r->n_relays, r->channel_model, n_tests, r->n_pdu, r->n_harq);
}
void write_results_data(FILE* f, results_t* r)
{
  int k;
  int l;
  for(k = 0; k < r->n_relays; k++)
    fprintf(f, "%f ", r->snr_hop1[k]);
  fprintf(f, "\n");
  for(k = 0; k < r->n_relays; k++)
    fprintf(f, "%f ", r->snr_hop2[k]);
  fprintf(f, "\n");

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
      fprintf(f, "%d ", r->n_rb_hop1[l][k]);
    fprintf(f, "\n");
  }
  for(l = 0; l < r->n_pdu; l++) {
    for(k = 0; k < r->n_harq; k++)
      fprintf(f, "%d ", r->n_rb_hop2[l][k]);
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
}

