rxsig0
rxsig1

CHSCH1_sync_4xf0
CHSCH1_sync_4xf1
SCH2_sync_4xf0
SCH2_sync_4xf1

%chsch0_syncf

%chbch10_sig
%chbch11_sig
%rxs0 = chbch10+chbch11;

%sch2_tx0
%sch2_tx1
%rxs0 = sch2tx0+sch2tx1;

c0 = abs(conv(rxs0,fliplr(ifft(chsch1_sync_f_4x0)')));
c1 = abs(conv(rxs0,fliplr(ifft(chsch1_sync_f_4x1)')));
csum = (c0.^2) + (c1.^2);

figure(1)
plot(csum)

[maxlev0,maxpos0] = max(csum);
sync_pos = maxpos0-1024

c0 = abs(conv(rxs0,fliplr(ifft(sch2_sync_f_4x0)')));
c1 = abs(conv(rxs0,fliplr(ifft(sch2_sync_f_4x1)')));
csum = (c0.^2) + (c1.^2);

figure(2)
plot(csum)

[maxlev0,maxpos0] = max(csum);
sync_pos = maxpos0-1024
