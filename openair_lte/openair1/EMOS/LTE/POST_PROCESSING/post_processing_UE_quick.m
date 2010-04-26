pathname = 'G:\EMOS\data\20100421 interference eNb + DL test\';
d = dir([pathname 'data_term3*.EMOS']);
filenames = {d.name};

% NFrames = floor([d.bytes]/CHANNEL_BUFFER_SIZE)*NO_ESTIMATES_DISK;
% for i=1:length(filenames)
%     file_idx(i) = sscanf(d(i).name,'data_term3_idx%d_%s.EMOS',1);
% end

decimation = 100;
NFrames_max = 100*60*10;

if (exist(fullfile(pathname,'results_eNB.mat'),'file'))
    load(fullfile(pathname,'results_eNB.mat'));
    start_idx = file_idx;
else
    timestamp_cat = [];
    rx_rssi_dBm_cat = [];
    gps_lon_cat = [];
    gps_lat_cat = [];
    frame_tx_cat = [];
    pbch_fer_cat = [];
    dlsch_fer_cat = [];
    mcs_cat = [];
    start_idx = 1;
end

for file_idx = start_idx:length(filenames)
    disp(filenames{file_idx});
    
    [path,file,ext,ver] = fileparts(filenames{file_idx});

    if file(10)=='1'
        is_eNb=1;
    else
        is_eNb=0;
    end

    [H, H_fq, estimates, gps_data, NFrames] = load_estimates_lte(fullfile(pathname,filenames{file_idx}),NFrames_max,decimation,is_eNb);

%%
    rx_rssi_dBm = zeros(NFrames/decimation,3);
    pbch_fer = zeros(NFrames/decimation,1);
    dlsch_fer = zeros(NFrames/decimation,1);
    mcs = zeros(NFrames/decimation,1);
    for i=1:NFrames/decimation
        rx_rssi_dBm(i,:) = estimates(i).phy_measurements(1).rx_rssi_dBm(:);
        pbch_fer(i) = estimates(i).pbch_fer(1);
        dlsch_fer(i) = estimates(i).dlsch_fer(1);
        mcs(i) = get_mcs(estimates(i).dci_alloc(1,6).dci_pdu);
    end
    
    timestamp_cat = [timestamp_cat [estimates.timestamp]];
    frame_tx_cat = [frame_tx_cat [estimates.frame_tx]];
    rx_rssi_dBm_cat = [rx_rssi_dBm_cat; rx_rssi_dBm];
    pbch_fer_cat = [pbch_fer_cat; pbch_fer];
    dlsch_fer_cat = [dlsch_fer_cat; dlsch_fer];
    mcs_cat = [mcs_cat mcs];
    gps_lon_cat = [gps_lon_cat [gps_data.longitude]];
    gps_lat_cat = [gps_lat_cat [gps_data.latitude]];
    
    save(fullfile(pathname,'results_UE.mat'),'timestamp_cat','frame_tx_cat','rx_rssi_dBm_cat','pbch_fer_cat','dlsch_fer_cat','mcs_cat','gps_lon_cat','gps_lat_cat','file_idx');

%     h_fig = figure(2);
%     hold off
%     plot(frame_tx_cat,rx_rssi_dBm_cat)
%     title('RX RSSI [dBm]')
%     xlabel('Frame number')
%     ylabel('RX RSSI [dBm]')
%     saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm.eps'),'epsc2')
% 
%     h_fig = figure(3);
%     hold off
%     plot_gps_coordinates([],gps_lon_cat, gps_lat_cat,rx_rssi_dBm_cat(:,1));
%     title('RX I0 [dBm]')
%     xlabel('Frame number')
%     ylabel('RX I0 [dBm]')
%     saveas(h_fig,fullfile(pathname,'RX_RSSI_dBm_gps.eps'),'epsc2')
end    

