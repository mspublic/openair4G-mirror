#include <math.h>
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#include "PHY/TOOLS/defs.h"
#include "defs.h"
#include "scm_corrmat.h"

//#define DEBUG_CH

channel_desc_t *new_channel_desc(u8 nb_tx,
				 u8 nb_rx, 
				 u8 nb_taps, 
				 u8 channel_length, 
				 double *amps, 
				 double *delays, 
				 double complex** R_sqrt, 
				 double Td, 
				 double BW, 
				 double ricean_factor, 
				 double aoa, 
				 double forgetting_factor,
				 double max_Doppler, 
				 s32 channel_offset, 
				 double path_loss_dB,
				 u8 random_aoa) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i,j;
  double delta_tau;

  msg("[CHANNEL] Getting new channel descriptor, nb_tx %d, nb_rx %d, nb_taps %d, channel_length %d\n",
      nb_tx,nb_rx,nb_taps,channel_length);

  chan_desc->nb_tx          = nb_tx;
  chan_desc->nb_rx          = nb_rx;
  chan_desc->nb_taps        = nb_taps;
  chan_desc->channel_length = channel_length;
  chan_desc->amps           = amps;
  msg("[CHANNEL] Doing delays ...\n");
  if (delays==NULL) {
    chan_desc->delays = (double*) malloc(nb_taps*sizeof(double));
    delta_tau = Td/nb_taps;
    for (i=0; i<nb_taps; i++)
      chan_desc->delays[i] = ((double)i)*delta_tau;
  }
  else
    chan_desc->delays         = delays;

  chan_desc->Td             = Td;
  chan_desc->BW             = BW;
  chan_desc->ricean_factor  = ricean_factor;
  chan_desc->aoa            = aoa;
  chan_desc->random_aoa  = random_aoa;
  chan_desc->forgetting_factor = forgetting_factor;
  chan_desc->channel_offset = channel_offset;
  chan_desc->path_loss_dB   = path_loss_dB;
  chan_desc->first_run      = 1;
  chan_desc->ip             = 0.0;
  chan_desc->max_Doppler    = max_Doppler;
  chan_desc->ch             = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
  chan_desc->chF            = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
  chan_desc->a              = (double complex**) malloc(nb_taps*sizeof(double complex*));

  msg("[CHANNEL] Filling ch \n");

  for (i = 0; i<nb_tx*nb_rx; i++) 
    chan_desc->ch[i] = (double complex*) malloc(channel_length * sizeof(double complex)); 

  for (i = 0; i<nb_tx*nb_rx; i++) 
    chan_desc->chF[i] = (double complex*) malloc(1200 * sizeof(double complex));  // allocate for up to 100 RBs, 12 samples per RB

  msg("[CHANNEL] Filling a (nb_taps %d)\n",nb_taps);
  for (i = 0; i<nb_taps; i++) {
    msg("tap %d (%p,%d)\n",i,&chan_desc->a[i],nb_tx*nb_rx * sizeof(double complex));
    chan_desc->a[i]         = (double complex*) malloc(nb_tx*nb_rx * sizeof(double complex));
  }

  msg("[CHANNEL] Doing R_sqrt ...\n");
  if (R_sqrt == NULL) {
    chan_desc->R_sqrt         = (double complex**) malloc(nb_taps*sizeof(double complex*));
    for (i = 0; i<nb_taps; i++) {
      chan_desc->R_sqrt[i]    = (double complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(double complex));
      for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	chan_desc->R_sqrt[i][j] = 1.0;
      }
    }
  }
  else {
    chan_desc->R_sqrt = R_sqrt;
  }

  for (i = 0; i<nb_taps; i++) {
    for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
      printf("Rsqrt[%d][%d] %f %f\n",i,j,creal(chan_desc->R_sqrt[i][j]),cimag(chan_desc->R_sqrt[i][j]));
    }
  }

  printf("[CHANNEL] RF %f\n",chan_desc->ricean_factor);
  for (i=0;i<chan_desc->nb_taps;i++)
    printf("[CHANNEL] tap %d: amp %f, delay %f\n",i,chan_desc->amps[i],chan_desc->delays[i]);

  chan_desc->nb_paths = 10;

  return(chan_desc);
}

double scm_c_delays[] = {0, 0.0125, 0.0250, 0.3625, 0.3750, 0.3875, 0.2500, 0.2625, 0.2750, 1.0375, 1.0500, 1.0625, 2.7250, 2.7375, 2.7500, 4.6000, 4.6125, 4.6250};
double scm_c_amps_dB[] = {0.00, -2.22, -3.98, -1.86, -4.08, -5.84, -1.08, -3.30, -5.06, -9.08, -11.30, -13.06, -15.14, -17.36, -19.12, -20.64, -22.85, -24.62};

double epa_delays[] = { 0,.03,.07,.09,.11,.19,.41};
double epa_amps_dB[] = {0.0,-1.0,-2.0,-3.0,-8.0,-17.2,-20.8};

double eva_delays[] = { 0,.03,.15,.31,.37,.71,1.09,1.73,2.51};
double eva_amps_dB[] = {0.0,-1.5,-1.4,-3.6,-0.6,-9.1,-7.0,-12.0,-16.9};

double etu_delays[] = { 0,.05,.12,.2,.23,.5,1.6,2.3,5.0};
double etu_amps_dB[] = {-1.0,-1.0,-1.0,0.0,0.0,0.0,-3.0,-5.0,-7.0};

double default_amps_lin[] = {0.3868472 , 0.3094778 , 0.1547389 , 0.0773694 , 0.0386847 , 0.0193424 , 0.0096712 , 0.0038685};
double default_amp_lin[] = {1};

//correlation matrix for a 2x2 channel with full Tx correlation 
double complex R_sqrt_22_corr_tap[16] = {0.70711, 0.0, 0.70711, 0.0, 
					0.0, 0.70711,0.0,0.70711,
					0.70711,0.0, 0.70711,0.0, 
					0.0,0.70711,0.0,0.70711};
double complex *R_sqrt_22_corr[1]     = {R_sqrt_22_corr_tap};

//correlation matrix for a fully correlated 2x1 channel (h1==h2)
double complex R_sqrt_21_corr_tap[4]  = {0.70711,0.70711, 0.70711, 0.70711}; 
double complex *R_sqrt_21_corr[1]      = {R_sqrt_21_corr_tap};

//correlation matrix for a 2x2 channel with full Tx anti-correlation 
double complex R_sqrt_22_anticorr_tap[16] = {0.70711, 0.0, -0.70711, 0.0, 
					     0.0, 0.70711, 0.0, -0.70711,
					     -0.70711, 0.0, 0.70711, 0.0, 
					     0.0, -0.70711, 0.0, 0.70711};
double complex *R_sqrt_22_anticorr[1]     = {R_sqrt_22_anticorr_tap};

//correlation matrix for a fully anti-correlated 2x1 channel (h1==-h2)
double complex R_sqrt_21_anticorr_tap[4]  = {0.70711,-0.70711, -0.70711,0.70711}; 
double complex *R_sqrt_21_anticorr[1]     = {R_sqrt_21_anticorr_tap};

double complex **R_sqrt_ptr2;


channel_desc_t *new_channel_desc_scm(u8 nb_tx, 
				     u8 nb_rx, 
				     SCM_t channel_model, 
				     double BW, 
				     double forgetting_factor,
				     s32 channel_offset, 
				     double path_loss_dB) {

  channel_desc_t *chan_desc = (channel_desc_t *)malloc(sizeof(channel_desc_t));
  u16 i,j;
  double sum_amps;
  double aoa,ricean_factor,Td,maxDoppler;
  int channel_length,nb_taps;

  chan_desc->nb_tx          = nb_tx;
  chan_desc->nb_rx          = nb_rx;
  chan_desc->BW             = BW;
  chan_desc->forgetting_factor = forgetting_factor;
  chan_desc->channel_offset = channel_offset;
  chan_desc->path_loss_dB   = path_loss_dB;
  chan_desc->first_run      = 1;
  chan_desc->ip             = 0.0;

  printf("\nChannel Model (inside of new_channel_desc_scm)=%d\n\n", channel_model);

  switch (channel_model) {
  case SCM_A:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  case SCM_B:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  case SCM_C:
    chan_desc->nb_taps        = 18;
    chan_desc->Td             = 4.625;
    chan_desc->channel_length = (int) (2*chan_desc->BW*chan_desc->Td + 1 + 2/(M_PI*M_PI)*log(4*M_PI*chan_desc->BW*chan_desc->Td));
    sum_amps = 0;
    chan_desc->amps           = (double*) malloc(chan_desc->nb_taps*sizeof(double));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->amps[i]      = pow(10,.1*scm_c_amps_dB[i]); 
      sum_amps += chan_desc->amps[i];
    }
    for (i = 0; i<chan_desc->nb_taps; i++)
      chan_desc->amps[i] /= sum_amps;
    chan_desc->delays         = scm_c_delays;
    chan_desc->ricean_factor  = 1;
    chan_desc->aoa            = 0;
    chan_desc->random_aoa     = 0;
    chan_desc->ch             = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->chF            = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->a              = (double complex**) malloc(chan_desc->nb_taps*sizeof(double complex*));
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->ch[i] = (double complex*) malloc(chan_desc->channel_length * sizeof(double complex)); 
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->chF[i] = (double complex*) malloc(1200 * sizeof(double complex)); 
    for (i = 0; i<chan_desc->nb_taps; i++) 
      chan_desc->a[i]         = (double complex*) malloc(nb_tx*nb_rx * sizeof(double complex));

    chan_desc->R_sqrt  = (double complex**) malloc(6*sizeof(double complex**));
    if (nb_tx==2 && nb_rx==2) {
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R22_sqrt[i][0];
    }
    else if (nb_tx==2 && nb_rx==1) {
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R21_sqrt[i][0];
    }
    else if (nb_tx==1 && nb_rx==2) {
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R12_sqrt[i][0];
    }
    else {
      for (i = 0; i<6; i++) {
	chan_desc->R_sqrt[i]    = (double complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(double complex));
	for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	  chan_desc->R_sqrt[i][j] = 1.0;
	}
	msg("correlation matrix not implemented for nb_tx==%d and nb_rx==%d, using identity\n", nb_tx, nb_rx);
      }
    }
    break;
  case SCM_D:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  case EPA:
    chan_desc->nb_taps        = 7;
    chan_desc->Td             = .410;
    chan_desc->channel_length = (int) (2*chan_desc->BW*chan_desc->Td + 1 + 2/(M_PI*M_PI)*log(4*M_PI*chan_desc->BW*chan_desc->Td));
    sum_amps = 0;
    chan_desc->amps           = (double*) malloc(chan_desc->nb_taps*sizeof(double));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->amps[i]      = pow(10,.1*epa_amps_dB[i]); 
      sum_amps += chan_desc->amps[i];
    }
    for (i = 0; i<chan_desc->nb_taps; i++)
      chan_desc->amps[i] /= sum_amps;
    chan_desc->delays         = epa_delays;
    chan_desc->ricean_factor  = 1;
    chan_desc->aoa            = 0;
    chan_desc->random_aoa     = 0;
    chan_desc->ch             = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->chF            = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->a              = (double complex**) malloc(chan_desc->nb_taps*sizeof(double complex*));
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->ch[i] = (double complex*) malloc(chan_desc->channel_length * sizeof(double complex)); 
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->chF[i] = (double complex*) malloc(1200 * sizeof(double complex)); 
    for (i = 0; i<chan_desc->nb_taps; i++) 
      chan_desc->a[i]         = (double complex*) malloc(nb_tx*nb_rx * sizeof(double complex));
    if (nb_tx==2 && nb_rx==2) {
      chan_desc->R_sqrt  = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R22_sqrt[i][0];
    }
    else {
      chan_desc->R_sqrt         = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) {
	chan_desc->R_sqrt[i]    = (double complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(double complex));
	for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	  chan_desc->R_sqrt[i][j] = 1.0;
	}
	msg("correlation matrix only implemented for nb_tx==2 and nb_rx==2, using identity\n");
      }
    }
    break;
  case EVA:
    chan_desc->nb_taps        = 9;
    chan_desc->Td             = 2.51;
    chan_desc->channel_length = (int) (2*chan_desc->BW*chan_desc->Td + 1 + 2/(M_PI*M_PI)*log(4*M_PI*chan_desc->BW*chan_desc->Td));
    sum_amps = 0;
    chan_desc->amps           = (double*) malloc(chan_desc->nb_taps*sizeof(double));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->amps[i]      = pow(10,.1*eva_amps_dB[i]); 
      sum_amps += chan_desc->amps[i];
    }
    for (i = 0; i<chan_desc->nb_taps; i++)
      chan_desc->amps[i] /= sum_amps;
    chan_desc->delays         = eva_delays;
    chan_desc->ricean_factor  = 1;
    chan_desc->aoa            = 0;
    chan_desc->random_aoa     = 0;
    chan_desc->ch             = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->chF            = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->a              = (double complex**) malloc(chan_desc->nb_taps*sizeof(double complex*));
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->ch[i] = (double complex*) malloc(chan_desc->channel_length * sizeof(double complex)); 
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->chF[i] = (double complex*) malloc(1200 * sizeof(double complex)); 
    for (i = 0; i<chan_desc->nb_taps; i++) 
      chan_desc->a[i]         = (double complex*) malloc(nb_tx*nb_rx * sizeof(double complex));
    if (nb_tx==2 && nb_rx==2) {
      chan_desc->R_sqrt  = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R22_sqrt[i][0];
    }
    else {
      chan_desc->R_sqrt         = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) {
	chan_desc->R_sqrt[i]    = (double complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(double complex));
	for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	  chan_desc->R_sqrt[i][j] = 1.0;
	}
	msg("correlation matrix only implemented for nb_tx==2 and nb_rx==2, using identity\n");
      }
    }
    break;
  case ETU:
    chan_desc->nb_taps        = 9;
    chan_desc->Td             = 5.0;
    chan_desc->channel_length = (int) (2*chan_desc->BW*chan_desc->Td + 1 + 2/(M_PI*M_PI)*log(4*M_PI*chan_desc->BW*chan_desc->Td));
    sum_amps = 0;
    chan_desc->amps           = (double*) malloc(chan_desc->nb_taps*sizeof(double));
    for (i = 0; i<chan_desc->nb_taps; i++) {
      chan_desc->amps[i]      = pow(10,.1*etu_amps_dB[i]); 
      sum_amps += chan_desc->amps[i];
    }
    for (i = 0; i<chan_desc->nb_taps; i++)
      chan_desc->amps[i] /= sum_amps;
    chan_desc->delays         = etu_delays;
    chan_desc->ricean_factor  = 1;
    chan_desc->aoa            = 0;
    chan_desc->random_aoa     = 0;
    chan_desc->ch             = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->chF            = (double complex**) malloc(nb_tx*nb_rx*sizeof(double complex*));
    chan_desc->a              = (double complex**) malloc(chan_desc->nb_taps*sizeof(double complex*));
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->ch[i] = (double complex*) malloc(chan_desc->channel_length * sizeof(double complex)); 
    for (i = 0; i<nb_tx*nb_rx; i++) 
      chan_desc->chF[i] = (double complex*) malloc(1200 * sizeof(double complex)); 
    for (i = 0; i<chan_desc->nb_taps; i++) 
      chan_desc->a[i]         = (double complex*) malloc(nb_tx*nb_rx * sizeof(double complex));
    if (nb_tx==2 && nb_rx==2) {
      chan_desc->R_sqrt  = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) 
	chan_desc->R_sqrt[i] = (double complex*) &R22_sqrt[i][0];
    }
    else {
      chan_desc->R_sqrt         = (double complex**) malloc(6*sizeof(double complex**));
      for (i = 0; i<6; i++) {
	chan_desc->R_sqrt[i]    = (double complex*) malloc(nb_tx*nb_rx*nb_tx*nb_rx * sizeof(double complex));
	for (j = 0; j<nb_tx*nb_rx*nb_tx*nb_rx; j+=(nb_tx*nb_rx+1)) {
	  chan_desc->R_sqrt[i][j] = 1.0;
	}
	msg("correlation matrix only implemented for nb_tx==2 and nb_rx==2, using identity\n");
      }
    }
    break;

  case Rayleigh8:
      nb_taps = 8;
      Td = 0.8;
      channel_length = (int)11+2*BW*Td;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 0;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amps_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rice8:
      nb_taps = 8;
      Td = 0.8;
      channel_length = (int)11+2*BW*Td;
      ricean_factor = 0.1;
      aoa = .03;
      maxDoppler = 0;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amps_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   1);
      break;

  case Rayleigh1:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 100;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rayleigh1_50:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 50;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rayleigh1_100:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 100;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rayleigh1_200:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 200;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rayleigh1_corr:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 0;

      if ((nb_tx==2) && (nb_rx==1)) {
	R_sqrt_ptr2 = R_sqrt_21_corr;
      }
      else if ((nb_tx==2) && (nb_rx==2)) {
	R_sqrt_ptr2 = R_sqrt_22_corr;
      }
      else
	R_sqrt_ptr2 = NULL;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   R_sqrt_ptr2,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rayleigh1_anticorr:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 1;
      aoa = .03;
      maxDoppler = 0;

      if ((nb_tx==2) && (nb_rx==1)) {
	R_sqrt_ptr2 = R_sqrt_21_anticorr;
      }
      else if ((nb_tx==2) && (nb_rx==2)) {
	R_sqrt_ptr2 = R_sqrt_22_anticorr;
      }
      else 
	R_sqrt_ptr2 = NULL;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   R_sqrt_ptr2,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rice1:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 0.1;
      aoa = .03;
      maxDoppler = 100;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case AWGN:
      nb_taps = 1;
      Td = 0;
      channel_length = 1;
      ricean_factor = 0.0;
      aoa = 0.0;
      maxDoppler = 0;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   NULL,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   0);
      break;

  case Rice1_corr:
      nb_taps = 1;
      Td = 0;
      channel_length = 10;
      ricean_factor = 0.1;
      aoa = .03;
      maxDoppler = 0;

      if ((nb_tx==2) && (nb_rx==1)) {
	R_sqrt_ptr2 = R_sqrt_21_corr;
      }
      else if ((nb_tx==2) && (nb_rx==2)) {
	R_sqrt_ptr2 = R_sqrt_22_corr;
      }
      else
	R_sqrt_ptr2 = NULL;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   R_sqrt_ptr2,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   1);
      break;

  case Rice1_anticorr:
      nb_taps = 1;
      Td = 0;
      channel_length = 10;
      ricean_factor = 0.1;
      aoa = .03;
      maxDoppler = 0;

      if ((nb_tx==2) && (nb_rx==1)) {
	R_sqrt_ptr2 = R_sqrt_21_anticorr;
      }
      else if ((nb_tx==2) && (nb_rx==2)) {
	R_sqrt_ptr2 = R_sqrt_22_anticorr;
      }
      else 
	R_sqrt_ptr2 = NULL;

      chan_desc = new_channel_desc(nb_tx,
				   nb_rx,
				   nb_taps,
				   channel_length,
				   default_amp_lin,
				   NULL,
				   R_sqrt_ptr2,
				   Td,
				   BW,
				   ricean_factor,
				   aoa,
				   forgetting_factor,
				   maxDoppler,
				   channel_offset, 
				   path_loss_dB,
				   1);
      break;

  default:
    msg("channel model not yet supported\n");
    free(chan_desc);
    return(NULL);
  }
  printf("[CHANNEL] RF %f\n",chan_desc->ricean_factor);
  for (i=0;i<chan_desc->nb_taps;i++)
    printf("[CHANNEL] tap %d: amp %f, delay %f\n",i,chan_desc->amps[i],chan_desc->delays[i]);

  chan_desc->nb_paths=10;

  return(chan_desc);
}


int random_channel(channel_desc_t *desc) {
		    
  double s;
  int i,k,l,aarx,aatx;
  double complex anew[NB_ANTENNAS_TX*NB_ANTENNAS_RX],acorr[NB_ANTENNAS_TX*NB_ANTENNAS_RX];
  double complex phase, alpha, beta;
  double tmp1,tmp2;
  
  if ((desc->nb_tx>NB_ANTENNAS_TX) || (desc->nb_rx > NB_ANTENNAS_RX)) {
    msg("random_channel.c: Error: temporary buffer for channel not big enough\n");
    return(-1);
  }


  for (i=0;i<(int)desc->nb_taps;i++) {
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {

	
	tmp1 = (sqrt(desc->ricean_factor*desc->amps[i]/2)*gaussdouble(0.0,1.0));
	tmp2 = (sqrt(desc->ricean_factor*desc->amps[i]/2)*gaussdouble(0.0,1.0));
	anew[aarx+(aatx*desc->nb_rx)] = tmp1+I*tmp2;

	if ((i==0) && (desc->ricean_factor != 1.0)) {
	  if (desc->random_aoa==1) {
	    desc->aoa = uniformrandom()*2*M_PI;
	  }

	  // this assumes that both RX and TX have linear antenna arrays with lambda/2 antenna spacing. 
	  // Furhter it is assumed that the arrays are parallel to each other and that they are far enough apart so 
	  // that we can safely assume plane wave propagation.
	  phase = cexp(I*M_PI*((aarx-aatx)*sin(desc->aoa)));
	  /*phase.x = cos(M_PI*((aarx-aatx)*sin(desc->aoa)));
	  phase.y = sin(M_PI*((aarx-aatx)*sin(desc->aoa)));*/
	  
 	  anew[aarx+(aatx*desc->nb_rx)] = anew[aarx+(aatx*desc->nb_rx)] + sqrt(1.0-desc->ricean_factor)*phase;
	  /*anew[aarx+(aatx*desc->nb_rx)].x += phase.x * sqrt(1-desc->ricean_factor);
	  anew[aarx+(aatx*desc->nb_rx)].y += phase.y * sqrt(1-desc->ricean_factor);*/
	}
#ifdef DEBUG_CH
	printf("(%d,%d,%d) %f->(%f,%f) (%f,%f) phase (%f,%f)\n",aarx,aatx,i,desc->amps[i],creal(anew[aarx+(aatx*desc->nb_rx)]),cimag(anew[aarx+(aatx*desc->nb_rx)]),desc->aoa,desc->ricean_factor,creal(phase),cimag(phase));
#endif	
      } //aatx
    } //aarx

    /*
    // for debugging set a=anew;
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
	desc->a[i][aarx+(aatx*desc->nb_rx)].x = anew[aarx+(aatx*desc->nb_rx)].x;
	desc->a[i][aarx+(aatx*desc->nb_rx)].y = anew[aarx+(aatx*desc->nb_rx)].y;
 	printf("anew(%d,%d) = %f+1j*%f\n",aatx,aarx,anew[aarx+(aatx*desc->nb_rx)].x, anew[aarx+(aatx*desc->nb_rx)].y);
     }
    }
    */
    //apply correlation matrix
    //compute acorr = R_sqrt[i] * anew
    alpha = 1.0;
    beta = 0.0;
    cblas_zgemv(CblasRowMajor, CblasNoTrans, desc->nb_tx*desc->nb_rx, desc->nb_tx*desc->nb_rx, 
		(void*) &alpha, (void*) desc->R_sqrt[i/3], desc->nb_rx*desc->nb_tx,
		(void*) anew, 1, (void*) &beta, (void*) acorr, 1);

    /*
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
	desc->a[i][aarx+(aatx*desc->nb_rx)].x = acorr[aarx+(aatx*desc->nb_rx)].x;
	desc->a[i][aarx+(aatx*desc->nb_rx)].y = acorr[aarx+(aatx*desc->nb_rx)].y;
	printf("tap %d, acorr1(%d,%d) = %f+1j*%f\n",i,aatx,aarx,acorr[aarx+(aatx*desc->nb_rx)].x, acorr[aarx+(aatx*desc->nb_rx)].y);
      }
    }
    */

    if (desc->first_run==1){
      cblas_zcopy(desc->nb_tx*desc->nb_rx, (void*) acorr, 1, (void*) desc->a[i], 1);
    }
    else {
      alpha = sqrt(1-desc->forgetting_factor);
     
      beta = sqrt(desc->forgetting_factor);
     
      cblas_zscal(desc->nb_tx*desc->nb_rx, (void*) &beta, (void*) desc->a[i], 1);
      cblas_zaxpy(desc->nb_tx*desc->nb_rx, (void*) &alpha, (void*) acorr, 1, (void*) desc->a[i], 1);

      //  desc->a[i][aarx+(aatx*desc->nb_rx)].x = (sqrt(desc->forgetting_factor)*desc->a[i][aarx+(aatx*desc->nb_rx)].x) + sqrt(1-desc->forgetting_factor)*anew.x;
      //  desc->a[i][aarx+(aatx*desc->nb_rx)].y = (sqrt(desc->forgetting_factor)*desc->a[i][aarx+(aatx*desc->nb_rx)].y) + sqrt(1-desc->forgetting_factor)*anew.y;
    }

    /*
    for (aarx=0;aarx<desc->nb_rx;aarx++) {
      for (aatx=0;aatx<desc->nb_tx;aatx++) {
 	//desc->a[i][aarx+(aatx*desc->nb_rx)].x = acorr[aarx+(aatx*desc->nb_rx)].x;
	//desc->a[i][aarx+(aatx*desc->nb_rx)].y = acorr[aarx+(aatx*desc->nb_rx)].y;
	printf("tap %d, a(%d,%d) = %f+1j*%f\n",i,aatx,aarx,desc->a[i][aarx+(aatx*desc->nb_rx)].x, desc->a[i][aarx+(aatx*desc->nb_rx)].y);
      }
    }
    */

  } //nb_taps      

  //memset((void *)desc->ch[aarx+(aatx*desc->nb_rx)],0,(int)(desc->channel_length)*sizeof(struct complex));
  
  for (aarx=0;aarx<desc->nb_rx;aarx++) {
    for (aatx=0;aatx<desc->nb_tx;aatx++) {
      if (desc->channel_length == 1) {
	desc->ch[aarx+(aatx*desc->nb_rx)][0] = desc->a[0][aarx+(aatx*desc->nb_rx)];
      }
      else {
	for (k=0;k<(int)desc->channel_length;k++) {
	  desc->ch[aarx+(aatx*desc->nb_rx)][k] = 0.0 + I*0.0;
	  /*desc->ch[aarx+(aatx*desc->nb_rx)][k].x = 0.0;
	    desc->ch[aarx+(aatx*desc->nb_rx)][k].y = 0.0;*/
	
	  for (l=0;l<desc->nb_taps;l++) {
	    if ((k - (desc->delays[l]*desc->BW) - NB_SAMPLES_CHANNEL_OFFSET) == 0)
	      s = 1.0;
	    else
	      s = sin(M_PI*(k - (desc->delays[l]*desc->BW) - NB_SAMPLES_CHANNEL_OFFSET))/
		(M_PI*(k - (desc->delays[l]*desc->BW) - NB_SAMPLES_CHANNEL_OFFSET));
	    desc->ch[aarx+(aatx*desc->nb_rx)][k] += s*desc->a[l][aarx+(aatx*desc->nb_rx)];
	    /*desc->ch[aarx+(aatx*desc->nb_rx)][k].x += s*desc->a[l][aarx+(aatx*desc->nb_rx)].x;
	      desc->cimag(ch[aarx+(aatx*desc->nb_rx)][k]) += s*cimag(desc->a[l][aarx+(aatx*desc->nb_rx)]);*/
	    //	  printf("l %d : desc->ch.x %f\n",l,desc->a[l][aarx+(aatx*desc->nb_rx)].x);
	  } //nb_taps
#ifdef DEBUG_CH
	printf("(%d,%d,%d)->(%f,%f)\n",k,aarx,aatx,creal(desc->ch[aarx+(aatx*desc->nb_rx)][k]),cimag(desc->ch[aarx+(aatx*desc->nb_rx)][k]));
#endif
	} //channel_length
      } //else (channel_length==0)
    } //aatx
  } //aarx

  if (desc->first_run==1)
    desc->first_run = 0;

  return (0);
}



#ifdef RANDOM_CHANNEL_MAIN
#define BW 5.0
#define Td 2.0
main(int argc,char **argv) {

  double amps[8] = {.8,.2,.1,.04,.02,.01,.005};
  double complex ch[(int)(1+2*BW*Td)],phase;
  int i;
  
  randominit();
  creal(phase) = 1.0;
  cimag(phase) = 0;
  random_channel(amps,Td, 8,BW,ch,(double)1.0,&phase);
  /*
  for (i=0;i<(11+2*BW*Td);i++){
    printf("%f + sqrt(-1)*%f\n",ch[i].x,ch[i].y);
  }
  */
}

#endif