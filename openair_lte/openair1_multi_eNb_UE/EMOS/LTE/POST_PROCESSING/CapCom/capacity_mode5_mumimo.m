load 'table_LTE_MODE_4_5.mat';
load 'SISO.mat';

[x, y, z] = size(estimates_UE_1(1).channel);

estimates_UE1 = estimates_UE_1(1,:);
estimates_UE2 = estimates_UE_1(2,:);

chcap_mumimo = zeros(1,1200, length(estimates_UE1));

subband_indices = 1:48:288;
subband_indices(7) = 289;
subband_indices(8) = 301;
for est=1:length(estimates_UE1)
    
    N0_1Rx = double(estimates_UE1(est).phy_measurements(11).n0_power(1));
    N0_2ndRx = double(estimates_UE2(est).phy_measurements(11).n0_power(1));
    
    H = estimates_UE1(est).channel;
    Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
    Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
    [val, ind1] = max(Hs);
    antenna_index1 = estimates_UE1(est).phy_measurements(11).selected_rx_antennas(:,1);
    
    H = estimates_UE2(est).channel;
    Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
    Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
    [val, ind2] = max(Hs);
    antenna_index2 = estimates_UE2(est).phy_measurements(11).selected_rx_antennas(:,1);
    
    %h11_eNB1 = double([estimates_UE1(est).channel(z+1:end,1,ind1).real]) + 1j*double([estimates_UE1(est).channel(z+1:end,1,ind1).imag]);
    channel_tmp = typecast(estimates_UE1(est).channel(201:end,1,ind1),'int16');
    h11_eNB1 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
   %% h11_eNB1 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
   
    h11_eNB1_srs1 = interp1(1:6:300,h11_eNB1(1:50),1:1:300,'linear','extrap');
    h11_eNB1_srs2 = interp1(4:6:300,h11_eNB1(51:100),1:1:300,'linear','extrap');
    h11_eNB1_srs3 = interp1(1:6:300,h11_eNB1(101:150),1:1:300,'linear','extrap');
    h11_eNB1_srs4 = interp1(4:6:300,h11_eNB1(151:200),1:1:300,'linear','extrap');
    
    h11_eNB1_comp = [h11_eNB1_srs1' h11_eNB1_srs2' h11_eNB1_srs3' h11_eNB1_srs4']';
    
    channel_tmp = typecast(estimates_UE1(est).channel(201:end,2,ind1),'int16');
    
     h12_eNB1 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
    %%h12_eNB1 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
    
    h12_eNB1_srs1 = interp1(1:6:300,h12_eNB1(1:50),1:1:300,'linear','extrap');
    h12_eNB1_srs2 = interp1(4:6:300,h12_eNB1(51:100),1:1:300,'linear','extrap');
    h12_eNB1_srs3 = interp1(1:6:300,h12_eNB1(101:150),1:1:300,'linear','extrap');
    h12_eNB1_srs4 = interp1(4:6:300,h12_eNB1(151:200),1:1:300,'linear','extrap');
    
    h12_eNB1_comp = [h12_eNB1_srs1' h12_eNB1_srs2' h12_eNB1_srs3' h12_eNB1_srs4']';
    
    channel_tmp = typecast(estimates_UE1(est).channel(201:end,3,ind1),'int16');
     h21_eNB1 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
    %%h21_eNB1 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
   
    h21_eNB1_srs1 = interp1(4:6:300,h21_eNB1(1:50),1:1:300,'linear','extrap');
    h21_eNB1_srs2 = interp1(1:6:300,h21_eNB1(51:100),1:1:300,'linear','extrap');
    h21_eNB1_srs3 = interp1(4:6:300,h21_eNB1(101:150),1:1:300,'linear','extrap');
    h21_eNB1_srs4 = interp1(1:6:300,h21_eNB1(151:200),1:1:300,'linear','extrap');
    
    h21_eNB1_comp = [h21_eNB1_srs1' h21_eNB1_srs2' h21_eNB1_srs3' h21_eNB1_srs4']';
    
    channel_tmp = typecast(estimates_UE1(est).channel(201:end,4,ind1),'int16');
     h22_eNB1 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
    %%h22_eNB1 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));

    h22_eNB1_srs1 = interp1(4:6:300,h22_eNB1(1:50),1:1:300,'linear','extrap');
    h22_eNB1_srs2 = interp1(1:6:300,h22_eNB1(51:100),1:1:300,'linear','extrap');
    h22_eNB1_srs3 = interp1(4:6:300,h22_eNB1(101:150),1:1:300,'linear','extrap');
    h22_eNB1_srs4 = interp1(1:6:300,h22_eNB1(151:200),1:1:300,'linear','extrap');
    
    h22_eNB1_comp = [h22_eNB1_srs1' h22_eNB1_srs2' h22_eNB1_srs3' h22_eNB1_srs4']';
    
        
    channel_tmp = typecast(estimates_UE2(est).channel(201:end,1,ind2),'int16');
    
     h11_eNB2 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
    %%h11_eNB2 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
    
    h11_eNB2_srs1 = interp1(1:6:300,h11_eNB2(1:50),1:1:300,'linear','extrap');
    h11_eNB2_srs2 = interp1(4:6:300,h11_eNB2(51:100),1:1:300,'linear','extrap');
    h11_eNB2_srs3 = interp1(1:6:300,h11_eNB2(101:150),1:1:300,'linear','extrap');
    h11_eNB2_srs4 = interp1(4:6:300,h11_eNB2(151:200),1:1:300,'linear','extrap');
      
    h11_eNB2_comp = [h11_eNB2_srs1' h11_eNB2_srs2' h11_eNB2_srs3' h11_eNB2_srs4']';
    
    channel_tmp = typecast(estimates_UE2(est).channel(201:end,2,ind2),'int16');
     h12_eNB2 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
   %% h12_eNB2 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
    
    h12_eNB2_srs1 = interp1(1:6:300,h12_eNB2(1:50),1:1:300,'linear','extrap');
    h12_eNB2_srs2 = interp1(4:6:300,h12_eNB2(51:100),1:1:300,'linear','extrap');
    h12_eNB2_srs3 = interp1(1:6:300,h12_eNB2(101:150),1:1:300,'linear','extrap');
    h12_eNB2_srs4 = interp1(4:6:300,h12_eNB2(151:200),1:1:300,'linear','extrap');
    
    h12_eNB2_comp = [h12_eNB2_srs1' h12_eNB2_srs2' h12_eNB2_srs3' h12_eNB2_srs4']';
    
    channel_tmp = typecast(estimates_UE2(est).channel(201:end,3,ind2),'int16');
    h21_eNB2 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
   %% h21_eNB2 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
    h21_eNB2_srs1 = interp1(4:6:300,h21_eNB2(1:50),1:1:300,'linear','extrap');
    h21_eNB2_srs2 = interp1(1:6:300,h21_eNB2(51:100),1:1:300,'linear','extrap');
    h21_eNB2_srs3 = interp1(4:6:300,h21_eNB2(101:150),1:1:300,'linear','extrap');
    h21_eNB2_srs4 = interp1(1:6:300,h21_eNB2(151:200),1:1:300,'linear','extrap');
    
    h21_eNB2_comp = [h21_eNB2_srs1' h21_eNB2_srs2' h21_eNB2_srs3' h21_eNB2_srs4']';
    
    
    channel_tmp = typecast(estimates_UE2(est).channel(201:end,4,ind2),'int16');
    h22_eNB2 = double(channel_tmp(1:2:end,1)) + 1j*double(channel_tmp(2:2:end,1));
   %% h22_eNB2 = double(channel_tmp(2:2:end,1)) + 1j*double(channel_tmp(1:2:end,1));
    
    h22_eNB2_srs1 = interp1(4:6:300,h22_eNB2(1:50),1:1:300,'linear','extrap');
    h22_eNB2_srs2 = interp1(1:6:300,h22_eNB2(51:100),1:1:300,'linear','extrap');
    h22_eNB2_srs3 = interp1(4:6:300,h22_eNB2(101:150),1:1:300,'linear','extrap');
    h22_eNB2_srs4 = interp1(1:6:300,h22_eNB2(151:200),1:1:300,'linear','extrap');
    
    h22_eNB2_comp = [h22_eNB2_srs1' h22_eNB2_srs2' h22_eNB2_srs3' h22_eNB2_srs4']';
    
    
   
%    plot(10*log10(abs(h12_eNB2).^2),'go');
%    pause
%end
%if 0
    Rx1_ue1 = [];
    Rx1_ue2 = [];
    Rx_ue1 = [];
    Rx_ue2 = [];
    Ry1_ue1 = [];
    Ry1_ue2 = [];
        
    for i = 1:7
        if (antenna_index1(i) == 0)
        h11 = h11_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        h21 = h21_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        q1(i) = mean(conj(h11(:)).*h21(:));
        pmi_ue1(i) = quantize_q(q1(i));
        else
        h12 = h12_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        h22 = h22_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        q1(i) = mean(conj(h12(:)).*h22(:));
        pmi_ue1(i) = quantize_q(q1(i));
        end
        
        if (antenna_index2(i) == 0)
        h11 = h11_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        h21 = h21_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        q2(i) = mean(conj(h11(:)).*h21(:));
        pmi_ue2(i) = quantize_q(q2(i));
        else
        h12 = h12_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        h22 = h22_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1));
        q2(i) = mean(conj(h12(:)).*h22(:));
        pmi_ue2(i) = quantize_q(q2(i));
        end
        
    
        % q1(i) = double(estimates_UE1(est).phy_measurements(11).subband_pmi_re((antenna_index1(i)+1),i,1))+1j*double(estimates_UE1(est).phy_measurements(11).subband_pmi_im((antenna_index1(i)+1),i,1));
       % q2(i) = double(estimates_UE2(est).phy_measurements(11).subband_pmi_re((antenna_index2(i)+1),i,1))+1j*double(estimates_UE2(est).phy_measurements(11).subband_pmi_im((antenna_index2(i)+1),i,1));
    end
    
    
    
    %q_ext; % calculating the maximizing precoder
    
    
   %qq1 = q_ue1;
   %qq2 = q_ue2;
   chcap_mumimo_1 = []; 
   for i = 1:1:7
        
       
        qq11(i) = map_q(pmi_ue1(i));
        qq22(i) = map_q(pmi_ue2(i));
        
        if ((qq11(i)==0 && qq22(i)==1) || (qq11(i)==1 && qq22(i)==0) || (qq11(i)==2 && qq22(i)==3) || (qq11(i)==3 && qq22(i)==2))
            ran(i) = 1;
        else
            ran(i) = 0;
        end
        
        % if MU-MIMO is switched off, we should always go in the else part
        %if (0)
     if(ran(i)==1) %MU-MIMO 
        
        
        
        %for i =1:1:7
            
           if(antenna_index1(i) == 0)
                Rx_ue1 =  abs(h11_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue1(i)*h21_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
                Ry_ue1 =  abs(h11_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) - pmi_ue1(i)*h21_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
            else
                Rx_ue1 =  abs(h12_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue1(i)*h22_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
                Ry_ue1 =  abs(h12_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) - pmi_ue1(i)*h22_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
            end
            
            if(antenna_index2(i) == 0)
                Rx_ue2 =  abs(h11_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue2(i)*h21_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
                Ry_ue2 =  abs(h11_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) - pmi_ue2(i)*h21_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
            else
                Rx_ue2 =  abs(h12_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue2(i)*h22_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
                Ry_ue2 =  abs(h12_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) - pmi_ue2(i)*h22_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2;
            end
            
%             Rx1_ue1 = [Rx1_ue1 Rx_ue1'];
%             Rx1_ue2 = [Rx1_ue2 Rx_ue2'];
%             
%             Ry1_ue1 = [Ry1_ue1 Ry_ue1'];
%             Ry1_ue2 = [Ry1_ue2 Ry_ue2'];
            %Rx222_ue2 = [Rx222_ue2 Rx22_ue2'];
          
        %end
        
        SNR_eNB_ue1 = 10*log10((Rx_ue1(:)/N0_1Rx));
        nan_in_SNR = isnan(SNR_eNB_ue1);
        SNR_eNB_ue1(nan_in_SNR) = 0;
        
        SNR_eNB_ue2 = 10*log10((Rx_ue2(:)/N0_2ndRx));
        nan_in_SNR = isnan(SNR_eNB_ue2);
        SNR_eNB_ue2(nan_in_SNR) = 0;
     
        %******************************************************************
        %% Red Zone
        I_ue1 = Ry_ue1(:)./Rx_ue1(:);
        I_ue2 = Ry_ue2(:)./Rx_ue2(:);
%         plot(I_ue1);
%         pause
        
        I_ue1 = round(I_ue1*10+1);
        I_ue2 = round(I_ue2*10+1);
                    
        I_ue1(I_ue1>11) = 11;
        I_ue2(I_ue2>11) = 11;
        
        % Read Tables here ... MUMIMO QPSK-QPSK
        for c=1:length(SNR_eNB_ue1)
        chcap_mumimo_ue1(c) = chcap_mode5_4_4_user1_cat(I_ue1(c), round(SNR_eNB_ue1(c)));
        chcap_mumimo_ue2(c) = chcap_mode5_4_4_user2_cat(I_ue2(c), round(SNR_eNB_ue2(c)));
        end
        
        %interp1(SNR_4_4_cat,chcap_mode5_4_4_user1_cat, SNR_eNB_ue1(1, :, est),'nearest','extrap');
        %chcap_mumimo_ue2(1, :, est) = interp1(SNR_4_4_cat,chcap_mode5_4_4_user2_cat, SNR_eNB_ue2(1, :, est),'nearest','extrap');
        
        chcap_mumimo_1 = [chcap_mumimo_1 (chcap_mumimo_ue1 + chcap_mumimo_ue2)];
        %******************************************************************
        %
        % call capacity_Mode5_mumimo_1 with parameters Channels, qq1,qq2, and
        % antenna index1 and 2
    else
        % select the user with the stronger precoded SNR and calculate its
        % capacity
        if (antenna_index1(i) == 0)
        Rx1 =  (abs(h11_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue1(i)*h21_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2)';
        else
            Rx1 =  (abs(h12_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue1(i)*h22_eNB1_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2)';
        end
        
        SNR_eNB_ue1 = 10*log10((Rx1(:))/N0_1Rx);
            
        nan_in_SNR = isnan(SNR_eNB_ue1);
        SNR_eNB_ue1(nan_in_SNR) = 0;
        
        if ((antenna_index2(i) == 0))
        Rx2 =  (abs(h11_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue2(i)*h21_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2)';
        else
           Rx2 =  (abs(h12_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1)) + pmi_ue2(i)*h22_eNB2_comp(:,(subband_indices(i):subband_indices(i+1)-1))).^2)'; 
        end
        
        SNR_eNB_ue2 = 10*log10((Rx2(:))/N0_2ndRx);
            
        nan_in_SNR = isnan(SNR_eNB_ue2);
        SNR_eNB_ue2(nan_in_SNR) = 0;
        
        if(sum(SNR_eNB_ue2) < sum(SNR_eNB_ue1))
            
            chcap_mumimo_1 = [chcap_mumimo_1 interp1(SNR,c_siso_4Qam,SNR_eNB_ue1','nearest','extrap')];
        else
            chcap_mumimo_1 = [chcap_mumimo_1 interp1(SNR,c_siso_4Qam,SNR_eNB_ue2','nearest','extrap')];
        end
        
    end
    
   end
   
    chcap_mumimo(1,:,est) = chcap_mumimo_1;
   
end