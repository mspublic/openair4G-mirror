%Channel Capacity of SISO system

load 'SISO.mat';
[x, y, z] = size(estimates_UE(1).channel);

z = x/4;

chcap_siso_single_stream_4Qam_2Rx = zeros(1,z, length(estimates_UE));
chcap_siso_single_stream_16Qam_2Rx = zeros(1,z, length(estimates_UE));
chcap_siso_single_stream_64Qam_2Rx = zeros(1,z, length(estimates_UE));

chcap_siso_single_stream_4Qam_1Rx = zeros(1,z, length(estimates_UE));
chcap_siso_single_stream_16Qam_1Rx = zeros(1,z, length(estimates_UE));
chcap_siso_single_stream_64Qam_1Rx = zeros(1,z, length(estimates_UE));

SNR_eNB = zeros(1,z, length(estimates_UE));

for est=1:length(estimates_UE)
    
    N0 = double(estimates_UE(est).phy_measurements(1).n0_power(1));
    
    % for checking the connected Sector
    H = estimates_UE(est).channel;
    Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
    Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
    [val, ind] = max(Hs);
    
         
        h11_eNB = double(estimates_UE(est).channel(1:2:2*z,1,ind)) + 1j*double(estimates_UE(est).channel(2:2:2*z,1,ind));
        h12_eNB = double(estimates_UE(est).channel(1:2:2*z,2,ind)) + 1j*double(estimates_UE(est).channel(2:2:2*z,2,ind));
        h21_eNB = double(estimates_UE(est).channel(1:2:2*z,3,ind)) + 1j*double(estimates_UE(est).channel(2:2:2*z,3,ind));
        h22_eNB = double(estimates_UE(est).channel(1:2:2*z,4,ind)) + 1j*double(estimates_UE(est).channel(2:2:2*z,4,ind));
    
     
    % version 1: using tabulated values
    SNR_eNB_1Rx(1, :, est) = 10*log10(abs(h11_eNB + h21_eNB).^2/N0);
    
    SNR_eNB_2Rx(1, :, est) = 10*log10(((abs(h11_eNB + h21_eNB).^2)+(abs(h12_eNB + h22_eNB).^2))/N0);
   
    nan_in_SNR = isnan(SNR_eNB_1Rx);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB_1Rx(nan_in_SNR) = 0;
    
    nan_in_SNR = isnan(SNR_eNB_2Rx);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB_2Rx(nan_in_SNR) = 0;
    
    while (min(SNR_eNB_1Rx(1, :, est)) < -20 )
        [value, index]  = min(SNR_eNB_1Rx(1, :, est));
        SNR_eNB_1Rx(1, index, est) = -20;
        continue
        
    end
    
    while (max(SNR_eNB_1Rx(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB_1Rx(1, :, est));
        SNR_eNB_1Rx(1, index, est) = 40;
        continue
        
    end
    
    
    while (min(SNR_eNB_2Rx(1, :, est)) < -20 )
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
            for c=1:z
                
                chcap_siso_single_stream_4Qam_1Rx(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                chcap_siso_single_stream_4Qam_2Rx(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
                
            end
        else if const==2
                for c=1:z
                    
                    chcap_siso_single_stream_16Qam_1Rx(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                    chcap_siso_single_stream_16Qam_2Rx(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
                    
                end
            else
                for c=1:z
                    
                    chcap_siso_single_stream_64Qam_1Rx(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB_1Rx(1, c, est))));
                    chcap_siso_single_stream_64Qam_2Rx(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB_2Rx(1, c, est))));
                    
                end
            end
        end
    end
end

%save 'chcap_SISO_Measurements.mat' 'chcap_siso*' 'SNR_eNB*'
