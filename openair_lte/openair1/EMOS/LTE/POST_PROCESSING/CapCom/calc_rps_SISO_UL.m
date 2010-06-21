function [Rate_4Qam,Rate_16Qam,Rate_64Qam, siso_SNR_persecond]  = calc_rps_SISO_UL(estimates_eNB, nb_rx, tweak)

if nargin < 3
    tweak = 0;
end

%Channel Capacity of SISO system

load 'SISO.mat';

if tweak
    srs_len = 113;
else
    srs_len = 144;
end

chcap_siso_single_stream_4Qam = zeros(1, 3*srs_len, length(estimates_eNB));
chcap_siso_single_stream_16Qam = zeros(1, 3*srs_len, length(estimates_eNB));
chcap_siso_single_stream_64Qam = zeros(1, 3*srs_len, length(estimates_eNB));

SNR_eNB = zeros(1,3*srs_len,length(estimates_eNB));

for est=1:length(estimates_eNB)
    
    % choose the right sector
    sector = estimates_eNB(est).eNb_UE_stats(2).sector;

    N0 = double(estimates_eNB(est).phy_measurements_eNb(sector+1).n0_power(1));
    
    H_abs_sqr = double(estimates_eNB(est).channel(1:2:end,:,:,1:3)).^2 + ...
                double(estimates_eNB(est).channel(2:2:end,:,:,1:3)).^2;

    % extract 144 carriers with constitute SRS
    if tweak
        H_abs_sqr = reshape(H_abs_sqr,450,2,2,3);
        ind = [363:2:450 2:2:138 ];
        H_abs_sqr_tot = sum(H_abs_sqr(ind,ones(1,nb_rx),1,:),2);
    else
        ind = [363:2:512 2:2:138];
        H_abs_sqr_tot = sum(H_abs_sqr(ind,1:nb_rx,sector+1,:),2);
    end
    
    SNR_eNB(1, :, est) = 10*log10(H_abs_sqr_tot(:)/N0);
   
%     SNR_eNB(isnan(SNR_eNB)) = 0;
%     
%     index = (SNR_eNB(1, :, est) < -20); 
%     SNR_eNB(1, index, est) = -20;
%     
%     index = (SNR_eNB(1, :, est) > 40);
%     SNR_eNB(1, index, est) = 40;
    
    chcap_siso_single_stream_4Qam(1, :, est) = interp1(SNR,c_siso_4Qam,SNR_eNB(1, :, est),'nearest','extrap');

    chcap_siso_single_stream_16Qam(1, :, est) = interp1(SNR,c_siso_16Qam,SNR_eNB(1, :, est),'nearest','extrap');
                    
    chcap_siso_single_stream_64Qam(1, :, est) = interp1(SNR,c_siso_64Qam,SNR_eNB(1, :, est),'nearest','extrap');
                    
end


siso_SNR_persecond = mean(SNR_eNB(:));

% for the UL we have 300 subcarriers times 9 symbols (2 are DRS, 1 is SRS)

Rate_4Qam = sum(sum(chcap_siso_single_stream_4Qam,2)*300/srs_len*9,3);
Rate_16Qam = sum(sum(chcap_siso_single_stream_16Qam,2)*300/srs_len*9,3);
Rate_64Qam = sum(sum(chcap_siso_single_stream_64Qam,2)*300/srs_len*9,3);



        
 
        
    
    