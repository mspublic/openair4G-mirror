pathname = fullfile(root_path,dir_names{d_idx});
    
    fd = dir(fullfile(pathname, 'data_term1*_results_eNB.mat'));
    filenames = {fd.name};
    
    if (exist(fullfile(pathname,'results_cat_eNb.mat'),'file'))
    
        load(fullfile(pathname,'results_cat_eNb.mat'));
        start_idx = f_idx + 1;
    else
        
        lon_cat = [];
        lat_cat = [];
        start_idx = 1;
    end
    
    for f_idx=start_idx:length(filenames)
        fname = fullfile(pathname,filenames{f_idx});
        load(fname);
        
        nn = fieldnames(throughput);
        if (f_idx==1)
            for n = 1:length(nn)
                eval([nn{n} '_cat = [];']);
            end
        end   
        for n = 1:length(nn)
            eval([nn{n} '_cat = [' nn{n} '_cat throughput.' nn{n} '];']);
        end

        nn = fieldnames(SNR);
        if (f_idx==1)
            for n = 1:length(nn)
                eval([nn{n} '_cat = [];']);
            end
        end   
        for n = 1:length(nn)
            eval([nn{n} '_cat = [' nn{n} '_cat SNR.' nn{n} '];']);
        end

        lat_cat = cat(2,lat_cat,[gps_data.latitude]);
        
        lon_cat = cat(2,lon_cat,[gps_data.longitude]);

            
        save(fullfile(pathname,'results_cat_eNb.mat'), '*_cat', 'lon_cat', 'lat_cat', 'f_idx');
    end