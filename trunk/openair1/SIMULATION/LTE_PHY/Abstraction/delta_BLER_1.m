function delta=delta_BLER_1(beta)
global SINR_p BLER_meas snr bler SINR_awgn abs_mode modu Pm tx_mode alpha seta

  if (any(beta==0))
    delta=Inf;
return
end

p = 50;
if(abs_mode==0) %MIESM Mapping
    
    eval(['load ' 'siso_MI_abs_' num2str(modu) 'Qam.mat'])
    
    [x y]= size(SINR_p);
    RBIR = [];
    %SINR_eff = [];
    for t=1:1:x
        s = SINR_p(t,:);
        %s(s<-10)=-10;
        %s(s>49)=49;
        
        eval(['SI_p = interp1(newSNR,newC_siso_' num2str(modu) 'QAM,s, ''linear'' , ''extrap'');']);
       RBIR(t) = (sum(SI_p/beta(1))/Pm);

    end
    SINR_eff = interp1(newRBIR, newSNR, RBIR,'linear');
    SINR_eff = SINR_eff .* beta(2);

elseif(abs_mode==2)
    
    eval(['load ' '/root/DEVEL/trunk/openair1/SIMULATION/LTE_PHY/Abstraction/siso_MI_abs_' num2str(modu) 'Qam.mat'])
    
    if(tx_mode~=5)
        error('IA-MIESM only for LTE TM5!!!')
    end
    
    if(modu==4)
        eval(['load ' '/root/DEVEL/trunk/openair1/SIMULATION/LTE_PHY/Abstraction/data_mode5_4_' num2str(modu) '_morerealizations.mat'])
    else
        if(modu==16)
            eval(['load ' '/root/DEVEL/trunk/openair1/SIMULATION/LTE_PHY/Abstraction/data_mode5_16_16.mat'])
        end
    end
    
    [x y]= size(SINR_p);
    RBIR = [];
    for t=1:1:x
        s = SINR_p(t,:);
        s(s<-10)=-10;
        s(s>49)=49;
        
        a = round(alpha(t,:)*100);
        a(a==0) = 1;
        a(a>200)=200;
        b = round(seta(t,:)*100);
        b(b==0) = 1;
        b(b>200)=200;
        
        for u=1:length(a)
            if(modu==16)
                SI_p(u) = data_mode5_16_16(b(u) ,a(u), round(s(u) + 11));
            else
                SI_p(u) = data_mode5_4_4(b(u) ,a(u), round(s(u) + 11));
            end
        end
        
        
        RBIR(t) = (sum(SI_p/beta(1))/Pm);
        
    end
    
    SINR_eff = interp1(newRBIR, newSNR, RBIR,'linear','extrap');
    SINR_eff = SINR_eff .* beta(2);
else  %% EESM Mapping
    
    
    SINR_eff= 10*log10(-1*beta(2)*log((1/p)*sum(exp((-10.^(SINR_p'./10))./beta(1)))));
    
    
end
delta = mean((SINR_awgn - SINR_eff).^2);
