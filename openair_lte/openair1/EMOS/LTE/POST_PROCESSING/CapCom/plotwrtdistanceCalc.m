% For Distance from Base Station. We take the reference of Eurecom's lat
% and long.
% Source http://itouchmap.com/latlong.html

BS_Lat = 44.084015;
BS_Long = 1.965765;
MS_Lat = [gps_data_cat.latitude];
MS_Long = [gps_data_cat.longitude];
dist = zeros(1,length(gps_data_cat));
%dist = Dist_Calc_from_GPS(BS_Lat, BS_Long, MS_Lat, MS_Long);

 for i = 1:length(gps_data_cat)
 
%     MS_Lat(i) = gps_data(i).latitude;
%     MS_Long(i) = gps_data(i).longitude;
     dist(i) = Dist_Calc_from_GPS(BS_Lat, BS_Long, MS_Lat(i), MS_Long(i));
%     % Please remember that the above calculated distance is in KiloMeters
 end
step = 100;
% if (decimation == 1)
% rx_rssi_dBm = zeros(1,NFrames/(decimation*100));
% step = 100;
% else if(decimation == 10)
%     rx_rssi_dBm = zeros(1,NFrames/(decimation*10));
%     step = 10;
%     else if decimation ==100
%             rx_rssi_dBm = zeros(1,NFrames/decimation);
%             step = 1;
%         end
%     end
% end
rxx_rssi_dBm = [minestimates_cat(1:100:end).rx_rssi_dBm];
% j = 0;
% for i=1:step:NFrames
%     j = j + 1;
%     rxx_rssi_dBm(j) = minestimates(i).rx_rssi_dBm;
% end
in = in+1;
h_fig = figure(in);
plot(dist, rxx_rssi_dBm, 'rx')
title('RX RSSI distance from BS [dBm]')
xlabel('Distance from BS [Km]')
ylabel('RX RSSI [dBm]')
saveas(h_fig, fullfile(pathname,'RX_RSSI_distance_from_BS.eps'),'epsc2');


% For Distance travelled we need to take first point as starting point and
% then the difference of starting point from every other measured point
% will give us the distance travelled. 

% start_Lat = gps_data(1).latitude;
% start_Long = gps_data(1).longitude;

% start_Lat = gps_lat_cat(1);
% start_Long = gps_long_cat(1);

% current_Lat = zeros(1,length(gps_data));
% current_Long = zeros(1,length(gps_data));
dist_travelled = zeros(1,length(gps_data_cat));

for i = 2:length(gps_data_cat);
    % current_Lat(i) = gps_data(i).latitude;
    % current_Long(i) = gps_data(i).longitude;
    dist_travelled(i) =  Dist_Calc_from_GPS(MS_Lat(i-1), MS_Long(i-1), MS_Lat(i), MS_Long(i)); 
    % current_Lat(i), current_Long(i));
    % Please remember that the above calculated distance is in KiloMeters
end
% gps_lat_cat(end) = [];
% gps_long_cat(end) = [];

dist_travelled = cumsum(dist_travelled);
in = in+1;
h_fig = figure(in);
plot(dist_travelled, rxx_rssi_dBm, 'rx')
title('RX RSSI _ distance travelled[dBm]')
xlabel('Distance travelled by MS [Km]')
ylabel('RX RSSI [dBm]')
saveas(h_fig, fullfile(pathname,'RX_RSSI_distance_travelled.eps'),'epsc2');
%saveas(h_fig,'RX_rssi_dBm_dist_from_BS.eps','epsc2')

