
#include "PHY/types.h"
#include "SIMULATION/TOOLS/defs.h"
#include <stdio.h>

#ifndef _FEMTO_UTILS
#define  _FEMTO_UTILS


typedef struct {
	 double snr_init;
	 double snr_max;	 
	 double snr_step;
	 int nframes;
	 int extended_prefix_flag;  
	 u8 frame_type;				//Frame type (0 FDD, 1 TDD). 
	 u8 transmission_mode;		//Transmission mode (1,2,6 for the moment)
	 u8 n_tx;
	 u8 n_rx;
	 int nInterf;
	 s8 *dbInterf;
	 u16 Nid_cell;	 
	 u8 oversampling;
	 SCM_t channel_model;
	 char interfLevels[100];
	 int awgn_flag;
	 int nsymb;
	 int num_layers;
	 u16 n_rnti;	
	 u8 mcs;				//Modulation and code scheme 
	 //TODO int n_ch_rlz = 1;   printf("-N Determines the number of Channel Realizations in Absraction mode. Default value is 1. \n");
	 
	 
	 u8 pilot1,pilot2,pilot3;
	 FILE *outputFile;
	 FILE *outputBler;
	 u8 num_rounds;
	 u8 subframe;
	 int eNB_id;
	 s16 amp;			//	Amplitude of QPSK symbols 
	 u8 dci_flag;		//1- Analysis  of errors on DCI, 0- No analysis of errors in DCI
	 int testNumber;
	 char folderName[50];
	 
	 char parameters[150];
	 
}options_t;

typedef struct{
	 double **s_re;
	 double **s_im;
	 double **r_re;
	 double **r_im;
}data_t;

void _parseOptions(options_t *opts, int argc, char ** argv);
void _printOptions(options_t *opts);
void _parseInterferenceLevels(s8 **dbInterf, char *interfLevels,int nInterf);
void _allocData(data_t *data, u8 n_tx,u8 n_rx,int Frame_length_complex_samples);
void _makeOutputDir(options_t *opts);


LTE_DL_FRAME_PARMS* _lte_param_init(options_t opts);
void _initDefaults(options_t *opts);
void _fill_Ul_CCCH_DLSCH_Alloc(options_t opts);
void _generatesRandomChannel(options_t opts);
void _allocBroadcastTransportChannel(options_t opts);
void _generateDCI(options_t opts,DCI_ALLOC_t *dci_alloc,DCI_ALLOC_t *dci_alloc_rx);//,u8 **input_buffer);
void _freeMemory(data_t data,options_t opts);
void _makeSimulation(data_t data,options_t opts,DCI_ALLOC_t *dci_alloc,DCI_ALLOC_t *dci_alloc_rx,u16 NB_RB,LTE_DL_FRAME_PARMS  *frame_parms);
void _printResults(u32 *errs,u32 *round_trials,u32 dci_errors,double rate);
void _printFileResults(double SNR, double rate,u32  *errs,u32  *round_trials,u32 dci_errors,options_t opts);
void _initErrsRoundsTrials(u32 **errs,u32 **trials,int allocFlag,options_t opts);
void _fillData(options_t opts,data_t data);
void _applyNoise(options_t opts,data_t data,double sigma2,double iqim);
u8 _generate_dci_top(int num_ue_spec_dci,int num_common_dci,DCI_ALLOC_t *dci_alloc,options_t opts,u8 num_pdcch_symbols);
void do_OFDM_mod(mod_sym_t **txdataF, s32 **txdata, u16 next_slot, LTE_DL_FRAME_PARMS *frame_parms);
void _apply_Multipath_Noise_Interference(options_t opts,data_t data,double sigma2_dB,double sigma2);
void _writeOuputOneFrame(options_t opts,u32 coded_bits_per_codeword,short *uncoded_ber_bit,u32 tbs);
void _dumpTransportBlockSegments(u32 C,u32 Cminus,u32 Kminus,u32 Kplus,  u8 ** c_UE,u8 ** c_eNB);
#endif
