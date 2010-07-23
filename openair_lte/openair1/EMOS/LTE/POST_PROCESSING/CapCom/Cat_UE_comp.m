clear all;
close all;
clc

 UE_mode_cat_comp = [];
 K_fac_cat_comp = [];
 gps_data_cat_comp = [];
 pmi1_cat_comp = [];
 pmi2_cat_comp = [];
 rank_ind_cat_comp = [];
 UE_sync_cat_comp = [];
 start_idx = 1;
 
root_path = '/extras/kaltenbe/CNES/emos_postprocessed_data/Mode2/';

d = dir(fullfile(root_path, '*mode*'));
dir_names = {d.name};

for d_idx=1:length(dir_names)
 
pathname = fullfile(root_path,dir_names{d_idx})
        try
load(fullfile(pathname, 'results_cat_UE.mat'))
        catch exception
            disp(exception.getReport)
            disp(sprintf('Detected error in folder %s, skipping it',pathname));
            continue
        end
   vars = whos('*_cat');
   nn = {vars.name};
    
    if (start_idx==1)
        for n = 1:length(nn)
            eval([nn{n} '_comp = [];']);
        end
    end
    for n = 1:length(nn)
        eval([nn{n} '_comp = [' nn{n} '_comp ' nn{n} '];']);
    end
       
    save(fullfile(root_path,'results_comp_UE.mat'), '*_comp');
    start_idx = start_idx + 1;

end
    
    