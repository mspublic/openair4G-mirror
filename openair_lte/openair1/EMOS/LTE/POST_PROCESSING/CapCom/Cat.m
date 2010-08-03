close all
clear all

root_path = '/emos/EMOS/Coverage/';

d = dir(fullfile(root_path, '*coverage*'));
dir_names = {d.name};

for d_idx=1:length(dir_names)
    pathname = fullfile(root_path,dir_names{d_idx});
    disp(pathname)
    Concatenate_results_UE_quick
    %Concatenate_results_eNb_quick
    
end


        