load(fullfile(pathname,'results_UE.mat'));
mm=imread('maps/cordes.png');

UE_synched = (UE_mode_cat>0);

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
good = (pbch_fer_cat<=100 & pbch_fer_cat>=0).';
plot(double(timestamp_cat(UE_synched & good))./1e9,pbch_fer_cat(UE_synched & good),'x');
title('PBCH FER')
xlabel('Time [sec]')
ylabel('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_FER.eps'),'epsc2')

h_fig = figure(5);
hold off
plot_gps_coordinates(mm,gps_lon_cat(UE_synched & good), gps_lat_cat(UE_synched & good),...
    double(pbch_fer_cat(UE_synched & good)));
title('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_fer_gps.jpg'),'jpg')

%%
h_fig = figure(6);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,double(UE_mode_cat));
title('UE mode')
saveas(h_fig,fullfile(pathname,'UE_mode_gps.jpg'),'jpg')

%%
h_fig = figure(7);
hold off
good = (dlsch_fer_cat<=100 & dlsch_fer_cat>=0).';
plot(double(timestamp_cat(UE_synched & good))./1e9,...
    (100-dlsch_fer_cat(UE_synched & good)).*tbs_cat(UE_synched & good));
xlabel('Time [sec]')
ylabel('Throughput L1 [bps]')
title('DLSCH Throughput')
saveas(h_fig,fullfile(pathname,'DLSCH_throughput.eps'),'epsc2')


h_fig = figure(8);
hold off
plot_gps_coordinates(mm,gps_lon_cat(UE_synched & good), gps_lat_cat(UE_synched & good),...
    double((100-dlsch_fer_cat(UE_synched & good)).*tbs_cat(UE_synched & good)));
title('DLSCH Throughput [bps]')
saveas(h_fig,fullfile(pathname,'DLSCH_troughput_gps.jpg'),'jpg')


