%% load data
estimates_eNB = load(fullfile(pathname,'results_eNB.mat'));

% calculate the indices where the ue and the eNB were connected (they might
% not have set the flag in the same frame)
eNB_connected = ([estimates_eNB.eNb_UE_stats_cat(:).UE_mode]==3);

%%
h_fig = figure(11);
hold off
plot(estimates_eNB.frame_tx_cat,estimates_eNB.rx_N0_dBm_cat,'x')
%plot(double(timestamp_cat)/1e9,rx_N0_dBm_cat,'x')
title('UL I0 [dBm]')
xlabel('Frames')
ylabel('UL I0 [dBm]')
legend('Sector 0','Sector 1','Sector 2')
saveas(h_fig,fullfile(pathname,'UL_I0_dBm.eps'),'epsc2')

%%
UL_rssi_cat = zeros(length(estimates_eNB.phy_measurements_cat),3);
for i=1:length(estimates_eNB.phy_measurements_cat)
    UL_rssi_cat(i,:) = double([estimates_eNB.phy_measurements_cat(i,:).rx_rssi_dBm]);
end
UL_rssi_cat(~eNB_connected,:) = -120;
h_fig = figure(12);
hold off
plot(estimates_eNB.frame_tx_cat,UL_rssi_cat,'x');
title('UL RSSI [dBm]')
xlabel('Frames')
ylabel('UL RSSI [dBm]')
legend('Sector 0','Sector 1','Sector 2')
saveas(h_fig,fullfile(pathname,'UL_RSSI_dBm.eps'),'epsc2')

%%
estimates_eNB.ulsch_fer_cat = [100 diff(estimates_eNB.ulsch_errors_cat)];
ulsch_throughput = double(estimates_eNB.tbs_cat) .* double(100 - estimates_eNB.ulsch_fer_cat) .* 3;
ulsch_throughput(1,~eNB_connected) = 0;
ulsch_throughput_ideal_1Rx = estimates_eNB.Rate_64Qam_1RX_cat;
ulsch_throughput_ideal_1Rx(~eNB_connected,1) = 0;
ulsch_throughput_ideal_2Rx = estimates_eNB.Rate_64Qam_2RX_cat;
ulsch_throughput_ideal_2Rx(~eNB_connected,1) = 0;
h_fig = figure(13);
hold off
plot(estimates_eNB.frame_tx_cat,ulsch_throughput,'x');
hold on
plot(estimates_eNB.frame_tx_cat,ulsch_throughput_ideal_1Rx*100,'rx');
hold on
plot(estimates_eNB.frame_tx_cat,ulsch_throughput_ideal_2Rx*100,'gx');
legend('modem','ideal 1 rx antenna','ideal 2 rx antennas');
title('UL Throughput [bps]')
xlabel('Frames')
ylabel('UL Throughput [bps]')
saveas(h_fig,fullfile(pathname,'UL_throughput.eps'),'epsc2')

