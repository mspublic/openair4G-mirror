%plot_results_cat

%clear all;
%close all;
%clc;

%%
%load '/homes/latif/test_devel/openair1/EMOS/LTE/POST_PROCESSING/CapCom/Parcour2_Part2_Results/results_UE.mat';
%mm=imread('cordes.png');
%plotwrtdistanceCalc;
in = 0;
thr_alamouti_1Rx = max(throughput.rateps_alamouti_64Qam_eNB1_1Rx, max(throughput.rateps_alamouti_16Qam_eNB1_1Rx,throughput.rateps_alamouti_4Qam_eNB1_1Rx));
thr_alamouti_2Rx = max(throughput.rateps_alamouti_64Qam_eNB1_2Rx, max(throughput.rateps_alamouti_16Qam_eNB1_2Rx,throughput.rateps_alamouti_4Qam_eNB1_2Rx));
thr_siso_1Rx = max(throughput.rateps_SISO_64Qam_eNB1_1Rx, max(throughput.rateps_SISO_16Qam_eNB1_1Rx,throughput.rateps_SISO_4Qam_eNB1_1Rx));
thr_siso_2Rx = max(throughput.rateps_SISO_64Qam_eNB1_2Rx, max(throughput.rateps_SISO_16Qam_eNB1_2Rx,throughput.rateps_SISO_4Qam_eNB1_2Rx));
thr_bmfr_maxq_1Rx = max(throughput.rateps_beamforming_64Qam_eNB1_1Rx_maxq, max(throughput.rateps_beamforming_16Qam_eNB1_1Rx_maxq,throughput.rateps_beamforming_4Qam_eNB1_1Rx_maxq));
thr_bmfr_maxq_2Rx = max(throughput.rateps_beamforming_64Qam_eNB1_2Rx_maxq, max(throughput.rateps_beamforming_16Qam_eNB1_2Rx_maxq,throughput.rateps_beamforming_4Qam_eNB1_2Rx_maxq));
thr_bmfr_feedbackq_2Rx = max(throughput.rateps_beamforming_64Qam_eNB1_2Rx_feedbackq, max(throughput.rateps_beamforming_16Qam_eNB1_2Rx_feedbackq,throughput.rateps_beamforming_4Qam_eNB1_2Rx_feedbackq));
thr_bmfr_feedbackq_1Rx = max(throughput.rateps_beamforming_64Qam_eNB1_1Rx_feedbackq, max(throughput.rateps_beamforming_16Qam_eNB1_1Rx_feedbackq,throughput.rateps_beamforming_4Qam_eNB1_1Rx_feedbackq));


UE_connected = all(reshape([minestimates.UE_mode]==3,100,[]),1);
thr_alamouti_1Rx(~UE_connected) = 0;
thr_alamouti_2Rx(~UE_connected) = 0;
thr_siso_1Rx(~UE_connected) = 0;
thr_siso_2Rx(~UE_connected) = 0;
thr_bmfr_maxq_1Rx(~UE_connected) = 0;
thr_bmfr_maxq_2Rx(~UE_connected) = 0;
thr_bmfr_feedbackq_1Rx(~UE_connected) = 0;
thr_bmfr_feedbackq_2Rx(~UE_connected) = 0;
in = in+1;    
h_fig = figure(in);
title('Effective Throughputs for 1RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
hold on;
plot(gps_data.speed, thr_siso_1Rx,  'bx');
plot(gps_data.speed,thr_alamouti_1Rx,'go');
plot(gps_data.speed,thr_bmfr_maxq_1Rx,'rd');
%plot(thr_bmfr_feedbackq_1Rx,'y*');
legend('SISO','Alamouti','Optimal Beamforming')

in = in+1;    
h_fig = figure(in);
title('Effective Throughputs for 2RX');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
hold on;
plot(gps_data.speed, thr_siso_2Rx,  'bx');
plot(gps_data.speed,thr_alamouti_2Rx,'go');
plot(gps_data.speed,thr_bmfr_maxq_2Rx,'rd');
%plot(thr_bmfr_feedbackq_2Rx,'y*');
legend('SISO','Alamouti','Optimal Beamforming')
% in = in+1;    
% h_fig = figure(in);
% [gps_x, gps_y] = plot_gps_coordinates(mm, gps_long_cat, gps_lat_cat, rps_SISO_16Qam_eNB1_cat);
% title('SISO Throughput 16QAM');


%%
in = in+1;    
h_fig = figure(in);
title('Alamouti');
xlabel('Time[Seconds]');
ylabel('Throughput');
hold on;
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Alamouti_4Qam_eNB1_cat,'bx');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Alamouti_16Qam_eNB1_cat,'go');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Alamouti_64Qam_eNB1_cat,'rd');
hold off;
legend('QPSK','16QAM','64QAM')

in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, gps_long_cat, gps_lat_cat, rps_Alamouti_16Qam_eNB1_cat);
title('Alamouti Througput 16QAM');

%%
in = in+1;    
h_fig = figure(in);
title('optimal Beamforming');
xlabel('Time[Seconds]');
ylabel('Throughput');
hold on;
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_4Qam_eNB1_maxq_cat,'bx');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_16Qam_eNB1_maxq_cat,'go');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_64Qam_eNB1_maxq_cat,'rd');
hold off;
legend('QPSK','16QAM','64QAM')

in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, gps_long_cat, gps_lat_cat, rps_Beamforming_16Qam_eNB1_maxq_cat);
title('optimal Beamforming');

%%
in = in+1;    
h_fig = figure(in);
title('Beamforming using feedback');
xlabel('Time[Seconds]');
ylabel('Throughput');
hold on;
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_4Qam_eNB1_feedbackq_cat,'bx');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_16Qam_eNB1_feedbackq_cat,'go');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_64Qam_eNB1_feedbackq_cat,'rd');
hold off;
legend('QPSK','16QAM','64QAM')

in = in+1;    
h_fig = figure(in);
[gps_x, gps_y] = plot_gps_coordinates(mm, gps_long_cat, gps_lat_cat, rps_Beamforming_16Qam_eNB1_feedbackq_cat);
title('Beamforming using feedback');



%%
in = in+1;
h_fig = figure(in);
title('Throughput SISO _ distance from BS ');
xlabel('Distance from BS [Km]');
ylabel('Throughput [Bits/sec]');
%dist(1:end);
plot(dist, rps_SISO_4Qam_eNB1_cat,'bx');
hold on;
plot(dist, rps_SISO_16Qam_eNB1_cat,'go');
plot(dist, rps_SISO_64Qam_eNB1_cat,'rd');
legend('QPSK','16QAM','64QAM')
 
in = in+1;
h_fig = figure(in);
title('Throughput SISO _ distance Travelled ')
xlabel('Distance travelled [Km]')
ylabel('Throughput [Bits/sec]')
plot(dist_travelled, rps_SISO_4Qam_eNB1_cat,'bx');
hold on
plot(dist_travelled, rps_SISO_16Qam_eNB1_cat,'go' );
plot(dist_travelled, rps_SISO_64Qam_eNB1_cat,'rd');

legend('QPSK','16QAM','64QAM')


%%
in = in+1;    
h_fig = figure(in);
title('Alamouti');
xlabel('Time[Seconds]');
ylabel('Throughput');
hold on;
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_SISO_16Qam_eNB1_cat,'bo');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Alamouti_16Qam_eNB1_cat,'rd');
plot(round(double(timestamp_cat(1:100:end))/1e9),rps_Beamforming_16Qam_eNB1_maxq_cat,'go');
hold off;
legend('SISO','ALAM','BMFR')


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
