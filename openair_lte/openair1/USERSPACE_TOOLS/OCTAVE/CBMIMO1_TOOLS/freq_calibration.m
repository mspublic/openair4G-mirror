close all
clear all
hold off

% Maxime Guillaud - created Wed May 10 18:08:04 CEST 2006

gpib_card=0;      % first GPIB PCI card in the computer
gpib_device=28;   % this is configured in the signal generator Utilities->System->GPIB->Address menu
freqband=3;            % frequency band used by the openair card (depricated)

cables_loss_dB = 6;    % we need to account for the power loss between the signal generator and the card input (splitter, cables)
dual_tx = 0;

fc = 1907600e3;   % this has to be the same as in the config file
fs = 7680e3;
%fs = 6500e3;
fref = fc+fs/4;
power_dBm=-70;


%gpib_send(gpib_card,gpib_device,'*RST;*CLS');   % reset and configure the signal generator
%gpib_send(gpib_card,gpib_device,['POW ' int2str(power_dBm+cables_loss_dB) 'dBm']);
%gpib_send(gpib_card,gpib_device,['FREQ 1.91860 Ghz']); % set the frequency 
%gpib_send(gpib_card,gpib_device,['FREQ ' int2str(fref/1e3) 'khz']); % set the frequency 
%gpib_send(gpib_card,gpib_device,'OUTP:STAT ON'); % activate output 

%keyboard;

oarf_config(freqband,'config.cfg','scenario.scn',dual_tx)

oarf_set_rx_rfmode(0);
    
oarf_set_rx_gain(70,70,0,0);

sleep(2)

size = 128;
tcxo_freq = 128;

f_off_min = 1e6;
tcxo_freq_min = 256;

do 

  size = size/2;
  tcxo_freq
  oarf_set_tcxo_dac(tcxo_freq);
  sleep(2);
  s=oarf_get_frame(freqband);   %oarf_get_frame

  % find the DC component
  m = mean(s);

  s_phase = unwrap(angle(s(5000:4:length(s),1).'));
  s_phase2 = unwrap(angle(s(5000:4:length(s),2).'));
  s_phase = s_phase - s_phase(1,1);
  s_phase2 = s_phase2 - s_phase2(1,1);

  f_off = mean(s_phase(2:length(s_phase))*fs/4./(1:(length(s_phase)-1))/2/pi)
  f_off2 = mean(s_phase2(2:length(s_phase2))*fs/4./(1:(length(s_phase2)-1))/2/pi)
  plot(1:length(s_phase),s_phase,'r',1:length(s_phase2),s_phase2,'g');
  
  if ((f_off+f_off2)/2 < f_off_min)
    tcxo_freq_min = tcxo_freq; 
    f_off_min = (f_off+f_off2)/2;
  end

  if ((f_off+f_off2)/2 > 0)
    tcxo_freq = tcxo_freq + size;
  else
    tcxo_freq = tcxo_freq - size;
  endif


until (size < 1)

write_tcxo

%gpib_send(gpib_card,gpib_device,'OUTP:STAT OFF');         %  deactivate output


