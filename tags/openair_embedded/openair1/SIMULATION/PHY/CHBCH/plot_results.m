close all
clear all

plot_style = {'r-x','r--x';'g-o','g--o';'b-s','b--s'};
%directory = 'results_flat_fading';
directory = '.';

%rice=7;
rice=9;
aoa=2;
ntx=2;
nrx=2;

%%
h_fig = figure(1);
hold off

for rx_type = 0:1
    filename = fullfile(directory,sprintf('results_rx%d_rice%d_aoa%d_ntx%d_nrx%d.csv',rx_type,rice,aoa,ntx,nrx));
    results = importdata(filename);
    results = sortrows(results,[1,-1]);
%     fer1 = reshape(results(:,10),3,3);
%     fer2 = reshape(results(:,11),3,3);
%     semilogy(0:-5:-10,fer1,plot_style{rx_type+1},'Linewidth',1)
    fer1 = reshape(results(:,10),6,3);
    fer2 = reshape(results(:,11),6,3);
    for c=1:3 
        semilogy(6:-3:-9,fer1(:,c),plot_style{c,rx_type+1},'Linewidth',2,'Markersize',10)
        hold on
    end
    %semilogy(6:-3:-9,fer2,plot_style{rx_type+1},'Linewidth',2)
end

% title('spatialy i.i.d. block Rayleigh fading, x1=x2=Rate 1/2 convolutional code, QPSK, 132 bytes')
legend('MMSE, SNR=5dB','MMSE, SNR=10dB','MMSE, SNR=15dB',...
    'ML, SNR=5dB','ML, SNR=10dB','ML, SNR=15dB',...
    'Location','SouthWest');
xlabel('Interference [dB]','Fontsize',14)
ylabel('FER Stream 1','Fontsize',14)
grid on
saveas(h_fig,fullfile(directory,sprintf('results_rice%d_aoa%d.eps',rice,aoa)),'epsc2');

%%
h_fig = figure(2);
hold off
for rx_type = 1
    filename = fullfile(directory,sprintf('results_rx%d_rice%d_aoa%d_ntx%d_nrx%d.csv',rx_type,rice,aoa,ntx,nrx));
    results = importdata(filename);
    results = sortrows(results,[1,-1]);
%     fer1 = reshape(results(:,10),3,3);
%     fer2 = reshape(results(:,11),3,3);
%     semilogy(0:-5:-10,fer1,plot_style{rx_type+1},'Linewidth',1)
    fer1 = reshape(results(:,10),6,3);
    fer2 = reshape(results(:,11),6,3);
    for c=1:3 
        semilogy(6:-3:-9,fer1(:,c),plot_style{c,rx_type+1},'Linewidth',2,'Markersize',10)
        hold on
    end
    %semilogy(6:-3:-9,fer2,plot_style{rx_type+1},'Linewidth',2)
end

% %% results from Rizwan's Simulation for SNR=4.5 dB
% alpha  =    [        0.1                                    0.2               0.3               0.4                0.5               0.6                   0.7                0.8                  0.9                 1.0                  1.5               5.0]; 
% FER1_MMSE = [   0.002380000000000                    0.009120000000000  0.021080000000000  0.038410000000000  0.060290000000000  0.086210000000000   0.114454864356864  0.146000000000000     0.178100000000000   0.208300000000000        0.3787               0.8064];
% 
% semilogy(10*log10(alpha),FER1_MMSE,'b-.','Linewidth',2);
% 
% alpha      =[         0.1             0.15                 0.20               0.3               0.4                0.5               0.6                   0.7                0.8                  0.9                 1.0                 1.5                5.0];
% FER1_ML    =[ 0.000310000000000  0.000670000000000   0.000935000000000  0.001920000000000  0.002400000000000  0.002770000000000  0.002800000000000   0.002810000000000   0.002520000000000    0.002400000000000   0.002170000000000  0.001400000000000   0.000300000000000 ];
% 
% semilogy(10*log10(alpha),FER1_ML,'b:','Linewidth',2);

% %% results from Rizwan's Simulation (without Tx antenna cyxling)
% sigmasq_x2=[5.0 1.5 1.0 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1]; %beta i.e. interference ratio
% 
% SNR=[5];
% FER1mf =  [0.130821559392988   0.182149362477231   0.193199381761978   0.199441563621859   0.204666393778142   0.205086136177194   0.207210940737671   0.207468879668050  0.205086136177194   0.204666393778142   0.190621425848265   0.166500166500166];
% semilogy(10*log10(sigmasq_x2),FER1mf,'b+-','linewidth',2,'markersize',10);
% 
% SNR=[10];
% FER1mf =   [0.021570319240725   0.027013885136960   0.030661679033544   0.031277367696735   0.032922894580892   0.035450935904708   0.037907505686126   0.040680172483931   0.043159257660768   0.045466945530599   0.046015092950488   0.041862022772940];
% semilogy(10*log10(sigmasq_x2),FER1mf,'g+-','linewidth',2,'markersize',10);
% 
% SNR=[15];
% FER1mf =  [0.002873447619923   0.003148277577337   0.003341776889608   0.003365620856079   0.003502332553481   0.003669778638952   0.003824793843612   0.003977598167123   0.004336889582791   0.004839568310507   0.005588153115395  0.006537571422968];
% semilogy(10*log10(sigmasq_x2),FER1mf,'r+-','linewidth',2,'markersize',10);
% 

% %% results from Rizwan's Simulation (with Tx antenna cyxling)

sigmasq_x2=[5.0 1.5 1.0 0.9 0.8 0.7 0.6 0.5 0.4 0.3 0.2 0.1]; %beta i.e. interference ratio

SNR=[5];
FER1mf =  [0.075849514563107   0.119645848289064   0.136388434260775   0.140409997191800   0.143225436837582   0.146886016451234   0.149342891278375   0.151745068285281   0.152951973080453   0.147928994082840   0.135135135135135   0.112714156898106];
semilogy(10*log10(sigmasq_x2),FER1mf,'rx-','linewidth',2,'markersize',10);
 
SNR=[10];
FER1mf =  [0.006704659738518   0.007570137322291   0.008603778779640   0.008786110915864   0.009359439931115   0.009812579727210   0.010791211637243   0.011618450098757   0.012857767377273   0.013881177123820   0.015307840675994   0.015196644580877];
semilogy(10*log10(sigmasq_x2),FER1mf,'go-','linewidth',2,'markersize',10);
 
SNR=[15];
FER1mf = [0.001731583741121            0.001524348417269                    0.001527407805665        0.001561660607424            0.001583255489938            0.001621844296460  0.001627938021144  0.001699489473362  0.001745785673384   0.001861095291801   0.002166894190557             0.002783065602422];
semilogy(10*log10(sigmasq_x2),FER1mf,'bs-','linewidth',2,'markersize',10);
 
% title('spatialy i.i.d. block Rayleigh fading, x1=x2=Rate 1/2 convolutional code, QPSK, 132 bytes')
legend('ML, SNR=5dB','ML, SNR=10dB','ML, SNR=15dB',...
    'ML Matlab SNR=5dB', 'ML Matlab SNR=10dB', 'ML Matlab SNR=15dB',...
    'Location','SouthWest');
xlabel('Interference [dB]','Fontsize',14)
ylabel('FER Stream 1','Fontsize',14)
grid on
saveas(h_fig,fullfile(directory,sprintf('results_matlab_rice%d_aoa%d.eps',rice,aoa)),'epsc2');




