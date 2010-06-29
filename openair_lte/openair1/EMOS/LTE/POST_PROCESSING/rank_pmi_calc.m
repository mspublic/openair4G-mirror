pmi1 = [];
pmi2 = [];
lat = [];
lon = [];
in = 0;
count = 0;

%%
for b = 1:100:length(minestimates_cat)
    
    %if (minestimates_cat(b).UE_mode == 3)
        count = count+1;
        q1 = double(minestimates_cat(b).phy_measurements.subband_pmi_re(1,:,1))+1j*double(minestimates_cat(b).phy_measurements.subband_pmi_im(1,:,1));
        q2 = double(minestimates_cat(b).phy_measurements.subband_pmi_re(2,:,1))+1j*double(minestimates_cat(b).phy_measurements.subband_pmi_im(2,:,1));
        
        for i = 1:1:7
            
            qq1(i) = quantize_q(q1(i));
            qq1(i) = map_q(qq1(i));
            qq2(i) = quantize_q(q2(i));
            qq2(i) = map_q(qq2(i));
            
            if ((qq1(i)==0 && qq2(i)==3) || (qq1(i)==3 && qq2(i)==0) || (qq1(i)==2 && qq2(i)==1) || (qq1(i)==1 && qq2(i)==2))
                ran(i) = 1;
            else
                ran(i) = 0;
            end
            
            
        end
        
       if sum(ran==1)>=4
            
            rank_Ind(count) = 1;
        else
            rank_Ind(count) = 0;
        end
        
        pmi1 = [pmi1 qq1];
        pmi2 = [pmi2 qq2];
    %end
    
end

%%

%for b = 1:1:length(gps_data_cat)
    lat = [gps_data_cat.latitude];
    lon = [gps_data_cat.longitude];
%end
pmi1 = reshape(pmi1,7,[]);
pmi2 = reshape(pmi2,7,[]);
%mm=imread('maps/cordes.png');

UE_synched = ([minestimates_cat(1:100:end).UE_mode] > 0);

%%
pathname = '/media/Expansion_Drive/New Folder/20100510_mode2_parcours1_part2/';
%%
in = in + 1;
h_fig = figure(in);
hold off
plot_gps_coordinates(mm,lon(UE_synched),lat(UE_synched),pmi1(1,UE_synched));
title('PMI')
saveas(h_fig,fullfile(pathname,'PMI_1stRX_subband1.jpg'),'jpg')

in = in + 1;
h_fig = figure(in);
hold off
plot_gps_coordinates(mm, lon(UE_synched),lat(UE_synched),pmi1(2,UE_synched));
title('PMI')
saveas(h_fig,fullfile(pathname,'PMI_1stRX_subband2.jpg'),'jpg')

in = in + 1;
h_fig = figure(in);
hold off
plot_gps_coordinates(mm, lon(UE_synched),lat(UE_synched),pmi2(1,UE_synched));
title('PMI')
saveas(h_fig,fullfile(pathname,'PMI_2ndRX_subband1.jpg'),'jpg')

in = in + 1;
h_fig = figure(in);
hold off
plot_gps_coordinates(mm, lon(UE_synched),lat(UE_synched),pmi2(2,UE_synched));
title('PMI')
saveas(h_fig,fullfile(pathname,'PMI_2ndRX_subband2.jpg'),'jpg')

%close all

%%
% [I] = find(c);
%
% figure
% title('Rank');
% cc = c(1:3:end);
% cc = cc(1:20:end);
%
% [II] = find(c);
%
% plot_gps_coordinates(mm, lon,lat,cc);


