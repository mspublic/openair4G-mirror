clc;
pathname = '/media/Expansion_Drive/New Folder/';
dd = dir(pathname);

for dir_idx=1:1:length(dd)
    if dd(dir_idx).isdir == 1
        
        if length(dd(dir_idx).name) > 10
            
            
            fpath = [pathname dd(dir_idx).name '/'];
 %%           
            %fpath = '/extras/kaltenbe/CNES/emos_raw_data/20100512_mode2_parcours2_part2/';
            d = dir([fpath 'data_term3*.EMOS']);
            filenames = {d.name};
          
            decimation = 1;
            NFrames_max = 100*60*10;
            
            for file_idx = 1:length(filenames)
                
                disp(filenames{file_idx});
                decimation
                [path,file,ext,ver] = fileparts(filenames{file_idx});
                
                if file(10)=='1'
                    is_eNb=1;
                else
                    is_eNb=0;
                end
                
                %[H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = load_estimates_lte_1(filename,NFrames_max,decimation,is_eNb);
                
                [H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = load_estimates_lte_1(fullfile(fpath,filenames{file_idx}),NFrames_max,decimation,is_eNb);             
                save(regexprep((fullfile(fpath,filenames{file_idx},'results_UE.mat')), '.EMOS/results_UE.mat', '_results_UE.mat'), 'gps_data', 'NFrames', 'SNR', 'throughput', 'minestimates', 'file_idx');
                
            end
        end
        
    end
end
