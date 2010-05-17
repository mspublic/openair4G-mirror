% For Distance from Base Station. We take the reference of Eurecom's lat
% and long.
% Source http://itouchmap.com/latlong.html

BS_Lat = 44.084015;
BS_Long = 1.965765;
MS_Lat = gps_lat_cat;
MS_Long = gps_long_cat;
dist = zeros(1,length(gps_lat_cat));
%dist = Dist_Calc_from_GPS(BS_Lat, BS_Long, MS_Lat, MS_Long);

 for i = 1:length(gps_lat_cat)
 
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
rxx_rssi_dBm = rx_rssi_dBm_cat(1:100:end);
% j = 0;
% for i=1:step:NFrames
%     j = j + 1;
%     rxx_rssi_dBm(j) = minestimates(i).rx_rssi_dBm;
% end
in = in+1;
h_fig = figure(in);
plot(dist, rxx_rssi_dBm, 'rx')
title('RX RSSI _ distance from BS [dBm]')
xlabel('Distance from BS [Km]')
ylabel('RX RSSI [dBm]')
%saveas(h_fig, fullfile(pathname,'RX_I0_dBm.eps')'RX_rssi_dBm_dist_from_BS.eps','epsc2')


% For Distance travelled we need to take first point as starting point and
% then the difference of starting point from every other measured point
% will give us the distance travelled. 

% start_Lat = gps_data(1).latitude;
% start_Long = gps_data(1).longitude;

start_Lat = gps_lat_cat(1);
start_Long = gps_long_cat(1);

% current_Lat = zeros(1,length(gps_data));
% current_Long = zeros(1,length(gps_data));
 dist_travelled = zeros(1,length(gps_lat_cat));
y = length(gps_lat_cat);

gps_lat_cat(y+1) = gps_lat_cat(y);
gps_long_cat(y+1) = gps_long_cat(y);
for i = 2:( y + 1)
    
%    current_Lat(i) = gps_data(i).latitude;
%    current_Long(i) = gps_data(i).longitude;
    dist_travelled(i-1) =  Dist_Calc_from_GPS(gps_lat_cat(i-1), gps_long_cat(i-1), gps_lat_cat(i), gps_long_cat(i)); 
    %current_Lat(i), current_Long(i));
    % Please remember that the above calculated distance is in KiloMeters
end
gps_lat_cat(end) = [];
gps_long_cat(end) = [];

dist_travelled = cumsum(dist_travelled);
in = in+1;
h_fig = figure(in);
plot(dist_travelled, rxx_rssi_dBm, 'rx')
title('RX RSSI _ distance travelled[dBm]')
xlabel('Distance travelled by MS [Km]')
ylabel('RX RSSI [dBm]')
%saveas(h_fig,'RX_rssi_dBm_dist_from_BS.eps','epsc2')

