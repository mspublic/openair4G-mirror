primary_synch0 = [exp(-j*pi*25*(0:30).*(1:31)/63) 0\
		  exp(-j*pi*25*(32:62).*(33:63)/63)];
primary_synch1 = [exp(-j*pi*29*(0:30).*(1:31)/63) 0\
		  exp(-j*pi*29*(32:62).*(33:63)/63)];
primary_synch2 = [exp(-j*pi*34*(0:30).*(1:31)/63) 0\
		  exp(-j*pi*34*(32:62).*(33:63)/63)];

psync_table = [0 exp(-j*2*pi*(0:62)/63)];
psync_table_mod = zeros(1,63*2);
psync_table_mod(1:2:end) = floor(32767/sqrt(2)*real(psync_table));
psync_table_mod(2:2:end) = floor(32767/sqrt(2)*imag(psync_table));

primary_synch0_tab = [mod(25*(0:30).*(1:31)/2,63)+1 0 mod(25*(32:62).*(33:63)/2,63)+1];
primary_synch1_tab = [mod(29*(0:30).*(1:31)/2,63)+1 0 mod(29*(32:62).*(33:63)/2,63)+1];
primary_synch2_tab = [mod(34*(0:30).*(1:31)/2,63)+1 0 mod(34*(32:62).*(33:63)/2,63)+1];

primary_synch0_mod = zeros(1,73*2);
primary_synch0_mod(11:2:(11+124)) = floor(32767*real(primary_synch0(1:63)));
primary_synch0_mod(12:2:(12+124)) = floor(32767*imag(primary_synch0(1:63)));

primary_synch1_mod = zeros(1,73*2);
primary_synch1_mod(11:2:(11+124)) = floor(32767*real(primary_synch1(1:63)));
primary_synch1_mod(12:2:(12+124)) = floor(32767*imag(primary_synch1(1:63)));

primary_synch2_mod = zeros(1,73*2);
primary_synch2_mod(11:2:(11+124)) = floor(32767*real(primary_synch2(1:63)));
primary_synch2_mod(12:2:(12+124)) = floor(32767*imag(primary_synch2(1:63)));

%primary_synch0_mod2 = zeros(1,128);
%primary_synch0_mod2((128-30):128)=primary_synch0(1:31);
%primary_synch0_mod2(1:32)=primary_synch0(32:end);
%primary_synch0_time = ifft(primary_synch0_mod2)*sqrt(128);
%primary_synch0_time2 = zeros(1,128*2);
%primary_synch0_time2(1:2:end) = floor(32767*real(primary_synch0_time));
%primary_synch0_time2(2:2:end) = floor(32767*imag(primary_synch0_time));


fd = fopen("primary_synch.h","w");
fprintf(fd,"short primary_synch0[146] = {");
fprintf(fd,"%d,",primary_synch0_mod(1:end-1));
fprintf(fd,"%d};\n",primary_synch0_mod(end));
fprintf(fd,"short primary_synch1[146] = {");
fprintf(fd,"%d,",primary_synch1_mod(1:end-1));
fprintf(fd,"%d};\n",primary_synch1_mod(end));
fprintf(fd,"short primary_synch2[146] = {");
fprintf(fd,"%d,",primary_synch2_mod(1:end-1));
fprintf(fd,"%d};\n",primary_synch2_mod(end));
%fprintf(fd,"short primary_synch0_time[256] = {");
%fprintf(fd,"%d,",primary_synch0_time2(1:end-1));
%fprintf(fd,"%d};\n",primary_synch0_time2(end));
fclose(fd);

fd = fopen("psync_table.h","w");
fprintf(fd,"short psync_table[126] = {");
fprintf(fd,"%d,",psync_table_mod(1:end-1));
fprintf(fd,"%d};\n",psync_table_mod(end));
fprintf(fd,"char primary_synch0[73] = {");
fprintf(fd,"%d,",primary_synch0_tab(1:end-1));
fprintf(fd,"%d};\n",primary_synch0_tab(end));
fprintf(fd,"char primary_synch1[73] = {");
fprintf(fd,"%d,",primary_synch1_tab(1:end-1));
fprintf(fd,"%d};\n",primary_synch1_tab(end));
fprintf(fd,"char primary_synch2[73] = {");
fprintf(fd,"%d,",primary_synch2_tab(1:end-1));
fprintf(fd,"%d};\n",primary_synch2_tab(end));
fclose(fd);
