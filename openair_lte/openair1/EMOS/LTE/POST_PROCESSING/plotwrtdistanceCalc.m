% For Distance from Base Station. We take the reference of Eurecom's lat
% and long.
% Source http://itouchmap.com/latlong.html

BS_Lat = 43.626740032605134;
BS_Long = 7.046270370483398;
MS_Lat = zeros(1,length(gps_data));
MS_Long = zeros(1,length(gps_data));
dist = zeros(1,length(gps_data));
for i = 1:length(gps_data)

    MS_Lat(i) = gps_data(i).latitude;
    MS_Long(i) = gps_data(i).longitude;
    dist(i) = Dist_Calc_from_GPS(BS_Lat, BS_Long, MS_Lat(i), MS_Long(i));
    % Please remember that the above calculated distance is in KiloMeters
end

rx_rssi_dBm = zeros(1,NFrames/(decimation*10));
j = 0;
for i=1:decimation:NFrames/decimation
    j = j + 1;
    rx_rssi_dBm(j) = estimates(i).phy_measurements(1).rx_rssi_dBm(1);
end
h_fig = figure(1);
plot(dist, rx_rssi_dBm)
title('RX RSSI _ distance from BS [dBm]')
xlabel('Distance from BS [Km]')
ylabel('RX RSSI [dBm]')
saveas(h_fig,'RX_rssi_dBm_dist_from_BS.eps','epsc2')


% For Distance travelled we need to take first point as starting point and
% then the difference of starting point from every other measured point
% will give us the distance travelled. 

start_Lat = gps_data(1).latitude;
start_Long = gps_data(1).longitude;

current_Lat = zeros(1,length(gps_data));
current_Long = zeros(1,length(gps_data));
dist_travelled = zeros(1,length(gps_data));
for i = 1:length(gps_data)
    
    current_Lat(i) = gps_data(i).latitude;
    current_Long(i) = gps_data(i).longitude;
    dist_travelled(i) =  Dist_Calc_from_GPS(start_Lat, start_Long, current_Lat(i), current_Long(i));
    % Please remember that the above calculated distance is in KiloMeters
end

h_fig = figure(2);
plot(dist_travelled, rx_rssi_dBm)
title('RX RSSI _ distance travelled[dBm]')
xlabel('Distance travelled by MS [Km]')
ylabel('RX RSSI [dBm]')
%saveas(h_fig,'RX_rssi_dBm_dist_from_BS.eps','epsc2')

