% file: eval_nomadic_points.m
% author: florian.kaltenberger@eurecom.fr
% purpose: plots the nomadic results from the UE for several directories

% in the following two lines set the root path and select the subfolders
% you want to plot
% root_path = '~/EMOS/data/';  
% d = dir(fullfile(root_path, '*mode1_parcours1'));
% dir_names = {d.name};

decimation = 100;

results_UE = [];
results_UE_nomadic = [];

for i=1:length(dir_names)
    if exist(fullfile(root_path,dir_names{i},'results_UE_new.mat'),'file')
        nomadic = load(fullfile(root_path,dir_names{i},'results_UE_new.mat'));
        results_UE_nomadic = cat(2,results_UE_nomadic,nomadic.minestimates);
    end
end

%%
addpath('maps')
mm='cordes';

%%
h_fig = figure(1);
hold off
%plot_gps_coordinates(mm,[results_UE.gps_lon_cat], [results_UE.gps_lat_cat], double([results_UE.UE_mode_cat]));
%hold on
for i=1:length(results_UE_nomadic)
    if i==1 
        mmi = mm;
    else
        mmi = [];
    end
    plot_gps_coordinates(mmi,[results_UE_nomadic(i).gps_data.longitude], [results_UE_nomadic(i).gps_data.latitude], ...
        double(results_UE_nomadic(i).UE_mode_cat),sprintf('N%d',i),'black','Marker','.','Markersize',20);
end
%saveas(h_fig,'UE_mode_cordes.jpg','jpg');

%%
h_fig = figure(2);
hold off
%rx_rssi_dBm_cat = cat(1,results_UE.rx_rssi_dBm_cat);
%plot_gps_coordinates(mm,[results_UE.gps_lon_cat], [results_UE.gps_lat_cat], double(rx_rssi_dBm_cat(:,1)));
%hold on
for i=1:length(results_UE_nomadic)
    if i==1 
        mmi = mm;
    else
        mmi = [];
    end
    plot_gps_coordinates(mmi,[results_UE_nomadic(i).gps_data.longitude], [results_UE_nomadic(i).gps_data.latitude], ...
        double(results_UE_nomadic(i).rx_rssi_dBm(:,1)),sprintf('N%d',i),'black','Marker','.','Markersize',20);
end
%saveas(h_fig,'RX_RSSI_cordes.jpg','jpg');

%%
fid = fopen('nomadic_results.csv','w');
for i=1:length(results_UE_nomadic)
    rx_rssi_dBm_mean(i) = mean(results_UE_nomadic(i).rx_rssi_dBm(:,1));

    UE_synched = (results_UE_nomadic(i).UE_mode_cat>0);
    UE_connected = (results_UE_nomadic(i).UE_mode_cat==3);
    mimo_mode = (results_UE_nomadic(i).mimo_mode);
    good = (results_UE_nomadic(i).dlsch_fer<=100 & results_UE_nomadic(i).dlsch_fer>=0).';
    idx = 1;
    for m=[1,2,6]
        throughput_mean(i,idx) = mean((100-results_UE_nomadic(i).dlsch_fer(UE_connected & good & mimo_mode==m)).*...
            results_UE_nomadic(i).tbs_cat(UE_connected & good & mimo_mode==m)*6);
        fprintf(fid,'%d; %d; %s; %f; %f; %d; %f; %f\n',i, m, results_UE_nomadic(i).filename, rx_rssi_dBm_mean(i), throughput_mean(i,idx), ...
            sum(UE_connected & good & mimo_mode==m), ...
            mean([results_UE_nomadic(i).gps_data.latitude]),mean([results_UE_nomadic(i).gps_data.longitude]));
        idx=idx+1;
    end
end
fclose(fid);


%%
figure(3)
hold off
%plot([results_UE.gps_time_cat],[results_UE.UE_mode_cat],'x')
%hold on
gps_data_cat = [results_UE_nomadic.gps_data];
plot([gps_data_cat.timestamp],[results_UE_nomadic.UE_mode_cat],'rx');

%%
figure(4)
hold off
%plot([results_UE.gps_time_cat],rx_rssi_dBm_cat(:,1),'x');
%hold on
rx_rssi_dBm_cat_nomadic = cat(1,results_UE_nomadic.rx_rssi_dBm);
plot([gps_data_cat.timestamp],rx_rssi_dBm_cat_nomadic(:,1),'rx');

%%
figure(5)
hold off
% UE_connected = ([results_UE.UE_mode_cat]==3);
% dlsch_fer_cat = cat(1,results_UE.dlsch_fer_cat).';
% tbs_cat = cat(1,results_UE.tbs_cat).';
% good = (dlsch_fer_cat<=100 & dlsch_fer_cat>=0);
% timebase = [results_UE.gps_time_cat];
% plot(timebase(UE_connected & good),(100-dlsch_fer_cat(UE_connected & good)).*tbs_cat(UE_connected & good)*7*4/3,'x');
% hold on
UE_connected = ([results_UE_nomadic.UE_mode_cat]==3);
dlsch_fer_cat = cat(1,results_UE_nomadic.dlsch_fer).';
tbs_cat = cat(1,results_UE_nomadic.tbs_cat).';
good = (dlsch_fer_cat<=100 & dlsch_fer_cat>=0);
gps_data = [results_UE_nomadic.gps_data];
timebase = [gps_data.timestamp];
plot(timebase(UE_connected & good),(100-dlsch_fer_cat(UE_connected & good)).*tbs_cat(UE_connected & good)*6,'rx');

