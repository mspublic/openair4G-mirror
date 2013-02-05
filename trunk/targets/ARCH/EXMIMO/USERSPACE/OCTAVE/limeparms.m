RXEN=1;
TXEN=2;

TXLPFENMASK=4;
TXLPFEN=4;


TXLPFMASK    =15*(2^3);
TXLPF14      =0;
TXLPF10      =1*(2^3);
TXLPF7       =2*(2^3);
TXLPF6       =3*(2^3);
TXLPF5       =4*(2^3);
TXLPF4375    =5*(2^3);
TXLPF35      =6*(2^3);
TXLPF3       =7*(2^3);
TXLPF275     =8*(2^3);
TXLPF25      =9*(2^3);
TXLPF192     =10*(2^3);
TXLPF15      =11*(2^3);
TXLPF1375    =12*(2^3);
TXLPF125     =13*(2^3);
TXLPF0875    =14*(2^3);
TXLPF075     =15*(2^3);
RXLPFENMASK=1*(2^7);
RXLPFEN    =128;
RXLPFMASK  =15*(2^8);
RXLPF14    =0;
RXLPF10    =1*(2^8);
RXLPF7     =2*(2^8);
RXLPF6     =3*(2^8);
RXLPF5     =4*(2^8);
RXLPF4375  =5*(2^8);
RXLPF35    =6*(2^8);
RXLPF3     =7*(2^8);
RXLPF275   =8*(2^8);
RXLPF25    =9*(2^8);
RXLPF192   =10*(2^8);
RXLPF15    =11*(2^8);
RXLPF1375  =12*(2^8);
RXLPF125   =13*(2^8);
RXLPF0875  =14*(2^8);
RXLPF075   =15*(2^8);
LNAMASK=3*(2^12);
LNADIS =0;
LNA1ON =1*(2^12);
LNA2ON =2*(2^12) ;
LNA3ON =3*(2^12);
LNAGAINMASK=3*(2^14);
LNAByp    =1*(2^14);
LNAMed    =2*(2^14);
LNAMax    =3*(2^14);

RFBBMASK  =7*(2^16);
RFBBNORM  =0;
RFBBRXLPF =1*(2^16);
RFBBRXVGA =2*(2^16);
RFBBOUTPUT=3*(2^16);
RFBBLNA1  =4*(2^16);
RFBBLNA2  =5*(2^16);
RFBBLNA3  =6*(2^16);

RXLPFMODEMASK=3*(2^19);
RXLPFNORM    =0;
RXLPFBYP     =1*(2^19);
RXLPFBPY2    =3*(2^19);

TXLPFMODEMASK=1*(2^21);
TXLPFNORM    =0;
TXLPFBYP     =1*(2^21);

RXOUTSW      =1*(2^22);

DMAMODE_TRXMASK =3*(2^23);
DMAMODE_RX      =1*(2^23);
DMAMODE_TX      =2*(2^23);

rf_local  = 31 + 31*(2^6) + 31*(2^12) + 31*(2^18);
rf_rxdc   = 128 + 128*(2^8); % DC offset ( DCOFF_I_RXFE [6:0], DCOFF_Q_RXFE[14:8] )
rf_vcocal = ((0xE)*(2^6)) + (0xE); % VCO calibration values for 1.9 GHz
