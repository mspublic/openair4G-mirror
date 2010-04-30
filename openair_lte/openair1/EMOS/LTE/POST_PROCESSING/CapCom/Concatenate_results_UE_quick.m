%load(fullfile(pathname,'results_UE.mat'));
addpath('../IntegrityCheck')
clear all;
clc;
%mm=imread('maps/cordes.png');
d = dir(['/homes/latif/devel/openair_lte/openair1/EMOS/LTE/POST_PROCESSING/IntegrityCheck/' '*.mat'])
filenames = {d.name};
rps_SISO_4Qam_eNB1_cat =[];
rps_SISO_16Qam_eNB1_cat =[];
rps_SISO_64Qam_eNB1_cat =[];

rps_Alamouti_4Qam_eNB1_cat = [];
rps_Alamouti_16Qam_eNB1_cat = [];
rps_Alamouti_64Qam_eNB1_cat = [];

 rps_Beamforming_4Qam_eNB1_maxq_cat = [];
 rps_Beamforming_16Qam_eNB1_maxq_cat = [];
 rps_Beamforming_64Qam_eNB1_maxq_cat = [];
 
 rps_Beamforming_4Qam_eNB1_feedbackq_cat = [];
 rps_Beamforming_16Qam_eNB1_feedbackq_cat = [];
 rps_Beamforming_64Qam_eNB1_feedbackq_cat = [];


gps_long_cat = [];
gps_lat_cat = [];

frame_tx_cat = [];
frame_rx_cat = [];

timestamp_cat = [];

rx_rssi_dBm_cat = [];

pbch_fer_cat = [];
in = 0;
for f = 1:length(filenames)
    s = cat(1, filenames{f});
    load (s);
    rps_SISO_4Qam_eNB1_cat = [rps_SISO_4Qam_eNB1_cat throughput.rateps_SISO_4Qam_eNB1];
    rps_SISO_16Qam_eNB1_cat = [rps_SISO_16Qam_eNB1_cat throughput.rateps_SISO_16Qam_eNB1];
    rps_SISO_64Qam_eNB1_cat = [rps_SISO_64Qam_eNB1_cat throughput.rateps_SISO_64Qam_eNB1];
    
    rps_Alamouti_4Qam_eNB1_cat = [rps_Alamouti_4Qam_eNB1_cat throughput.rateps_alamouti_4Qam_eNB1];
    rps_Alamouti_16Qam_eNB1_cat = [rps_Alamouti_16Qam_eNB1_cat throughput.rateps_alamouti_16Qam_eNB1];
    rps_Alamouti_64Qam_eNB1_cat = [rps_Alamouti_64Qam_eNB1_cat throughput.rateps_alamouti_64Qam_eNB1];
    
    
     rps_Beamforming_4Qam_eNB1_feedbackq_cat = [rps_Beamforming_4Qam_eNB1_feedbackq_cat throughput.rateps_beamforming_4Qam_eNB1_feedbackq];
     rps_Beamforming_16Qam_eNB1_feedbackq_cat = [rps_Beamforming_16Qam_eNB1_feedbackq_cat throughput.rateps_beamforming_16Qam_eNB1_feedbackq];
     rps_Beamforming_64Qam_eNB1_feedbackq_cat = [rps_Beamforming_64Qam_eNB1_feedbackq_cat throughput.rateps_beamforming_64Qam_eNB1_feedbackq];

     rps_Beamforming_4Qam_eNB1_maxq_cat = [rps_Beamforming_4Qam_eNB1_maxq_cat throughput.rateps_beamforming_4Qam_eNB1_maxq];
     rps_Beamforming_16Qam_eNB1_maxq_cat = [rps_Beamforming_16Qam_eNB1_maxq_cat throughput.rateps_beamforming_16Qam_eNB1_maxq];
     rps_Beamforming_64Qam_eNB1_maxq_cat = [rps_Beamforming_64Qam_eNB1_maxq_cat throughput.rateps_beamforming_64Qam_eNB1_maxq];

     
    gps_long_cat = [gps_long_cat gps_data.longitude];
    gps_lat_cat = [gps_lat_cat gps_data.latitude];
    
    frame_tx_cat = [frame_tx_cat minestimates.frame_tx];
    frame_rx_cat = [frame_rx_cat minestimates.frame_rx];
    
    timestamp_cat = [timestamp_cat minestimates.timestamp];
    
    rx_rssi_dBm_cat = [rx_rssi_dBm_cat minestimates.rx_rssi_dBm];
    
    pbch_fer_cat = [pbch_fer_cat minestimates.pbch_fer];
    
    
end

save 'results_UE.mat'
% 
% mkdir('/homes/latif/devel/openair_lte/openair1/EMOS/LTE/POST_PROCESSING/SIM_Results', s);
% 
% plotwrtdistanceCalc;
% in = in+1;    
% h_fig = figure(in);
% title([s '__SISO']);
% xlabel('Time[Seconds]');
% ylabel('Throughput');
% hold on;
% plot(throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(throughput.rateps_SISO_16Qam_eNB1,'g-o');
% plot(throughput.rateps_SISO_64Qam_eNB1,'r-o');
% hold off;
% 
% in = in+1;
% h_fig = figure(in);
% hold on;
% title('Throughput SISO _ distance from BS ');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput [Bits/sec]');
% %dist(1:end);
% plot(dist, throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(dist, throughput.rateps_SISO_16Qam_eNB1,'g-o');
% plot(dist, throughput.rateps_SISO_64Qam_eNB1,'r-o');
% hold off;
%  
% in = in+1;
% h_fig = figure(in);
% plot(dist_travelled, throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(dist_travelled, throughput.rateps_SISO_16Qam_eNB1,'g-o' );
% plot(dist_travelled, throughput.rateps_SISO_64Qam_eNB1,'r-o');
% title('Throughput SISO _ distance Travelled ')
% xlabel('Distance travelled [Km]')
% ylabel('Throughput [Bits/sec]')
% 
% end
