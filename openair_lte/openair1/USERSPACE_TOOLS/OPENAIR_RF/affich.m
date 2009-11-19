clear all

fd = fopen("rx_frame.dat","r");

temp = fread(fd,"int16");
fclose(fd);

rxs = temp(1:2:length(temp)) + sqrt(-1)*temp(2:2:length(temp));
rxs = reshape(rxs,[],2);

figure(1)
plot(real(rxs))

fd = fopen("tx_frame.dat","r");

temp = fread(fd,"int8");
fclose(fd);

%txs(:,1) = temp(1:4:length(temp)) + sqrt(-1)*temp(2:4:length(temp));
%txs(:,2) = temp(3:4:length(temp)) + sqrt(-1)*temp(4:4:length(temp));

load('../../PHY/LTE_REFSIG/mod_table.mat')

txs = temp;

figure(2)
plot(real(txs))

