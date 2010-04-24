load(fullfile(pathname,'results_UE.mat'));
mm=imread('maps/cordes.png');

h_fig = figure(2);
hold off
plot(frame_tx_cat,rx_rssi_dBm_cat)
title('RX RSSI [dBm]')
xlabel('Frame number')
ylabel('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm.eps'),'epsc2')

h_fig = figure(3);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,rx_rssi_dBm_cat(:,1));
title('RX I0 [dBm]')
xlabel('Frame number')
ylabel('RX I0 [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_gps.eps'),'epsc2')

h_fig = figure(4);
pbch_fer_cat(pbch_fer_cat<=100)=0;
plot(frame_tx_cat,pbch_fer_cat);
title('PBCH FER')
xlabel('Frame number')
ylabel('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_FER.eps'),'epsc2')
