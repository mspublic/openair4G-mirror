close all
clear all

addpath('../IMPORT_FILTER')
addpath('maps')
struct_template
decimation = 100;

%% post processing
pathname = 'G:\EMOS\data\20100520_mode2_parcours1_part4_part5\';
post_processing_UE_quick
post_processing_eNb_quick
pathname = 'G:\EMOS\data\20100520_mode2_parcours1_part4_part5\nomadic\';
post_processing_UE_quick
post_processing_eNb_quick

%% plot results
pathname = 'G:\EMOS\data\20100520_mode2_parcours1_part4_part5\';
plot_results_UE_quick
plot_results_eNb_quick

