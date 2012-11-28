La = 5;
M = 512;
L = 30;
ant = 2;
frames = 201:210;
frames2 = 501:510;

%%
%H_UE2 = reshape(permute(H_UE(frames,2049:2560,:),[2 1 3]),512,[],2);
H_UE2 = reshape(permute(H_UE(:,2561:3072,:),[2 1 3]),512,[],2);
H_UE3 = complex(zeros(size(H_UE2)));
H_UE3(363:512,:,:) = H_UE2(6:155,:,:); 
H_UE3(1:151,:,:) = H_UE2(156:306,:,:);
H_UE_t = ifft(conj(H_UE3),512,1); %the channel estimates are stored in conjugated format
H_UE_t = circshift(H_UE_t,[-15,0,0]); %shift the peak to the desired position
figure(2)
hold off
%waterfall(squeeze(20*log10(abs(H_UE_t))).')
plot(squeeze(20*log10(abs(H_UE_t(:,frames,ant)))))
title('PDP DL channel')

%%
H_eNB2 = reshape(permute(H_eNB(:,1501:1800,:),[2 1 3]),300,[],2);
H_eNB3 = complex(zeros(512,NFrames,2));
H_eNB3(363:512,:,:) = H_eNB2(1:150,:,:);
H_eNB3(1:150,:,:) = H_eNB2(151:300,:,:);
H_eNB_t = ifft(H_eNB3,512,1);
H_eNB_t = circshift(H_eNB_t,[8,0,0]);
figure(3)
hold off
%waterfall(squeeze(20*log10(abs(H_eNB_t))).')
plot(squeeze(20*log10(abs(H_eNB_t(:,frames,ant)))))
title('PDP UL channel')

%% compensate the time drift
Lanczos=@(x,a)sinc(x).*sinc(x/a);
Lt=@(t)toeplitz([Lanczos((0:-1:-La)+t,La).'; zeros(M-1-La,1)],[Lanczos((0:1:La)+t,La) zeros(1,M-1-La)]);

abs_timing_offset=double([estimates_UE(1:1000).timing_offset]);
slope = (abs_timing_offset(1000)-abs_timing_offset(122))./(1000-122);
rel_timing_offset=abs_timing_offset-abs_timing_offset(122)-slope*(-121:1000-122);

for i=1:NFrames
    for j=1:2
    H_UE_t(:,i,j) = Lt(rel_timing_offset(i))*H_UE_t(:,i,j);
    H_eNB_t(:,i,j) = Lt(-rel_timing_offset(i))*H_eNB_t(:,i,j);
    end
end

H_UE_t = H_UE_t(1:L,:,:);
H_eNB_t = H_eNB_t(1:L,:,:);

figure(12)
hold off
waterfall(squeeze(20*log10(abs(H_UE_t(:,frames,ant)))).')
title('PDP compensated DL channel')
figure(13)
hold off
waterfall(squeeze(20*log10(abs(H_eNB_t(:,frames,ant)))).')
title('PDP compensated UL channel')

%%
addpath('E:\Synchro\kaltenbe\My Documents\Matlab\reciprocity')
fprintf('\ntlsdeconvangles\n')
for j=1:2
    [p(:,j),phi_hat,costnew(j)]=tlsdeconvangles(H_UE_t(:,frames,j),H_eNB_t(:,frames,j),20);
    phi_hat_last(:,j) = phi_hat(:,end);
end

%%
colors=['b' 'g' 'r' 'c' 'm' 'y' 'k' 'b' 'g' 'r'];

figure(5)
clf; hold on
niter=size(phi_hat,2);
for k=2:size(phi_hat,1);
  polar(phi_hat(k,:),1:niter,colors(k-1));
  %polar(-reference(k)/180*pi,niter,[colors(k-1) 'o']);
end

figure(6)
hold off
plot(abs(p))
title('p')

%% test the estimated reciprocity filter p by applying it to a differnt part of the measurements
g = H_eNB_t(:,frames2,:);
h = H_UE_t(:,frames2,:);
% reconstruct g from h
for j=1:2
g_hat(:,:,j) = filter(p(:,j),1,h(:,:,j));
% for i=1:length(frames)
%     g_hat(:,i,j) = exp(-1j*phi_hat_last(i,j)).*g_hat(:,i,j);
% end
end
G = fft(g,512,1);
G_hat = fft(g_hat,512,1);

% normalize G and G_hat
scale_G = sqrt(mean(mean(mean(abs(G).^2,1),2),3));
scale_G_hat = sqrt(mean(mean(mean(abs(G_hat).^2,1),2),3));

G=G./scale_G;
G_hat = G_hat./scale_G_hat;

addpath('E:\Synchro\kaltenbe\My Documents\Matlab\reciprocity\ICC paper');
SNR_vec_dB = -10:10:40;
SNR_vec = 10.^(SNR_vec_dB./10);
C_CSIR = zeros(512,length(frames2),length(SNR_vec));
C_CSIT_corr = zeros(512,length(frames2),length(SNR_vec));
C_CSIT_est = zeros(512,length(frames2),length(SNR_vec));
for i=1:512
    for j=1:length(frames2)
        [C_CSIR(i,j,:), C_CSIT_corr(i,j,:), C_CSIT_est(i,j,:)] = cap_miso(squeeze(G(i,j,:)).', squeeze(G_hat(i,j,:)).', SNR_vec.');
    end
end

scale_C = 1/(1e-3/14) * 1/4.5e6;

figure(10)
hold off
plot(SNR_vec_dB,scale_C*squeeze(mean(sum(C_CSIR,1),2)));
hold on
plot(SNR_vec_dB,scale_C*squeeze(mean(sum(C_CSIT_corr,1),2)),'r');
plot(SNR_vec_dB,scale_C*squeeze(mean(sum(C_CSIT_est,1),2)),'g');
legend('CSIR only', 'CSIR + estimated CSIT', 'CSIR + perfect CSIT')
xlabel('SNR [dB]')
ylabel('Capacity [bit/sec/Hz]')

%%
figure(7)
waterfall(squeeze(20*log10(abs(g(:,:,ant)))).')
zlim([0 50])
title('|g|^2 [dB]')

figure(8)
waterfall(squeeze(20*log10(abs(g_hat(:,:,ant)))).')
zlim([0 50])
title('|\hat{g}|^2 [dB]')

figure(9)
hold off
mse = abs(g-g_hat).^2;
surf(squeeze(10*log10(mse(:,:,ant))).')
title('mse [dB]')