%pathname = '/extras/kaltenbe/CNES/emos_postprocessed_data/20100510_mode2_parcours1_part1/';

d = dir(fullfile(pathname, 'data_term3*.mat'));
filenames = {d.name};

gps_data_cat = [];

%minestimates_cat = [];
UE_mode_cat = [];

K_fac_cat = [];

%%
for f = 1:length(filenames)
    s = fullfile(pathname, filenames{f});
    load(s);
    
    nn = fieldnames(throughput);
    if (f==1)
        for n = 1:length(nn)
            eval([nn{n} '_cat = [];']);
        end
    end   
    for n = 1:length(nn)
        eval([nn{n} '_cat = [' nn{n} '_cat throughput.' nn{n} '];']);
    end

    nn = fieldnames(SNR);
    if (f==1)
        for n = 1:length(nn)
            eval([nn{n} '_cat = [];']);
        end
    end   
    for n = 1:length(nn)
        eval([nn{n} '_cat = [' nn{n} '_cat SNR.' nn{n} '];']);
    end

    gps_data_cat = cat(2,gps_data_cat,gps_data);
    
    %minestimates_cat = cat(2,minestimates_cat,minestimates);  
    
    UE_mode_cat = cat(2,UE_mode_cat,[minestimates.UE_mode]);
    
    K_fac_cat = cat(2,K_fac_cat,K_fac);
    
end

% save 'results_UE.mat'
% 
% mkdir('/homes/latif/devel/openair_lte/openair1/EMOS/LTE/POST_PROCESSING/SIM_Results', s);
% 
% in = 0;
% plotwrtdistanceCalc;
% in = in+1;    
% h_fig = figure(in);
% title([s '__SISO']);
% xlabel('Time[Seconds]');
% ylabel('Throughput');
% hold on;
% plot(throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(throughput.rateps_SISO_16Qam_eNB1,'g-o');
% plot(throughput.rateps_SISO_64Qam_eNB1,'r-o');
% hold off;
% 
% in = in+1;
% h_fig = figure(in);
% hold on;
% title('Throughput SISO _ distance from BS ');
% xlabel('Distance from BS [Km]');
% ylabel('Throughput [Bits/sec]');
% %dist(1:end);
% plot(dist, throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(dist, throughput.rateps_SISO_16Qam_eNB1,'g-o');
% plot(dist, throughput.rateps_SISO_64Qam_eNB1,'r-o');
% hold off;
%  
% in = in+1;
% h_fig = figure(in);
% plot(dist_travelled, throughput.rateps_SISO_4Qam_eNB1,'b-o');
% plot(dist_travelled, throughput.rateps_SISO_16Qam_eNB1,'g-o' );
% plot(dist_travelled, throughput.rateps_SISO_64Qam_eNB1,'r-o');
% title('Throughput SISO _ distance Travelled ')
% xlabel('Distance travelled [Km]')
% ylabel('Throughput [Bits/sec]')
% 
% end
