clear all

fd = fopen("rx_frame.dat","r");

temp = fread(fd,20480*8,"int16");
fclose(fd);

rxs = temp(1:2:length(temp)) + sqrt(-1)*temp(2:2:length(temp));
rxs = reshape(rxs,[],2);

figure(2)
plot(real(rxs))

