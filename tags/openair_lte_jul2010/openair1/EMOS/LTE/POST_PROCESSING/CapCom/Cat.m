close all
clear all

root_path = '/media/Expansion_Drive/Mode2/';

d = dir(fullfile(root_path, '*mode2*'));
dir_names = {d.name};

for d_idx=1:length(dir_names)
    Cat_UE
    Cat_eNb
    
end


        