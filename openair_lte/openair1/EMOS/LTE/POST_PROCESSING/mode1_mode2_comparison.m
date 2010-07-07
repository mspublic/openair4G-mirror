mode1=load('G:\EMOS\data\20100526_mode1_parcours1_part4_part5\results_UE.mat');
mode2=load('G:\EMOS\data\20100520_mode2_parcours1_part4_part5\results_UE.mat');
mm='cordes';


%%
for i = 2:length(mode1.gps_lat_cat)
    mode1.dist_travelled(i-1) =  Dist_Calc_from_GPS(mode1.gps_lat_cat(i-1), mode1.gps_lon_cat(i-1), mode1.gps_lat_cat(i), mode1.gps_lon_cat(i)); 
    % Please remember that the above calculated distance is in KiloMeters
end
mode1.dist_travelled = cumsum(mode1.dist_travelled);

for i = 2:length(mode2.gps_lat_cat)
    mode2.dist_travelled(i-1) =  Dist_Calc_from_GPS(mode2.gps_lat_cat(i-1), mode2.gps_lon_cat(i-1), mode2.gps_lat_cat(i), mode2.gps_lon_cat(i)); 
    % Please remember that the above calculated distance is in KiloMeters
end
mode2.dist_travelled = cumsum(mode2.dist_travelled);

figure(1)
hold off
plot(mode1.gps_lon_cat)
hold on
plot(mode2.gps_lon_cat,'r')

figure(2)
hold off
plot(mode1.dist_travelled)
hold on
plot(mode2.dist_travelled,'r')

%%
start_mode1 = 746;
length_mode1 = 300;
start_mode2 = 330;
length_mode2 = 380;

figure(3)
hold off
plot(mode1.dist_travelled(start_mode1:start_mode1+length_mode1)-mode1.dist_travelled(start_mode1), ...
    mode1.rx_rssi_dBm_cat(start_mode1:start_mode1+length_mode1,1),'x')
hold on
plot(mode2.dist_travelled(start_mode2:start_mode2+length_mode2)-mode2.dist_travelled(start_mode2), ...
    mode2.rx_rssi_dBm_cat(start_mode2:start_mode2+length_mode2,1),'rx')

h_fig = figure(4);
plot_gps_coordinates(mm,mode1.gps_lon_cat(start_mode1:start_mode1+length_mode1), ...
    mode1.gps_lat_cat(start_mode1:start_mode1+length_mode1),...
    double(mode1.rx_rssi_dBm_cat(start_mode1:start_mode1+length_mode1,1)));
saveas(h_fig,'mode1_mode2_comparison_rssi_dBm.jpg','jpg');

%%
mode1.UE_connected = (mode1.UE_mode_cat==3);
%mode1.good = (mode1.dlsch_fer_cat<=100 & mode1.dlsch_fer_cat>=0).';
mode1.throughput = (100-mode1.dlsch_fer_cat).*mode1.tbs_cat*6;
mode1.throughput(~mode1.UE_connected) = 0;
mode2.UE_connected = (mode2.UE_mode_cat==3);
%mode2.good = (mode2.dlsch_fer_cat<=100 & mode2.dlsch_fer_cat>=0).';
mode2.throughput = (100-mode2.dlsch_fer_cat).*mode2.tbs_cat*6;
mode2.throughput(~mode2.UE_connected) = 0;

h_fig = figure(5);
hold off
plot(mode1.dist_travelled(start_mode1:start_mode1+length_mode1)-mode1.dist_travelled(start_mode1),...
    mode1.throughput(start_mode1:start_mode1+length_mode1),'x');
hold on
plot(mode2.dist_travelled(start_mode2:start_mode2+length_mode2)-mode2.dist_travelled(start_mode2),...
    mode2.throughput(start_mode2:start_mode2+length_mode2),'rx');
legend('Mode1','Mode2')
xlabel('Distance travelled [km]')
ylabel('Throughput [bps]')
saveas(h_fig,'mode1_mode2_comparison_troughput_distance_travelled.eps','epsc2');


