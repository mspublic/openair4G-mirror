clear all
usefull_carriers = [1:80 256-79:256];
%usefull_carriers = 1:256;

chsch1_channelF0
chsch1_channelF1
chsch2_channelF0
chsch2_channelF1
H(:,1,1)=chsch1_chanF0;
H(:,1,2)=chsch2_chanF0;
H(:,2,1)=chsch1_chanF1;
H(:,2,2)=chsch2_chanF1;

for i=1:size(H,1)
  HHm(i,:,:) = ((squeeze(H(i,:,:))'*squeeze(H(i,:,:))) + 7*eye(2));
  [Qm(i,:,:), Rm(i,:,:),iRm(i,:,:)] = myqr(squeeze(HHm(i,:,:)));
  HHim(i,:,:) = inv(squeeze(HHm(i,:,:))) * pow2(15);
  Mm(i,:,:) = squeeze(HHim(i,:,:))*squeeze(H(i,:,:))';
end


HH00
HH01
HH10
HH11
HH(:,1,1)=h00;
HH(:,1,2)=h01;
HH(:,2,1)=h10;
HH(:,2,2)=h11;

Q00
Q01
Q10
Q11
Q(:,1,1)=q00;
Q(:,1,2)=q01;
Q(:,2,1)=q10;
Q(:,2,2)=q11;

IR00
IR01
IR10
IR11
IR(:,1,1)=ir00;
IR(:,1,2)=ir01;
IR(:,2,1)=ir10;
IR(:,2,2)=ir11;


determ
ideterm

for i=1:size(H,1)
  HHim2(i,:,:) = round(HH(i,:,:)*id(i)./pow2(4));
  dm(i,1) = real(det(squeeze(HH(i,:,:))));
end
dm(81:(256-80))=0;
dim = floor(hex2dec('7FFF') .* min(dm(usefull_carriers)) ./ dm);


M00
M01
M10
M11
M1(:,1,1)=m00;
M1(:,1,2)=m01;
M1(:,2,1)=m10;
M1(:,2,2)=m11;

%HHi00
%HHi01
%HHi10
%HHi11
%HHi(:,1,1)=hi00;
%HHi(:,1,2)=hi01;
%HHi(:,2,1)=hi10;
%HHi(:,2,2)=hi11;

%for i=1:size(H,1)
%  Mm2(i,:,:) = squeeze(HHi(i,:,:))*squeeze(H(i,:,:))';
%end

chsch1_mmse_00
chsch1_mmse_10
chsch1_mmse_01
chsch1_mmse_11
M(:,1,1)=h00;
M(:,1,2)=h01;
M(:,2,1)=h10;
M(:,2,2)=h11;

% M is in alternative complex format!
M = conj(M);

chbch1_rxsigF0
chbch1_rxsigF1

for n=1:8
    for i=1:256
        x = [chbch1_rxF0((n-1)*256+i); chbch1_rxF1((n-1)*256+i)];
        y_mmse(n,i,:) = (squeeze(Mm(i,:,:)) * x)./pow2(5);
        y_mf(n,i,1) = conj(H(i,1,1)) * x(1);
        y_mf(n,i,2) = conj(H(i,2,1)) * x(2);
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
