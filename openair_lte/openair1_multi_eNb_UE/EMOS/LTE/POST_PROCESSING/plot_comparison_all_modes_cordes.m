% file: plot_comparison_all_modes_cordes
% script not used anymore. use plot_comparison_all_modes instead

close all
%clear all
h_fig = 0;
%pathname = '/emos/EMOS/results';

%mode1 = load('/emos/EMOS/Mode1/results/results_UE.mat');
%mode2 = load('/emos/EMOS/Mode2/results/results_UE.mat');
%mode6 = load('/emos/EMOS/Mode6/results/results_UE.mat');
%ul    = load('/emos/EMOS/Mode6/results/results_eNB.mat');
%mode2_ideal = load('/emos/EMOS/Mode2/results/results_cat_UE.mat');

%%
%mode1.UE_synched = (mode1.UE_mode_cat>0);
%mode1.UE_connected = (mode1.UE_mode_cat==3);
%mode1.throughput = double(100./(100+mode1.dlsch_fer_cat).*mode1.tbs_cat.*6.*100);
%mode1.good = (mode1.dlsch_fer_cat<=100 & mode1.dlsch_fer_cat>=0).';

%mode2.UE_synched = (mode2.UE_mode_cat>0);
%mode2.UE_connected = (mode2.UE_mode_cat==3);
%mode2.throughput = double(100./(100+mode2.dlsch_fer_cat).*mode2.tbs_cat.*6.*100);
%mode2.good = (mode2.dlsch_fer_cat<=100 & mode2.dlsch_fer_cat>=0).';

%mode6.UE_synched = (mode6.UE_mode_cat>0);
%mode6.UE_connected = (mode6.UE_mode_cat==3);
%mode6.throughput = double(100./(100+mode6.dlsch_fer_cat).*mode6.tbs_cat.*6.*100);
%mode6.good = (mode6.dlsch_fer_cat<=100 & mode6.dlsch_fer_cat>=0).';

%% set throughput to 0 when UE was not connected
mode2_ideal.UE_connected = (mode2_ideal.UE_mode_cat(1:100:end)==3);
mode2_ideal.UE_synched = (mode2_ideal.UE_mode_cat(1:100:end)>0);

%% Coded throughput CDF comparison 
mode1.throughput(~mode1.UE_connected | ~mode1.good) = 0;
mode2.throughput(~mode2.UE_connected | ~mode2.good) = 0;
mode6.throughput(~mode6.UE_connected | ~mode6.good) = 0;
nn = fieldnames(mode2_ideal);
for n = strmatch('rateps',nn).'
    mode2_ideal.(nn{n})(~mode2_ideal.UE_synched) = 0;
end

%%
h_fig = h_fig+1;
figure(h_fig);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = strmatch('rateps',nn).'
    hold on
    si = strfind(nn{n},'supportedQam');
    if si
        [f,x] = ecdf(scale_ideal_tp(mode2_ideal.(nn{n})));
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
[f,x] = ecdf(mode1.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'TX mode 1';
        ni=ni+1;
[f,x] = ecdf(mode2.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 2';
        ni=ni+1;
[f,x] = ecdf(mode6.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 6';
        ni=ni+1;

title('DLSCH throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
legend(legend_str,'Interpreter','none','Location','SouthOutside');
grid on
saveas(h_fig,fullfile(pathname,'throughput_cdf_comparison.eps'),'epsc2')

%% FDD
h_fig = h_fig+1;
figure(h_fig);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = strmatch('rateps',nn).'
    hold on
    si = strfind(nn{n},'supportedQam');
    if si
        [f,x] = ecdf(scale_ideal_tp(mode2_ideal.(nn{n}))*10/6);
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
[f,x] = ecdf(mode1.throughput*10/6);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'TX mode 1';
        ni=ni+1;
[f,x] = ecdf(mode2.throughput*10/6);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 2';
        ni=ni+1;
[f,x] = ecdf(mode6.throughput*10/6);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 6';
        ni=ni+1;

title('DLSCH throughput CDF for FDD')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
legend(legend_str,'Interpreter','none','Location','SouthOutside');
grid on
saveas(h_fig,fullfile(pathname,'throughput_cdf_comparison_fdd.eps'),'epsc2')

%% Uncoded throughput CDF comparison 
h_fig = h_fig+1;
figure(h_fig);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = strmatch('rateps',nn).'
    hold on
    si = strfind(nn{n},'supportedQam');
    if si
        [f,x] = ecdf(coded2uncoded(scale_ideal_tp(mode2_ideal.(nn{n})),'DL'));
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
[f,x] = ecdf(coded2uncoded(mode1.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'TX mode 1';
        ni=ni+1;
[f,x] = ecdf(coded2uncoded(mode2.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 2';
        ni=ni+1;
[f,x] = ecdf(coded2uncoded(mode6.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 6';
        ni=ni+1;

title('DLSCH uncoded throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
legend(legend_str,'Interpreter','none','Location','SouthOutside');
grid on
saveas(h_fig,fullfile(pathname,'DLSCH_uncoded_throughput_cdf_comparison.eps'),'epsc2')

%% Coded throughput CDF comparison (when connected)
mode1.throughput(~mode1.UE_connected | ~mode1.good) = nan;
mode2.throughput(~mode2.UE_connected | ~mode2.good) = nan;
mode6.throughput(~mode6.UE_connected | ~mode6.good) = nan;
nn = fieldnames(mode2_ideal);
for n = strmatch('rateps',nn).'
    mode2_ideal.(nn{n})(~mode2_ideal.UE_connected) = nan;
end

%%
h_fig = h_fig+1;
figure(h_fig);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = strmatch('rateps',nn).'
    hold on
    si = strfind(nn{n},'supportedQam');
    if si
        [f,x] = ecdf(scale_ideal_tp(mode2_ideal.(nn{n})));
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
[f,x] = ecdf(mode1.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'TX mode 1';
        ni=ni+1;
[f,x] = ecdf(mode2.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 2';
        ni=ni+1;
[f,x] = ecdf(mode6.throughput);
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 6';
        ni=ni+1;

title('DLSCH throughput CDF (when connected)')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
legend(legend_str,'Interpreter','none','Location','SouthOutside');
grid on
saveas(h_fig,fullfile(pathname,'throughput_connected_cdf_comparison.eps'),'epsc2')

%% Uncoded throughput CDF comparison (when connected)
h_fig = h_fig+1;
figure(h_fig);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = strmatch('rateps',nn).'
    hold on
    si = strfind(nn{n},'supportedQam');
    if si
        [f,x] = ecdf(coded2uncoded(scale_ideal_tp(mode2_ideal.(nn{n})),'DL'));
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
[f,x] = ecdf(coded2uncoded(mode1.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'TX mode 1';
        ni=ni+1;
[f,x] = ecdf(coded2uncoded(mode2.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 2';
        ni=ni+1;
[f,x] = ecdf(coded2uncoded(mode6.throughput,'DL'));
plot(x,f,colors{ni},'Linewidth',2)
        legend_str{ni} = 'Tx mode 6';
        ni=ni+1;

title('DLSCH uncoded throughput CDF (when connected)')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
legend(legend_str,'Interpreter','none','Location','SouthOutside');
grid on
saveas(h_fig,fullfile(pathname,'DLSCH_uncoded_throughput_connected_cdf_comparison.eps'),'epsc2')


% %% Unoded throughput CDF comparison (when connected)
% h_fig = h_fig+1;
% figure(h_fig);
% [f,x] = ecdf(coded2uncoded(mode1.throughput,'DL'));
% plot(x,f,'b','Linewidth',2)
% hold on
% [f,x] = ecdf(coded2uncoded(mode2.throughput,'DL'));
% plot(x,f,'g','Linewidth',2)
% [f,x] = ecdf(coded2uncoded(mode6.throughput,'DL'));
% plot(x,f,'r','Linewidth',2)
% title('DLSCH Uncoded Throughput CDF')
% xlabel('Throughput [bps]')
% ylabel('P(x<abscissa)')
% grid on
% saveas(h_fig,fullfile(pathname,'DLSCH_uncoded_throughput_cdf_comparison.eps'),'epsc2')


% %% plot uncoded throughput as CDFs
% in = in+1;    
% h_fig = figure(in);
% colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
% legend_str = {};
% ni=1;
% for n = 1:length(nn)
%     hold on
%     si = strfind(nn{n},'supportedQam');
%     if si
%         eval(['[f,x] = ecdf(coded2uncoded(' nn{n} '_cat,''DL''));']);
%         plot(x,f,colors{ni},'Linewidth',2);
%         legend_tmp = nn{n};
%         legend_tmp(si:si+10) = [];
%         legend_str{ni} = legend_tmp;
%         ni=ni+1;
%     end
% end
% % [f,x] = ecdf(rateps_uncoded_modem);
% % plot(x,f,colors{ni},'Linewidth',2);
% % legend_str{ni} = 'rateps_modem';
% % ni=ni+1;
% 
% 
% legend(legend_str,'Interpreter','none','Location','SouthOutside');
% xlabel('Uncoded throughput [bps]')
% ylabel('P(x<abscissa)')
% grid on
% saveas(h_fig,fullfile(pathname,'uncoded_throughput_cdf_comparison.eps'),'epsc2');


%% fit path loss model for all measurements
%[mode1.dist, mode1.dist_travelled] = calc_dist(mode1.gps_lat_cat,mode1.gps_lon_cat);
%[mode2.dist, mode2.dist_travelled] = calc_dist(mode2.gps_lat_cat,mode2.gps_lon_cat);
%[mode6.dist, mode6.dist_travelled] = calc_dist(mode6.gps_lat_cat,mode6.gps_lon_cat);
dist = [coverage.dist mode1.dist mode2.dist mode2_update.dist mode6.dist];
dist_ok = (dist>0.5).';
rx_rssi_dBm_cat = [coverage.rx_rssi_dBm_cat; mode1.rx_rssi_dBm_cat; mode2.rx_rssi_dBm_cat; ...
    mode2_update.rx_rssi_dBm_cat; mode6.rx_rssi_dBm_cat;];
good = (rx_rssi_dBm_cat<40 & rx_rssi_dBm_cat>-120);

%%
h_fig = h_fig+1;
figure(h_fig);
hold off
semilogx(dist(dist_ok & good(:,1)), double(rx_rssi_dBm_cat(dist_ok & good(:,1),1)), 'rx')
hold on
max_dist = ceil(max(dist(dist_ok & good(:,1))));
PL = double(rx_rssi_dBm_cat(dist_ok & good(:,1),1)) - 43;
d = logspace(-1,log10(max_dist),100);
res = [ones(length(PL),1) log10(dist(dist_ok & good(:,1)).')]\PL;
semilogx(d,res(1)+res(2)*log10(d)+43,'b')
semilogx(d,43-cost231_hata(d),'k')
legend('measured',sprintf('fitted (PL_0=%4.2fdB, R=%4.2f)',res(1)+43,res(2)/10),'COST231-Hata','Location','Best');
title('RX RSSI distance from BS [dBm]')
xlabel('Distance from BS [Km]')
ylabel('RX RSSI [dBm]')
xlim([0.1 max_dist])
ylim([-110 -30]);
saveas(h_fig, fullfile(pathname,'RX_RSSI_dBm_dist_with_PL.eps'),'epsc2');



%%
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_in_bins(dist(good(:,1)),double(rx_rssi_dBm_cat(good(:,1),1)),0:max_dist);
xlabel('Distance [km]')
ylabel('RX RSSI [dBm]')
ylim([-110 -30]);
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_dist_bars.eps'),'epsc2');



