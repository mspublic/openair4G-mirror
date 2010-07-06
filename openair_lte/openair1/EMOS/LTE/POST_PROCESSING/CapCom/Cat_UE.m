pathname = fullfile(root_path,dir_names{d_idx});
    
    fd = dir(fullfile(pathname, 'data_term3*_results_UE.mat'));
    filenames = {fd.name};
    
    if (exist(fullfile(pathname,'results_cat_UE.mat'),'file'))
    
        load(fullfile(pathname,'results_cat_UE.mat'));
        if (exist('K_fac_cat','var'))
            start_idx = f_idx + 1;
        else
            lon_cat = [];
        lat_cat = [];
        UE_mode_cat = [];
        K_fac_cat = [];
        start_idx = 1;
        end
            
    else
        
        lon_cat = [];
        lat_cat = [];
        UE_mode_cat = [];
        K_fac_cat = [];
        start_idx = 1;
    end
    
    for f_idx=start_idx:length(filenames)
        fname = fullfile(pathname,filenames{f_idx});
        if (~exist(regexprep(fname, '_results_UE.mat', '_K_factor.mat'),'file'))
            continue;
        end
        
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

        UE_mode_cat = cat(2,UE_mode_cat,[minestimates.UE_mode]);
        
        load(regexprep(fname, '_results_UE.mat', '_K_factor.mat'));
        K_fac_cat = [K_fac_cat K_fac];

    
        save(fullfile(pathname,'results_cat_UE.mat'), '*_cat', 'UE_mode_cat', 'lon_cat', 'lat_cat', 'f_idx', 'K_fac_cat');
    end