clear all
close all

%pathname = '/extras/kaltenbe/EMOS/data/20100317_FIRST_LTE_TEST/';
%filename_UE = 'data_term3_idx0_2010324_12628.EMOS';
%filename_eNB = 'data_term1_idx0_2010324_12630.EMOS';

pathname = '/home/kaltenbe/EMOS/data/';
filename_UE = 'data_term3_idx9_2010416_145915.EMOS';

decimation = 1;
NFrames_max = 100*60*10;

[H, H_fq, estimates_UE, gps_data_UE, NFrames_UE] = load_estimates_lte(fullfile(pathname, filename_UE),NFrames_max,decimation,0);
%[H, H_fq, estimates_eNB, gps_data_eNB, NFrames_eNB] = load_estimates_lte(fullfile(pathname, filename_eNB),NFrames_max,decimation,1);

load 'SISO.mat';

capacity_siso
capacity_Alamouti
capacity_Mode6_beamforming

