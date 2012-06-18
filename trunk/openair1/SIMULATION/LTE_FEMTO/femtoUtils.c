#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "femtoUtils.h"

#ifndef _FEMTO_UTILS

#include "PHY/types.h"
#include "SIMULATION/TOOLS/defs.h"

#endif


void _parseOptions(options_t *opts, int argc, char ** argv) {
    char c;
    char aux[100];

    while ((c = getopt (argc, argv, "hs:S:T:n:xdt:y:z:I:j:N:o:g:f:ab:w:c:e")) != -1)
    {
		
        switch (c)
        {
        case 'a':
            opts->awgn_flag=1;
               opts->channel_model=AWGN;
            sprintf(opts->parameters,"%s -a",opts->parameters);
            break;
         case 'e':
            opts->dci_flag=1;               
            sprintf(opts->parameters,"%s -d",opts->parameters);
            break;
        case 's':
            opts->snr_init=atof(optarg);
            sprintf(opts->parameters,"%s  -s%f",opts->parameters,opts->snr_init);
            break;
        case 'S':
            opts->snr_max=atof(optarg);
            sprintf(opts->parameters,"%s  -S%f",opts->parameters,opts->snr_max);
            break;
        case 'T':
            opts->snr_step=atof(optarg);
            sprintf(opts->parameters,"%s  -T%f",opts->parameters,opts->snr_step);
            break;
        case 'n':
            opts->nframes=atoi(optarg);
            sprintf(opts->parameters,"%s  -n%d",opts->parameters,opts->nframes);
            break;
        case 'x':
            opts->extended_prefix_flag=1;
            sprintf(opts->parameters,"%s  -x",opts->parameters);
            if (opts->extended_prefix_flag == 0)
            {
                opts->nsymb = 14 ;
                opts->pilot1 = 4;
                opts->pilot2 = 7;
                opts->pilot3 = 11;
            } else
            {
                opts->nsymb = 12;
                opts->pilot1 = 3;
                opts->pilot2 = 6;
                opts->pilot3 = 9;
            }
            break;
        case 'd':
            opts->frame_type= 1;
            sprintf(opts->parameters,"%s  -d",opts->parameters);
            break;
        case 't':
            opts->transmission_mode=atoi(optarg);
            sprintf(opts->parameters,"%s  -t%d",opts->parameters,opts->transmission_mode);
            if ((opts->transmission_mode!=1) &&  (opts->transmission_mode!=2) && (opts->transmission_mode!=6))
            {
                printf("Unsupported transmission mode %d\n",opts->transmission_mode);
                exit(-1);
            }
            break;
        case 'y':
            opts->n_tx=atoi(optarg);
            sprintf(opts->parameters,"%s  -y%d",opts->parameters,opts->n_tx);
            break;
        case 'z':
            opts->n_rx=atoi(optarg);
            sprintf(opts->parameters,"%s  -z%d",opts->parameters, opts->n_rx);
            break;
        case 'I':
            opts->nInterf=atoi(optarg);
            sprintf(opts->parameters,"%s  -I%d",opts->parameters, opts->nInterf);
            break;
        case 'w':
			//TODO : fix this to set de interference level differents             
            sprintf(aux,"%d",atoi(optarg));
            strcpy(opts->interfLevels,aux);
            sprintf(opts->parameters,"%s  -w%s", opts->parameters,opts->interfLevels);
            break;
        case 'N':
            opts->Nid_cell = atoi(optarg);
            sprintf(opts->parameters,"%s  -N%d",opts->parameters, opts->Nid_cell);
            break;
          case 'c':
            opts->interCellId = atoi(optarg);
            sprintf(opts->parameters,"%s  -c%d",opts->parameters, opts->interCellId);
            break;
        case 'o':
            opts->oversampling=atoi(optarg);
            sprintf(opts->parameters,"%s  -o%d",opts->parameters, opts->oversampling);
            break;
        case 'b':
            opts->testNumber=atoi(optarg);
            sprintf(opts->parameters,"%s  -b%d",opts->parameters, opts->testNumber);
            break;
        case 'g':
            sprintf(opts->parameters,"%s  -g%s",opts->parameters, optarg);
            switch ((char)*optarg) {
            case 'A':
                opts->channel_model=SCM_A;
                break;
            case 'B':
                opts->channel_model=SCM_B;
                break;
            case 'C':
                opts->channel_model=SCM_C;
                break;
            case 'D':
                opts->channel_model=SCM_D;
                break;
            case 'E':
                opts->channel_model=EPA;
                break;
            case 'F':
                opts->channel_model=EVA;
                break;
            case 'G':
                opts->channel_model=ETU;
                break;
            default:
                msg("Unsupported channel model!\n");                
                exit(-1);
            }
            break;
        default:
        case 'h':
            printf("-h    This message\n");
            printf("-s    Starting SNR default value is %f\n",opts->snr_init);
            printf("-S    Ending SNR default value is %f\n",opts->snr_max);
            printf("-T    Step size of SNR, default value is %f\n",opts->snr_step);
            printf("-n    Number of frames, default value is %d\n",opts->nframes);
            printf("-x    Use extended prefix mode  flag, default value is Normal\n");
            printf("-d    Use TDD flag\n");
            printf("-t    Transmission mode (1,2,6 for the moment),default value is %d\n",opts->transmission_mode);
            printf("-y    Number of TX antennas used in eNB, default value is %d\n",opts->n_tx);
            printf("-z    Number of RX antennas used in UE, default value is %d\n",opts->n_rx);
            printf("-I    Number of interference to apply, default value is %d \n",opts->nInterf);
            printf("-w    Relative strength of  inteference list (in dB) \n");
            printf("-N    Nid_cell, default value is %d \n",opts->Nid_cell);
            printf("-o    Oversampling factor (1,2,4,8,16), default value is %d \n",opts->oversampling);
            printf("-g    [A,B,C,D,E,F,G] Use 3GPP SCM (A,B,C,D) or 36-101 (E-EPA,F-EVA,G-ETU) models (ignores delay spread and Ricean factor), default value is AWGN\n");
            printf("-f    Output filename (.txt format) for Pe/SNR results\n");
            printf("-a    Use AWGN channel and not multipath\n");
            printf("-b    Test Number\n");
            exit (-1);
            break;


        }
    }

    sprintf(opts->folderName,"%d_resp",opts->testNumber);
    if (opts->nInterf>0)
        _parseInterferenceLevels(opts,opts->interfLevels,opts->nInterf);
}

void _printOptions(options_t *opts)
{
    int i;
    printf("\n----------Options----------");
    printf("\nsnr_init:\t\t%f",opts->snr_init);
    printf("\nsnr_max:\t\t%f",opts->snr_max);
    printf("\nsnr_step:\t\t%f",opts->snr_step);
    printf("\nnframes:\t\t%d",opts->nframes);
    printf("\nextended_prefix_flag:\t%d",opts->extended_prefix_flag);
    printf("\nframe_type:\t\t%d",opts->frame_type);
    printf("\ntransmission_mode:\t%d",opts->transmission_mode);
    printf("\nn_tx:\t\t\t%d",opts->n_tx);
    printf("\nn_rx:\t\t\t%d",opts->n_rx);
    printf("\nNid_cell:\t\t%d",opts->Nid_cell);
    printf("\noversampling:\t\t%d",opts->oversampling);
    printf("\nchannel_model:\t\t%d",opts->channel_model);
    printf("\nawgn_flag:\t\t%d",opts->awgn_flag);
    printf("\nnInterf:\t\t%d",opts->nInterf);
    printf("\nxx:%p",(void *)opts->outputFile);

    for (i=0; i<opts->nInterf; i++)
    {
        printf("\n\tInterference n%d:%d",i+1,(int)opts->dbInterf[i]);
    }




    printf("\n");


}

void _parseInterferenceLevels(options_t *opts, char *interfLevels,int nInterf)
{
    int i;
    char * pch;
    opts->dbInterf=malloc(sizeof(s8)*nInterf);
    for (i=0; i<nInterf; i++)
    {        
        opts->dbInterf[i]=atoi(interfLevels);
        printf("%d\n",atoi(interfLevels));
        printf("%d\n",opts->dbInterf[i]);
	}
/*
    pch = strtok (interfLevels,",");
    i=0;
    while (pch != NULL)
    {

        (*dbInterf)[i]=atoi(pch);
        i++;
        pch = strtok (NULL,",");
    }
*/
}

void _allocData(options_t opts, data_t *data ,u8 n_tx,u8 n_rx, int Frame_length_complex_samples)
{
    int i,j;
    data->s_re = (double**)malloc(n_tx*sizeof(double*));
    data->s_im = (double**)malloc(n_tx*sizeof(double*));
    data->r_re = (double**)malloc(n_rx*sizeof(double*));
    data->r_im = (double**)malloc(n_rx*sizeof(double*));
    
    if(opts.nInterf>0)
    {
		data->is_re=(double***)malloc(opts.nInterf*sizeof(double**));
		data->is_im=(double***)malloc(opts.nInterf*sizeof(double**));
		data->ir_re=(double***)malloc(opts.nInterf*sizeof(double**));
		data->ir_im=(double***)malloc(opts.nInterf*sizeof(double**));
		for(i=0;i<opts.nInterf;i++)
		{
			data->is_re[i]=(double**)malloc(n_tx*sizeof(double*));
			data->is_im[i]=(double**)malloc(n_tx*sizeof(double*));
			data->ir_re[i]=(double**)malloc(n_tx*sizeof(double*));
			data->ir_im[i]=(double**)malloc(n_tx*sizeof(double*));
		}
	}

    for (i=0; i<n_tx; i++)
    {

        data->s_re[i] =(double*)malloc(Frame_length_complex_samples*sizeof(double));
        data->s_im[i] = (double*)malloc(Frame_length_complex_samples*sizeof(double));
        data->r_re[i] =(double*)malloc(Frame_length_complex_samples*sizeof(double));
        data->r_im[i] = (double*)malloc(Frame_length_complex_samples*sizeof(double));
        
        bzero(data->s_re[i],Frame_length_complex_samples*sizeof(double));        
        bzero(data->s_im[i],Frame_length_complex_samples*sizeof(double));
        bzero(data->r_re[i],Frame_length_complex_samples*sizeof(double));       
        bzero(data->r_im[i],Frame_length_complex_samples*sizeof(double));
        
        for(j=0;j<opts.nInterf;j++)
        {
			data->is_re[j][i] =(double*)malloc(Frame_length_complex_samples*sizeof(double));
			data->is_im[j][i] = (double*)malloc(Frame_length_complex_samples*sizeof(double));
			data->ir_re[j][i] =(double*)malloc(Frame_length_complex_samples*sizeof(double));
			data->ir_im[j][i] = (double*)malloc(Frame_length_complex_samples*sizeof(double));
			
			bzero(data->is_re[j][i],Frame_length_complex_samples*sizeof(double));        
			bzero(data->is_im[j][i],Frame_length_complex_samples*sizeof(double));
			bzero(data->ir_re[j][i],Frame_length_complex_samples*sizeof(double));       
			bzero(data->ir_im[j][i],Frame_length_complex_samples*sizeof(double));
		}
        
    }


}


void _makeOutputDir(options_t *opts)
{
    int status;
    char auxDir[100]; 
    char auxFile[100];   
    FILE *controlFile;

    status=mkdir ("testResults",S_IRWXU | S_IRWXG | S_IRWXO);
    status=chdir("testResults");
    sprintf(auxDir,"%s",opts->folderName);
    status=mkdir(auxDir,S_IRWXU | S_IRWXG | S_IRWXO);

    status=chdir(auxDir);
    
    sprintf(auxFile,"OutpuSimulation_%df_%dI_%sdB_%dch.m",opts->nframes,opts->nInterf,opts->interfLevels,opts->channel_model);

    opts->outputFile =fopen(auxFile,"w");
    opts->outputBler =fopen("OuputBler.svn","w");

    controlFile=fopen("ControlTest.txt","w");

    fprintf(controlFile,"Parameters\n");

    fprintf(controlFile,"./femtosim %s\n\n",opts->parameters);


    fprintf(controlFile,"testNumber:\t\t\n",opts->testNumber);

    fprintf(controlFile,"awgn_flag:\t\t%d\n",opts->awgn_flag);
    fprintf(controlFile,"snr_init:\t\t%d\n",opts->snr_init);
    fprintf(controlFile,"snr_max;\t\t%d\n",opts->snr_max);
    fprintf(controlFile,"snr_step:\t\t%d\n",opts->snr_step);
    fprintf(controlFile,"nframes:\t\t%d\n",opts->nframes);
    fprintf(controlFile,"extended_prefix_flag:\t\t%d\n",opts->extended_prefix_flag);
    fprintf(controlFile,"frame_type:\t\t%d\n",opts->frame_type);
    fprintf(controlFile,"transmission_mode:\t\t%d\n",opts->transmission_mode);
    fprintf(controlFile,"n_tx:\t\t%d\n",opts->n_tx);
    fprintf(controlFile,"n_rx:\t\t%d\n",opts->n_rx);
    fprintf(controlFile,"nInterf:\t\t%d\n",opts->nInterf);
    fprintf(controlFile,"interfLevels:\t\t%s\n",opts->interfLevels);
    fprintf(controlFile,"Nid_cell:\t\t%d\n",opts->Nid_cell);
    fprintf(controlFile,"oversampling:\t\t%d\n",opts->oversampling);

    fclose(controlFile);

}


