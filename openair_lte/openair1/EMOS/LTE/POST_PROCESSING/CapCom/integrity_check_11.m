close all
clear all
clc;
pathname = '/emos/EMOS/';
dd(1).name = '/Mode6/20100610_VTP_MODE6_ZONES_PUSCH_UPDATE.2/';
dd(1).isdir = 1;
% dd(1).name = 'Mode6/20100526_mode1_parcours1_part4_part5';
% dd(1).isdir = 1;
% dd(2).name = 'Mode2/20100520_mode2_parcours1_part4_part5';
% dd(2).isdir = 1;


for dir_idx=1:1:length(dd)
    if dd(dir_idx).isdir == 1
        
        if length(dd(dir_idx).name) > 10
            
            fpath = fullfile(pathname,dd(dir_idx).name);
            
            d = dir(fullfile(fpath, 'data_term3*.EMOS'));
            filenames = {d.name};
            
            decimation = 1;
            NFrames_max = 100*60*10;
            
            for file_idx = 1:length(filenames)
                
                disp(filenames{file_idx});
                [path,file,ext,ver] = fileparts(filenames{file_idx});
                
                time_idx = regexp(file,'\d{8}T\d{6}');
                start_time = datenum(file(time_idx:end),'yyyymmddTHHMMSS');
                
                if (start_time >= datenum('20100610T000000','yyyymmddTHHMMSS'));
                    version = 1;
                else
                    version = 0;
                end
                
                
                if file(10)=='1'
                    is_eNb=1;
                    mat_file = [file '_results_eNB.mat'];
                else
                    is_eNb=0;
                    mat_file = [file '_results_UE.mat'];
                end
                
                if (exist(fullfile(fpath,mat_file),'file'))
                    continue
                end
                
                %[H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = load_estimates_lte_1(filename,NFrames_max,decimation,is_eNb);
                
                % use try-catch here to skip the file if the load_estimates
                % function returns an error
                try
                    [H, H_fq, gps_data, NFrames, minestimates, throughput, SNR, K_fac] = load_estimates_lte_1(fullfile(fpath,filenames{file_idx}),NFrames_max,decimation,is_eNb,version);
                catch exception
                    disp(exception.getReport)
                    disp(sprintf('Detected error in file %s, skipping it',fullfile(fpath,filenames{file_idx})));
                    continue
                end
                
                save(fullfile(fpath,mat_file), 'gps_data', 'NFrames', 'SNR', 'throughput', 'minestimates', 'file_idx', 'K_fac');
                
                % We need to call the concatenate script here
                
            end
        end
        
    end
end
%then at the end we need to call the plot concatenate results

