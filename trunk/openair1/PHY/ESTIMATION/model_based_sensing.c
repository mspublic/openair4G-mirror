#include "rtai_math.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"

#define PI 3.14
#define FFT_SIZE 32768

static short local_rev[FFT_SIZE]  __attribute__ ((aligned(16)));
static short y[2*FFT_SIZE] __attribute__ ((aligned(16)));
static short Y[4*FFT_SIZE] __attribute__ ((aligned(16))); 
static short data[2*FFT_SIZE]  __attribute__ ((aligned(16)));

int model_based_detection(void)
{
        int N =FFT_SIZE/2;
	short *samples; 
	int i,j,tmp1,tmp2,s1,s2;
	float s3, AIC[2], L,nu,sigma_carre,delta_AIC;
	short mean_r,mean_i, detect;
	static int counter = 0;

	// Begin Sensing Algorithm "Model selection based detection"
	samples = (s16 *)PHY_vars->rx_vars[0].RX_DMA_BUFFER;

	detect = 0;
	// change the operating channel
 	
	//filter(samples,temp0,FRAME_LENGTH_SAMPLES/2); 
	// ############ Model Selection based detector
	tmp1 = 0;
	tmp2 = 0;
	
	for (i=0;i<FRAME_LENGTH_SAMPLES/2;i++)
	{
		tmp1 += samples[2*i]; 
		tmp2 += samples[2*i+1];
	}
	
	mean_r = (2*tmp1/FRAME_LENGTH_SAMPLES);
	mean_i = (2*tmp2/FRAME_LENGTH_SAMPLES);	

	
	Zero_Buffer(y,2*FFT_SIZE*sizeof(short));
	// substruct the mean from the received signal
	for (i=0;i<FRAME_LENGTH_SAMPLES/2;i++)
	{
		y[2*i] = samples[2*i] - mean_r;
		y[2*i+1] = samples[2*i+1] - mean_i;
	}
		

	init_fft(FFT_SIZE,15,local_rev);
	fft(y, Y,twiddle_fft32768, local_rev, 15, 6, 0);   /// 0 means 64-bit complex notation else packed complex notation
	
	s1 = 0;
	s2 = 0;
	s3 = 0;

	for (i=0;i<4*N;i+=4)
	{	//data[i/2] = Y[i];
		//data[1 + i/2] = Y[i+1];
		tmp1 = (Y[i]*Y[i] + Y[i+1]*Y[i+1]);
		s2 += tmp1;
		s1 = s1 + sqrt(tmp1);
		if (tmp1 !=0)
		  s3 += 0.5*log(tmp1);
	}	
	
	
	sigma_carre = s2/(2*N);
	//msg("sigma_carre = %f\n",sigma_carre);
	L = s3 - N*log(sigma_carre) - N;
	//msg("L = %f\n",L);  
	AIC[0] = -2*L + 2;
	//msg("\AIC_0 = %f\n",AIC[0]);

	// Rice distribution
	if (((16*s1*s1) - (12*N*s2))<=0)
		nu = 4*s1/(6*N);
	else
		nu = (4*s1 + sqrt((16*s1*s1) - (12*N*s2)))/(6*N);
		
	//msg("nu = %f\n",nu);
	sigma_carre = (N*nu*nu + s2 - 2*nu*s1)/N;
	L = (s3 - N*(log(2*PI*sigma_carre*nu) +1))/2;
	AIC[1] = -2*L + 4;
	//msg("AIC_1 = %f\n",AIC[1]);
	
	delta_AIC = AIC[0] - AIC[1];
	
	if (delta_AIC>0)
		detect = 1;
	//msg("[OPENAIR][SENSING] count %d, detect = %d\n",counter++,detect);
	return detect;

}
