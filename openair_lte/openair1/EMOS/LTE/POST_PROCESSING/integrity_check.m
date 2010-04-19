addpath('../IMPORT_FILTER')

%% load file
[filename, pathname] = uigetfile('*.EMOS', 'Pick a file');

cancel = 0;
if isequal(filename,0) || isequal(pathname,0)
    disp('canceled')
    %exit
    cancel = 1;
end
if cancel == 0
if filename(10)=='1'
    is_eNb=1;
else
    is_eNb=0;
end

decimation = 10;
NFrames_max = 100*60*10;

[H, H_fq, estimates, gps_data, NFrames] = load_estimates_lte(fullfile(pathname, filename),NFrames_max,decimation,is_eNb);

save('estimates.mat')

%%
h_fig = figure(1);
plot([estimates.frame_tx],[estimates.frame_rx])
title('Frame number')
ylabel('Received frame number');
xlabel('Transmitted frame number');
saveas(h_fig,'frame_tx.eps','epsc2')

rx_rssi_dBm = zeros(1,NFrames/decimation);
for i=1:NFrames/decimation
    rx_rssi_dBm(i) = estimates(i).phy_measurements(1).rx_rssi_dBm(1);
end
h_fig = figure(2);
plot([estimates.frame_tx],rx_rssi_dBm)
title('RX RSSI [dBm]')
xlabel('Frame number')
ylabel('RX RSSI [dBm]')
saveas(h_fig,'RX_rssi_dBm.eps','epsc2')

h_fig = figure(3);
plot_gps_coordinates([],[gps_data.longitude], [gps_data.latitude],rx_rssi_dBm);
saveas(h_fig,'gps_trace.eps','epsc2')

pbch_fer = zeros(1,NFrames/decimation,1);
for i=1:NFrames/decimation
    pbch_fer(i,:) = estimates(i).pbch_fer(1);
end
h_fig = figure(4);
plot([estimates.frame_tx], pbch_fer)
title('PBCH FER')
xlabel('Transmitted frame number');
ylabel('PBCH FER')
saveas(h_fig,'pbch_fer.eps','epsc2')

end