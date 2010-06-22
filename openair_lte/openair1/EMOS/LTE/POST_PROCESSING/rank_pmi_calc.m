load 'estimates1.mat'
c = [];
pmi1 = [];
pmi2 = [];
lat = [];
lon = [];
%%
for b = 1:1:length(estimates)
    
    lat = [lat gps_data(b).latitude];
    lon = [lon gps_data(b).longitude];
    
    %q = double(estimates(b).phy_measurements(1).subband_pmi_re((antenna_index(i)+1),i,1))+1j*double(estimates(b).phy_measurements(1).subband_pmi_im((antenna_index(i)+1),i,1));
    q1 = double(estimates(b).phy_measurements(1).subband_pmi_re(1,:,1))+1j*double(estimates(b).phy_measurements(1).subband_pmi_im(1,:,1));
    q2 = double(estimates(b).phy_measurements(1).subband_pmi_re(2,:,1))+1j*double(estimates(b).phy_measurements(1).subband_pmi_im(2,:,1));
    
    for i = 1:1:7
        
        qq1(i) = quantize_q(q1(i));
        qq1(i) = map_q(qq1(i));
        qq2(i) = quantize_q(q2(i));
        qq2(i) = map_q(qq2(i));
        
        if ((qq1(i)==0 && qq2(i)==2) || (qq1(i)==2 && qq2(i)==0) || (qq1(i)==3 && qq2(i)==1) || (qq1(i)==1 && qq2(i)==3))
            ran(i) = 1;
        else
            ran(i) = 0;
        end
        
        
    end
    if sum(ran==1)>=4
   
    rank_Ind(b) = 1;
    else 
        rank_Ind(b) = 0;
    end
    
    pmi1 = [pmi1 qq1];
    pmi2 = [pmi2 qq2];
end

%%
mm=imread('maps/cordes.png');

pmi1 = reshape(pmi1,7,[]);
figure
title('PMI');
plot_gps_coordinates(mm, lon,lat,pmi1(1,:));

%%
[I] = find(c);

figure
title('Rank');
cc = c(1:3:end);
cc = cc(1:20:end);

[II] = find(c);

plot_gps_coordinates(mm, lon,lat,cc);


