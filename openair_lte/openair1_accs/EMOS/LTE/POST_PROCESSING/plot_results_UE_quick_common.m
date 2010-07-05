%%
UE_synched = (UE_mode_cat>0);
UE_connected = (UE_mode_cat==3);
timebase = gps_time_cat-gps_time_cat(1);
if (nomadic_flag)
    nomadic.UE_synched = (nomadic.UE_mode_cat>0);
    nomadic.UE_connected = (nomadic.UE_mode_cat==3);    
    nomadic.timebase = nomadic.gps_time_cat-gps_time_cat(1);
end

[dist, dist_traveled] = calc_dist(gps_lat_cat,gps_lon_cat);

%% Frame TX number over time
h_fig = h_fig+1;
figure(h_fig);
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

%% RX RSSI over time and GPS coordinates
h_fig = h_fig+1;
figure(h_fig);
hold off
good = (rx_rssi_dBm_cat<40 & rx_rssi_dBm_cat>-120);
% n0_power_tot_dBm = [phy_measurements_cat(:).n0_power_tot_dBm];
% if isa(n0_power_tot_dBm,'uint16') % it was read with the wrong datatype
%     n0_power_tot_dBm = double(n0_power_tot_dBm)-pow2(16);
% else
%     n0_power_tot_dBm = double(n0_power_tot_dBm);
% end
    
plot(timebase(good(:,1)),rx_rssi_dBm_cat(good(:,1),1),'x')
hold on
%plot(timebase(good(:,1)),n0_power_tot_dBm(good(:,1)),'g.')
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

%%
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_gps_coordinates(mm,gps_lon_cat(good(:,1)), gps_lat_cat(good(:,1)),...
    double(rx_rssi_dBm_cat(good(:,1),1)));
if (nomadic_flag)
    hold on
    plot_gps_coordinates([],nomadic.gps_lon_cat(nomadic.good(:,1)),nomadic.gps_lat_cat(nomadic.good(:,1)), ...
        nomadic.rx_rssi_dBm_cat(nomadic.good(:,1),1),'nomadic');
end
title('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_gps.jpg'),'jpg')

%%
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_in_bins(dist(good(:,1)),double(rx_rssi_dBm_cat(good(:,1),1)),0:ceil(max(dist)));
xlabel('Distance [km]')
ylabel('RX RSSI [dBm]')
saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_dist_bars.eps'),'epsc2');

%% fit path loss model
h_fig = h_fig+1;
figure(h_fig);
hold off
plot(dist(good(:,1)), double(rx_rssi_dBm_cat(good(:,1),1)), 'rx')
hold on
PL = double(rx_rssi_dBm_cat(good(:,1),1)) - 43;
d = 0:0.1:ceil(max(dist(good(:,1))));
res = [ones(length(PL),1) log10(dist(good(:,1)).')]\PL;
plot(d,res(1)+res(2)*log10(d)+43,'b')
plot(d,43-cost231_hata(d),'k')
legend('measured','fitted','COST231-Hata');
title('RX RSSI distance from BS [dBm]')
xlabel('Distance from BS [Km]')
ylabel('RX RSSI [dBm]')
saveas(h_fig, fullfile(pathname,'RX_RSSI_dBm_dist_with_PL.eps'),'epsc2');


%% PBCH FER over time and GPS coordinates
h_fig = h_fig+1;
figure(h_fig);
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

h_fig = h_fig+1;
figure(h_fig);
hold off
plot_gps_coordinates(mm,gps_lon_cat(UE_synched & good), gps_lat_cat(UE_synched & good),...
    double(pbch_fer_cat(UE_synched & good)));
title('PBCH FER')
saveas(h_fig,fullfile(pathname,'PBCH_fer_gps.jpg'),'jpg')

%% UE mode over GPS coordinates
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,double(UE_mode_cat));
title('UE mode')
saveas(h_fig,fullfile(pathname,'UE_mode_gps.jpg'),'jpg')

%% DLSCH throughput over time 
h_fig = h_fig+1;
figure(h_fig);
hold off
dlsch_throughput = double((100-dlsch_fer_cat).*tbs_cat.*6);
good = (dlsch_fer_cat<=100 & dlsch_fer_cat>=0).';
dlsch_throughput(~UE_connected | ~good) = 0;
plot(timebase,dlsch_throughput,'x');
if (nomadic_flag)
    hold on
    nomadic.dlsch_throughput = (100-nomadic.dlsch_fer_cat).* nomadic.tbs_cat.*6;
    nomadic.good = (nomadic.dlsch_fer_cat<=100 & nomadic.dlsch_fer_cat>=0).';
    nomadic.dlsch_throughput(~nomadic.UE_connected | ~nomadic.good) = 0;
    plot(nomadic.timebase,nomadic.dlsch_throughput,'rx')
    legend('vehicular','nomadic')
end
xlabel('Time [sec]')
ylabel('Throughput L1 [bps]')
title('DLSCH Throughput')
saveas(h_fig,fullfile(pathname,'DLSCH_throughput.eps'),'epsc2')

%% DLSCH throughput over GPS coordinates
h_fig = h_fig+1;
figure(h_fig);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,dlsch_throughput);
title('DLSCH Throughput [bps]')
saveas(h_fig,fullfile(pathname,'DLSCH_troughput_gps.jpg'),'jpg')


% %% MCS over GPS coordinates
% h_fig = h_fig+1;
% figure(h_fig);
% hold off
% plot_gps_coordinates(mm,gps_lon_cat(UE_connected & good), gps_lat_cat(UE_connected & good),...
%     double(mcs_cat(UE_connected & good)));
% title('DLSCH MCS')
% saveas(h_fig,fullfile(pathname,'DLSCH_MCS_gps.jpg'),'jpg')

%% Coded throughput CDF comparison
h_fig = h_fig+1;
figure(h_fig);
[f,x] = ecdf(dlsch_throughput);
plot(x,f,'b','Linewidth',2)
title('DLSCH Throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'DLSCH_throughput_cdf_comparison.eps'),'epsc2')

%% Unoded throughput CDF comparison
h_fig = h_fig+1;
figure(h_fig);
[f,x] = ecdf(coded2uncoded(dlsch_throughput,'DL'));
plot(x,f,'b','Linewidth',2)
title('DLSCH Uncoded Throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'DLSCH_uncoded_throughput_cdf_comparison.eps'),'epsc2')


%% plot througput as a function of speed
h_fig = h_fig+1;    
h_fig = figure(h_fig);
plot_in_bins(gps_speed_cat, dlsch_throughput,  0:5:40);
title('DLSCH Throughput vs Speed');
xlabel('Speed[Meters/Second]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'DLSCH_throughput_speed.eps'),'epsc2');

%% plot througput as a function of distance
h_fig = h_fig+1;    
h_fig = figure(h_fig);
plot_in_bins(dist, dlsch_throughput,  0:15);
title('DLSCH Throughput vs Dist');
xlabel('Dist[km]');
ylabel('Throughput[Bits/sec]');
saveas(h_fig,fullfile(pathname,'DLSCH_throughput_dist.eps'),'epsc2');


