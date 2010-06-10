function [Rate_4Qam_1Rx,Rate_16Qam_1Rx,Rate_64Qam_1Rx, alam_SNR_1Rx_persecond, Rate_4Qam_2Rx,Rate_16Qam_2Rx,Rate_64Qam_2Rx, alam_SNR_2Rx_persecond]  = calc_rps_Alamouti(estimates_UE)

capacity_Alamouti_1

alam_SNR_1Rx_persecond = mean(SNR_eNB_1Rx(:));
alam_SNR_2Rx_persecond = mean(SNR_eNB_2Rx(:));

%plot(SNR_eNB1, 'g-x');

%[x y z] = size(chcap_siso_single_stream_4Qam_eNB1);

% Following will hold the data rate of one whole Frame

Frame_DL_sum_4Qam_1Rx = zeros(1, length(estimates_UE));
Frame_DL_sum_16Qam_1Rx = zeros(1,length(estimates_UE));
Frame_DL_sum_64Qam_1Rx = zeros(1,length(estimates_UE));

Frame_DL_sum_4Qam_2Rx = zeros(1, length(estimates_UE));
Frame_DL_sum_16Qam_2Rx = zeros(1,length(estimates_UE));
Frame_DL_sum_64Qam_2Rx = zeros(1,length(estimates_UE));

for i = 1:length(estimates_UE)
    %following holds accumulated data rate of 200 resource elements
    
    RS_sum_4Qam_1Rx = sum(chcap_alamouti_4Qam_1Rx(1,:,i));
    RS_sum_16Qam_1Rx = sum(chcap_alamouti_16Qam_1Rx(1,:,i));
    RS_sum_64Qam_1Rx = sum(chcap_alamouti_64Qam_1Rx(1,:,i));
    
    RS_sum_4Qam_2Rx = sum(chcap_alamouti_4Qam_2Rx(1,:,i));
    RS_sum_16Qam_2Rx = sum(chcap_alamouti_16Qam_2Rx(1,:,i));
    RS_sum_64Qam_2Rx = sum(chcap_alamouti_64Qam_2Rx(1,:,i));
    
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam_1Rx = RS_sum_4Qam_1Rx * 12;
    Subframe_DL_sum_16Qam_1Rx = RS_sum_16Qam_1Rx * 12;
    Subframe_DL_sum_64Qam_1Rx = RS_sum_64Qam_1Rx * 12;
    
    Subframe_DL_sum_4Qam_2Rx = RS_sum_4Qam_2Rx * 12;
    Subframe_DL_sum_16Qam_2Rx = RS_sum_16Qam_2Rx * 12;
    Subframe_DL_sum_64Qam_2Rx = RS_sum_64Qam_2Rx * 12;
    
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    Frame_DL_sum_4Qam_1Rx(i) = Subframe_DL_sum_4Qam_1Rx * 6;
    Frame_DL_sum_16Qam_1Rx(i) = Subframe_DL_sum_16Qam_1Rx * 6;
    Frame_DL_sum_64Qam_1Rx(i) = Subframe_DL_sum_64Qam_1Rx * 6;
    
      Frame_DL_sum_4Qam_2Rx(i) = Subframe_DL_sum_4Qam_2Rx * 6;
    Frame_DL_sum_16Qam_2Rx(i) = Subframe_DL_sum_16Qam_2Rx * 6;
    Frame_DL_sum_64Qam_2Rx(i) = Subframe_DL_sum_64Qam_2Rx * 6;
end

        % add rate of all 100 frames each of 10ms length to compute data rate/sec
        Rate_4Qam_1Rx = sum(Frame_DL_sum_4Qam_1Rx(:));
        Rate_16Qam_1Rx = sum(Frame_DL_sum_16Qam_1Rx(:));
        Rate_64Qam_1Rx = sum(Frame_DL_sum_64Qam_1Rx(:));
        
         Rate_4Qam_2Rx = sum(Frame_DL_sum_4Qam_2Rx(:));
        Rate_16Qam_2Rx = sum(Frame_DL_sum_16Qam_2Rx(:));
        Rate_64Qam_2Rx = sum(Frame_DL_sum_64Qam_2Rx(:));
        
 
        
    
    