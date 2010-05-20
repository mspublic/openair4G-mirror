mm=imread('maps/cordes.png');

load(fullfile(pathname,'results_UE.mat'));
if exist(fullfile(pathname,'nomadic'),'dir')
    nomadic = load(fullfile(pathname,'nomadic','results_UE.mat'));
    nomadic_flag = true;
else
    nomadic_flag = false;
end

UE_synched = (UE_mode_cat>0);
UE_connected = (UE_mode_cat==3);
timebase = gps_time_cat-gps_time_cat(1);
if (nomadic_flag)
    nomadic.UE_synched = (nomadic.UE_mode_cat>0);
    nomadic.UE_connected = (nomadic.UE_mode_cat==3);    
    nomadic.timebase = nomadic.gps_time_cat-gps_time_cat(1);
end

%%
h_fig = figure(1);
hold off
%frame_tx_cat(frame_tx_cat>100*60*60*8) = NaN;
plot(timebase,frame_tx_cat,'x')
if (nomadic_flag)
    hold on
    plot(nomadic.timebase,nomadic.frame_tx_cat,'rx')
    legend('vehicular','nomadic')
end
title('Time vs TX Frame')
xlabel('Time [sec]')
ylabel('TX Frame')
saveas(h_fig,fullfile(pathname,'frame_tx.eps'),'epsc2')

%%
h_fig = figure(2);
hold off
good = (rx_rssi_dBm_cat<40 & rx_rssi_dBm_cat>-120);
plot(timebase(good(:,1)),rx_rssi_dBm_cat(good(:,1),1),'x')
if (nomadic_flag)
    hold on
    nomadic.good = (nomadic.rx_rssi_dBm_cat<40 & nomadic.rx_rssi_dBm_cat>-120);
    plot(nomadic.timebase(nomadic.good(:,1)),nomadic.rx_rssi_dBm_cat(nomadic.good(:,1),1),'rx')
    legend('vehicular','nomadic')
end
title('RX RSSI [dBm]')
xlabel('Time [sec]')
ylabel('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm.eps'),'epsc2')

h_fig = figure(3);
hold off
plot_gps_coordinates(mm,gps_lon_cat(good(:,1)), gps_lat_cat(good(:,1)),...
    double(rx_rssi_dBm_cat(good(:,1),1)));
if (nomadic_flag)
    hold on
    plot_gps_coordinates([],nomadic.gps_lon_cat(nomadic.good(:,1)),nomadic.gps_lat_cat(nomadic.good(:,1)), ...
        nomadic.rx_rssi_dBm_cat(nomadic.good(:,1),1),'nomadic')
end
title('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_gps.jpg'),'jpg')

%%
h_fig = figure(4);
hold off
good = (pbch_fer_cat<=100 & pbch_fer_cat>=0).';
plot(timebase(UE_synched & good),pbch_fer_cat(UE_synched & good),'x');
if (nomadic_flag)
    hold on
    nomadic.good = (nomadic.pbch_fer_cat<=100 & nomadic.pbch_fer_cat>=0).';
    plot(nomadic.timebase(nomadic.good & nomadic.UE_synched),nomadic.pbch_fer_cat(nomadic.good & nomadic.UE_synched),'rx')
    legend('vehicular','nomadic')
end
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
plot(timebase(UE_connected & good),...
    (100-dlsch_fer_cat(UE_connected & good)).*tbs_cat(UE_connected & good)*7,'x');
if (nomadic_flag)
    hold on
    nomadic.good = (nomadic.dlsch_fer_cat<=100 & nomadic.dlsch_fer_cat>=0).';
    plot(nomadic.timebase(nomadic.UE_connected & nomadic.good),...
        (100-nomadic.dlsch_fer_cat(nomadic.UE_connected & nomadic.good)).*nomadic.tbs_cat(nomadic.UE_connected & nomadic.good)*7,'rx')
    legend('vehicular','nomadic')
end
xlabel('Time [sec]')
ylabel('Throughput L1 [bps]')
title('DLSCH Throughput')
saveas(h_fig,fullfile(pathname,'DLSCH_throughput.eps'),'epsc2')


h_fig = figure(8);
hold off
plot_gps_coordinates(mm,gps_lon_cat(UE_connected & good), gps_lat_cat(UE_connected & good),...
    double((100-dlsch_fer_cat(UE_connected & good)).*tbs_cat(UE_connected & good))*7);
title('DLSCH Throughput [bps]')
saveas(h_fig,fullfile(pathname,'DLSCH_troughput_gps.jpg'),'jpg')

%%
h_fig = figure(9);
hold off
plot_gps_coordinates(mm,gps_lon_cat(UE_connected & good), gps_lat_cat(UE_connected & good),...
    double(mcs_cat(UE_connected & good)));
title('DLSCH MCS')
saveas(h_fig,fullfile(pathname,'DLSCH_MCS_gps.jpg'),'jpg')



