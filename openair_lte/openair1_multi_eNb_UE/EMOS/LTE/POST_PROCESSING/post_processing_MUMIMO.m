%clear all
close all

pathname     = '/extras/kaltenbe/CNES/ressults_MUMIMO/';
UE1_pathname = '/extras/kaltenbe/CNES/MUMIMO_data/UE1/';
UE2_pathname = '/extras/kaltenbe/CNES/MUMIMO_data/UE2/';

load select_mu-mimo_ambialet.mat

decimation = 1;
NFrames_max = 100*60*10;

for file_idx = 1:3; %min(length(UE1_files),length(UE2_files))

    disp(UE1_files{file_idx});
    disp(UE2_files{file_idx});

    is_eNb=0;
    
    [H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = ...
        load_estimates_lte_2({fullfile(UE1_pathname,UE1_files{file_idx}), fullfile(UE2_pathname,UE2_files{file_idx})}, ...
        NFrames_max,decimation,is_eNb);

       save(fullfile(pathname,results,sprintf('results_MU-MIMO_idx%d.mat',file_idx)),...
        'gps_data', 'NFrames', 'SNR', 'throughput', 'minestimates', 'file_idx');

end
    

