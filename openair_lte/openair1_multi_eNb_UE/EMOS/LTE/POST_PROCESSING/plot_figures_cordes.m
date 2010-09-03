% file: plot_figures_top.m
% this file reads the data post processed measurement data and procudes all
% the figures requested by Mr. Scot. The comparison between different modes
% is done with the script plot_comparison_all_modes.m and
% distance_travelled_comparison.m
clear all
close all

%decimation = 1;
%NFrames_max = 100*60*10;
mm='cordes';

%% results from raw channel estimates (ideal throughput)
pathname = '/emos/EMOS/Mode2/results/';
%pathname = '/extras/kaltenbe/CNES/emos_postprocessed_data/Mode2/results';
%Concatenate_results_UE_quick
%plot_results_cat

%% results from quick post processing (modem throughput)
pathname = '/emos/EMOS/Mode1/results';
plot_results_UE_quick
plot_results_eNb_quick

%%
pathname = '/emos/EMOS/Mode2/results_with_coverage';
plot_results_UE_quick
%plot_results_eNb_quick

%%
pathname = '/emos/EMOS/Mode2_update/results';
plot_results_UE_quick
plot_results_eNb_quick

%%
pathname = '/emos/EMOS/Mode6/results';
plot_results_UE_quick
plot_results_eNb_quick


