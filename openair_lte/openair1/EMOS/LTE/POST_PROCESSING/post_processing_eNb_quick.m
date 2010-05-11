d = dir([pathname 'data_term1*.EMOS']);
filenames = {d.name};
filedates = {d.date};
[filedates,idx] = sort(filedates);
filenames = filenames(idx);

% NFrames = floor([d.bytes]/CHANNEL_BUFFER_SIZE)*NO_ESTIMATES_DISK;
% for i=1:length(filenames)
%     file_idx(i) = sscanf(d(i).name,'data_term1_idx%d_%s.EMOS',1);
% end

decimation = 100;
NFrames_max = 100*60*10;

if (exist(fullfile(pathname,'results_eNB.mat'),'file'))
    load(fullfile(pathname,'results_eNB.mat'));
    start_idx = file_idx + 1
else
    timestamp_cat = [];
    phy_measurements_cat = struct([]);
    eNb_UE_stats_cat = struct([]);
    rx_N0_dBm_cat = [];
    %rx_N0_subband_dBm_cat = [];
    gps_lon_cat = [];
    gps_lat_cat = [];
    frame_tx_cat = [];
    ulsch_errors_cat = [];
    NFrames = zeros(1,length(filenames));
    start_time = zeros(1,length(filenames));
    start_idx = 1;
end

%%
for file_idx = start_idx:length(filenames)
    disp(filenames{file_idx});
    
    [path,file,ext,ver] = fileparts(filenames{file_idx});

    if file(10)=='1'
        is_eNb=1;
    else
        is_eNb=0;
    end

    [H, H_fq, estimates, gps_data, NFrames(file_idx)] = load_estimates_lte(fullfile(pathname,filenames{file_idx}),NFrames_max,decimation,is_eNb);
    start_time(file_idx) = datenum(file(18:32),'yyyymmddTHHMMSS');

%%
    phy_measurements = repmat(phy_measurements_eNb_struct,NFrames(file_idx)/decimation,3);
    eNb_UE_stats = repmat(eNb_UE_stats_struct,1,NFrames(file_idx)/decimation);
    for i=1:NFrames(file_idx)/decimation
        phy_measurements(i,:) = estimates(i).phy_measurements_eNb;
        eNb_UE_stats(i) = estimates(i).eNb_UE_stats(3,1);
    end
    
    rx_N0_dBm = zeros(NFrames(file_idx)/decimation,3);
    %rx_N0_subband_dBm = zeros(NFrames(file_idx)/decimation,3,25);
    for i=1:NFrames(file_idx)/decimation
        for j=1:3
           rx_N0_dBm(i,j) = estimates(i).phy_measurements_eNb(j).n0_power_tot_dBm;
           %rx_N0_subband_dBm(i,j,:) = estimates(i).phy_measurements_eNb(j).n0_subband_power_tot_dBm;
        end
    end
    
    phy_measurements_cat = [phy_measurements_cat; phy_measurements];
    eNb_UE_stats_cat = [eNb_UE_stats_cat eNb_UE_stats];
    timestamp_cat = [timestamp_cat [estimates.timestamp]];
    frame_tx_cat = [frame_tx_cat [estimates.frame_tx]];
    ulsch_errors_cat = [ulsch_errors_cat [estimates.ulsch_errors]];
    rx_N0_dBm_cat = [rx_N0_dBm_cat; rx_N0_dBm];
    %rx_N0_subband_dBm_cat = cat(1,rx_N0_subband_dBm_cat,rx_N0_subband_dBm);
    gps_lon_cat = [gps_lon_cat [gps_data.longitude]];
    gps_lat_cat = [gps_lat_cat [gps_data.latitude]];
    
    save(fullfile(pathname,'results_eNB.mat'),'phy_measurements_cat',...
        'eNb_UE_stats_cat','timestamp_cat','frame_tx_cat','ulsch_errors_cat',...
        'ulsch_errors_cat','rx_N0_dBm_cat','gps_lon_cat','gps_lat_cat',...
        'file_idx','NFrames','start_time','filenames','filedates');

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



