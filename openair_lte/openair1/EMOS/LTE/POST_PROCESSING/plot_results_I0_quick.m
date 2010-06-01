close all
clear all

%% load file
%pathname = '/media/Iomega HDD/EMOS/data/20100420 interference measurement/';
%pathname = 'G:\EMOS\data\20100420 interference measurement\';
pathname = 'G:\EMOS\data\20100422 interference drive test\';
d = dir([pathname 'data_term1*.EMOS']);

NFrames = floor([d.bytes]/CHANNEL_BUFFER_SIZE)*NO_ESTIMATES_DISK;
for i=1:length(filenames)
    file_idx(i) = sscanf(d(i).name,'data_term1_idx%d_%s.EMOS',1);
end

load(fullfile(pathname,'results.mat'));
mm=imread('maps/cordes.png');

%%
h_fig = figure(2);
hold off
plot(frame_tx_cat,rx_N0_dBm_cat,'x')
title('RX I0 [dBm]')
xlabel('Frame number')
ylabel('RX I0 [dBm]')
%saveas(h_fig,fullfile(pathname,'RX_I0_dBm.eps'),'epsc2')

%%
h_fig = figure(3);
hold off
plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,rx_N0_dBm_cat);
title('RX I0 [dBm]')
xlabel('Frame number')
ylabel('RX I0 [dBm]')
%saveas(h_fig,fullfile(pathname,'RX_I0_dBm_gps.eps'),'epsc2')



