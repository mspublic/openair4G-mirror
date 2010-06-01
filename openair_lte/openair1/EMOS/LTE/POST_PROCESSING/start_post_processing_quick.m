close all
clear all

addpath('../IMPORT_FILTER')
decimation = 100;
struct_template;

root_path = '/media/Iomega HDD/EMOS/data';

d = dir(fullfile(root_path, '*mode1_parcours1*'));
dir_names = {d.name};

%% post processing for nomadic points
for i=1:length(dir_names)
    pathname = fullfile(root_path,dir_names{i},'nomadic');
    if exist(pathname,'dir')
        post_processing_UE_nomadic_quick
    end
end

eval_nomadic_points


%% post processing for vehicular measurements
for i=1:length(dir_names)
    pathname = fullfile(root_path,dir_names{i});
    post_processing_UE_quick
    post_processing_eNb_quick
end

for i=1:length(dir_names)
    pathname = fullfile(root_path,dir_names{i});
    plot_results_UE_quick
    plot_results_eNb_quick
end
