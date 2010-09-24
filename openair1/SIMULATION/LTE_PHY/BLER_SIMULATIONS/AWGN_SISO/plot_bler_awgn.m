load 'bler_awgn.mat';

set(0, 'DefaultLineMarkerSize', 10);
set(0, 'Defaultaxesfontsize', 14);
set(0, 'DefaultLineLineWidth', 2);
set(0, 'DefaultAxesFontName', 'Courier');
set(0, 'DefaultTextFontName', 'Courier');
figure(2);
semilogy(bler0(:,1), bler0(:,2), 'b-*')
hold on
grid on
semilogy(bler1(:,1), bler1(:,2),'r-*')
semilogy(bler2(:,1), bler2(:,2),'g-*')
semilogy(bler3(:,1), bler3(:,2), 'y-*')
semilogy(bler4(:,1), bler4(:,2), 'k-*')
semilogy(bler5(:,1), bler5(:,2), '--b*')
semilogy(bler6(:,1), bler6(:,2), '--r*')
semilogy(bler7(:,1), bler7(:,2), '--g*')
semilogy(bler8(:,1), bler8(:,2), '--y*')
semilogy(bler9(:,1), bler9(:,2), '--k*')
semilogy(bler10(:,1), bler10(:,2), 'b-d')
semilogy(bler11(:,1), bler11(:,2), 'r-d')
semilogy(bler12(:,1), bler12(:,2), 'g-d')
semilogy(bler13(:,1), bler13(:,2), 'y-d')
semilogy(bler14(:,1), bler14(:,2), 'k-d')
semilogy(bler15(:,1), bler15(:,2), '--bd')
semilogy(bler16(:,1), bler16(:,2), '--rd')
semilogy(bler17(:,1), bler17(:,2), 'b-s')
semilogy(bler18(:,1), bler18(:,2), 'r-s')
semilogy(bler19(:,1), bler19(:,2), 'g-s')
semilogy(bler20(:,1), bler20(:,2), 'y-s')
semilogy(bler21(:,1), bler21(:,2), 'k-s')
semilogy(bler22(:,1), bler22(:,2), '--bs')
semilogy(bler23(:,1), bler23(:,2), '--rs')
semilogy(bler24(:,1), bler24(:,2), '--gs')
legend('mcs0(QPSK)','mcs1(QPSK)','mcs2(QPSK)','mcs3(QPSK)','mcs4(QPSK)',...
       'mcs5(QPSK)','mcs6(QPSK)','mcs7(QPSK)','mcs8(QPSK)','mcs9(QPSK)',...
       'mcs10(16QAM)','mcs11(16QAM)','mcs12(16QAM)','mcs13(16QAM)',...
       'mcs14(16QAM)','mcs15(16QAM)','mcs16(16QAM)','mcs17(64QAM)',...
       'mcs18(64QAM)','mcs19(64QAM)','mcs20(64QAM)','mcs21(64QAM)',...
       'mcs22(64QAM)','mcs23(64QAM)','mcs24(64QAM)','location','westoutside');
title 'BLER Vs. SNR for SISO LTE with respect to MCS'
ylabel 'BLER'
xlabel 'SNR'