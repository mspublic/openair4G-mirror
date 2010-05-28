% addpath('../IMPORT_FILTER')
% 
% %% load file
% [filename, pathname] = uigetfile('*.EMOS', 'Pick a file');
% 
% cancel = 0;
% if isequal(filename,0) || isequal(pathname,0)
%     disp('canceled')
%     %exit
%     cancel = 1;
% end
% if cancel == 0
% if filename(10)=='1'
%     is_eNb=1;
% else
%     is_eNb=0;
% end
% 
% what_scheme = 1; % 1 -> SISO, 2 -> Alamouti, 3 -> Beamforming
%clear all;
clc;
pathname = '/media/Expansion_Drive/New Folder/20100512_mode2_parcours2_part7/';
%pathname = '/me/media/Expansion_Drive/New Folder/20100512_mode2_parcours2_part7/dia/Iomega_HDD/EMOS/data/20100506 coverage run part 1/';
%pathname = '/extras/kaltenbe/EMOS/lte_cnes_data/20100511_mode2_parcours2_part1/';
       
d = dir([pathname 'data_term3*.EMOS']);
filenames = {d.name};
%filename = '/extras/kaltenbe/EMOS/lte_cnes_data/20100511_mode2_parcours2_part2/data_term3_idx00_20100512T104109.EMOS';
decimation = 1;
NFrames_max = 100*60*10;

for file_idx = 1:length(filenames)
   % n = round(length(filenames)/2);
    disp(filenames{file_idx});
    decimation
    [path,file,ext,ver] = fileparts(filenames{file_idx});

    if file(10)=='1'
        is_eNb=1;
    else
        is_eNb=0;
    end
    
%[H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = load_estimates_lte_1(filename,NFrames_max,decimation,is_eNb);

[H, H_fq, gps_data, NFrames, minestimates, throughput, SNR] = load_estimates_lte_1(fullfile(pathname,filenames{file_idx}),NFrames_max,decimation,is_eNb);

save(regexprep((fullfile(filenames{file_idx},'results_UE.mat')), '/', '_'), 'gps_data', 'NFrames', 'SNR', 'throughput', 'minestimates', 'file_idx');

end
    

% 
% %%
% h_fig = figure(1);
% plot([estimates.frame_tx],[estimates.frame_rx])
% title('Frame number')
% ylabel('Received frame number');
% xlabel('Transmitted frame number');
% saveas(h_fig,'frame_tx.eps','epsc2')
% 
% rx_rssi_dBm = zeros(1,NFrames/decimation);
% for i=1:NFrames/decimation
%     rx_rssi_dBm(i) = estimates(i).phy_measurements(1).rx_rssi_dBm(1);
% end
% h_fig = figure(2);
% plot([estimates.frame_tx],rx_rssi_dBm)
% title('RX RSSI [dBm]')
% xlabel('Frame number')
% ylabel('RX RSSI [dBm]')
% saveas(h_fig,'RX_rssi_dBm.eps','epsc2')
% 
% h_fig = figure(3);
% plot_gps_coordinates([],[gps_data.longitude], [gps_data.latitude],rx_rssi_dBm);
% saveas(h_fig,'gps_trace.eps','epsc2')
% 
% pbch_fer = zeros(1,NFrames/decimation,1);
% for i=1:NFrames/decimation
%     pbch_fer(i,:) = estimates(i).pbch_fer(1);
% end
% h_fig = figure(4);
% plot([estimates.frame_tx], pbch_fer)
% title('PBCH FER')
% xlabel('Transmitted frame number');
% ylabel('PBCH FER')
% saveas(h_fig,'pbch_fer.eps','epsc2')
% 
% end