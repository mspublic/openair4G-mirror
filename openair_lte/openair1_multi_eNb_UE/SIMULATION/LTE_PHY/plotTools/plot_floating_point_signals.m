r_real_a0
r_real_a1
r_imag_a0
r_imag_a1
r_a0 = r_re_a0 + 1i*r_im_a0;
r_a1 = r_re_a1 + 1i*r_im_a1;
figure(7)
subplot(2,2,1);
plot(abs(r_a0))
title('time - antenna 0');
subplot(2,2,2);
plot(abs(r_a1))
title('time - antenna 1');
if (length(r_re_a0) == 640)
subplot(2,2,3);
plot(abs(fft(r_a0(129:640),512)))
title('frequency - antenna 0');  
subplot(2,2,4);
plot(abs(fft(r_a1(129:640),512)))
title('frequency - antenna 1');  
end
