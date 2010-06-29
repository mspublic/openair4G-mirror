function [dist, dist_travelled] = calc_dist(MS_Lat, MS_Long)
% [dist, dist_traveled] = calc_dist(MS_Lat, MS_Long)
% Calculates distance from Base Station as well as distance travelled in km.
% We take the reference of the BS in Cordes
% Source http://itouchmap.com/latlong.html

BS_Lat = 44.084015;
BS_Long = 1.965765;

dist = zeros(1,length(MS_Lat));
for i = 1:length(MS_Lat)
    dist(i) = Dist_Calc_from_GPS(BS_Lat, BS_Long, MS_Lat(i), MS_Long(i));
end

dist_travelled = zeros(1,length(MS_Lat));
for i = 2:length(MS_Lat);
    dist_travelled(i) =  Dist_Calc_from_GPS(MS_Lat(i-1), MS_Long(i-1), MS_Lat(i), MS_Long(i));
end
dist_travelled = cumsum(dist_travelled);
