dual_tx=0;
limeparms;
rf_mode = (RXEN+TXEN+TXLPFNORM+TXLPFEN+TXLPF25+RXLPFNORM+RXLPFEN+RXLPF25+LNA1ON+LNAMax+RFBBNORM)*[1 1 1 1];
freq_rx = 1907600000*[1 1 1 1];
freq_tx = freq_rx+1920000;
tx_gain = 25*[1 1 1 1];
rx_gain = 15*[1 1 1 1];
rf_local= [8254744   8255063   8257340   8257340]; %rf_local*[1 1 1 1];
rf_rxdc = rf_rxdc*[1 1 1 1];
rf_vcocal=rf_vcocal*[1 1 1 1];
eNBflag = 1;

oarf_config_exmimo(freq_rx,freq_tx,1,dual_tx,rx_gain,tx_gain,eNBflag,rf_mode,rf_rxdc,rf_local,rf_vcocal)

gpib_card=0;      % first GPIB PCI card in the computer
gpib_device=28;   % this is configured in the signal generator Utilities->System
cables_loss_dB = 6;    % we need to account for the power loss between the signa
power_dBm = -95;

%gpib_send(gpib_card,gpib_device,['POW ' int2str(power_dBm+cables_loss_dB) 'dBm']);
%gpib_send(gpib_card,gpib_device,'OUTP:STAT ON'); %  activate output 


s=oarf_get_frame(0);
f = (7.68*(0:length(s(:,1))-1)/(length(s(:,1))))-3.84;
spec0 = 20*log10(abs(fftshift(fft(s(:,1)))));
spec1 = 20*log10(abs(fftshift(fft(s(:,2)))));

clf
plot(f',spec0,'r',f',spec1,'b')
axis([-3.84,3.84,40,160]);
%gpib_send(gpib_card,gpib_device,'OUTP:STAT OFF'); %  activate output 
legend('Antenna Port 0','Antenna Port 1');
grid
