load(fullfile(pathname,'results_UE.mat'));
mm=imread('maps/cordes.png');

%%
h_fig = figure(1);
hold off
frame_tx_cat(frame_tx_cat>100*60*60*8) = NaN;
plot(double(timestamp_cat)./1e9,frame_tx_cat,'x')
title('Time vs TX Frame')
xlabel('Time [sec]')
ylabel('TX Frame')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm.eps'),'epsc2')

%%
h_fig = figure(2);
hold off
plot(double(timestamp_cat)./1e9,rx_rssi_dBm_cat,'x')
title('RX RSSI [dBm]')
xlabel('Time [sec]')
ylabel('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm.eps'),'epsc2')

h_fig = figure(3);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,double(rx_rssi_dBm_cat(:,1)));
title('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_gps.jpg'),'jpg')

%%
h_fig = figure(4);
hold off
pbch_fer_cat(pbch_fer_cat>100)=0;
plot(double(timestamp_cat)./1e9,pbch_fer_cat,'x');
title('PBCH FER')
xlabel('Time [sec]')
ylabel('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_FER.eps'),'epsc2')

h_fig = figure(5);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,double(pbch_fer_cat));
title('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_fer_gps.jpg'),'jpg')

%%
h_fig = figure(6);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,double(UE_mode_cat));
title('UE mode')
saveas(h_fig,fullfile(pathname,'UE_mode_gps.jpg'),'jpg')

