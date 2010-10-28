function [Ratepersec_4Qam_MUMIMO_1stRx,Ratepersec_4Qam_MUMIMO_2ndRx,Ratepersec_4Qam_MUMIMO_2Rx] = ...
        calc_rps_mu_mimo(estimates_UE1, estimates_UE2)
    
% this function calculates the MU-MIMO rate of the two users
% first we have to determine the compatible subbands (based on PMI feedback)
% on the compatible subbands we should use the MU-MIMO capacity, on the
% others we use the single layer precoding capacity and select the user
% with the higher capacity


