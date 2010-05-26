%Channel Capacity of Alamouti MIMO system

load 'SISO.mat';

[x, y, z] = size(estimates_UE(1).channel);

z = x/4;

MultiAntenna_Rx = 1; % 0 for Single antenna
                     % 1 for Multi Antenna (2 antennas here)
h = zeros(1,z);

chcap_alamouti_4Qam = zeros(1,length(h), length(estimates_UE));
chcap_alamouti_16Qam = zeros(1,length(h), length(estimates_UE));
chcap_alamouti_64Qam = zeros(1,length(h), length(estimates_UE));

SNR_eNB = zeros(1,length(h), length(estimates_UE));

for est=1:length(estimates_UE)
    
    N0 = double(estimates_UE(est).phy_measurements(1).n0_power(1));
    
    % for checking the connected Sector
    H = estimates_UE(est).channel;
    Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
    Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
    [val, ind] = max(Hs);
    
    if MultiAntenna_Rx == 1
    
        if (ind == 1)
        h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,1));
        h12_eNB = double(estimates_UE(est).channel(1:2:2*z,2,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,2,1));
        h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,1));
        h22_eNB = double(estimates_UE(est).channel(1:2:2*z,4,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,4,1));
        else if (ind == 2)
                h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,2));
                h12_eNB = double(estimates_UE(est).channel(1:2:2*z,2,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,2,2));
                h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,2));
                h22_eNB = double(estimates_UE(est).channel(1:2:2*z,4,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,4,2));
            else
                h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,3));
                h12_eNB = double(estimates_UE(est).channel(1:2:2*z,2,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,2,3));
                h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,3));
                h22_eNB = double(estimates_UE(est).channel(1:2:2*z,4,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,4,3));
            end
        end
        
        
        %SNR calculation for Alamouti Scheme
        SNR_eNB(1, :, est) = 10*log10(((abs(h11_eNB).^2) + (abs(h12_eNB).^2) + (abs(h21_eNB).^2) + (abs(h22_eNB).^2))/N0);
        
    else
        
        if (ind == 1)
        h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,1));
        h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,1)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,1));
        
        else if (ind == 2)
                h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,2));
                h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,2)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,2));
                
            else
                h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,3));
                h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,3)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,3));
                
            end
        end
        
        
        %SNR calculation for Alamouti Scheme
        SNR_eNB(1, :, est) = 10*log10(((abs(h11_eNB).^2) + (abs(h21_eNB).^2))/N0);
        
    end
    
    nan_in_SNR = isnan(SNR_eNB);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB(nan_in_SNR) = 0;
    
    while (min(SNR_eNB(1, :, est)) < -20)
        [value, index]  = min(SNR_eNB(1, :, est));
        SNR_eNB(1, index, est) = -20;
        continue
        
    end
    
    while (max(SNR_eNB(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB(1, :, est));
        SNR_eNB(1, index, est) = 40;
        continue
        
    end
    
    for const=1:3
        if const ==1
            for c=1:length(h)
                chcap_alamouti_4Qam(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB(1, c, est))));
            end
        else if const==2
                for c=1:length(h)
                    chcap_alamouti_16Qam(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB(1, c, est))));
                end
            else
                for c=1:length(h)
                    chcap_alamouti_64Qam(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB(1, c, est))));
                end
            end
        end
    end
end
% if MultiAntenna_Rx == 1
%     save 'chcap_alamouti_MultiAntennaUE_Measurements.mat' 'chcap_alamouti*' 'SNR_eNB*'
% else
%     save 'chcap_alamouti_SingleAntennaUE_Measurements.mat' 'chcap_alamouti*' 'SNR_eNB*'
% end
