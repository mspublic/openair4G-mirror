close all
clear all

addpath('../IMPORT_FILTER')

%%
pathname = 'G:\EMOS\data\20100420 interference measurement\';
%post_processing_eNb_quick
plot_results_eNb_quick
%%
pathname = 'G:\EMOS\data\20100421 interference eNb + DL test\';
%post_processing_eNb_quick
plot_results_eNb_quick
%%
pathname = 'G:\EMOS\data\20100421 interference eNb + DL test\';
%post_processing_UE_quick
plot_results_UE_quick
%%
pathname = 'G:\EMOS\data\20100422 interference drive test\';
%post_processing_eNb_quick
plot_results_eNb_quick

