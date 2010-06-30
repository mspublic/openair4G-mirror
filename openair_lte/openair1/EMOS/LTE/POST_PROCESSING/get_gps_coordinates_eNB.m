estimates_UE = load(fullfile(pathname,'results_UE.mat'));
estimates_eNB = load(fullfile(pathname,'results_eNB.mat'));

for i=1:length(estimates_eNB.frame_tx_cat)
    idx = find(estimates_UE.frame_tx_cat==estimates_eNB.frame_tx_cat(i));
    
    if ~isempty(idx)
        if length(idx)>1
            warning('More than one GPS coordinate matches');
        end
        estimates_eNB.gps_lat_cat(i) = estimates_UE.gps_lat_cat(idx(1)); 
        estimates_eNB.gps_lon_cat(i) = estimates_UE.gps_lon_cat(idx(1)); 
        estimates_eNB.gps_time_cat(i) = estimates_UE.gps_time_cat(idx(1)); 
        estimates_eNB.gps_speed_cat(i) = estimates_UE.gps_speed_cat(idx(1)); 
    else
        estimates_eNB.gps_lat_cat(i) = nan;
        estimates_eNB.gps_lon_cat(i) = nan;
        estimates_eNB.gps_time_cat(i) = nan;
        estimates_eNB.gps_speed_cat(i) = nan;
    end
        
end

save(fullfile(pathname,'results_eNB.mat'), '-struct', 'estimates_eNB');
