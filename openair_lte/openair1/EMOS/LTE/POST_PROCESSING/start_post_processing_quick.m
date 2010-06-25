close all
clear all

addpath('../IMPORT_FILTER')
addpath('maps')
addpath('CapCom')
decimation = 100;
struct_template;

root_path = 'G:\';

d = dir(fullfile(root_path, '*'));
dir_names = {d.name};

%% post processing

% pathname = 'H:\EMOS\20100608_VTP_MODE2_PARCOURS1_S7_9\';
% post_processing_UE_quick
% post_processing_eNb_quick
% 
% pathname = 'H:\EMOS\20100608_VTP_MODE2_PARCOURS1_S8\';
% post_processing_UE_quick
% post_processing_eNb_quick

pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART1\';
post_processing_UE_quick
post_processing_eNb_quick

pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART2\';
post_processing_UE_quick
post_processing_eNb_quick

pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART3\';
post_processing_UE_quick
post_processing_eNb_quick

pathname = 'H:\EMOS\20100609_VTP_MODE6_ZONES_PUSCH_PART4\';
post_processing_UE_quick
post_processing_eNb_quick

% these measurements contain fixed UL storage
pathname = 'H:\EMOS\20100610_VTP_MODE6_ZONES_PUSCH_UPDATE.1\';
post_processing_UE_quick
post_processing_eNb_quick

pathname = 'H:\EMOS\20100610_VTP_MODE6_ZONES_PUSCH_UPDATE.2\';
post_processing_UE_quick
post_processing_eNb_quick

%% post processing for nomadic points
% 
% for i=1:length(dir_names)
%     pathname = fullfile(root_path,dir_names{i},'nomadic');
%     if exist(pathname,'dir')
%         post_processing_UE_nomadic_quick
%     end
% end
% 
% %eval_nomadic_points
% 

%% plot results

% pathname = 'H:\EMOS\20100608_VTP_MODE2_PARCOURS1_S7_9';
% plot_results_UE_quick
% plot_results_eNb_quick
% 
% pathname = 'H:\EMOS\20100608_VTP_MODE2_PARCOURS1_S8\';
% plot_results_UE_quick
% plot_results_eNb_quick

pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART1\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

%%
pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART2\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

%%
pathname = 'H:\EMOS\20100608_VTP_MODE6_ZONES_PUSCH_PART3\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

%%
pathname = 'H:\EMOS\20100609_VTP_MODE6_ZONES_PUSCH_PART4\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

%%
pathname = 'H:\EMOS\20100610_VTP_MODE6_ZONES_PUSCH_UPDATE.1\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

%%
pathname = 'H:\EMOS\20100610_VTP_MODE6_ZONES_PUSCH_UPDATE.2\';
%plot_results_UE_quick
plot_results_eNb_quick
plot_results_eNb_aligned

 
%% post processing for vehicular measurements
% for i=1:length(dir_names)
%     pathname = fullfile(root_path,dir_names{i});
%     post_processing_UE_quick
%     post_processing_eNb_quick
% end
% 
% for i=1:length(dir_names)
%     pathname = fullfile(root_path,dir_names{i});
%     plot_results_UE_quick
%     plot_results_eNb_quick
% end
