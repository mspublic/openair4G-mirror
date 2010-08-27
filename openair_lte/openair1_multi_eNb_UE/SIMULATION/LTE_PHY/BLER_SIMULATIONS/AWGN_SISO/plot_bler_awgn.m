

load 'bler_awgn.mat';

set(0, 'DefaultLineMarkerSize', 10);
set(0, 'Defaultaxesfontsize', 14);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesFontName', 'Courier');
set(0, 'DefaultTextFontName', 'Courier');
figure(2);
hold off
semilogy(bler0(:,1), (1-bler0(:,2))*get_tbs(0,25)*6*100, 'b-*')
hold on
grid on
semilogy(bler1(:,1), (1-bler1(:,2))*get_tbs(1,25)*6*100,'r-*')
semilogy(bler2(:,1), (1-bler2(:,2))*get_tbs(2,25)*6*100,'g-*')
semilogy(bler3(:,1), (1-bler3(:,2))*get_tbs(3,25)*6*100, 'y-*')
semilogy(bler4(:,1), (1-bler4(:,2))*get_tbs(4,25)*6*100, 'k-*')
semilogy(bler5(:,1), (1-bler5(:,2))*get_tbs(5,25)*6*100, '--b*')
semilogy(bler6(:,1), (1-bler6(:,2))*get_tbs(6,25)*6*100, '--r*')
semilogy(bler7(:,1), (1-bler7(:,2))*get_tbs(7,25)*6*100, '--g*')
semilogy(bler8(:,1), (1-bler8(:,2))*get_tbs(8,25)*6*100, '--y*')
semilogy(bler9(:,1), (1-bler9(:,2))*get_tbs(9,25)*6*100, '--k*')
semilogy(bler10(:,1), (1-bler10(:,2))*get_tbs(10,25)*6*100, 'b-d')
semilogy(bler11(:,1), (1-bler11(:,2))*get_tbs(11,25)*6*100, 'r-d')
semilogy(bler12(:,1), (1-bler12(:,2))*get_tbs(12,25)*6*100, 'g-d')
semilogy(bler13(:,1), (1-bler13(:,2))*get_tbs(13,25)*6*100, 'y-d')
semilogy(bler14(:,1), (1-bler14(:,2))*get_tbs(14,25)*6*100, 'k-d')
semilogy(bler15(:,1), (1-bler15(:,2))*get_tbs(15,25)*6*100, '--bd')
semilogy(bler16(:,1), (1-bler16(:,2))*get_tbs(16,25)*6*100, '--rd')
semilogy(bler17(:,1), (1-bler17(:,2))*get_tbs(17,25)*6*100, 'b-s')
semilogy(bler18(:,1), (1-bler18(:,2))*get_tbs(18,25)*6*100, 'r-s')
semilogy(bler19(:,1), (1-bler19(:,2))*get_tbs(19,25)*6*100, 'g-s')
semilogy(bler20(:,1), (1-bler20(:,2))*get_tbs(20,25)*6*100, 'y-s')
semilogy(bler21(:,1), (1-bler21(:,2))*get_tbs(21,25)*6*100, 'k-s')
legend('mcs0(QPSK)','mcs1(QPSK)','mcs2(QPSK)','mcs3(QPSK)','mcs4(QPSK)',...
       'mcs5(QPSK)','mcs6(QPSK)','mcs7(QPSK)','mcs8(QPSK)','mcs9(QPSK)',...
       'mcs10(16QAM)','mcs11(16QAM)','mcs12(16QAM)','mcs13(16QAM)',...
       'mcs14(16QAM)','mcs15(16QAM)','mcs16(16QAM)','mcs17(64QAM)',...
       'mcs18(64QAM)','mcs19(64QAM)','mcs20(64QAM)','mcs21(64QAM)','location','westoutside');
title 'BLER Vs. SNR for SISO LTE with respect to MCS'
ylabel 'BLER'
xlabel 'SNR'