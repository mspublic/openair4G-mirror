clear all
close all
usefull_carriers = [1:80 256-79:256];

chsch1_channelF0
chsch1_channelF1
chsch2_channelF0
chsch2_channelF1
H(:,1,1)=chsch1_chanF0;
H(:,1,2)=chsch2_chanF0;
H(:,2,1)=chsch1_chanF1;
H(:,2,2)=chsch2_chanF1;

for i=1:size(H,1)
  HHm(i,:,:) = (round(squeeze(H(i,:,:))'*squeeze(H(i,:,:)))./pow2(7) + 7*eye(2));
  HHim(i,:,:) = inv(squeeze(HHm(i,:,:)));
  %Mm(i,:,:) = squeeze(HHim(i,:,:))*squeeze(H(i,:,:))';
end


		      X0,X1,X2,X3,X4,X5,X6,X7,X8,X9,X10,X11,X12;

x0m=H(:,1,1).*conj(H(:,1,1));
x1m=H(:,1,2).*conj(H(:,1,2));
x2m=H(:,2,1).*conj(H(:,2,1));
x3m=H(:,2,2).*conj(H(:,2,2));
x4m=H(:,1,2).*conj(H(:,1,1));
x5m=H(:,2,2).*conj(H(:,2,1));

x6m=x0m+x2m;
x7m=x1m+x3m;
x8m=x4m+x5m;

s1 = 960;
s2 = 1055;

x9m=x6m*s2;
x10m=x7m*s1;
x11m=x6m.*x7m;
x12m=x8m.*conj(x8m);

dm = x9m + x10m + s1*s2 + x11m - x12m;
idm = 1./dm;

Am(:,1,1) = (x1+x3+s1)./pow2(7);
Am(:,1,2) = (x4+x5)./pow2(7);
Am(:,2,1) = conj(Am(:,1,2));
Am(:,2,2) = (x0+x2+s2)./pow2(7);

A00,A01,A10,A11
A(:,1,1)=a00;
A(:,1,2)=a01;
A(:,2,1)=a10;
A(:,2,2)=a11;

for i=1:size(H,1)
   Mm(i,:,:) = squeeze(A(i,:,:))*squeeze(H(i,:,:))'./pow2(7);
end

chsch1_mmse_00
chsch1_mmse_10
chsch1_mmse_01
chsch1_mmse_11
M(:,1,1)=h00;
M(:,1,2)=h01;
M(:,2,1)=h10;
M(:,2,2)=h11;

%M(:,1,2) = -M(:,1,2);
%M(:,2,1) = conj(M(:,1,2));

% M is in alternative complex format!
M = conj(M);

determ;
ideterm;

chbch1_rxsigF0
chbch1_rxsigF1
chbch2_rxsigF0
chbch2_rxsigF1

for n=1:8
    for i=1:256
        x1 = [chbch1_rxF0((n-1)*256+i); chbch1_rxF1((n-1)*256+i)];
        x2 = [chbch2_rxF0((n-1)*256+i); chbch2_rxF1((n-1)*256+i)];
        y_mmse(n,i,1) = 1./d(i).*(reshape(M(i,1,:),1,2) * x1);
        y_mmse(n,i,2) = 1./d(i).*(reshape(M(i,2,:),1,2) * x2);
        y_mf(n,i,1) = conj(H(i,1,1)) * x1(1);
        y_mf(n,i,2) = conj(H(i,2,1)) * x2(2);
    end
end


figure(1)
plot(reshape(y_mmse(:,usefull_carriers,1),1,[]),'bx');
figure(2)
plot(reshape(y_mmse(:,usefull_carriers,2),1,[]),'rx');

chbch1_mmse_output0
figure(3)
plot(chbch1_mmse_out0,'bx')
chbch2_mmse_output1
figure(4)
plot(chbch2_mmse_out1,'rx')
