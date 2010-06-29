% file: test_figures
% this file reads the data from exaclty one measurement and procudes all
% the figures requested by Mr. Scot. 
clear all
close all

decimation = 1;
NFrames_max = 100*60*10;
pathname = '/emos/EMOS/Mode2/20100520_mode2_parcours1_part4_part5/';

%%
Concatenate_results_UE_quick

plot_results_cat

%%
Concatenate_results_eNB_quick

plot_results_eNb_cat