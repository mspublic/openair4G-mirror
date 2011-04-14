function delta=delta_BLER(beta)

global SINR_p BLER_meas snr bler SINR_awgn

p = 50;

SINR_eff= 10*log10(-1*beta*log((1/p)*sum(exp((-10.^(SINR_p'./10))./beta))));

BLER_pred = interp1(snr,bler,SINR_eff,'cubic');
BLER_pred(SINR_eff<snr(1)) = 1;
BLER_pred(SINR_eff>snr(end)) = 0;
%if any(isnan(BLER_pred))
%    error('shold never happen')
%end

BLER_pred =BLER_pred';

delta=sum((BLER_pred - BLER_meas).^2);

%SINR_eff= 10*log10(-1*beta*log((1/p)*sum(exp((-10.^(SINR_p'./10))./beta))));

%delta = mean((SINR_awgn - SINR_eff).^2);