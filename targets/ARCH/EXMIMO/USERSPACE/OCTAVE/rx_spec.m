dual_tx=0;
card=0;
limeparms;
chan_sel = zeros(1,4);
chan_sel(ch) = 1;
%chan_sel = [1 0 1 0];
%rf_mode = (RXEN+TXEN+TXLPFNORM+TXLPFEN+TXLPF25+RXLPFNORM+RXLPFEN+RXLPF25+LNA1ON+LNAMax+RFBBNORM)*[1 1 1 1];
rf_mode = (RXEN+TXLPFNORM+TXLPFEN+TXLPF25+RXLPFNORM+RXLPFEN+RXLPF25+LNA1ON+LNAMax+RFBBNORM+DMAMODE_RX)*chan_sel;
freq_rx = 1907591325*[1 1 1 1];
%freq_rx = 1912600000*[1 1 1 1];
%freq_rx = 859500000*[1 1 1 1];
freq_tx = freq_rx;
tx_gain = 0*[1 1 1 1];
rx_gain = 20*[1 1 1 1];
rf_local= rf_local*[1 1 1 1];
rf_rxdc = rf_rxdc*[1 1 1 1];
%rf_vcocal=rf_vcocal_859*[1 1 1 1];
rf_vcocal=rf_vcocal_19G*[1 1 1 1];
eNBflag = 0;
tdd_config = DUPLEXMODE_FDD + TXRXSWITCH_TESTRX; 
%tdd_config = DUPLEXMODE_FDD + TXRXSWITCH_LSB;
syncmode = SYNCMODE_FREE;
rffe_rxg_low = 61*[1 1 1 1];
rffe_rxg_final = 61*[1 1 1 1];
rffe_band = B19G_TDD*[1 1 1 1];

oarf_config_exmimo(card,freq_rx,freq_tx,tdd_config,syncmode,rx_gain,tx_gain,eNBflag,rf_mode,rf_rxdc,rf_local,rf_vcocal,rffe_rxg_low,rffe_rxg_final,rffe_band)

gpib_card=0;      % first GPIB PCI card in the computer
gpib_device=28;   % this is configured in the signal generator Utilities->System
cables_loss_dB = 6;    % we need to account for the power loss between the signa
power_dBm = -95;

%gpib_send(gpib_card,gpib_device,['POW ' int2str(power_dBm+cables_loss_dB) 'dBm']);
%gpib_send(gpib_card,gpib_device,'OUTP:STAT ON'); %  activate output 


s=oarf_get_frame(card);
f = (7.68*(0:length(s(:,1))-1)/(length(s(:,ch))))-3.84;
spec0 = 20*log10(abs(fftshift(fft(s(:,ch)))));
%spec1 = 20*log10(abs(fftshift(fft(s(:,4)))));

clf
plot(f',spec0,'r');
%hold on
%plot(f',spec1,'b');
%hold off
axis([-3.84,3.84,40,160]);
%gpib_send(gpib_card,gpib_device,'OUTP:STAT OFF'); %  activate output 
legend('Antenna Port 0','Antenna Port 1');
grid
