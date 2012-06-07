
#include "PHY/types.h"
#include "SIMULATION/TOOLS/defs.h"
#include <stdio.h>

#ifndef _FEMTO_UTILS
#define  _FEMTO_UTILS

/*! \file SIMULATION/LTE_FEMTO/femtoUtils.h
* \brief Defines structure and function  headers 
* \author L. Garcia
* \date 2012
* \version 0.1
* \company Eurecom
* \note
* \warning
*/

///Top-level data Structure for general options of simulation and other global variables
typedef struct {
	
    double snr_init;
    double snr_max;
    double snr_step;
    int nframes;
    int extended_prefix_flag;
    ///Frame type (0 FDD, 1 TDD).
    u8 frame_type;				
    ///Transmission mode (1 for the moment)
    u8 transmission_mode;		
    ///Number of Transmit antennas in node.  
    u8 n_tx;
    ///Number of Receive antennas in node. 
    u8 n_rx;
    ///Number of interference to simulate
    int nInterf;
    ///Array with interference  level in dB 
    s8 *dbInterf;
    char interfLevels[100];
    
    u16 Nid_cell;
    u8 oversampling;
    SCM_t channel_model;    
    int awgn_flag;
    int nsymb;
    int num_layers;
    u16 n_rnti;
    ///Modulation and code scheme
    u8 mcs;				    


    u8 pilot1,pilot2,pilot3;
    ///Pointer  to the output file SNRvsBLER
    FILE *outputFile;
    ///Pointer  to the output file errors and trials for each SNR 
    FILE *outputBler;
    
    u8 num_rounds;
    u8 subframe;
    int eNB_id;
    ///	Amplitude of QPSK symbols
    s16 amp;			
    ///1- Analysis  of errors on DCI, 0- No analysis of errors in DCI
    u8 dci_flag;		    
    int testNumber;
    char folderName[50];
    char parameters[150];

} options_t;

/// Store signal data
typedef struct {
    double **s_re;
    double **s_im;
    double **r_re;
    double **r_im;
} data_t;

/// Parses the command line options and assigns values ​​to  pilots, num_symbols, etc.  modified by the options selected
void _parseOptions(options_t *opts, int argc, char ** argv);
void _printOptions(options_t *opts);
/// Interference Levels are recivend in form  num,num,num this function parse the string and fill dbInterf array 
void _parseInterferenceLevels(s8 **dbInterf, char *interfLevels,int nInterf);
/// Allocate memory  for signal data arrays 
void _allocData(data_t *data, u8 n_tx,u8 n_rx,int Frame_length_complex_samples);
/// Generate output dir with the prefix specified in testNumber
void _makeOutputDir(options_t *opts);

/// Initializes  lte_frame_parms structure and make the structures of the  eNB and EU involved in the simulation, including interference, if any.
LTE_DL_FRAME_PARMS* _lte_param_init(options_t opts);
/// Set defaults values for the simulations
void _initDefaults(options_t *opts);
/// Allocate and fill  UL, CCCH and DLSCH structures
void _fill_Ul_CCCH_DLSCH_Alloc(options_t opts);
/// Allocate the eNB2UE structure for transmision and interference
void _generatesRandomChannel(options_t opts);
/** @brief This function allocates structures for a particular DLSCH at eNB.
 *
 * modifications  will be required with more than 2 users
 * It's used to generate the dci. they contain informations such as the modulation and coding scheme
 * either if the transmit power control is required or not and so on...
 * dlsch_eNB can suport 8 users.
 * Create transport channel structures for 2 transport blocks (MIMO)
*/
void _allocDLSChannel(options_t opts);
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
