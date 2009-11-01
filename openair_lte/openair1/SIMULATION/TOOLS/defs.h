#include "PHY/TOOLS/defs.h"

unsigned int *generate_gauss_LUT(unsigned char,unsigned char);
int gauss(unsigned int *,unsigned char);

double gaussdouble(double,double);
void randominit(void);
 

//void QAM_input(struct complex *,short,int,int,char);

void set_taus_seed(void);
unsigned int taus(void);
//void ofdm_channel(double *,double, int,double,int,struct complex *);

void random_channel(double *amps,
		    double t_max, 
		    int channel_length,
		    double bw,
		    struct complex *ch,
		    double ricean_factor,
		    struct complex *phase);

void multipath_channel(struct complex **ch,
		       double **tx_sig_re, 
		       double **tx_sig_im, 
		       double **rx_sig_re,
		       double **rx_sig_im,
		       double *amps, 
		       double Td, 
		       double BW, 
		       double ricean_factor,
		       double aoa,
		       unsigned char nb_antennas_tx,
		       unsigned char nb_antennas_rx,
		       unsigned int length,
		       unsigned int channel_length,
		       unsigned int path_loss_dB);
