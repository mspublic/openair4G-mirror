function [Rate_4Qam,Rate_16Qam,Rate_64Qam, alam_SNR_persecond]  = calc_rps_Alamouti(estimates_UE)

capacity_Alamouti_1

alam_SNR_persecond = mean(SNR_eNB(:));

%plot(SNR_eNB1, 'g-x');

%[x y z] = size(chcap_siso_single_stream_4Qam_eNB1);

% Following will hold the data rate of one whole Frame

Frame_DL_sum_4Qam = zeros(1, length(estimates_UE));
Frame_DL_sum_16Qam = zeros(1,length(estimates_UE));
Frame_DL_sum_64Qam = zeros(1,length(estimates_UE));


for i = 1:length(estimates_UE)
    %following holds accumulated data rate of 200 resource elements
    
    RS_sum_4Qam = sum(chcap_alamouti_4Qam(1,:,i));
    RS_sum_16Qam = sum(chcap_alamouti_16Qam(1,:,i));
    RS_sum_64Qam = sum(chcap_alamouti_64Qam(1,:,i));
    
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam = RS_sum_4Qam * 12;
    Subframe_DL_sum_16Qam = RS_sum_16Qam * 12;
    Subframe_DL_sum_64Qam = RS_sum_64Qam * 12;
    
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
    Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
    Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
    
end

        % add rate of all 100 frames each of 10ms length to compute data rate/sec
        Rate_4Qam = sum(Frame_DL_sum_4Qam(:));
        Rate_16Qam = sum(Frame_DL_sum_16Qam(:));
        Rate_64Qam = sum(Frame_DL_sum_64Qam(:));
        
 
        
    
    