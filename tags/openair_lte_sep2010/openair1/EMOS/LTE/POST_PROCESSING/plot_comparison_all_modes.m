% file: plot_comparison_all_modes

close all
clear all
h_fig = 0;

% pathname = '/media/disk/PENNE/';
% mm = 'penne';
% pathname = '/emos/AMBIALET/';
% mm = 'ambialet';
pathname = '/emos/EMOS/';
mm = 'cordes';

mode1 = load(fullfile(pathname,'Mode1/results/results_UE.mat'));
mode1_ul = load(fullfile(pathname,'Mode1/results/results_eNB.mat'));
mode2 = load(fullfile(pathname,'Mode2/results/results_UE.mat'));
if strcmp(mm,'cordes')
    mode2_ul = load(fullfile(pathname,'Mode2_update/results/results_eNB.mat'));
else
    mode2_ul = load(fullfile(pathname,'Mode2/results/results_eNB.mat'));
end
mode6 = load(fullfile(pathname,'Mode6/results/results_UE.mat'));
mode6_ul = load(fullfile(pathname,'Mode6/results/results_eNB.mat'));

mode2_ideal = load(fullfile(pathname,'/Mode2/results/results_cat_UE.mat'));

%%
mode1.UE_synched = (mode1.UE_mode_cat>0);
mode1.UE_connected = (mode1.UE_mode_cat==3);
mode1.throughput = double(100./(100+mode1.dlsch_fer_cat).*mode1.tbs_cat.*6.*100);
mode1.good = (mode1.dlsch_fer_cat<=100 & mode1.dlsch_fer_cat>=0).';

mode2.UE_synched = (mode2.UE_mode_cat>0);
mode2.UE_connected = (mode2.UE_mode_cat==3);
mode2.throughput = double(100./(100+mode2.dlsch_fer_cat).*mode2.tbs_cat.*6.*100);
mode2.good = (mode2.dlsch_fer_cat<=100 & mode2.dlsch_fer_cat>=0).';

mode6.UE_synched = (mode6.UE_mode_cat>0);
mode6.UE_connected = (mode6.UE_mode_cat==3);
mode6.throughput = double(100./(100+mode6.dlsch_fer_cat).*mode6.tbs_cat.*6.*100);
mode6.good = (mode6.dlsch_fer_cat<=100 & mode6.dlsch_fer_cat>=0).';

%%
mode1_ul.ulsch_fer_cat = [100 diff(mode1_ul.ulsch_errors_cat)];
mode1_ul.ulsch_throughput = double(mode1_ul.tbs_cat) .* double(100 - mode1_ul.ulsch_fer_cat) .* 3;
mode1_ul.eNB_connected = ([mode1_ul.eNb_UE_stats_cat(:).UE_mode]==3);
mode1_ul.ulsch_throughput(1,~mode1_ul.eNB_connected) = 0;
mode1_ul.ulsch_throughput_ideal_1Rx = scale_ideal_tp(mode1_ul.Rate_64Qam_1RX_cat*100);
mode1_ul.ulsch_throughput_ideal_2Rx = scale_ideal_tp(mode1_ul.Rate_64Qam_2RX_cat*100);
mode1_ul.good = ~isnan(mode1_ul.ulsch_throughput_ideal_1Rx);
mode1_ul.ulsch_throughput_ideal_1Rx(~mode1_ul.eNB_connected,1) = 0;
mode1_ul.ulsch_throughput_ideal_2Rx(~mode1_ul.eNB_connected,1) = 0;

mode2_ul.ulsch_fer_cat = [100 diff(mode2_ul.ulsch_errors_cat)];
mode2_ul.ulsch_throughput = double(mode2_ul.tbs_cat) .* double(100 - mode2_ul.ulsch_fer_cat) .* 3;
mode2_ul.eNB_connected = ([mode2_ul.eNb_UE_stats_cat(:).UE_mode]==3);
mode2_ul.ulsch_throughput(1,~mode2_ul.eNB_connected) = 0;
mode2_ul.ulsch_throughput_ideal_1Rx = scale_ideal_tp(mode2_ul.Rate_64Qam_1RX_cat*100);
mode2_ul.ulsch_throughput_ideal_2Rx = scale_ideal_tp(mode2_ul.Rate_64Qam_2RX_cat*100);
mode2_ul.good = ~isnan(mode2_ul.ulsch_throughput_ideal_1Rx);
mode2_ul.ulsch_throughput_ideal_1Rx(~mode2_ul.eNB_connected,1) = 0;
mode2_ul.ulsch_throughput_ideal_2Rx(~mode2_ul.eNB_connected,1) = 0;

mode6_ul.ulsch_fer_cat = [100 diff(mode6_ul.ulsch_errors_cat)];
mode6_ul.ulsch_throughput = double(mode6_ul.tbs_cat) .* double(100 - mode6_ul.ulsch_fer_cat) .* 3;
mode6_ul.eNB_connected = ([mode6_ul.eNb_UE_stats_cat(:).UE_mode]==3);
mode6_ul.ulsch_throughput(1,~mode6_ul.eNB_connected) = 0;
mode6_ul.ulsch_throughput_ideal_1Rx = scale_ideal_tp(mode6_ul.Rate_64Qam_1RX_cat*100);
mode6_ul.ulsch_throughput_ideal_2Rx = scale_ideal_tp(mode6_ul.Rate_64Qam_2RX_cat*100);
mode6_ul.good = ~isnan(mode6_ul.ulsch_throughput_ideal_1Rx);
mode6_ul.ulsch_throughput_ideal_1Rx(~mode6_ul.eNB_connected,1) = 0;
mode6_ul.ulsch_throughput_ideal_2Rx(~mode6_ul.eNB_connected,1) = 0;


%% calc distance
[mode1.dist, mode1.dist_travelled] = calc_dist(mode1.gps_lat_cat,mode1.gps_lon_cat,mm);
[mode2.dist, mode2.dist_travelled] = calc_dist(mode2.gps_lat_cat,mode2.gps_lon_cat,mm);
[mode6.dist, mode6.dist_travelled] = calc_dist(mode6.gps_lat_cat,mode6.gps_lon_cat,mm);

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
saveas(h_fig,fullfile(pathname,'results','throughput_cdf_comparison.eps'),'epsc2')

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
saveas(h_fig,fullfile(pathname,'results','throughput_cdf_comparison_fdd.eps'),'epsc2')


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
saveas(h_fig,fullfile(pathname,'results','DLSCH_uncoded_throughput_cdf_comparison.eps'),'epsc2')

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
saveas(h_fig,fullfile(pathname,'results','throughput_connected_cdf_comparison.eps'),'epsc2')

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
saveas(h_fig,fullfile(pathname,'results','DLSCH_uncoded_throughput_connected_cdf_comparison.eps'),'epsc2')

%% UL Coded throughput CDF comparison
h_fig = h_fig+1;
figure(h_fig);
hold off
[f,x] = ecdf([mode1_ul.ulsch_throughput mode2_ul.ulsch_throughput mode6_ul.ulsch_throughput]);
plot(x,f,'b','Linewidth',2)
hold on
[f,x] = ecdf([mode1_ul.ulsch_throughput_ideal_1Rx; mode2_ul.ulsch_throughput_ideal_1Rx; mode6_ul.ulsch_throughput_ideal_1Rx]);
plot(x,f,'g','Linewidth',2)
[f,x] = ecdf([mode1_ul.ulsch_throughput_ideal_2Rx; mode2_ul.ulsch_throughput_ideal_2Rx; mode6_ul.ulsch_throughput_ideal_2Rx]);
plot(x,f,'r','Linewidth',2)
xlim([0 4.86e6])
legend('modem','ideal 1 rx antenna','ideal 2 rx antennas','Location','SouthEast');
title('UL Throughput CDF')
xlabel('UL Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'results','UL_throughput_cdf_comparison.eps'),'epsc2')

%% UL FDD
h_fig = h_fig+1;
figure(h_fig);
hold off
[f,x] = ecdf([mode1_ul.ulsch_throughput mode2_ul.ulsch_throughput mode6_ul.ulsch_throughput]*10/3);
plot(x,f,'b','Linewidth',2)
hold on
[f,x] = ecdf([mode1_ul.ulsch_throughput_ideal_1Rx; mode2_ul.ulsch_throughput_ideal_1Rx; mode6_ul.ulsch_throughput_ideal_1Rx]*10/3);
plot(x,f,'g','Linewidth',2)
[f,x] = ecdf([mode1_ul.ulsch_throughput_ideal_2Rx; mode2_ul.ulsch_throughput_ideal_2Rx; mode6_ul.ulsch_throughput_ideal_2Rx]*10/3);
plot(x,f,'r','Linewidth',2)
legend('modem','ideal 1 rx antenna','ideal 2 rx antennas','Location','SouthEast');
title('UL Throughput CDF for FDD')
xlabel('UL Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'results','UL_throughput_cdf_comparison_fdd.eps'),'epsc2')

%% UL Unoded throughput CDF comparison
h_fig = h_fig+1;
figure(h_fig);
hold off
[f,x] = ecdf(coded2uncoded([mode1_ul.ulsch_throughput mode2_ul.ulsch_throughput mode6_ul.ulsch_throughput],'UL'));
plot(x,f,'b','Linewidth',2)
hold on
[f,x] = ecdf(coded2uncoded([mode1_ul.ulsch_throughput_ideal_1Rx; mode2_ul.ulsch_throughput_ideal_1Rx; mode6_ul.ulsch_throughput_ideal_1Rx],'UL'));
plot(x,f,'g','Linewidth',2)
[f,x] = ecdf(coded2uncoded([mode1_ul.ulsch_throughput_ideal_2Rx; mode2_ul.ulsch_throughput_ideal_2Rx; mode6_ul.ulsch_throughput_ideal_2Rx],'UL'));
plot(x,f,'r','Linewidth',2)
xlim([0 4.86e6])
legend('modem','ideal 1 rx antenna','ideal 2 rx antennas','Location','SouthEast');
title('UL Uncoded throughput CDF')
xlabel('UL Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'results','UL_uncoded_throughput_cdf_comparison.eps'),'epsc2')

%% fit path loss model for all measurements
dist = [mode1.dist mode2.dist mode6.dist];
dist_ok = ((dist>0.5) & (dist<100)).';
rx_rssi_dBm_cat = [mode1.rx_rssi_dBm_cat; mode2.rx_rssi_dBm_cat; mode6.rx_rssi_dBm_cat];
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
saveas(h_fig, fullfile(pathname,'results','RX_RSSI_dBm_dist_with_PL.eps'),'epsc2');



%%
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_in_bins(dist(good(:,1)),double(rx_rssi_dBm_cat(good(:,1),1)),0:max_dist);
xlabel('Distance [km]')
ylabel('RX RSSI [dBm]')
ylim([-110 -30]);
saveas(h_fig,fullfile(pathname,'results','RX_RSSI_dBm_dist_bars.eps'),'epsc2');


%% distance travelled comparison
% to select regions use the gps_link_brush.m tool
close all

switch mm
    case 'cordes'
        plot_distance_travelled_cordes
    case 'penne'
        load penne_zoom1.mat
        file_id = 1;
        plot_distance_travelled

        load penne_zoom2.mat
        file_id = 2;
        plot_distance_travelled

        load penne_zoom3.mat
        file_id = 3;
        plot_distance_travelled
    case 'ambialet'
        load ambialet_zoom1.mat
        file_id = 1;
        plot_distance_travelled

        load ambialet_zoom2.mat
        file_id = 2;
        plot_distance_travelled

        load ambialet_zoom3.mat
        file_id = 3;
        plot_distance_travelled
        
end
