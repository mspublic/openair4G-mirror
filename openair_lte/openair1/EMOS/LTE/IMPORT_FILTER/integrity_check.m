%%function integrity_check()
cancel = 0;

%% load file
[filename, pathname] = uigetfile('*.EMOS', 'Pick a file');

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

decimation = 1;
NFrames_max = 100*60*10;

[H, H_fq, estimates, gps_data, NFrames] = load_estimates_lte(fullfile(pathname, filename),NFrames_max,decimation,is_eNb);

save('estimates.mat')

%%
figure(1)
plot([estimates.frame_tx])
title('Tx Frame number')

rx_rssi_dBm = zeros(1,NFrames/decimation);
for i=1:NFrames/decimation
    rx_rssi_dBm(i) = estimates(i).phy_measurements(1).rx_rssi_dBm(1);
end
figure(2)
plot(rx_rssi_dBm)
title('RX_rssi_dBm')

figure(3)
plot([gps_data.longitude], [gps_data.latitude],'x');
end