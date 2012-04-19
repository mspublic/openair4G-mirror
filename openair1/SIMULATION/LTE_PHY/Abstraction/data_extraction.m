close all
clear all

global SINR_p BLER_meas snr bler SINR_awgn

set(0, 'DefaultLineMarkerSize', 10);
set(0, 'Defaultaxesfontsize', 10);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesFontName', 'Helvetica');
set(0, 'DefaultTextFontName', 'Helvetica');

% plot_style = {'b*';'r*';'g*';'y*';'k*';...
%     'bo';'ro';'go';'yo';'ko';...
%     'bs';'rs';'gs';'ys';'ks';...
%     'bd';'rd';'gd';'yd';'kd';...
%     'bx';'rx';'gx';'yx';'kx';...
%     'b+';'r+';'g+';'y+';'k+'};

echo off;

load '/extras/kaltenbe/L2S_map_results/Real_CE_EESM/matfiles/mcs_channel.mat';
pathname = '/extras/kaltenbe/L2S_map_results/Real_CE_EESM/results_scmc/';
root_path = '/homes/latif/OpenAirPlatform/test_devel/openair1_multi_eNb_UE/SIMULATION/LTE_PHY/BLER_SIMULATIONS/awgn_bler_new/';
mcs = [5:22];
s2=[];
s3=[];
for m=1:1:length(mcs)
    %% SINR_P Extraction per Sub carrier basis
    eval(['data_all = data_all' num2str(mcs(m)) ';']);
    data_all = data_all(data_all(:,end)<0.6,:);
    data_all = data_all(data_all(:,end)>0.01,:);
    
    SINR_p = data_all(:,2:end-1);
    %% EESM Mapping
    p=50;
    %data = dlmread(fullfile(root_path,sprintf('scmc_bler_%d.csv',mcs(m))),';',1,0);
    data = dlmread(fullfile(root_path,sprintf('bler_%d.csv',mcs(m))),';',1,0);
    snr = data(:,1);
    bler = data(:,5)./data(:,6); % round 1
    BLER_meas = data_all(:,end);
    
    %% SNR Method
    % SINR_awgn = interp1(bler, snr, BLER_meas,'cubic');
    % SINR_awgn = SINR_awgn';
    
    %% for coarse beta calculation:
    beta_vec = 0:1:30;
    delta = zeros(size(beta_vec));
    for beta=1:length(beta_vec)
        delta(beta)=delta_BLER(beta_vec(beta));
    end
    [val ind] = min(delta);
    opt_beta_new(mcs(m)) = beta_vec(ind);
    %% for optmimized beta calculation
    opt_beta(mcs(m)) = fminsearch(@delta_BLER,opt_beta_new(mcs(m)));
    
    %% BLER Plot w.r.t Effective SINR
    %opt_beta(mcs(m)) = 1;
    SINR_eff= 10*log10(-1*opt_beta(mcs(m))*log((1/p)*sum(exp((-10.^(SINR_p'./10))./opt_beta(mcs(m))))));
    h_fig = figure(m);
    hold off
    semilogy(SINR_eff,BLER_meas,'rd')
    hold on
    grid on
    semilogy(snr,bler,'y-*');
    xlim([-10 20])
    ylim([1e-3 1])
    s = strcat('L2S Map using EESM for LTE MCS ', num2str(mcs(m)), ' and beta= ',num2str(opt_beta(mcs(m))));
    title(s);
    ylabel 'BLER'
    xlabel 'SINR_{effective}'
    s2 = strcat('BLER_{meas} MCS ', num2str(mcs(m)));
    s3 = strcat('BLER_{AWGN} MCS ', num2str(mcs(m)));
    legend(s2,s3, 'Location',  'Best');
     saveas(h_fig,fullfile(pathname,strcat('mcs',num2str(mcs(m)) ,'.eps')),'epsc2');
     saveas(h_fig,fullfile(pathname,strcat('mcs',num2str(mcs(m)) ,'.fig')),'fig');
     saveas(h_fig,fullfile(pathname,strcat('mcs',num2str(mcs(m)) ,'.jpg')),'jpg');
    
end
%% Saving adjustment factor (beta)
save(fullfile(pathname, 'opt_beta.dat'),'opt_beta');
