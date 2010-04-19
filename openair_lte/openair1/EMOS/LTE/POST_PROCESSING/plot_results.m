%**************************************************************************
%For SISO Scheme

[x y z] = size(chcap_siso_single_stream_4Qam_eNB1);

%for this case
y = y/2;
z = (round(z/100))*100;
Frame_DL_sum_4Qam = zeros(1,z);
Frame_DL_sum_16Qam = zeros(1,z);
Frame_DL_sum_64Qam = zeros(1,z);
counter = 0;
k = 0;
for i = 1:z
    
RS_sum_4Qam = sum(chcap_siso_single_stream_4Qam_eNB1(1,1:y,i));
RS_sum_16Qam = sum(chcap_siso_single_stream_16Qam_eNB1(1,1:y,i));
RS_sum_64Qam = sum(chcap_siso_single_stream_64Qam_eNB1(1,1:y,i));

Subframe_DL_sum_4Qam = RS_sum_4Qam * 21;
Subframe_DL_sum_16Qam = RS_sum_16Qam * 21;
Subframe_DL_sum_64Qam = RS_sum_64Qam * 21;

%for LTE Confguration 3

Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
counter = counter + 1;

if counter == 100
    counter = 0;
    k = k+1;
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

%for this case
%y = y/2;
z = (round(z/100))*100;
Frame_DL_sum_4Qam = zeros(1,z);
Frame_DL_sum_16Qam = zeros(1,z);
Frame_DL_sum_64Qam = zeros(1,z);
counter = 0;
k = 0;
for i = 1:z
    
RS_sum_4Qam = sum(chcap_alamouti_4Qam_eNB1(1,1:y,i));
RS_sum_16Qam = sum(chcap_alamouti_16Qam_eNB1(1,1:y,i));
RS_sum_64Qam = sum(chcap_alamouti_64Qam_eNB1(1,1:y,i));

Subframe_DL_sum_4Qam = RS_sum_4Qam * 10.5;
Subframe_DL_sum_16Qam = RS_sum_16Qam * 10.5;
Subframe_DL_sum_64Qam = RS_sum_64Qam * 10.5;

%for LTE Confguration 3

Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
counter = counter + 1;

if counter == 100
    counter = 0;
    k = k+1;
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
%For Beamforming
[x y z] = size(chcap_beamforming_4Qam_eNB1);

%for this case
%y = y/2;
z = (round(z/100))*100;
Frame_DL_sum_4Qam = zeros(1,z);
Frame_DL_sum_16Qam = zeros(1,z);
Frame_DL_sum_64Qam = zeros(1,z);
counter = 0;
k = 0;
for i = 1:z
    
RS_sum_4Qam = sum(chcap_beamforming_4Qam_eNB1(1,1:y,i));
RS_sum_16Qam = sum(chcap_beamforming_16Qam_eNB1(1,1:y,i));
RS_sum_64Qam = sum(chcap_beamforming_64Qam_eNB1(1,1:y,i));

Subframe_DL_sum_4Qam = RS_sum_4Qam * 10.5;
Subframe_DL_sum_16Qam = RS_sum_16Qam * 10.5;
Subframe_DL_sum_64Qam = RS_sum_64Qam * 10.5;

%for LTE Confguration 3

Frame_DL_sum_4Qam(i) = Subframe_DL_sum_4Qam * 6;
Frame_DL_sum_16Qam(i) = Subframe_DL_sum_16Qam * 6;
Frame_DL_sum_64Qam(i) = Subframe_DL_sum_64Qam * 6;
counter = counter + 1;

if counter == 100
    counter = 0;
    k = k+1;
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