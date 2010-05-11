function [gps_x, gps_y] = plot_gps_coordinates(mm, longitude, latitude, rx_rssi, label, color)
% h = plot_gps_coordinates(mm, longitude, latitude, rx_rssi, label, color)
%
%  This function plots the gps coordinates given by langitude and latutude
%  onto the map of garbejaire. If langitude and latutude are matrices, each
%  row is treated as a seperate user and plotted in a different color.
%  This function uses the Plate carr�e projection
%  (http://en.wikipedia.org/wiki/Plate_carr%C3%A9e_projection). We assume
%  the coordinates of 3 reference points are known in lat/lon as well as
%  x/y coordinates. This projection method is only fairly accurate. Also
%  there will be inaccuracies in the reference points. It is therefore
%  necessary the check the result and include a correction factor if
%  neccesary.
%
%  The function takes the following parameters
%   mm ... image matrix (from imread) of the map; can be empty for no map
%   longitude, latitude  ... from gps data
%   rx_rssi ... data to be plotted
%   label ... text label for data
%   color ... plot data in color instead of using data in rx_rssi

if (nargin == 4 && ~isempty(rx_rssi))
    m = colormap;
    cmin = min(rx_rssi);
    cmax = max(rx_rssi);
    s = (cmax-cmin)/(length(m)-1);
    if (s==0)
        cidx = ones(size(rx_rssi));
    else
        cidx = ceil((rx_rssi-cmin)/s+1);
    end
end

if nargin <= 5
    color = 'blue';
end

load('gps_calib_cordes.mat')

x = image_points(:,1);
y = image_points(:,2);
lat = gps_points_num(:,1);
lon = gps_points_num(:,2);

%plot(x,y,'o')
%keyboard;

% calculate the scale factor of the projection by averaging
pairs = nchoosek(1:length(x),2);
npairs = nchoosek(length(x),2);
for i = 1:npairs
    scalex(i) = (x(pairs(i,1)) - x(pairs(i,2)))/(lon(pairs(i,1))-lon(pairs(i,2)));
    scaley(i) = (y(pairs(i,1)) - y(pairs(i,2)))/(lat(pairs(i,1))-lat(pairs(i,2)));
end

% apply the correction factor, which was estimated manually by inspection
scaley = mean(scaley);
scalex = mean(scalex);

gps_y = (latitude-lat(1))*mean(scaley) + y(1);     % gps_x is the north-south direction
gps_x = (longitude-lon(1))*mean(scalex) + x(1);     % gps_y is the east-west direction

if ~isempty(mm)
    image(mm);  % plots the image itself
    hold on
end
    
if (nargin == 4 && ~isempty(rx_rssi))
    for i=1:length(gps_x)
        plot(gps_x(i),gps_y(i),'x','color',m(cidx(i),:))
        hold on
    end
    yt = round(linspace(min(rx_rssi),max(rx_rssi),8));
    hcb = colorbar;
    set(hcb,'YTickMode','manual');
    set(hcb,'YTick',8:8:length(m));
    set(hcb,'YTickLabel',yt);
else
    plot(gps_x,gps_y,'x','Color',color);
end
if nargin >= 5
    h = text(mean(gps_x),mean(gps_y),label);
    set(h,'Color',color,'FontWeight','bold');
end
% if ~isempty(mm)
%     hold off
% end
%if isempty(gps_x) || isempty(gps_y) || any(latitude==0) || any(longitude==0)
%    axis([3264    5819    2610    4556]);
%else
%    axis([min(gps_x)-100, max(gps_x)+100, min(gps_y)-100, max(gps_y)+100]);
%end
axis image
