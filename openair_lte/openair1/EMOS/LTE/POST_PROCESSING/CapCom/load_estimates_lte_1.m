function [H, H_fq, gps_data, NFrames, minestimates, throughput, SNR, K_fac] = load_estimates_lte_1(filename, NFrames_max, decimation, is_eNb)
%
% EMOS Single User Import Filter
%
% [H, H_fq, estimates, gps_data, NFrames] =
%       load_estimates_lte(filename, NFrames_max, version)
%
% Parameters:
% filename          - filename(s) of the EMOS data file
% NFrames_max       - Maximum number of estimates. Leave it blank to get up to the
%                     maximum file contents
% decimation        - read every 'decimation' frame
% is_eNb            - if ~= 0 we load data from an eNb
% version           - for backward compatibility (see details below)
%
% Returns:
% estimates         - A structure array containing timestamp, etc
% gps_data          - A structure array containing gps data
% NFrames           - the number of read estimates

% Author: Florian Kaltenberger
% Copyright: Eurecom Sophia Antipolis

% Version History
%   Date      Version   Comment
%   20100317  0.1       Created based on load_estimates

if nargin < 4
    is_eNb = 0;
end
if nargin < 3
    decimation = 1;
end
if nargin < 2
    NFrames_max = Inf;
end

% NTx = 2;
% NRx = 2;
% NFreq = 512;
% NZFreq = 300;

% Logfile structure:
%  - 100 entries of type fifo_dump_emos (defined in phy_procedures_emos.h)
%  - 1 entry of type gps_fix_t defined in gps.h

if exist('/homes/latif/test_devel/openair1/EMOS/LTE/IMPORT_FILTER/a.out','file')
    [dummy,result] = system('/homes/latif/test_devel/openair1/EMOS/LTE/IMPORT_FILTER/a.out')
    eval(result);
else
    warning('File dump_size.c has to be compiled to enable error checking of sizes');
    %PHY_measurements_size = 1120;
    %UCI_data_t_size = 49;
    %DCI_alloc_t_size = 16;
    %eNb_UE_stats_size = 20;
    %fifo_dump_emos_UE_size = 33492;
    %fifo_dump_emos_eNb_size = 36980;
    %gps_fix_t_size = 108;
end

struct_template;

gps_fix_t_size = gps_data_struct_a.size;
fifo_dump_emos_UE_size = fifo_dump_emos_struct_UE_a.size;
fifo_dump_emos_eNb_size = fifo_dump_emos_struct_eNb_a.size;


NO_ESTIMATES_DISK = 100;
if (is_eNb)
    CHANNEL_BUFFER_SIZE = NO_ESTIMATES_DISK * fifo_dump_emos_eNb_size + gps_fix_t_size;
else
    CHANNEL_BUFFER_SIZE = NO_ESTIMATES_DISK * fifo_dump_emos_UE_size + gps_fix_t_size;
end

if (mod(NO_ESTIMATES_DISK,decimation) ~= 0)
    error('Decimation must be a divisor of %d',NO_ESTIMATES_DISK);
end

% Estimate the size of the file for a pre-allocation of memory
if ~iscell(filename)
    filename = {filename};
end
NFiles = length(filename);
NFrames_file = zeros(1,NFiles);
for n=1:NFiles
    if ~exist(filename{n},'file')
        error('File does not exist!')
    end
    info_file = dir(filename{n});
    NFrames_file(n) = floor(info_file.bytes/CHANNEL_BUFFER_SIZE)*NO_ESTIMATES_DISK;
    if (mod(info_file.bytes,CHANNEL_BUFFER_SIZE) ~= 0)
        warning('File size not a multiple of buffer size. File might be corrupt or is_eNb flag is wrong.');
    end
end
NFrames = min(sum(NFrames_file), NFrames_max);

% if (is_eNb)
%     estimates = repmat(fifo_dump_emos_struct_eNb,1,floor(NFrames/decimation));
% else
%     estimates = repmat(fifo_dump_emos_struct_UE,1,floor(NFrames/decimation));
% end

if (is_eNb)
    estimates = repmat(fifo_dump_emos_struct_eNb,1,100);
        
    Ratepersec_4Qam_SISO_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_SISO_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_SISO_1Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_SISO_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_SISO_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_SISO_2Rx = zeros(1,floor(NFrames/100));

    siso_SNR_1Rx = zeros(1,floor(NFrames/100));
    siso_SNR_2Rx = zeros(1,floor(NFrames/100));

    minestimates = [];
    %minestimates = repmat(min_estimates_struct, 1,NFrames);
else
    estimates = repmat(fifo_dump_emos_struct_UE,1,100);
    
    Ratepersec_4Qam_SISO_1stRx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_SISO_1stRx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_SISO_1stRx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_SISO_2ndRx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_SISO_2ndRx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_SISO_2ndRx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_SISO_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_SISO_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_SISO_2Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_alamouti_1stRx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_alamouti_1stRx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_alamouti_1stRx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_alamouti_2ndRx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_alamouti_2ndRx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_alamouti_2ndRx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_alamouti_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_alamouti_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_alamouti_2Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_beamforming_maxq_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_beamforming_maxq_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_beamforming_maxq_1Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_beamforming_maxq_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_beamforming_maxq_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_beamforming_maxq_2Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_beamforming_feedbackq_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_beamforming_feedbackq_1Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_beamforming_feedbackq_1Rx = zeros(1,floor(NFrames/100));
    
    Ratepersec_4Qam_beamforming_feedbackq_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_16Qam_beamforming_feedbackq_2Rx = zeros(1,floor(NFrames/100));
    Ratepersec_64Qam_beamforming_feedbackq_2Rx = zeros(1,floor(NFrames/100));
    
    
    siso_SNR_1stRx = [];
    siso_SNR_2ndRx = [];
    alam_SNR_1stRx = [];
    alam_SNR_2ndRx = [];
    bmfr_maxq_SNR_1Rx = [];
    bmfr_fbq_SNR_1Rx = [];
    
    siso_SNR_2Rx = [];
    alam_SNR_2Rx = [];
    bmfr_maxq_SNR_2Rx = [];
    bmfr_fbq_SNR_2Rx = [];
    
    minestimates = repmat(min_estimates_struct, 1,NFrames);
end
gps_data = repmat(gps_data_struct,1,NFrames/100);

k = 1;
l = 1;
sec = 1;
count =0;
for n=1:NFiles
    fid = fopen(filename{n},'r');
    
    while (~feof(fid) && (k <= min(sum(NFrames_file(1:n)),NFrames_max)))
        
        if (is_eNb)
            estimates_tmp = binread(fid,fifo_dump_emos_struct_eNb,1,4,'l');
 
            minestimates(k).mcs = get_mcs(estimates_tmp.dci_alloc(10,1).dci_pdu,'format0');
            minestimates(k).tbs = get_tbs(minestimates(k).mcs,25);
            minestimates(k).rx_rssi_dBm = estimates_tmp.phy_measurements(1).rx_rssi_dBm(1);
            minestimates(k).frame_tx = estimates_tmp.frame_tx;
            minestimates(k).timestamp = estimates_tmp.timestamp;
            minestimates(k).UE_mode = estimates_tmp.eNb_UE_stats(1).UE_mode;
            minestimates(k).phy_measurements = estimates_tmp.phy_measurements(1);
            minestimates(k).ulsch_errors = estimates_tmp.ulsch_errors;
            minestimates(k).ulsch_fer = estimates_tmp.ulsch_fer;
            minestimates(k).mimo_mode = estimates_tmp.mimo_mode;
            minestimates(k).eNb_id = estimates_tmp.eNb_UE_stats.sector;

        else
            estimates_tmp = binread(fid,fifo_dump_emos_struct_UE,1,4,'l');
            
            count = count +1;
            estimates(1,count) = estimates_tmp;
            minestimates(k).mcs = get_mcs(estimates_tmp.dci_alloc(7,1).dci_pdu);
            minestimates(k).tbs = get_tbs(minestimates(k).mcs,25);
            minestimates(k).rx_rssi_dBm = estimates_tmp.phy_measurements(1).rx_rssi_dBm(1);
            minestimates(k).frame_tx = estimates_tmp.frame_tx;
            minestimates(k).frame_rx = estimates_tmp.frame_rx;
            minestimates(k).pbch_fer = estimates_tmp.pbch_fer(1);
            minestimates(k).timestamp = estimates_tmp.timestamp;
            minestimates(k).UE_mode = estimates_tmp.UE_mode;
            minestimates(k).phy_measurements = estimates_tmp.phy_measurements(1);
            minestimates(k).dlsch_fer = estimates_tmp.dlsch_fer;
            minestimates(k).dlsch_errors = estimates_tmp.dlsch_errors;
            minestimates(k).mimo_mode = estimates_tmp.mimo_mode;
            minestimates(k).eNb_id = estimates_tmp.eNb_id;
        
        end

        % Ricean Factor calculation
             H = estimates_tmp.channel;
             Hc = double(H(1:2:end,:,:))+1j*double(H(2:2:end,:,:));
             Hs = squeeze(10*log10(sum(sum(abs(Hc).^2,1),2)));
             [val, ind] = max(Hs);
             
      
              h11_eNB = double(estimates_tmp.channel(1:2:end,1,ind)) + 1j*double(estimates_tmp.channel(2:2:end,1,ind));
              h12_eNB = double(estimates_tmp.channel(1:2:end,2,ind)) + 1j*double(estimates_tmp.channel(2:2:end,2,ind));
              h21_eNB = double(estimates_tmp.channel(1:2:end,3,ind)) + 1j*double(estimates_tmp.channel(2:2:end,3,ind));
              h22_eNB = double(estimates_tmp.channel(1:2:end,4,ind)) + 1j*double(estimates_tmp.channel(2:2:end,4,ind));
                 
              com_h = [(abs(h11_eNB').^2) (abs(h12_eNB').^2) (abs(h21_eNB').^2) (abs(h22_eNB').^2)];
             
             K_fac(k) = estimate_rice(com_h);
               
        
        %read GPS data and estimates every second
        if ((mod(k,NO_ESTIMATES_DISK)==0) && ~feof(fid))
            gps_data(l) = binread(fid,gps_data_struct,1,4,'l');
            l=l+1;
            count = 0;
            
            if (is_eNb)
                 [Ratepersec_4Qam_SISO_1Rx(sec),Ratepersec_16Qam_SISO_1Rx(sec),Ratepersec_64Qam_SISO_1Rx(sec),siso_SNR_1Rx(sec)] = calc_rps_SISO_UL(estimates,1);,...
                 [Ratepersec_4Qam_SISO_2Rx(sec),Ratepersec_16Qam_SISO_2Rx(sec),Ratepersec_64Qam_SISO_2Rx(sec),siso_SNR_2Rx(sec)] = calc_rps_SISO_UL(estimates,2);
            else
                
                [Ratepersec_4Qam_SISO_1stRx(sec),Ratepersec_16Qam_SISO_1stRx(sec),Ratepersec_64Qam_SISO_1stRx(sec),Ratepersec_4Qam_SISO_2ndRx(sec),...
                    Ratepersec_16Qam_SISO_2ndRx(sec),Ratepersec_64Qam_SISO_2ndRx(sec),siso_SNR_1stRx_persecond,siso_SNR_2ndRx_persecond,...
                    Ratepersec_4Qam_SISO_2Rx(sec),Ratepersec_16Qam_SISO_2Rx(sec),Ratepersec_64Qam_SISO_2Rx(sec),...
                    siso_SNR_2Rx_persecond]  = calc_rps_SISO(estimates);
                
                siso_SNR_1stRx = [siso_SNR_1stRx siso_SNR_1stRx_persecond];
                siso_SNR_2ndRx = [siso_SNR_2ndRx siso_SNR_2ndRx_persecond];
                siso_SNR_2Rx = [siso_SNR_2Rx siso_SNR_2Rx_persecond];
                
                % include results for 1 Rx with other Rx antenna
                [Ratepersec_4Qam_alamouti_1stRx(sec),Ratepersec_16Qam_alamouti_1stRx(sec),Ratepersec_64Qam_alamouti_1stRx(sec),...
                    Ratepersec_4Qam_alamouti_2ndRx(sec),Ratepersec_16Qam_alamouti_2ndRx(sec),Ratepersec_64Qam_alamouti_2ndRx(sec),...
                    alam_SNR_1stRx_persecond,alam_SNR_2ndRx_persecond,Ratepersec_4Qam_alamouti_2Rx(sec),Ratepersec_16Qam_alamouti_2Rx(sec),...
                    Ratepersec_64Qam_alamouti_2Rx(sec),alam_SNR_2Rx_persecond]  = calc_rps_Alamouti(estimates);
                
                alam_SNR_1stRx = [alam_SNR_1stRx alam_SNR_1stRx_persecond];
                alam_SNR_2ndRx = [alam_SNR_2ndRx alam_SNR_2ndRx_persecond];
                alam_SNR_2Rx = [alam_SNR_2Rx alam_SNR_2Rx_persecond];
                
                [Ratepersec_4Qam_beamforming_maxq_1Rx(sec),Ratepersec_16Qam_beamforming_maxq_1Rx(sec),...
                    Ratepersec_64Qam_beamforming_maxq_1Rx(sec),Ratepersec_4Qam_beamforming_feedbackq_1Rx(sec),...
                    Ratepersec_16Qam_beamforming_feedbackq_1Rx(sec),Ratepersec_64Qam_beamforming_feedbackq_1Rx(sec),...
                    bmfr_optmq_SNR_1Rx, bmfr_feedbkq_SNR_1Rx, Ratepersec_4Qam_beamforming_maxq_2Rx(sec),...
                    Ratepersec_16Qam_beamforming_maxq_2Rx(sec),Ratepersec_64Qam_beamforming_maxq_2Rx(sec),...
                    Ratepersec_4Qam_beamforming_feedbackq_2Rx(sec),Ratepersec_16Qam_beamforming_feedbackq_2Rx(sec),...
                    Ratepersec_64Qam_beamforming_feedbackq_2Rx(sec),bmfr_optmq_SNR_2Rx, bmfr_feedbkq_SNR_2Rx]  = calc_rps_Beamforming(estimates);
                
                bmfr_maxq_SNR_1Rx = [bmfr_maxq_SNR_1Rx bmfr_optmq_SNR_1Rx];
                bmfr_fbq_SNR_1Rx = [bmfr_fbq_SNR_1Rx bmfr_feedbkq_SNR_1Rx];
                bmfr_maxq_SNR_2Rx = [bmfr_maxq_SNR_2Rx bmfr_optmq_SNR_2Rx];
                bmfr_fbq_SNR_2Rx = [bmfr_fbq_SNR_2Rx bmfr_feedbkq_SNR_2Rx];
                
            end
            
            sec = sec +1;
            
            
        end
        k=k+1;
    end
    
    fclose(fid);
end

H=[];
H_fq=[];

if (is_eNb)
    throughput.rateps_SISO_4Qam_eNB1_1Rx = Ratepersec_4Qam_SISO_1Rx;
    throughput.rateps_SISO_16Qam_eNB1_1Rx = Ratepersec_16Qam_SISO_1Rx;
    throughput.rateps_SISO_64Qam_eNB1_1Rx = Ratepersec_64Qam_SISO_1Rx;
    throughput.rateps_SISO_4Qam_eNB1_2Rx = Ratepersec_4Qam_SISO_2Rx;
    throughput.rateps_SISO_16Qam_eNB1_2Rx = Ratepersec_16Qam_SISO_2Rx;
    throughput.rateps_SISO_64Qam_eNB1_2Rx = Ratepersec_64Qam_SISO_2Rx;
    SNR.siso_1Rx = siso_SNR_1Rx;
    SNR.siso_2Rx = siso_SNR_2Rx;
else
    
    throughput.rateps_SISO_4Qam_eNB1_1stRx = Ratepersec_4Qam_SISO_1stRx;
    throughput.rateps_SISO_16Qam_eNB1_1stRx = Ratepersec_16Qam_SISO_1stRx;
    throughput.rateps_SISO_64Qam_eNB1_1stRx = Ratepersec_64Qam_SISO_1stRx;
    
    throughput.rateps_SISO_4Qam_eNB1_2ndRx = Ratepersec_4Qam_SISO_2ndRx;
    throughput.rateps_SISO_16Qam_eNB1_2ndRx = Ratepersec_16Qam_SISO_2ndRx;
    throughput.rateps_SISO_64Qam_eNB1_2ndRx = Ratepersec_64Qam_SISO_2ndRx;
    
    throughput.rateps_alamouti_4Qam_eNB1_1stRx = Ratepersec_4Qam_alamouti_1stRx;
    throughput.rateps_alamouti_16Qam_eNB1_1stRx = Ratepersec_16Qam_alamouti_1stRx;
    throughput.rateps_alamouti_64Qam_eNB1_1stRx = Ratepersec_64Qam_alamouti_1stRx;
    
     throughput.rateps_alamouti_4Qam_eNB1_2ndRx = Ratepersec_4Qam_alamouti_2ndRx;
    throughput.rateps_alamouti_16Qam_eNB1_2ndRx = Ratepersec_16Qam_alamouti_2ndRx;
    throughput.rateps_alamouti_64Qam_eNB1_2ndRx = Ratepersec_64Qam_alamouti_2ndRx;
    
    throughput.rateps_beamforming_4Qam_eNB1_1Rx_feedbackq = Ratepersec_4Qam_beamforming_feedbackq_1Rx;
    throughput.rateps_beamforming_16Qam_eNB1_1Rx_feedbackq = Ratepersec_16Qam_beamforming_feedbackq_1Rx;
    throughput.rateps_beamforming_64Qam_eNB1_1Rx_feedbackq = Ratepersec_64Qam_beamforming_feedbackq_1Rx;
    
    throughput.rateps_beamforming_4Qam_eNB1_1Rx_maxq = Ratepersec_4Qam_beamforming_maxq_1Rx;
    throughput.rateps_beamforming_16Qam_eNB1_1Rx_maxq = Ratepersec_16Qam_beamforming_maxq_1Rx;
    throughput.rateps_beamforming_64Qam_eNB1_1Rx_maxq = Ratepersec_64Qam_beamforming_maxq_1Rx;
    
    SNR.siso_1stRx = siso_SNR_1stRx;
    SNR.siso_2ndRx = siso_SNR_2ndRx;
    SNR.alamouti_1stRx = alam_SNR_1stRx;
    SNR.alamouti_2ndRx = alam_SNR_2ndRx;
    SNR.bmfr_maxq_1Rx = bmfr_maxq_SNR_1Rx;
    SNR.bmfr_feedbackq_1Rx = bmfr_fbq_SNR_1Rx;
    
    throughput.rateps_SISO_4Qam_eNB1_2Rx = Ratepersec_4Qam_SISO_2Rx;
    throughput.rateps_SISO_16Qam_eNB1_2Rx = Ratepersec_16Qam_SISO_2Rx;
    throughput.rateps_SISO_64Qam_eNB1_2Rx = Ratepersec_64Qam_SISO_2Rx;
    
    throughput.rateps_alamouti_4Qam_eNB1_2Rx = Ratepersec_4Qam_alamouti_2Rx;
    throughput.rateps_alamouti_16Qam_eNB1_2Rx = Ratepersec_16Qam_alamouti_2Rx;
    throughput.rateps_alamouti_64Qam_eNB1_2Rx = Ratepersec_64Qam_alamouti_2Rx;
    
    throughput.rateps_beamforming_4Qam_eNB1_2Rx_feedbackq = Ratepersec_4Qam_beamforming_feedbackq_2Rx;
    throughput.rateps_beamforming_16Qam_eNB1_2Rx_feedbackq = Ratepersec_16Qam_beamforming_feedbackq_2Rx;
    throughput.rateps_beamforming_64Qam_eNB1_2Rx_feedbackq = Ratepersec_64Qam_beamforming_feedbackq_2Rx;
    
    throughput.rateps_beamforming_4Qam_eNB1_2Rx_maxq = Ratepersec_4Qam_beamforming_maxq_2Rx;
    throughput.rateps_beamforming_16Qam_eNB1_2Rx_maxq = Ratepersec_16Qam_beamforming_maxq_2Rx;
    throughput.rateps_beamforming_64Qam_eNB1_2Rx_maxq = Ratepersec_64Qam_beamforming_maxq_2Rx;
    
    SNR.siso_2Rx = siso_SNR_2Rx;
    SNR.alamouti_2Rx = alam_SNR_2Rx;
    SNR.bmfr_maxq_2Rx = bmfr_maxq_SNR_2Rx;
    SNR.bmfr_feedbackq_2Rx = bmfr_fbq_SNR_2Rx;
end

% H_fq = complex(zeros(NRx,NTx,NFreq/NTx,NFrames));
% H_fq(1,:,:,:) = reshape(chan0,NTx,NFreq/NTx,NFrames);
% H_fq(2,:,:,:) = reshape(chan1,NTx,NFreq/NTx,NFrames);
%
% H = ifft(H_fq,[],3);
%
% % remove zero carriers from Frequency response
% NZFreq_ind = [(176/NTx+1):(256/NTx) 1:(80/NTx)]; % Non-zero frequency indices
% H_fq = H_fq(:,:,NZFreq_ind,:);
%

