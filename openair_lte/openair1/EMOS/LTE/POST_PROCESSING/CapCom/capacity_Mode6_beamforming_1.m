%Channel Capacity for LTE Mode 6 Beamforming
load 'SISO.mat'

[x, y, z] = size(estimates_UE(1).channel);

z = x/2;
h = zeros(1,z);

%precoding matrix values q
qi = [1 -1 1i -1i];

chcap_beamforming_4Qam_1Rx = zeros(1,length(h), length(estimates_UE));
chcap_beamforming_16Qam_1Rx = zeros(1,length(h), length(estimates_UE));
chcap_beamforming_64Qam_1Rx = zeros(1,length(h), length(estimates_UE));

chcap_beamforming_4Qam_2Rx = zeros(1,length(h), length(estimates_UE));
chcap_beamforming_16Qam_2Rx = zeros(1,length(h), length(estimates_UE));
chcap_beamforming_64Qam_2Rx = zeros(1,length(h), length(estimates_UE));

SNR_eNB_1Rx = zeros(1,length(h), length(estimates_UE));
SNR_eNB_2Rx = zeros(1,length(h), length(estimates_UE));

q_index = [0 32 64 96 128 160 192 200];

for est=1:length(estimates_UE)
    
    N0 = double(estimates_UE(est).phy_measurements(1).n0_power(1));
   
    % for checking the connected Sector
    H = estimates_UE(est).channel;
    Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
    Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
    [val, ind] = max(Hs);
    antenna_index = estimates_UE(est).phy_measurements(1).selected_rx_antennas(:,1);
     
        h11_eNB = double(estimates_UE(est).channel(1:2:end,1,ind)) + 1j*double(estimates_UE(est).channel(2:2:end,1,ind));
        h12_eNB = double(estimates_UE(est).channel(1:2:end,2,ind)) + 1j*double(estimates_UE(est).channel(2:2:end,2,ind));
        h21_eNB = double(estimates_UE(est).channel(1:2:end,3,ind)) + 1j*double(estimates_UE(est).channel(2:2:end,3,ind));
        h22_eNB = double(estimates_UE(est).channel(1:2:end,4,ind)) + 1j*double(estimates_UE(est).channel(2:2:end,4,ind));
        
        for i=1:1:7
            
            for qq=1:4
       
            Rx1(qq).eNB =  abs(h11_eNB((q_index(i)+1):q_index(i+1)) + qi(qq)*h21_eNB((q_index(i)+1):q_index(i+1))).^2;
            Rx2(qq).eNB =  abs(h12_eNB((q_index(i)+1):q_index(i+1)) + qi(qq)*h22_eNB((q_index(i)+1):q_index(i+1))).^2;
                
            end
            
            
            for ii=1:4
            
                SNR_eNB_q_1Rx(ii).snr = 10*log10((Rx1(ii).eNB)/N0);
                sum_eNB_q_1Rx(ii) = sum(SNR_eNB_q_1Rx(ii).snr);
                
                SNR_eNB_q_2Rx(ii).snr = 10*log10((Rx1(ii).eNB + Rx2(ii).eNB)/N0);
                sum_eNB_q_2Rx(ii) = sum(SNR_eNB_q_2Rx(ii).snr);
        
            end
        
        [value_eNB_1Rx, index_eNB_1Rx] = max(sum_eNB_q_1Rx);
        [value_eNB_2Rx, index_eNB_2Rx] = max(sum_eNB_q_2Rx);

        %SNR calculation
        SNR_eNB_1Rx(1, ((q_index(i)+1):q_index(i+1)), est) = SNR_eNB_q_1Rx(index_eNB_1Rx).snr;
        SNR_eNB_2Rx(1, ((q_index(i)+1):q_index(i+1)), est) = SNR_eNB_q_2Rx(index_eNB_2Rx).snr;
        end
        
        
        % after feedback in 3rd subframe
        
        Rx111 = [];
        Rx222 = [];
        
       % q = double(estimates_UE(est).phy_measurements(1).subband_pmi_re((antenna_index(i)+1),i,ind))+1j*double(estimates_UE(est).phy_measurements(1).subband_pmi_im((antenna_index(i)+1),i,ind));
       % q = double(estimates_UE(est).phy_measurements(1).subband_pmi_re(ind,:,1))+1j*double(estimates_UE(est).phy_measurements(1).subband_pmi_im(ind,:,1));
        for i =1:1:7
           
           q = double(estimates_UE(est).phy_measurements(1).subband_pmi_re((antenna_index(i)+1),i,1))+1j*double(estimates_UE(est).phy_measurements(1).subband_pmi_im((antenna_index(i)+1),i,1));
            
            qq(i) = quantize_q(q);
        
            Rx11 =  abs(h11_eNB((q_index(i)+1):q_index(i+1)) + qq(i)*h21_eNB((q_index(i)+1):q_index(i+1))).^2;
            Rx22 =  abs(h12_eNB((q_index(i)+1):q_index(i+1)) + qq(i)*h22_eNB((q_index(i)+1):q_index(i+1))).^2;
            
            Rx111 = [Rx111 Rx11'];
            Rx222 = [Rx222 Rx22'];
        end
        
            SNR_eNB_1Rx(1, 201:end, est) = 10*log10((Rx111)/N0);
            SNR_eNB_2Rx(1, 201:end, est) = 10*log10((Rx111 + Rx222)/N0);
            
    nan_in_SNR = isnan(SNR_eNB_1Rx);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB_1Rx(nan_in_SNR) = 0;
    
    nan_in_SNR = isnan(SNR_eNB_2Rx);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB_2Rx(nan_in_SNR) = 0;
    
    while (min(SNR_eNB_1Rx(1, :, est)) < -20)
        [value, index]  = min(SNR_eNB_1Rx(1, :, est));
        SNR_eNB_1Rx(1, index, est) = -20;
        continue
        
    end
    
    while (max(SNR_eNB_1Rx(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB_1Rx(1, :, est));
        SNR_eNB_1Rx(1, index, est) = 40;
        continue
        
    end
 
    while (min(SNR_eNB_2Rx(1, :, est)) < -20)
        [value, index]  = min(SNR_eNB_2Rx(1, :, est));
        SNR_eNB_2Rx(1, index, est) = -20;
        continue
        
    end
    
    while (max(SNR_eNB_2Rx(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB_2Rx(1, :, est));
        SNR_eNB_2Rx(1, index, est) = 40;
        continue
    end
    
    for const=1:3
        if const ==1
            for c=1:length(h)
                chcap_beamforming_4Qam_1Rx(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                chcap_beamforming_4Qam_2Rx(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
            end
        else if const==2
                for c=1:length(h)
                    chcap_beamforming_16Qam_1Rx(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                    chcap_beamforming_16Qam_2Rx(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
                end
            else
                for c=1:length(h)
                    chcap_beamforming_64Qam_1Rx(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                    chcap_beamforming_64Qam_2Rx(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
                end
            end
        end
    end
end

% if MultiAntenna_Rx == 1
%     save 'chcap_beamforming_MultiAntennaUE_Measurements.mat' 'chcap_beamforming*' 'SNR_eNB' 'SNR_eNB2' 'SNR_eNB3'
% else
%     save 'chcap_beamforming_SingleAntennaUE_Measurements' 'chcap_beamforming*' 'SNR_eNB' 'SNR_eNB2' 'SNR_eNB3'
% end
