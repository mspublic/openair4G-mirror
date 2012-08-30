%% Header
clear all;
close all;

addpath('../../../PHY/LTE_TRANSPORT/mexfiles');
addpath('../../../PHY/TOOLS/mexfiles');
addpath('../../../SIMULATION/TOOLS/mexfiles');

%% System parameters
nt = 2;
nr = 2;
N = 100; % number of frames (codewords)
nSNR = 1;
SNRdB = 30;
% nSNR = 4;
% SNRdB = linspace(100,103,nSNR);
MCS = [16 16]; % MCS for the 2 users
j = sqrt(-1);
amp = 1/32;
XFORMS = 0;

%% Initialize simparms
simparms = InitSimparms( nt, nr, MCS, N, SNRdB);
[tmp simparms.tseeds] = taus(1,simparms.tseeds);
data_idx = [901:1400 1501:2300 2401:3500 3601:4200];
data_idx_int = [1801:2800 3001:4600 4801:7000 7201:8400];
data_idx_int_r = data_idx_int(1:2:length(data_idx_int));
data_idx_int_i = data_idx_int(2:2:length(data_idx_int));

%% Allocate memory
llr0 = zeros(simparms.codeword(1).G, 1,'int16');
y_fxp = zeros(simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,simparms.nb_re_per_frame,'int16');
y_fxp_t = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
ymf0 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
% Effective channel will contain the channel estimate at pilot positions
Heff0 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');

Hmag0 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
Hmagb0 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
ymf1 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
Heff1 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
Hmag1 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
Hmagb1 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');
rho10 = zeros(2*simparms.nb_re_per_frame,simparms.NB_ANTENNAS_RX*simparms.NB_ANTENNAS_TX,'int16');

%% XFORMS
if (XFORMS)
	scrsz = get(0,'ScreenSize');
	figure('Position',[1 scrsz(4)/2 scrsz(3)/2 scrsz(4)/2])
	subplot(1,2,1);
	xlim([1 simparms.codeword(1).G]);
	title('LLRs of UE 0');
	
	fig_llr0 = scatter(1:simparms.codeword(1).G,llr0,'.','YDataSource','llr0');
	
	subplot(1,2,2);
	title('MF output of UE 0');
	fig_ymf = scatter(ymf0(data_idx_int_r),ymf0(data_idx_int_i),'.','XDataSource','ymf0(data_idx_int_r)','YDataSource','ymf0(data_idx_int_i)');
end

%% Encode and modulate transmit signal
[data0 simparms.tseeds] = taus(simparms.codeword(1).TBS/8,simparms.tseeds);
[data1 simparms.tseeds] = taus(simparms.codeword(2).TBS/8,simparms.tseeds);

% Add 4 bytes CRC
data0 = [data0; zeros(4,1,'uint8')];
data1 = [data1; zeros(4,1,'uint8')];

edata0 = dlsch_encoding(data0,simparms,simparms.codeword(1)); % user 1
edata1 = dlsch_encoding(data1,simparms,simparms.codeword(2)); % user 2

% Modulation
edata_enc(:,1) = simparms.codeword(1).base2*double(reshape(edata0,simparms.codeword(1).mod_order,simparms.nb_re));
edata_enc(:,2) = simparms.codeword(2).base2*double(reshape(edata1,simparms.codeword(2).mod_order,simparms.nb_re));
s(:,1) = simparms.codeword(1).const(edata_enc(:,1)+1);
s(:,2) = simparms.codeword(2).const(edata_enc(:,2)+1);

Ptx = sum(diag(s'*s))/simparms.nb_re; % average transmit power per RE

%% Loop over Frames
for iSNR=1:length(simparms.snr)
	cSNR = simparms.snr(iSNR); % current SNR
	for n=1:simparms.n_frames
		%% Random data
% 		data0 = uint8(randi([0,255],simparms.codeword(1).TBS/8+4,1));
% 		data1 = uint8(randi([0,255],simparms.codeword(2).TBS/8+4,1));
		
% 		[data0 tseeds] = taus(simparms.codeword(1).TBS/8+4,tseeds);
% 		[data1 tseeds] = taus(simparms.codeword(2).TBS/8+4,tseeds);		
		
		%% Encode data
% 		edata0 = dlsch_encoding(data0,simparms,simparms.codeword(1)); % user 1
% 		edata1 = dlsch_encoding(data1,simparms,simparms.codeword(2)); % user 2
				
% 		dlsch0_e;
% 		dlsch3_e;
% 		edata0 = e0;
% 		edata1 = e1;
		
		%% Modulation
% 		edata_enc(:,1) = simparms.codeword(1).base2*double(reshape(edata0,simparms.codeword(1).mod_order,simparms.nb_re));
% 		edata_enc(:,2) = simparms.codeword(2).base2*double(reshape(edata1,simparms.codeword(2).mod_order,simparms.nb_re));
% 		s(:,1) = simparms.codeword(1).const(edata_enc(:,1)+1);
% 		s(:,2) = simparms.codeword(2).const(edata_enc(:,2)+1);		
		
		%% Channel
		% Rayleigh channel, constant for whole codeblock/frame
		H = (randn(2,nt) + j*randn(2,nt))/sqrt(2);
% 		H(:,1) = H(:,2);
% 		H = eye(2);
% 		H = [0.032345 + j*0.284367 0.238980 + j*1.018463; 0 ,0];
% 		h00 = 0.0323448  + j*0.2843666;
% 		h10 = 0.23897994 + j*1.01846337;
% 		h01 = -0.1211888495 + j*1.02075798;
% 		h11 = 0.51452828 - j*0.8063001067;
% 		H = [h00 h10; h01 h11];
		
		if (nr==1)
			H(2,:) = 0;
		end				
		
		%% PMI computation
		pmi = 1;
		r = H(:,2)'*H(:,1);
        rr = real(r);
        ri = imag(r);
		if (rr>ri && rr>-ri)
			pmi = 1;
		elseif (rr<ri && rr>-ri)
			pmi = 3;
		elseif (rr<ri && rr<-ri)
			pmi = 2;
		elseif (rr>ri && rr<-ri)
			pmi = 4;			
		end
        
        % opposite pmi       
		if(pmi<3)
			pmi_o = mod(pmi,2) + 1;
		else
			pmi_o = mod(pmi,2) + 3;
		end
		
		pmi_ext = uint8(repmat(pmi-1,1,simparms.nb_rb));
		pmi_ext_o = uint8(repmat(pmi_o-1,1,simparms.nb_rb));

        % precoding vectors
        g0 = simparms.CB(:,pmi);
        g1 = simparms.CB(:,pmi_o);
		
		%% Transmit signal (frequency domain)
		x = [g0,g1]*conj(s')/sqrt(2); % E[|x|^2] = 2*amp^2
		% 		Ptx = sum(diag(x'*x))/simparms.nb_re;
% 		Ptx = 2*amp^2;
		
		%% Noise		
		sigma2 = (Ptx/cSNR);
		% E[|noise|^2] = 2*amp^2/sqrt(SNR) -> SNR=1/sigma^2
		noise = sqrt(sigma2)*(randn(2,simparms.nb_re) + j*randn(2,simparms.nb_re))/sqrt(2);
		if (nr==1)			
			noise(2,:) = 0;
		end		
		
% 		Pn = sum(diag(noise'*noise))/simparms.nb_re;
% 		disp(10*log10(Ptx/Pn));
		
		%% Received signal
		y = H*x + noise;
		
		% Quantization
		y_fxp_data = int16(floor(y*pow2(15)));
		H_fxp = int16(fix(H*floor(amp*pow2(15)))); % Perfect Chest		
		
		% insert dummy pilots		
		y_fxp(1:2,901:1400) = y_fxp_data(:,1:500);
		y_fxp(1:2,1501:2300) = y_fxp_data(:,501:1300);
		y_fxp(1:2,2401:3500) = y_fxp_data(:,1301:2400);
		y_fxp(1:2,3601:end) = y_fxp_data(:,2401:end);
				
		
		% reorder for processing
		H_fxp_t = repmat([real(H_fxp(:)) imag(H_fxp(:))]',simparms.nb_re_per_frame,1);
		y_fxp_t(:,1) = reshape([real(y_fxp(1,:)); imag(y_fxp(1,:))],2*simparms.nb_re_per_frame,1);		
		y_fxp_t(:,2) = reshape([real(y_fxp(2,:)); imag(y_fxp(2,:))],2*simparms.nb_re_per_frame,1);
		
		
		%% Compute Scaling
		avg = dlsch_channel_level_prec(H_fxp_t,pmi_ext,simparms);
		simparms.log2_maxh = max(double(log2_approx(avg))-13,0);		
				
		%% Inner receiver loop
		llrp = 1;
		for slot = 4:14
			idxs = 2*(slot-1)*simparms.nb_re_per_symbol + 1;
			idxe = 2*(slot-1)*simparms.nb_re_per_symbol + 2*simparms.nb_re_per_symbol;
			
			%% Preprocessing
			[ymf0(idxs:idxe,:)...
				Heff0(idxs:idxe,:)...
				Hmag0(idxs:idxe,:)...
				Hmagb0(idxs:idxe,:)]...
				= dlsch_channel_compensation_prec(y_fxp_t,H_fxp_t,pmi_ext,simparms,simparms.codeword(1),slot-1);
			
			% Interfering user
			[ymf1(idxs:idxe,:)...
				Heff1(idxs:idxe,:)...
				Hmag1(idxs:idxe,:)...
				Hmagb1(idxs:idxe,:)]...
				= dlsch_channel_compensation_prec(y_fxp_t,H_fxp_t,pmi_ext_o,simparms,simparms.codeword(2),slot-1);
			
			% Correlation coefficient
			rho10(idxs:idxe,:) = dlsch_dual_stream_correlation(Heff0,Heff1,simparms,slot-1);
			
			if (nr>1)
				[ymf0(idxs:idxe,:)...
					ymf1(idxs:idxe,:)...
					Hmag0(idxs:idxe,:)...
					Hmagb0(idxs:idxe,:)...
					Hmag1(idxs:idxe,:)...
					Hmagb1(idxs:idxe,:)...
					rho10(idxs:idxe,:)]...
					= dlsch_detection_mrc(ymf0,ymf1,Hmag0,Hmag1,Hmagb0,Hmagb1,rho10,simparms,slot-1);								
			end
			
			
			%% LLR computation
			llr = dlsch_mu_mimo_llr(ymf0(idxs:idxe,1),...
				ymf1(idxs:idxe,1),...
				Hmag0(idxs:idxe,1),...
				Hmag1(idxs:idxe,1),...
				rho10(idxs:idxe,1),...
				simparms,...
				slot-1);
			
			llr0(llrp:llrp+length(llr)-1,:) = llr;
			llrp = llrp + length(llr);
		end
		
		if (XFORMS)
			refreshdata(fig_llr0,'caller');
			drawnow;
			
			refreshdata(fig_ymf,'caller');
			drawnow; pause(0.1);
		end
		
% 		ymf00 = double(reshape(ymf0(:,1),2,simparms.nb_re_per_frame));		
% 		ymf00 = int16(ymf00(1,:) + j*ymf00(2,:));
% 		ymf00 = ymf00(:);
% 		
% 		ymf10 = double(reshape(ymf1(:,1),2,simparms.nb_re_per_frame));		
% 		ymf10 = int16(ymf10(1,:) + j*ymf10(2,:));
% 		ymf10 = ymf10(:);
		
		%% Channel decoding
		llr0 = -llr0; % invert (since no scrambling applied)				
		ret0 = dlsch_decoding(llr0,simparms,simparms.codeword(1));						
		

		
		if (ret0 > simparms.MAX_TURBO_ITERATIONS)
			simparms.frame_errors(iSNR,1) = simparms.frame_errors(iSNR,1) + 1;
			% HD
% 			hd = llr0>0;
% 			biterrors = sum(hd~=edata0);
% 			llrcomp(:,1) = llr0;
% 			llrcomp(:,2) = edata0;
% 			llrcomp(:,3) = hd~=edata0;
% 			maxllrwrong = max(abs(llr0(logical(llrcomp(:,3)))));
% 			fprintf('Frame: %d \t biterrors: %d\n',n, biterrors);
% 			fprintf('Unable to decode frame %d\n',n);
% 			fprintf('log2_maxh = %d\n',simparms.log2_maxh);
		end
		

		
	end
	fprintf('********************SNR = %3.1f dB processed on %s********************\n',real(simparms.snr_db(iSNR)),datestr(now));
	fprintf('Errors: %d/%d\n',simparms.frame_errors(iSNR,1),N);	
end

%% Post Processing
% total_frame_erros = sum(frame_errors,2);
% fer = total_frame_erros/N;
% figure; semilogy(SNRdB,fer); grid;
% 
% disp(fer);

% profile viewer 


