%plot_results_cat

%load '/homes/latif/test_devel/openair1/EMOS/LTE/POST_PROCESSING/CapCom/Parcour2_Part2_Results/results_UE.mat';

%%
mm=imread('cordes.png');
in = 0;

plotwrtdistanceCalc;


%% plot K-factor
in=in+1;
h_fig = figure(in);
plot_gps_coordinates(mm, [gps_data_cat.longitude], [gps_data_cat.latitude], K_fac_cat);
title('K_factor')
saveas(h_fig,fullfile(pathname,'K_factor_gps.jpg'),'jpg');

%% set throughput to 0 when UE was not connected
UE_connected = all(reshape(UE_mode_cat,100,[])==3,1);
nn = fieldnames(throughput);
for n = 1:length(nn)
    eval([nn{n} '_cat(~UE_connected) = 0;']);
end

% %% get real throughput from modem
% dlsch_error = double([1 diff([minestimates_cat.dlsch_errors])]);
% rateps_modem = (1-dlsch_error) .* double([minestimates_cat.tbs]);
% rateps_modem([minestimates.UE_mode]~=3) = 0;
% rateps_modem = sum(reshape(rateps_modem,100,[]),1)*6;
% 
% dlsch_mcs = [minestimates_cat.mcs];
% mod_order = zeros(size(dlsch_mcs));
% mod_order(dlsch_mcs<10) = 2;
% mod_order(dlsch_mcs>=10 & dlsch_mcs<17) = 4;
% mod_order(dlsch_mcs>=17) = 6;
% rateps_uncoded_modem = 2400*mod_order;
% rateps_uncoded_modem([minestimates.UE_mode]~=3) = 0;
% rateps_uncoded_modem = sum(reshape(rateps_uncoded_modem,100,[]),1)*6;


%% plot coded throughput as CDFs
in = in+1;    
h_fig = figure(in);
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = 1:length(nn)
    hold on
    si = strfind(nn{n},'64Qam');
    if si
        eval(['[f,x] = ecdf(' nn{n} '_cat);']);
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
% [f,x] = ecdf(rateps_modem);
% plot(x,f,colors{ni},'Linewidth',2);
% legend_str{ni} = 'rateps_modem';
% ni=ni+1;

legend(legend_str,'Interpreter','none','Location','SouthOutside');
title('DL Throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'coded_throughput_cdf_comparison.eps'),'epsc2');


%% plot uncoded throughput as CDFs
in = in+1;    
h_fig = figure(in);
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
legend_str = {};
ni=1;
for n = 1:length(nn)
    hold on
    si = strfind(nn{n},'64Qam');
    if si
        eval(['[f,x] = ecdf(coded2uncoded(' nn{n} '_cat,''DL''));']);
        plot(x,f,colors{ni},'Linewidth',2);
        legend_tmp = nn{n};
        legend_tmp(si:si+10) = [];
        legend_str{ni} = legend_tmp;
        ni=ni+1;
    end
end
% [f,x] = ecdf(rateps_uncoded_modem);
% plot(x,f,colors{ni},'Linewidth',2);
% legend_str{ni} = 'rateps_modem';
% ni=ni+1;


legend(legend_str,'Interpreter','none','Location','SouthOutside');
xlabel('Uncoded throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'uncoded_throughput_cdf_comparison.eps'),'epsc2');


%% plot as a function of the speed of the UE (histogram)

% in = in+1;    
% h_fig = figure(in);
% title('Effective Throughputs for 1stRX');
% xlabel('Speed[Meters/Second]');
% ylabel('Throughput[Bits/sec]');
% hold on;
% plot([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_1stRx_cat,  'bx');
% plot([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_1stRx_cat,'go');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_speed_1stRx.eps'),'epsc2');
%
%
% in = in+1;    
% h_fig = figure(in);
% title('Effective Throughputs for 2ndRX');
% xlabel('Speed[Meters/Second]');
% ylabel('Throughput[Bits/sec]');
% hold on;
% plot([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_2ndRx_cat,  'bx');
% plot([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_2ndRx_cat,'go');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_speed_2ndRx.eps'),'epsc2');
% 
% 
% in = in+1;    
% h_fig = figure(in);
% title('Effective Throughputs for 2RX');
% xlabel('Speed[Meters/Second]');
% ylabel('Throughput[Bits/sec]');
% hold on;
% plot([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_2Rx_cat,  'bx');
% plot([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_2Rx_cat,'go');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,'rd');
% plot([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_speed_2Rx.eps'),'epsc2');

% 1st Rx antenna
in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_1stRx_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode1, 1stRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode1_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_1stRx_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode2, 1stRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode2_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode6, ideal feedback, 1stRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode6_maxq_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode6, real feedback, 1stRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode6_feedbackq_1stRx.eps'),'epsc2');

% 2nd Rx antenna
in = in+1;    
h_fig = figure(in);
title('Ideal Throughput vs Speed for Mode1, 2ndRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
plot_in_bins([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_2ndRx_cat,  0:5:40);
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode1_2ndRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_2ndRx_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode2, 2ndRX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode2_2ndRx.eps'),'epsc2');

% both Rx antennas
in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_SISO_64Qam_eNB1_2Rx_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode1, 2RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode1_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_alamouti_64Qam_eNB1_2Rx_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode2, 2RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode2_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode6, ideal feedback, 2RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode6_maxq_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins([gps_data_cat.speed], rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,  0:5:40);
title('Ideal Throughput vs Speed for Mode6, real feedback, 2RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_speed_mode6_feedbackq_2Rx.eps'),'epsc2');



%% plot on map

in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, [gps_data_cat.longitude], [gps_data_cat.latitude], rateps_SISO_64Qam_eNB1_2Rx_cat);
title('Ideal Throughput (TX mode 1, 2 Rx)');
saveas(h_fig,fullfile(pathname,'coded_throughput_SISO_gps_2Rx.jpg'),'jpg');
in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, [gps_data_cat.longitude], [gps_data_cat.latitude], rateps_alamouti_64Qam_eNB1_2Rx_cat);
title('Ideal Througput (TX mode 2, 2 Rx)');
saveas(h_fig,fullfile(pathname,'coded_throughput_Alamouti_gps_2Rx.jpg'),'jpg');
in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, [gps_data_cat.longitude], [gps_data_cat.latitude], rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat);
title('Ideal Throughput (TX mode 6, optimal feedback, 2 Rx)');
saveas(h_fig,fullfile(pathname,'coded_throughput_optBeamforming_gps_2Rx.jpg'),'jpg');
in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, [gps_data_cat.longitude], [gps_data_cat.latitude], rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat);
title('Ideal Throughput (TX mode 6, real feedback, 2Rx)');
saveas(h_fig,fullfile(pathname,'coded_throughput_feedbackBeamforming_gps_2Rx.jpg'),'jpg');



%% plot as a function of time
in = in+1;    
h_fig = figure(in);
title('Ideal Throughputs for 1stRX');
xlabel('Time[Seconds]');
ylabel('Throughput[Bits/sec]');
hold on;
plot(rateps_SISO_64Qam_eNB1_1stRx_cat,  'bx');
plot(rateps_alamouti_64Qam_eNB1_1stRx_cat,'go');
plot(rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
plot(rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
legend('TX mode 1','TX mode 2','TX mode 6, optimal feedback', 'TX mode 6, real feedback')
saveas(h_fig,fullfile(pathname,'coded_throughput_time_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
title('Ideal Throughputs for 2ndRX');
xlabel('Time[Seconds]');
ylabel('Throughput[Bits/sec]');
hold on;
plot(rateps_SISO_64Qam_eNB1_2ndRx_cat,  'bx');
plot(rateps_alamouti_64Qam_eNB1_2ndRx_cat,'go');
plot(rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
plot(rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
legend('TX mode 1','TX mode 2','TX mode 6, optimal feedback', 'TX mode 6, real feedback')
saveas(h_fig,fullfile(pathname,'coded_throughput_time_2ndRx.eps'),'epsc2');


in = in+1;    
h_fig = figure(in);
title('Ideal Throughputs for 2RX');
xlabel('Time[Seconds]');
ylabel('Throughput[Bits/sec]');
hold on;
plot(rateps_SISO_64Qam_eNB1_2Rx_cat,  'bx');
plot(rateps_alamouti_64Qam_eNB1_2Rx_cat,'go');
plot(rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,'rd');
plot(rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,'c*');
legend('TX mode 1','TX mode 2','TX mode 6, optimal feedback', 'TX mode 6, real feedback')
saveas(h_fig,fullfile(pathname,'coded_throughput_time_2Rx.eps'),'epsc2');


%% comparison coded and uncoded
in = in+1;    
h_fig = figure(in);
title('Uncoded Ideal Throughputs for 2RX');
xlabel('Time[Seconds]');
ylabel('Throughput[Bits/sec]');
hold on;
%plot(rateps_SISO_64Qam_eNB1_2Rx_cat,  'bx');
%plot(rateps_alamouti_64Qam_eNB1_2Rx_cat,'go');
%plot(rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,'rd');
%plot(rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,'c*');
plot(coded2uncoded(rateps_SISO_64Qam_eNB1_2Rx_cat,'DL'),  'bx');
plot(coded2uncoded(rateps_alamouti_64Qam_eNB1_2Rx_cat,'DL'),'go');
plot(coded2uncoded(rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,'DL'),'rd');
plot(coded2uncoded(rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,'DL'),'c*');
legend('TX mode 1','TX mode 2','TX mode 6, optimal feedback', 'TX mode 6, real feedback')
saveas(h_fig,fullfile(pathname,'ideal_uncoded_throughput_time_2Rx.eps'),'epsc2');


%% plot as a function of distance from BS (histogram)

% in = in+1;
% h_fig = figure(in);
% title('Effective Throughputs for 1stRX');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput [Bits/sec]');
% hold on;
% plot(dist,rateps_SISO_64Qam_eNB1_1stRx_cat,  'bx');
% plot(dist,rateps_alamouti_64Qam_eNB1_1stRx_cat,'go');
% plot(dist,rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
% plot(dist,rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_distance_1stRx.eps'),'epsc2');
% 
% 
% in = in+1;    
% h_fig = figure(in);
% title('Effective Throughputs for 2ndRX');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput[Bits/sec]');
% hold on;
% plot(dist,rateps_SISO_64Qam_eNB1_2ndRx_cat,  'bx');
% plot(dist,rateps_alamouti_64Qam_eNB1_2ndRx_cat,'go');
% plot(dist,rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,'rd');
% plot(dist,rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_distance_2ndRx.eps'),'epsc2');
% 
% 
% in = in+1;    
% h_fig = figure(in);
% title('Effective Throughputs for 2RX');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput[Bits/sec]');
% hold on;
% plot(dist,rateps_SISO_64Qam_eNB1_2Rx_cat,  'bx');
% plot(dist,rateps_alamouti_64Qam_eNB1_2Rx_cat,'go');
% plot(dist,rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,'rd');
% plot(dist,rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,'c*');
% legend('SISO','Alamouti','Optimal Beamforming', 'Beamforming with feedback')
% saveas(h_fig,fullfile(pathname,'coded_throughput_distance_2Rx.eps'),'epsc2');
%  

% 1st Rx antenna
in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_SISO_64Qam_eNB1_1stRx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode1, 1stRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode1_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_alamouti_64Qam_eNB1_1stRx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode2, 1stRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode2_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_beamforming_64Qam_eNB1_1Rx_maxq_cat,  0:15);
title('Ideal Throughput vs Dist for Mode6, ideal feedback, 1stRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode6_maxq_1stRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_beamforming_64Qam_eNB1_1Rx_feedbackq_cat,  0:15);
title('Ideal Throughput vs Dist for Mode6, real feedback, 1stRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode6_feedbackq_1stRx.eps'),'epsc2');

% 2nd Rx antenna
in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_SISO_64Qam_eNB1_2ndRx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode1, 2ndRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode1_2ndRx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_alamouti_64Qam_eNB1_2ndRx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode2, 2ndRX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode2_2ndRx.eps'),'epsc2');

% both Rx antennas
in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_SISO_64Qam_eNB1_2Rx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode1, 2RX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode1_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_alamouti_64Qam_eNB1_2Rx_cat,  0:15);
title('Ideal Throughput vs Dist for Mode2, 2RX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode2_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_beamforming_64Qam_eNB1_2Rx_maxq_cat,  0:15);
title('Ideal Throughput vs Dist for Mode6, ideal feedback, 2RX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode6_maxq_2Rx.eps'),'epsc2');

in = in+1;    
h_fig = figure(in);
plot_in_bins(dist, rateps_beamforming_64Qam_eNB1_2Rx_feedbackq_cat,  0:15);
title('Ideal Throughput vs Dist for Mode6, real feedback, 2RX');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'ideal_throughput_dist_mode6_feedbackq_2Rx.eps'),'epsc2');



%%
% in = in+1;
% h_fig = figure(in);
% title('Throughput SISO _ distance Travelled ')
% xlabel('Distance travelled [Km]')
% ylabel('Throughput [Bits/sec]')
% plot(dist_travelled, rps_SISO_4Qam_eNB1_cat,'bx');
% hold on
% plot(dist_travelled, rps_SISO_16Qam_eNB1_cat,'go' );
% plot(dist_travelled, rps_SISO_64Qam_eNB1_cat,'rd');
% 
% legend('QPSK','16QAM','64QAM')

%%
% in = in+1;    
% h_fig = figure(in);
% title('Alamouti');
% xlabel('Time[Seconds]');
% ylabel('Throughput');
% hold on;
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_SISO_16Qam_eNB1_cat,'bo');
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Alamouti_16Qam_eNB1_cat,'rd');
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_16Qam_eNB1_maxq_cat,'go');
% hold off;
% legend('SISO','ALAM','BMFR')


%%
% in = in+1;    
% h_fig = figure(in);
% title(['Alamouti']);
% xlabel('Time[Seconds]');
% ylabel('Throughput');
% hold on;
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_SISO_4Qam_eNB1_cat,'bx');
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_SISO_16Qam_eNB1_cat,'go');
% plot(round(double(timestamp_cat(1:100:end))/1e9),rps_SISO_64Qam_eNB1_cat,'rd');
% hold off;
% legend('QPSK','16QAM','64QAM')
% 
% in = in+1;
% h_fig = figure(in);
% title('Throughput SISO _ distance from BS ');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput [Bits/sec]');
% %dist(1:end);
% plot(dist, rps_SISO_4Qam_eNB1_cat,'bx');
% hold on;
% plot(dist, rps_SISO_16Qam_eNB1_cat,'go');
% plot(dist, rps_SISO_64Qam_eNB1_cat,'rd');
% hold off;
% legend('QPSK','16QAM','64QAM')
%  
% in = in+1;
% h_fig = figure(in);
% plot(dist_travelled, rps_SISO_4Qam_eNB1_cat,'bx');
% hold on
% plot(dist_travelled, rps_SISO_16Qam_eNB1_cat,'go' );
% plot(dist_travelled, rps_SISO_64Qam_eNB1_cat,'rd');
% title('Throughput SISO _ distance Travelled ')
% xlabel('Distance travelled [Km]')
% ylabel('Throughput [Bits/sec]')
% legend('QPSK','16QAM','64QAM')
