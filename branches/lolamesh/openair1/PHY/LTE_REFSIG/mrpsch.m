mrpsch = [zeros(1,5) exp(-1j*pi*23*(0:30).*(1:31)/63) exp(-1j*pi*23*(32:62).*(33:63)/63) zeros(1,5)];
mrpsch_tab = [zeros(1,5) mod(23*((0:30).*(1:31))/2,63)+85 mod(23*((32:62).*(33:63))/2,63)+85 zeros(1,5)];

mrpsch_real = floor(32767*real(mrpsch));
mrpsch_imag = floor(32767*imag(mrpsch));

fd = fopen('../LTE_TRANSPORT/mrpsch_tab.c','w');
fprintf(fd,'short mrpsch_real[72] = {');
fprintf(fd,'%d,',mrpsch_real(1:end-1));
fprintf(fd,'%d};\n',mrpsch_real(end));
fprintf(fd,'\n');
fprintf(fd,'short mrpsch_imag[72] = {');
fprintf(fd,'%d,',mrpsch_imag(1:end-1));
fprintf(fd,'%d};\n',mrpsch_imag(end));
fprintf(fd,'\n');
fprintf(fd,'unsigned char mrpsch_tab[72] = {');
fprintf(fd,'%d,',mrpsch_tab(1:end-1));
fprintf(fd,'%d};\n',mrpsch_tab(end));
fclose(fd);

