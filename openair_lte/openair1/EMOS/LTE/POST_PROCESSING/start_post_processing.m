clear all
close all

pathname = '/extras/kaltenbe/EMOS/data/20100317_FIRST_LTE_TEST/';
filename_UE = 'data_term3_idx0_2010324_12628.EMOS';
filename_eNB = 'data_term1_idx0_2010324_12630.EMOS';

decimation = 1;
NFrames_max = 100*60*10;

[~, ~, estimates_UE, gps_data_UE, NFrames_UE] = load_estimates_lte(fullfile(pathname, filename_UE),NFrames_max,decimation,0);
[~, ~, estimates_eNB, gps_data_eNB, NFrames_eNB] = load_estimates_lte(fullfile(pathname, filename_eNB),NFrames_max,decimation,1);

load 'SISO.mat';

% make sure you only store the necessary things in those scripts!
capacity_siso
%capacity_Alamouti
%capacity_Mode6_beamforming

