close all

N_samples = floor(length(mode2.throughput)/4)*4;
mode2.throughput(~mode2.UE_connected) = 0;
dlsch_throughput4 = reshape(mode2.throughput(1:N_samples),4,[]);
dlsch_throughput_pfair = sum(dlsch_throughput4.^2,1)./sum(dlsch_throughput4,1);

%%
h_fig = figure(1);
hold off
plot(dlsch_throughput4.')
hold on
plot(dlsch_throughput_pfair,'k','Linewidth',2)
ylabel('Throughput [bps]')
ylabel('Time [sec]')
saveas(h_fig,fullfile(pathname,'pfair_throughput_4users.eps'),'epsc2');


%%
h_fig = figure(2);
hold off
colors = {'b','g','r','c','m','y','k','b--','g--','r--','c--','m--','y--','k--'};
for n = 1:4
    [f,x] = ecdf(dlsch_throughput4(n,:));
    plot(x,f,colors{n});
    hold on
end
[f,x] = ecdf(dlsch_throughput_pfair);
plot(x,f,'k','Linewidth',2);
legend('User 1','User 2','User 3','User 4','Sum rate');
title('DL Throughput CDF')
xlabel('Throughput [bps]')
ylabel('P(x<abscissa)')
grid on
saveas(h_fig,fullfile(pathname,'pfair_throughput_cdf_4users.eps'),'epsc2');

%% throughput over ricean factor
h_fig = h_fig+1;
figure(h_fig);
hold off
plot(10*log10(mode2_ideal.K_fact_cat),mode2.throughput, 'x');
xlabel('Ricean K-factor [dB]')
ylabel('Throughput [bps]')
saveas(h_fig,fullfile(pathname,'throughput_vs_Kfactor.eps'),'epsc2');
