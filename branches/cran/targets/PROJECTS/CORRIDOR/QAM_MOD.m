function [sig,sig_length] = QAM_MOD(size,input)

% sig - output symbols
% size - modulation size (4,16,256)
% input - vector of bytes to be modulated

AM2 = [-1 1];
AM4 = [-3 -1 1 3]; AM4 = 2*AM4/sqrt(AM4*AM4');
AM16 = [-15 -13 -11 -9 -7 -5 -3 -1 1 3 5 7 9 11 13 15]; AM16 = 4*AM16/sqrt(AM16*AM16');

sig = zeros(1,length(input));
sig_length = length(input);

for l=1:length(input)
    if (size == 256)
        sig(l) = (AM16(1+ floor((input(l)/16))) + sqrt(-1)*AM16(1+rem(input(l),16)))/sqrt(2);
        
    elseif (size == 16)
        sig(l) = (AM4(1+rem(floor(input(l)/4),4)) + sqrt(-1)*AM4(1+rem(input(l),4)))/sqrt(2);
        
    elseif (size == 4)
        sig(l) = (AM2(1+rem(floor(input(l)/2),2)) + sqrt(-1)*AM2(1+rem(input(l),2)))/sqrt(2);
        
    end
end
