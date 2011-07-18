#ifndef __MISC_PROTO_H__
#define __MISC_PROTO_H__
#include "complex.h"

char dB_fixed(unsigned int );
int write_output(const char *,const char *,void *,int,int,char);

unsigned int *generate_gauss_LUT(unsigned char,unsigned char);
int gauss(unsigned int *,unsigned char);

double gaussdouble(double,double);
void randominit(void);
 

void Zero_Buffer(void *,unsigned int);
void mmxcopy(void *,void *,int);
void QAM_input(struct complex *,short,int,int,char);
int signal_energy(int *,unsigned int);
int iSqrt(int value);
void set_taus_seed(void);
unsigned int taus(void);
void ofdm_channel(double *,double, int,double,int,struct complex *);

unsigned char log2_approx(unsigned int);
#endif
