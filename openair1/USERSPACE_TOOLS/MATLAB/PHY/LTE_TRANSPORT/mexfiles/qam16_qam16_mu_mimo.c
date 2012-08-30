#include "defs.h"

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]
                )
{
    
    /* Declare */
    short *y1mf,*y2mf,*h1es,*h2es,*rhos,*llrs;
        
    /* Allocate input */
    y1mf = (short*) mxGetData(prhs[0]);
    y2mf = (short*) mxGetData(prhs[1]);
    h1es = (short*) mxGetData(prhs[2]);
    h2es = (short*) mxGetData(prhs[3]);
    rhos = (short*) mxGetData(prhs[4]);
    
    /* Allocate Output */
    /* 4 symbols = 16 LLRs*/
    plhs[0] = mxCreateNumericMatrix(1,16,mxINT16_CLASS,mxREAL);
    llrs = (short*) mxGetPr(plhs[0]);            
    
    /* Algo */
    /* function takes inputs of 8 shorts!! == 4 cmplx symbols */
    qam16_qam16(y1mf,y2mf,h1es,h2es,llrs,rhos,300);               
}

