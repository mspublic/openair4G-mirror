%Channel Capacity of SISO system
%clear all;
%clc;
%close all;
%warning off all
%f%ormat('long');
%load 'estimates.mat';
load 'SISO.mat';
%M1=4;% QAM on first antenna

[x, y, z] = size(estimates(1).channel);

z = z/4;
%h = zeros(1,200);

chcap_siso_single_stream_4Qam_eNB1 = zeros(1,z, length(estimates));
chcap_siso_single_stream_4Qam_eNB2 = zeros(1,z, length(estimates));
chcap_siso_single_stream_4Qam_eNB3 = zeros(1,z, length(estimates));

chcap_siso_single_stream_16Qam_eNB1 = zeros(1,z, length(estimates));
chcap_siso_single_stream_16Qam_eNB2 = zeros(1,z, length(estimates));
chcap_siso_single_stream_16Qam_eNB3 = zeros(1,z, length(estimates));

chcap_siso_single_stream_64Qam_eNB1 = zeros(1,z, length(estimates));
chcap_siso_single_stream_64Qam_eNB2 = zeros(1,z, length(estimates));
chcap_siso_single_stream_64Qam_eNB3 = zeros(1,z, length(estimates));

SNR_eNB1 = zeros(1,z, length(estimates));
SNR_eNB2 = zeros(1,z, length(estimates));
SNR_eNB3 = zeros(1,z, length(estimates));


for est=1:length(estimates)
    
    N0 = double(estimates(est).phy_measurements(1).n0_power(1));
    
    h_eNB1 = double(estimates(est).channel(1,1,1:2:2*z)) + 1j*double(estimates(est).channel(1,1,2:2:2*z));
    h_eNB2 = double(estimates(est).channel(2,1,1:2:2*z)) + 1j*double(estimates(est).channel(2,1,2:2:2*z));
    h_eNB3 = double(estimates(est).channel(3,1,1:2:2*z)) + 1j*double(estimates(est).channel(3,1,2:2:2*z));
    
    % version 1: using tabulated values
    SNR_eNB1(1, :, est) = 10*log10(abs(h_eNB1).^2/N0);
    SNR_eNB2(1, :, est) = 10*log10(abs(h_eNB2).^2/N0);
    SNR_eNB3(1, :, est) = 10*log10(abs(h_eNB3).^2/N0);
    
    nan_in_SNR = isnan(SNR_eNB1);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB1(nan_in_SNR) = 0;
    
    nan_in_SNR = isnan(SNR_eNB2);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB2(nan_in_SNR) = 0;
    
    nan_in_SNR = isnan(SNR_eNB3);
    nan_in_SNR = find(nan_in_SNR == 1);
    SNR_eNB3(nan_in_SNR) = 0;
    
    while (min(SNR_eNB1(1, :, est)) < -20 )
        [value, index]  = min(SNR_eNB1(1, :, est));
        SNR_eNB1(1, index, est) = -20;
        continue
        
    end
    
    while (max(SNR_eNB1(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB1(1, :, est));
        SNR_eNB1(1, index, est) = 40;
        continue
        
    end
    
    while (min(SNR_eNB2(1, :, est)) < -20)
        [value, index]  = min(SNR_eNB2(1, :, est));
        SNR_eNB2(1, index, est) = -20;
        continue
        
    end
    while (max(SNR_eNB2(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB2(1, :, est));
        SNR_eNB2(1, index, est) = 40;
        continue
        
    end
    
    while (min(SNR_eNB3(1, :, est)) < -20)
        [value, index]  = min(SNR_eNB3(1, :, est));
        SNR_eNB3(1, index, est) = -20;
        continue
        
    end
    while (max(SNR_eNB3(1, :, est)) > 40)
        [value, index]  = max(SNR_eNB3(1, :, est));
        SNR_eNB3(1, index, est) = 40;
        continue
        
    end
    
%     
    for const=1:3
        if const ==1
            for c=1:z
                
                chcap_siso_single_stream_4Qam_eNB1(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB1(1, c, est))));
                chcap_siso_single_stream_4Qam_eNB2(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB2(1, c, est))));
                chcap_siso_single_stream_4Qam_eNB3(1, c, est) = c_siso_4Qam(find(SNR == round(SNR_eNB3(1, c, est))));
                
            end
        else if const==2
                for c=1:z
                    
                    chcap_siso_single_stream_16Qam_eNB1(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB1(1, c, est))));
                    chcap_siso_single_stream_16Qam_eNB2(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB2(1, c, est))));
                    chcap_siso_single_stream_16Qam_eNB3(1, c, est) = c_siso_16Qam(find(SNR == round(SNR_eNB3(1, c, est))));
                    
                end
            else
                for c=1:z
                    
                    chcap_siso_single_stream_64Qam_eNB1(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB1(1, c, est))));
                    chcap_siso_single_stream_64Qam_eNB2(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB2(1, c, est))));
                    chcap_siso_single_stream_64Qam_eNB3(1, c, est) = c_siso_64Qam(find(SNR == round(SNR_eNB3(1, c, est))));
                    
                end
            end
        end
    end
end

save 'chcap_SISO_Measurements.mat' 'chcap_siso*' 'SNR_eNB*'
% % version 2: do Monte Carlo simulation for every channel
% N0 = double(estimates(1).phy_measurements(1).n0_power(1));
%
% N = 1000;  % No of noise realizations
% for v=1:z
%     seed(1) = 13579;
%     seed(2) = 24680;
%     rand('state', seed(1));
%     randn('state', seed(2));
%     %N0=10^(-SNR(v)/10);
%     %P=10^(SNR(v)/10);
%     sigmasq_x1=1;
%     [stream1]=sqrt(sigmasq_x1)*mapping(M1);     %x1   1x648
%     logsum_siso = 0;
%     %h=sqrt(1/2).*(randn(1,1)+sqrt(-1)*randn(1,1));
%     %h = estimates(1).channel(1,1,i+1)
%     for s1=1:M1
%         x1=stream1(s1);
%         for k=1:N
%             z=sqrt(N0)*sqrt(1/2).*(randn(1,1)+sqrt(-1)*randn(1,1));
%             alpha_siso=h(v);
%             y_siso=alpha_siso*x1+z;
%             den_siso=exp((-1/N0)*norm(y_siso-alpha_siso*x1)^2);
%             neu_siso=0;
%             for g=1:M1;
%                 x1_dd=stream1(g);
%                 neu_siso=neu_siso+exp((-1/N0)*norm(y_siso-alpha_siso*x1_dd)^2);
%             end
%             logsum_siso=logsum_siso+log2(neu_siso/den_siso);
%         end
%     end
%     chcap_siso_single_stream(v)=log2(M1)-logsum_siso/(M1*N);  % For SISO
% end









