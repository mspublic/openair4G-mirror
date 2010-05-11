% align the gps measurements of the UE with the eNB
estimates_UE = load(fullfile(pathname,'results_UE.mat'));
estimates_eNB = load(fullfile(pathname,'results_eNB.mat'));

%%
UE_connected = (estimates_UE.UE_mode_cat==3);
eNB_connected = ([estimates_eNB.eNb_UE_stats_cat(:).UE_mode]==3);

framestamp_min = min(estimates_UE.frame_tx_cat(1),estimates_eNB.frame_tx_cat(1));
framestamp_max = max(estimates_UE.frame_tx_cat(end),estimates_eNB.frame_tx_cat(end));
NFrames = framestamp_max-framestamp_min+1;

align_matrix = false(2,NFrames/decimation);
align_matrix(1,floor(estimates_eNB.frame_tx_cat/decimation) - floor(framestamp_min/decimation) + 1) = eNB_connected;
align_matrix(2,floor(estimates_UE.frame_tx_cat/decimation) - floor(framestamp_min/decimation) + 1) = UE_connected;

UE_aligned  = align_matrix(1,align_matrix(2,:));
eNB_aligned = align_matrix(2,align_matrix(1,:));

% now estimates_UE.frame_tx_connected_cat(UE_aligned) ==
% estimates_eNB.frame_tx_cat(eNB_aligned)

%%
mm=imread('maps/cordes.png');

%%
h_fig = figure(11);
hold off
plot(estimates_eNB.frame_tx_cat,estimates_eNB.rx_N0_dBm_cat,'x')
%plot(double(timestamp_cat)/1e9,rx_N0_dBm_cat,'x')
title('RX I0 [dBm]')
xlabel('Frames')
ylabel('RX I0 [dBm]')
saveas(h_fig,fullfile(pathname,'RX_I0_dBm.eps'),'epsc2')

%%
UL_CQI_cat = double([estimates_eNB.phy_measurements_cat(:,1).wideband_cqi_tot]);
h_fig = figure(12);
hold off
plot_gps_coordinates(mm,estimates_UE.gps_lon_cat(UE_aligned), ...
        estimates_UE.gps_lat_cat(UE_aligned),...
        UL_CQI_cat(eNB_aligned));
title('UL CQI')
saveas(h_fig,fullfile(pathname,'UL_CQI_gps.jpg'),'jpg')

%%
UL_rssi_cat = zeros(length(estimates_eNB.eNb_UE_stats_cat),2);
for i=1:length(estimates_eNB.eNb_UE_stats_cat)
    UL_rssi_cat(i,:) = double(estimates_eNB.eNb_UE_stats_cat(i).UL_rssi);
end
h_fig = figure(13);
hold off
plot_gps_coordinates(mm,estimates_UE.gps_lon_cat(UE_aligned), ...
        estimates_UE.gps_lat_cat(UE_aligned), ...
        UL_rssi_cat(eNB_aligned,1));
title('UL RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'UL_RSSI_dBm_gps.jpg'),'jpg')


