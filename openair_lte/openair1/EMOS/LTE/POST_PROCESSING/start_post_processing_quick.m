close all
clear all

addpath('../IMPORT_FILTER')
decimation = 100;
struct_template;

root_path = '/media/Iomega HDD/EMOS/data';

d = dir(fullfile(root_path, '*mode1_parcours1*'));
dir_names = {d.name};

%% post processing
for i=1:length(dir_names)
    pathname = fullfile(root_path,dir_names{i},'nomadic');
    if exist(pathname,'dir')
        post_processing_UE_quick_new
    end
end

eval_nomadic_points


