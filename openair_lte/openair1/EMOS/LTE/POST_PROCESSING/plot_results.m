%% Plot Reults 
%% Created on 19-04-2010
%% Author Imran Latif (imran.latif@eurecom.fr)
%% Eurecom Instuitute - Communications Mobile 

% This file is used to plot different results from LTE Measurements
% The measurements are considered for three different cases.
% SISO, Alamouti Scheme and Beamforming
% In LTE for 5 MHz Bandwidth:
% One Subframe contains 25 * (12 *14) = 4200 Resource Elements for normal cyclic prefix.
% We measure channel for 200 resource elements in first subframe of each frame. And we
% interpolate for the rest 4000 resource elements of that subframe.

load 'chcap_alamouti_MultiAntennaUE_Measurements.mat'
load 'chcap_beamforming_MultiAntennaUE_Measurements.mat'
load 'chcap_SISO_Measurements.mat'

%**************************************************************************
% %For SISO Scheme
% %
[x y z] = size(chcap_siso_single_stream_4Qam_eNB1);


% Rounding of Number of frames to the multiple of 100 is done below:
TotalFrames = (round(z/100))*100;

% Following will hold the data rate of one whole Frame

Frame_DL_sum_4Qam = zeros(1, TotalFrames);
Frame_DL_sum_16Qam = zeros(1,TotalFrames);
Frame_DL_sum_64Qam = zeros(1,TotalFrames);

% Following holds the accumulated data rate of 100 Frames. Since one frame
% lasts for 10ms in LTE so for data rate per second we need to add data of
% such a 100 Frames.

Ratepersec_4Qam = zeros(1, TotalFrames/100);
Ratepersec_16Qam = zeros(1, TotalFrames/100);
Ratepersec_64Qam = zeros(1, TotalFrames/100);

counter = 0;
k = 0;
for i = 1:TotalFrames
    %following holds accumulated data rate of 200 resource elements
    
    RS_sum_4Qam = sum(chcap_siso_single_stream_4Qam_eNB1(1,:,i));
    RS_sum_16Qam = sum(chcap_siso_single_stream_16Qam_eNB1(1,:,i));
    RS_sum_64Qam = sum(chcap_siso_single_stream_64Qam_eNB1(1,:,i));
    
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
    Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
    Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;
    
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
    Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
    Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
    counter = counter + 1;
    
    if counter == 100 % means one second
        counter = 0;
        k = k+1;
        % add rate of all 100 frames each of 10ms length to compute data rate/sec
        Ratepersec_4Qam(1,k) = sum(Frame_DL_sum_4Qam(i-99 : i));
        Ratepersec_16Qam(1,k) = sum(Frame_DL_sum_16Qam(i-99 : i));
        Ratepersec_64Qam(1,k) = sum(Frame_DL_sum_64Qam(i-99 : i));
        
    end
    
    
end

figure
Title('SISO')
xlabel('Time[Seconds]');
ylabel('Rate');
hold on
plot(Ratepersec_4Qam,'b-o' );
plot(Ratepersec_16Qam,'g-o' );
plot(Ratepersec_64Qam,'r-o' );
hold off;

%*******************************************************************************************************
%For Alamouti Scheme
[x y z] = size(chcap_alamouti_4Qam_eNB1);


% Rounding of Number of frames to the multiple of 100 is done below:
TotalFrames = (round(z/100))*100;

% Following will hold the data rate of one whole Frame

Frame_DL_sum_4Qam = zeros(1, TotalFrames);
Frame_DL_sum_16Qam = zeros(1,TotalFrames);
Frame_DL_sum_64Qam = zeros(1,TotalFrames);

% Following holds the accumulated data rate of 100 Frames. Since one frame
% lasts for 10ms in LTE so for data rate per second we need to add data of
% such a 100 Frames.

Ratepersec_4Qam = zeros(1, TotalFrames/100);
Ratepersec_16Qam = zeros(1, TotalFrames/100);
Ratepersec_64Qam = zeros(1, TotalFrames/100);

counter = 0;
k = 0;
for i = 1:TotalFrames
    
    %following holds accumulated data rate of 200 resource elements
    
    RS_sum_4Qam = sum(chcap_alamouti_4Qam_eNB1(1,:,i));
    RS_sum_16Qam = sum(chcap_alamouti_16Qam_eNB1(1,:,i));
    RS_sum_64Qam = sum(chcap_alamouti_64Qam_eNB1(1,:,i));
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
    Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
    Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
    Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
    Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
    counter = counter + 1;
    
    if counter == 100 % means one second
        counter = 0;
        k = k+1;
        % add rate of all 100 frames each of 10ms length to compute data rate/sec
        Ratepersec_4Qam(1,k) = sum(Frame_DL_sum_4Qam(i-99 : i));
        Ratepersec_16Qam(1,k) = sum(Frame_DL_sum_16Qam(i-99 : i));
        Ratepersec_64Qam(1,k) = sum(Frame_DL_sum_64Qam(i-99 : i));
        
    end
    
    
end
figure
Title('alamouti')
xlabel('Time[Seconds]');
ylabel('Rate');
hold on
plot(Ratepersec_4Qam,'b-o' );
plot(Ratepersec_16Qam,'g-o' );
plot(Ratepersec_64Qam,'r-o' );
hold off;
%++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%*******************************************************************************************************
%For Beamforming uPDATED
[x y z] = size(chcap_beamforming_4Qam_eNB1);


% Rounding of Number of frames to the multiple of 100 is done below:
TotalFrames = (round(z/100))*100;

% Following will hold the data rate of one whole Frame

Frame_DL_sum_4Qam = zeros(1, TotalFrames);
Frame_DL_sum_16Qam = zeros(1,TotalFrames);
Frame_DL_sum_64Qam = zeros(1,TotalFrames);

% Following holds the accumulated data rate of 100 Frames. Since one frame
% lasts for 10ms in LTE so for data rate per second we need to add data of
% such a 100 Frames.

Ratepersec_4Qam = zeros(1, TotalFrames/100);
Ratepersec_16Qam = zeros(1, TotalFrames/100);
Ratepersec_64Qam = zeros(1, TotalFrames/100);

counter = 0;
k = 0;
for i = 1:TotalFrames
    
    %following holds accumulated data rate of 200 resource elements
    
    RS_sum_4Qam = sum(chcap_beamforming_4Qam_eNB1(1,1:200,i));
    RS_sum_16Qam = sum(chcap_beamforming_16Qam_eNB1(1,1:200,i));
    RS_sum_64Qam = sum(chcap_beamforming_64Qam_eNB1(1,1:200,i));
    
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
    Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
    Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;
    
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    Initial_Frame_DL_sum_4Qam = Subframe_DL_sum_4Qam * 2;
    Initial_Frame_DL_sum_16Qam = Subframe_DL_sum_16Qam * 2;
    Initial_Frame_DL_sum_64Qam = Subframe_DL_sum_64Qam * 2;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    RS_sum_4Qam = sum(chcap_beamforming_4Qam_eNB1(1,201:end,i));
    RS_sum_16Qam = sum(chcap_beamforming_16Qam_eNB1(1,201:end,i));
    RS_sum_64Qam = sum(chcap_beamforming_64Qam_eNB1(1,201:end,i));
    
    % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
    % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
    
    Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
    Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
    Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;
    
    %In LTE Confguration 3 we have 6 downlink subframes in one frame so
    
    WithFB_Frame_DL_sum_4Qam = Subframe_DL_sum_4Qam * 4;
    WithFB_Frame_DL_sum_16Qam = Subframe_DL_sum_16Qam * 4;
    WithFB_Frame_DL_sum_64Qam = Subframe_DL_sum_64Qam * 4;
    
    
     Frame_DL_sum_4Qam(i) = WithFB_Frame_DL_sum_4Qam + Initial_Frame_DL_sum_4Qam;
     Frame_DL_sum_16Qam(i) = WithFB_Frame_DL_sum_16Qam + Initial_Frame_DL_sum_16Qam;
     Frame_DL_sum_64Qam(i) = WithFB_Frame_DL_sum_64Qam + Initial_Frame_DL_sum_64Qam;
     
    
    counter = counter + 1;
    
    if counter == 100 % means one second
        counter = 0;
        k = k+1;
        % add rate of all 100 frames each of 10ms length to compute data rate/sec
        Ratepersec_4Qam(1,k) = sum(Frame_DL_sum_4Qam(i-99 : i));
        Ratepersec_16Qam(1,k) = sum(Frame_DL_sum_16Qam(i-99 : i));
        Ratepersec_64Qam(1,k) = sum(Frame_DL_sum_64Qam(i-99 : i));
        
    end
    
    
end
figure
Title('beamforming')
xlabel('Time[Seconds]');
ylabel('Rate');
hold on
plot(Ratepersec_4Qam,'b-o' );
plot(Ratepersec_16Qam,'g-o' );
plot(Ratepersec_64Qam,'r-o' );
hold off;
%++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

% %*******************************************************************************************************
% %For Beamforming
% [x y z] = size(chcap_beamforming_4Qam_eNB1);
% 
% 
% % Rounding of Number of frames to the multiple of 100 is done below:
% TotalFrames = (round(z/100))*100;
% 
% % Following will hold the data rate of one whole Frame
% 
% Frame_DL_sum_4Qam = zeros(1, TotalFrames);
% Frame_DL_sum_16Qam = zeros(1,TotalFrames);
% Frame_DL_sum_64Qam = zeros(1,TotalFrames);
% 
% % Following holds the accumulated data rate of 100 Frames. Since one frame
% % lasts for 10ms in LTE so for data rate per second we need to add data of
% % such a 100 Frames.
% 
% Ratepersec_4Qam = zeros(1, TotalFrames/100);
% Ratepersec_16Qam = zeros(1, TotalFrames/100);
% Ratepersec_64Qam = zeros(1, TotalFrames/100);
% 
% counter = 0;
% k = 0;
% for i = 1:TotalFrames
%     
%     %following holds accumulated data rate of 200 resource elements
%     
%     RS_sum_4Qam = sum(chcap_beamforming_4Qam_eNB1(1,:,i));
%     RS_sum_16Qam = sum(chcap_beamforming_16Qam_eNB1(1,:,i));
%     RS_sum_64Qam = sum(chcap_beamforming_64Qam_eNB1(1,:,i));
%     
%     % since 4200/200 = 21 groups of 200 resource elements are there in one subframe, so to get
%     % the estimate of whole subframe we need to multiply RS_sum_XQam by 21.
%     
%     Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
%     Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
%     Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;
%     
%     %In LTE Confguration 3 we have 6 downlink subframes in one frame so
%     
%     Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
%     Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
%     Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
%     counter = counter + 1;
%     
%     if counter == 100 % means one second
%         counter = 0;
%         k = k+1;
%         % add rate of all 100 frames each of 10ms length to compute data rate/sec
%         Ratepersec_4Qam(1,k) = sum(Frame_DL_sum_4Qam(i-99 : i));
%         Ratepersec_16Qam(1,k) = sum(Frame_DL_sum_16Qam(i-99 : i));
%         Ratepersec_64Qam(1,k) = sum(Frame_DL_sum_64Qam(i-99 : i));
%         
%     end
%     
%     
% end
% figure
% Title('beamforming')
% xlabel('Time[Seconds]');
% ylabel('Rate');
% hold on
% plot(Ratepersec_4Qam,'b-o' );
% plot(Ratepersec_16Qam,'g-o' );
% plot(Ratepersec_64Qam,'r-o' );
% hold off;
% %++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++