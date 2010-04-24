d = dir([pathname 'data_term1*.EMOS']);
filenames = {d.name};

% NFrames = floor([d.bytes]/CHANNEL_BUFFER_SIZE)*NO_ESTIMATES_DISK;
% for i=1:length(filenames)
%     file_idx(i) = sscanf(d(i).name,'data_term1_idx%d_%s.EMOS',1);
% end

decimation = 100;
NFrames_max = 100*60*10;

timestamp_cat = [];
rx_N0_dBm_cat = [];
gps_lon_cat = [];
gps_lat_cat = [];
frame_tx_cat = [];
rx_N0_subband_dBm_cat = [];

%%
for file_idx = 16:length(filenames)
    disp(filenames{file_idx});
    
    [path,file,ext,ver] = fileparts(filenames{file_idx});

    if file(10)=='1'
        is_eNb=1;
    else
        is_eNb=0;
    end

    [H, H_fq, estimates, gps_data, NFrames] = load_estimates_lte(fullfile(pathname,filenames{file_idx}),NFrames_max,decimation,is_eNb);

%%
    rx_N0_dBm = zeros(1,NFrames/decimation);
    rx_N0_subband_dBm = zeros(NFrames/decimation,25);
    for i=1:NFrames/decimation
        rx_N0_dBm(i) = estimates(i).phy_measurements_eNb(1).n0_power_tot_dBm(1);
        rx_N0_subband_dBm(i,:) = estimates(i).phy_measurements_eNb(1).n0_subband_power_tot_dBm;
    end
    
    timestamp_cat = [timestamp_cat [estimates.timestamp]];
    frame_tx_cat = [frame_tx_cat [estimates.frame_tx]];
    rx_N0_dBm_cat = [rx_N0_dBm_cat rx_N0_dBm];
    rx_N0_subband_dBm_cat = [rx_N0_subband_dBm_cat; rx_N0_subband_dBm];
    gps_lon_cat = [gps_lon_cat [gps_data.longitude]];
    gps_lat_cat = [gps_lat_cat [gps_data.latitude]];
    
    save(fullfile(pathname,'results_eNB.mat'),'timestamp_cat','frame_tx_cat','rx_N0_dBm_cat','rx_N0_subband_dBm_cat','gps_lon_cat','gps_lat_cat','file_idx');

    %%
%     h_fig = figure(2);
%     hold off
%     plot(timestamp_cat,rx_N0_dBm_cat,'x')
%     title('RX I0 [dBm]')
%     xlabel('Frame number')
%     ylabel('RX I0 [dBm]')
%     saveas(h_fig,fullfile(pathname,'RX_I0_dBm.eps'),'epsc2')
% 
%     %%
%     h_fig = figure(3);
%     hold off
%     plot_gps_coordinates(mm,gps_lon_cat, gps_lat_cat,rx_N0_dBm_cat);
%     title('RX I0 [dBm]')
%     xlabel('Frame number')
%     ylabel('RX I0 [dBm]')
%     saveas(h_fig,fullfile(pathname,'RX_I0_dBm_gps.eps'),'epsc2')
end    



