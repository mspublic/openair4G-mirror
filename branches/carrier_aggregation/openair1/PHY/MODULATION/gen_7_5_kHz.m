s6 = floor(32767*exp(sqrt(-1)*2*pi*(0:255)*7.5e3/1.92e6));
s6_2 = zeros(1,512);
s6_2(1:2:end) = real(s6);
s6_2(2:2:end) = imag(s6);

s15 = floor(32767*exp(sqrt(-1)*2*pi*(0:511)*7.5e3/3.84e6));
s15_2 = zeros(1,1024);
s15_2(1:2:end) = real(s15);
s15_2(2:2:end) = imag(s15);

s25 = floor(32767*exp(sqrt(-1)*2*pi*(0:1023)*7.5e3/7.68e6));
s25_2 = zeros(1,2048);
s25_2(1:2:end) = real(s25);
s25_2(2:2:end) = imag(s25);

s50 = floor(32767*exp(sqrt(-1)*2*pi*(0:2047)*7.5e3/15.36e6));
s50_2 = zeros(1,4096);
s50_2(1:2:end) = real(s50);
s50_2(2:2:end) = imag(s50);

s75 = floor(32767*exp(sqrt(-1)*2*pi*(0:3839)*7.5e3/23.04e6));
s75_2 = zeros(1,7680);
s75_2(1:2:end) = real(s75);
s75_2(2:2:end) = imag(s75);

s100 = floor(32767*exp(sqrt(-1)*2*pi*(0:4095)*7.5e3/30.72e6));
s100_2 = zeros(1,8192);
s100_2(1:2:end) = real(s100);
s100_2(2:2:end) = imag(s100);

fd=fopen("kHz_7_5.h","w");
fprintf(fd,"s6_kHz_7_5 = {");
fprintf(fd,"%d,",s6_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s6_2(end));

fprintf(fd,"s15_kHz_7_5 = {");
fprintf(fd,"%d,",s15_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s15_2(end));

fprintf(fd,"s25_kHz_7_5 = {");
fprintf(fd,"%d,",s25_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s25_2(end));

fprintf(fd,"s50_kHz_7_5 = {");
fprintf(fd,"%d,",s50_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s50_2(end));

fprintf(fd,"s75_kHz_7_5 = {");
fprintf(fd,"%d,",s75_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s75_2(end));

fprintf(fd,"s100_kHz_7_5 = {");
fprintf(fd,"%d,",s100_2(1:(end-1)));
fprintf(fd,"%d};\n\n",s100_2(end));

fclose(fd);