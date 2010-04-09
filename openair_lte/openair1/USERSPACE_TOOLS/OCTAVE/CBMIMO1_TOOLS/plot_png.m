close all

load gain_table_v2_7_800_s1

figure(1)
plot(real(NF0),'r;RX0;',real(NF1),'g;RX1;')
title('Noise Figure vs. Digital Gain (Sector 0, CBMIMO1 v2.7)')
xlabel('Digital Gain')
ylabel('Noise Figure (dB)')

figure(2)
plot(real(G0),'r;RX0;',real(G1),'g;RX1;')
title('RF Gain vs. Digital Gain (Sector 0, CBMIMO1 v2.7)')
xlabel('Digital Gain')
ylabel('RF Gain (dB)')

load gain_table_v2_8_800_s2

figure(3)
plot(real(NF0),'r;RX0;',real(NF1),'g;RX1;')
title('Noise Figure vs. Digital Gain (Sector 1, CBMIMO v2.8)')
xlabel('Digital Gain')
ylabel('Noise Figure (dB)')

figure(4)
plot(real(G0),'r;RX0;',real(G1),'g;RX1;')
title('RF Gain vs. Digital Gain (Sector 1, CBMIMO1 v2.8)')
xlabel('Digital Gain')
ylabel('RF Gain (dB)')

load gain_table_v2_13_800_s3


figure(5)
plot(real(NF0),'r;RX0;',real(NF1),'g;RX1;')
title('Noise Figure vs. Digital Gain (Sector 2, CBMIMO v2.13)')
xlabel('Digital Gain')
ylabel('Noise Figure (dB)')

figure(6)
plot(real(G0),'r;RX0;',real(G1),'g;RX1;')
title('RF Gain vs. Digital Gain (Sector 2, CBMIMO1 v2.13)')
xlabel('Digital Gain')
ylabel('RF Gain (dB)')



