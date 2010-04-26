
load(fullfile(pathname,'results_eNB.mat'));
mm=imread('maps/cordes.png');

%%
h_fig = figure(2);
hold off
plot(frame_tx_cat,rx_N0_dBm_cat,'x')
%plot(double(timestamp_cat)/1e9,rx_N0_dBm_cat,'x')
title('RX I0 [dBm]')
xlabel('Frames')
ylabel('RX I0 [dBm]')
saveas(h_fig,fullfile(pathname,'RX_I0_dBm.eps'),'epsc2')

%%
h_fig = figure(3);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,rx_N0_dBm_cat);
title('RX I0 [dBm]')
xlabel('Frame number')
ylabel('RX I0 [dBm]')
saveas(h_fig,fullfile(pathname,'RX_I0_dBm_gps.jpg'),'jpg')



